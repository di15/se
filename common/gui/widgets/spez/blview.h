#ifndef BUILDINGVIEW_H
#define BUILDINGVIEW_H

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

class BlView : public WindowW
{
public:
	BlView(Widget* parent, const char* n, void (*reframef)(Widget* thisw));

	Button m_close;
	Button m_set;

	//void draw();
	//void drawover();
	//void reframe();
	//void inev(InEv* ie);
	//void frameupd();
	void regen(Selection* sel);
	void regvals(Selection* sel);
	virtual void subreframe();
	virtual void subdraw();
	virtual void subdrawover();
	virtual void subinev(InEv* ie);
};

void Resize_BV_Cl(Widget* thisw);

#endif
