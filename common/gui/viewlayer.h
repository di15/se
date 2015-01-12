#ifndef VIEWLAYER_H
#define VIEWLAYER_H

#include "widget.h"

class ViewLayer : public Widget
{
public:

	ViewLayer()
	{
		m_name = "";
		m_opened = false;
		m_type = WIDGET_VIEWLAYER;
		m_parent = NULL;
	}

	ViewLayer(Widget* parent, const char* n) : Widget()
	{
		m_name = n;
		m_opened = false;
		m_type = WIDGET_VIEWLAYER;
		m_parent = parent;
		
		reframe();
	}

	virtual void draw();
	virtual void drawover();
	virtual void inev(InEv* ie);
	virtual void reframe();
};

#endif
