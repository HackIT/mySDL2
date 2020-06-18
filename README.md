
##                        Simple DirectMedia Layer

###                                  (SDL)

                                Version 2.0

---
https://www.libsdl.org/

Simple DirectMedia Layer is a cross-platform development library designed
to provide low level access to audio, keyboard, mouse, joystick, and graphics
hardware via OpenGL and Direct3D. It is used by video playback software,
emulators, and popular games including Valve's award winning catalog
and many Humble Bundle games.

More extensive documentation is available in the docs directory, starting
with README.md

Enjoy!
	Sam Lantinga				(slouken@libsdl.org)


modified clipboard handling to support X11 clipboard(s) XA_CLIPBOARD and XA_PRIMARY.
- XA_CLIPBOARD is the common clipboard everybody knows about (ctrl+c/ctrl+v)...
- XA_PRIMARY is known as a selection clipboard(X11 GNU/LINUX)

XA_PRIMARY doesn't require the user to use a key,
but just select some text and paste it with the middle mouse button...
you ofc have to implement this into your app...

3 new functions took place into SDL clipboard handling to handle XA_PRIMARY clipboard...
The 3 original functions that were about an SDL_CUTBUFFER are now handling XA_CLIPBOARD
the correct way. 

int SDL_SetSelectionClipboardText(const char *text);

char * SDL_GetSelectionClipboardText(void);

SDL_bool SDL_HasSelectionClipboardText(void);

Enjoy!
    [HackIT]                     (boxx1@free.fr)