#ifndef CONSTRUCTIONVIEW_H
#define CONSTRUCTIONVIEW_H

#include "../../../platform.h"
#include "../button.h"
#include "../image.h"
#include "../text.h"
#include "../editbox.h"
#include "../touchlistener.h"
#include "../../widget.h"
#include "../viewportw.h"
#include "../../../sim/selection.h"
#include "../windoww.h"

class CstrView : public WindowW
{
public:
	CstrView(Widget* parent, const char* n, void (*reframef)(Widget* thisw), void (*movefunc)(), void (*cancelfunc)(), void (*proceedfunc)(), void (*estimatefunc)());

	void (*movefunc)();
	void (*cancelfunc)();
	void (*proceedfunc)();
	void (*estimatefunc)();

	//void draw();
	//void drawover();
	//void reframe();
	//void inev(InEv* ie);
	//void frameupd();
	void regen(Selection* sel);
};

#endif
