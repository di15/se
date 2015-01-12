#ifndef NEWHOSTVIEW_H
#define NEWHOSTVIEW_H

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

class NewHost : public WindowW
{
public:
	NewHost(Widget* parent, const char* n, void (*reframef)(Widget* thisw));

	Button m_create;
	EditBox m_gamename;
	Text m_gnlab;	//game name label

	//void draw();
	//void drawover();
	//void reframe();
	//void inev(InEv* ie);
	//void frameupd();
	//void regen(Selection* sel);
	void regen();
	virtual void subdraw();
	virtual void subdrawover();
	virtual void subreframe();
	virtual void subinev(InEv* ie);
};

#endif
