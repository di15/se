

#ifndef LINK_H
#define LINK_H

#include "widget.h"

class Link : public Widget
{
public:
	Link(Widget* parent, const char* t, int f, void (*reframef)(Widget* thisw), void (*click)()) : Widget()
	{
		m_parent = parent;
		m_type = WIDGET_LINK;
		m_over = false;
		m_ldown = false;
		m_text = t;
		m_font = f;
		reframefunc = reframef;
		clickfunc = click;
		reframe();
	}

	void draw();
	bool lbuttonup(bool moved)
	{
		mousemove();

		if(m_over && m_ldown)
		{
			if(clickfunc != NULL)
				clickfunc();

			m_over = false;
			m_ldown = false;

			return true;	// intercept mouse event
		}
		
		m_over = false;
		m_ldown = false;

		return false;
	}
	bool lbuttondown()
	{
		mousemove();

		if(m_over)
		{
			m_ldown = true;
			return true;	// intercept mouse event
		}

		return false;
	}
	bool mousemove();
};

#endif