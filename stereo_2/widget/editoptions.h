#ifndef LEARN_KEYS_H
#define LEARN_KEYS_H

#include "coolwidget.h"

/* each command can be bound to three different keys */
struct key_list {
    char *key_name;
    int command;
    unsigned int keycode0, state0;
    unsigned int keycode1, state1;
    unsigned int keycode2, state2;
};

#ifndef EDIT_OPTIONS_C
extern struct key_list klist[];
#endif

int get_defined_key (struct key_list klist[], unsigned int state, unsigned int keycode);
char *find_section (char *t, const char *section);
char *get_options_section (const char *file, const char *section);
int save_options_section (const char *file, const char *section, const char *text);
int load_user_defined_keys (struct key_list klist[], const char *file);
int save_user_defined_keys (struct key_list klist[], const char *file);
char *get_key_text (struct key_list klist[]);
CWidget *Cdrawlearnkeys (Window parent, int x, int y, int width, int height);
int user_defined_key (unsigned int state, unsigned int keycode);
int load_keys (const char *file);


#endif				/* LEARN_KEYS_H */
