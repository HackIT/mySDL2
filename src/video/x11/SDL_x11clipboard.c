/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2019 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_X11

#include <limits.h> /* For INT_MAX */

#include "SDL_events.h"
#include "SDL_x11video.h"
#include "SDL_timer.h"


/* If you don't support UTF-8, you might use XA_STRING here */
#ifdef X_HAVE_UTF8_STRING
#define TEXT_FORMAT X11_XInternAtom(display, "UTF8_STRING", False)
#else
#define TEXT_FORMAT XA_STRING
#endif

/* Get any application owned window handle for clipboard association */
static Window
GetWindow(_THIS)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;

    /* We create an unmapped window that exists just to manage the clipboard,
       since X11 selection data is tied to a specific window and dies with it.
       We create the window on demand, so apps that don't use the clipboard
       don't have to keep an unnecessary resource around. */
    if (data->clipboard_window == None) {
        Display *dpy = data->display;
        Window parent = RootWindow(dpy, DefaultScreen(dpy));
        XSetWindowAttributes xattr;
        data->clipboard_window = X11_XCreateWindow(dpy, parent, -10, -10, 1, 1, 0,
                                                   CopyFromParent, InputOnly,
                                                   CopyFromParent, 0, &xattr);
        X11_XFlush(data->display);
    }

    return data->clipboard_window;
}

static Time
get_timestamp (_THIS)
{
    Display *display = ((SDL_VideoData *) _this->driverdata)->display;
    Window window = GetWindow(_this);
    XEvent event;

    X11_XSelectInput (display, window, PropertyChangeMask);
    X11_XChangeProperty (display, window, XA_WM_NAME, XA_STRING, 8,
                   PropModeAppend, NULL, 0);

    while (1) {
        X11_XNextEvent (display, &event);

        if (event.type == PropertyNotify)
            return event.xproperty.time;
    }
}

int
SetClipboardText(_THIS, Atom selection, const char *text)
{
    Display *display = ((SDL_VideoData *) _this->driverdata)->display;
    Atom format;
    Window window;
    Time timestamp = get_timestamp(_this);

    /* Get the SDL window that will own the selection */
    window = GetWindow(_this);
    if (window == None) {
        return SDL_SetError("Couldn't find a window to own the selection");
    }

    /* Save the selection on the root window */
    format = TEXT_FORMAT;
    X11_XChangeProperty(display, window,
        selection, format, 8, PropModeReplace,
        (const unsigned char *)text, SDL_strlen(text));

    if (selection != None &&
        X11_XGetSelectionOwner(display, selection) != window) {
        X11_XSetSelectionOwner(display, selection, window, timestamp);
    }

    return 0;
}

char *
GetClipboardText(_THIS, Atom selection)
{
    SDL_VideoData *videodata = (SDL_VideoData *) _this->driverdata;
    Display *display = videodata->display;
    Atom format;
    Window window;
    Window owner;
    Atom seln_type;
    int seln_format;
    unsigned long nbytes;
    unsigned long overflow;
    unsigned char *src;
    char *text;
    Uint32 waitStart;
    Uint32 waitElapsed;

    text = NULL;

    /* Get the window that holds the selection */
    window = GetWindow(_this);
    format = TEXT_FORMAT;
    owner = X11_XGetSelectionOwner(display, selection);

    if (owner == None) {
        owner = DefaultRootWindow(display);
        /* no owner were given, switch to root window */
    } else if (owner == window) {
        /* owner were clipboard_window, no action required */
    } else {
        /* Request that the selection owner copy the data to our window */
        owner = window;
        X11_XConvertSelection(display, selection, format, selection, owner,
            CurrentTime);

        /* When using synergy on Linux and when data has been put in the clipboard
           on the remote (Windows anyway) machine then selection_waiting may never
           be set to False. Time out after a while. */
        waitStart = SDL_GetTicks();
        videodata->selection_waiting = SDL_TRUE;
        while (videodata->selection_waiting) {
            SDL_PumpEvents();
            waitElapsed = SDL_GetTicks() - waitStart;
            /* Wait one second for a clipboard response. */
            if (waitElapsed > 1000) {
                videodata->selection_waiting = SDL_FALSE;
                SDL_SetError("Clipboard timeout");
                /* We need to set the clipboard text so that next time we won't
                   timeout, otherwise we will hang on every call to this function. */
                X11_SetClipboardText(_this, "");
                return SDL_strdup("");
            }
        }
    }

    if (X11_XGetWindowProperty(display, owner, selection, 0, INT_MAX/4, False,
            format, &seln_type, &seln_format, &nbytes, &overflow, &src)
            == Success) {
        if (seln_type == format) {
            text = (char *)SDL_malloc(nbytes+1);
            if (text) {
                SDL_memcpy(text, src, nbytes);
                text[nbytes] = '\0';
            }
        }
        X11_XFree(src);
    }

    if (!text) {
        text = SDL_strdup("");
    }

    return text;
}

SDL_bool
HasClipboardText(_THIS, Atom selection)
{
    SDL_bool result = SDL_FALSE;
    char *text = GetClipboardText(_this, selection);
    if (text) {
        result = text[0] != '\0' ? SDL_TRUE : SDL_FALSE;
        SDL_free(text);
    }
    return result;
}

/* We use our own cut-buffer for intermediate storage instead of  
   XA_CUT_BUFFER0 because their use isn't really defined for holding UTF8.
Atom
X11_GetSDLCutBufferClipboardType(Display *display)
{
    return X11_XInternAtom(display, "SDL_CUTBUFFER", False);
}
*/

int
X11_SetClipboardText(_THIS, const char *text) {
    Display *display = ((SDL_VideoData *) _this->driverdata)->display;
    Atom selection = X11_XInternAtom(display, "CLIPBOARD", 0);
    return SetClipboardText(_this, selection, text);
}

int
X11_SetSelectionClipboardText(_THIS, const char *text) {
    Display *display = ((SDL_VideoData *) _this->driverdata)->display;
    Atom selection = X11_XInternAtom(display, "PRIMARY", 0);
    return SetClipboardText(_this, selection, text);
}

char *
X11_GetClipboardText(_THIS) {
    Display *display = ((SDL_VideoData *) _this->driverdata)->display;
    Atom selection = X11_XInternAtom(display, "CLIPBOARD", 0);
    return GetClipboardText(_this, selection);
}

char *
X11_GetSelectionClipboardText(_THIS) {
    Display *display = ((SDL_VideoData *) _this->driverdata)->display;
    Atom selection = X11_XInternAtom(display, "PRIMARY", 0);
    return GetClipboardText(_this, selection);
}

SDL_bool
X11_HasClipboardText(_THIS) {
    Display *display = ((SDL_VideoData *) _this->driverdata)->display;
    Atom selection = X11_XInternAtom(display, "CLIPBOARD", 0);
    return HasClipboardText(_this, selection);
}

SDL_bool
X11_HasSelectionClipboardText(_THIS) {
    Display *display = ((SDL_VideoData *) _this->driverdata)->display;
    Atom selection = X11_XInternAtom(display, "PRIMARY", 0);
    return HasClipboardText(_this, selection);
}

#endif /* SDL_VIDEO_DRIVER_X11 */

/* vi: set ts=4 sw=4 expandtab: */
