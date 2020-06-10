
                         Simple DirectMedia Layer

                                  (SDL)

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



### modified to avoid touching stuff it shouldn't... 

here I noticed XA_CLIPBOARD and XA_PRIMARY mixed a wrong way...

now these 3 functions interacts only with XA_CLIPBOARD and SDL_CLIPBOARD?

I still don't understand why they using "xsel" way (fake window, own properties...) as any SDL software should own a window...

int SDL_SetClipboardText(const char *text)

char *SDL_GetClipboardText(void)

SDL_bool SDL_HasClipboardText(void)

### added these 3 to handle XA_PRIMARY clipboard which is standard to X11 since YEARS... 

notice a bad use of these 3 functions may result to shit perf in your app...

Doesn't call it like it's your mom...

int SDL_SetSelectionClipboardText(const char *text)

char *SDL_GetSelectionClipboardText(void)

SDL_bool SDL_HasSelectionClipboardText(void)

