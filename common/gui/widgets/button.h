
#ifndef BUTTON_H
#define BUTTON_H

#include "../widget.h"

#define BUTTON_BGIMAGE			0
#define BUTTON_LINEBASED		1
#define BUTTON_LEFTIMAGE		2

class Button : public Widget
{
public:

	int m_style;

	Button();
	Button(Widget* parent, const char* name, const char* filepath, const std::string label, const std::string tooltip,int f, int style, void (*reframef)(Widget* thisw), void (*click)(), void (*click2)(int p), void (*overf)(), void (*overf2)(int p), void (*out)(), int parm);

	virtual void draw();
	virtual void drawover();
	virtual void inev(InEv* ev);
	virtual void reframe();
};

#endif
