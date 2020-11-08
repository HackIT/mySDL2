# mySDL2

Thank to ![xsel](https://github.com/kfish/xsel) author for his tool.

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

###### Enjoy, HackIT.

###### BTC: bc1q5jw0dsgc4x96l0um6vqexp5kq7wthlfvz944uc

