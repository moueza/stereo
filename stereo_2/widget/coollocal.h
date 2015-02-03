void Crenderbutton (CWidget *w);
void Crendertextinput (CWidget *w);
void Cinputtobox (CWidget *w, CWidget *box);
void Crenderwindow (CWidget *w);
void Crenderbar (CWidget *w);
void Crendertext (CWidget *w);
void Crenderscrollbar (CWidget *w);
void Crendersunken (CWidget *w);
int Cscrollwhereis (int bx, int by, CWidget *w);
long Csettextboxpos (CWidget *w, int which, long p);
long Ccounttextboxlines (CWidget *w, int all);
void Crenderprogress (CWidget *w);
void Crenderswitch (CWidget *w, int state);
int (*Cdefaulthandler (int kindofwidget)) (CWidget *, XEvent *, CEvent *);
CWidget **Cfindemptywidgetentry();
CWidget *Callocatewidget(Window newwin,
	const char *ident, Window parent, int x, int y,
	int width, int height, int kindofwidget);
int eh_picture (struct cool_widget *w, XEvent * xevent, CEvent * cwevent);
