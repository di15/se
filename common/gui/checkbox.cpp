

#include "widget.h"
#include "barbutton.h"
#include "button.h"
#include "checkbox.h"
#include "editbox.h"
#include "dropdowns.h"
#include "hscroller.h"
#include "image.h"
#include "insdraw.h"
#include "link.h"
#include "listbox.h"
#include "text.h"
#include "textarea.h"
#include "textblock.h"
#include "touchlistener.h"


CheckBox::CheckBox(Widget* parent, const char* n, const char* t, int f, void (*reframef)(Widget* thisw), int sel, float r, float g, float b, float a, void (*change)()) : Widget()
{
	m_parent = parent;
	m_type = WIDGET_CHECKBOX;
	m_name = n;
	m_text = t;
	m_font = f;
	reframefunc = reframef;
	m_ldown = false;
	m_rgba[0] = r;
	m_rgba[1] = g;
	m_rgba[2] = b;
	m_rgba[3] = a;
	m_selected = sel;
	changefunc = change;
	CreateTexture(m_frametex, "gui\\frame.jpg", true);
	CreateTexture(m_filledtex, "gui\\accept.png", true);
	reframe();
}

bool CheckBox::mousemove()
{
	if(g_mouse.x >= m_pos[0] && g_mouse.y >= m_pos[1] && 
		g_mouse.x <= m_pos[2] && 
		g_mouse.y <= m_pos[3])
	{
		m_over = true;

		return true;
	}
	else
	{
		m_over = false;

		return false;
	}
}

int CheckBox::square()
{
	return g_font[m_font].gheight;
}

void CheckBox::draw()
{
	DrawImage(g_texture[m_frametex].texname,  m_pos[0], m_pos[1], m_pos[0]+square(), m_pos[1]+square());

	if(m_selected > 0)
		DrawImage(g_texture[m_filledtex].texname, m_pos[0], m_pos[1], m_pos[0]+square(), m_pos[1]+square());

	DrawShadowedText(m_font, m_pos[0]+square()+5, m_pos[1], m_text.c_str());
}

bool CheckBox::lbuttondown()
{
	if(m_over)
	{
		m_ldown = true;
		return true;	// intercept mouse event
	}

	return false;
}

bool CheckBox::lbuttonup(bool moved)
{
	if(m_over && m_ldown)
	{
		if(m_selected <= 0)
			m_selected = 1;
		else
			m_selected  = 0;

		if(changefunc != NULL)
			changefunc();

		m_ldown = false;

		return true;
	}
	
	m_ldown = false;

	return false;
}

