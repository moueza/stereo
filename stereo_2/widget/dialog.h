#ifndef DIALOGUE_H
#define DIALOGUE_H

#include <X11/Xlib.h>

void Cfatalerrordialog (int x, int y, const char *fmt,...);
char *Cinputdialog (Window in, int x, int y, int min_width, const char *def, const char *heading, const char *fmt,...);
int Cquerydialog (Window in, int x, int y, const char *heading, const char *first,...);
void Cerrordialog (Window in, int x, int y, const char *heading, const char *fmt,...);
void Cmessagedialog (Window in, int x, int y, const char *heading, const char *fmt,...);
void Ctextboxmessagedialog (Window in, int x, int y, int width, int height, const char *heading, const char *text, int line);
char *get_sys_error (const char *s);
short Crawkeyquery (Window in, int x, int y, const char *heading, const char *fmt,...);
CWidget *Cmanpagedialog (Window in, int x, int y, int width, int height, const char *manpage);

#endif

