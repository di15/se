

#include "widget.h"
#include "gui.h"
#include "font.h"
#include "../window.h"

void Widget::select(int which)
{
	m_selected = which;

	if(m_selected >= m_options.size())
		m_selected = m_options.size()-1;

	if(m_selected < 0)
		m_selected = 0;

	if(changefunc2 != NULL)
		changefunc2(m_selected);
}

void Widget::clear()
{
	m_selected = -1;
	m_scroll[1] = 0;
	m_options.clear();
	m_text = "";
}

void Widget::changetext(const char* newt)
{
	m_text = newt;

	if(m_type == WIDGET_BUTTON)
	{
		float length = 0;
		for(int i=0; i<strlen(newt); i++)
			length += g_font[m_font].glyph[newt[i]].origsize[0];
		m_tpos[0] = (m_pos[0]+m_pos[2])/2.0f - length/2.0f;
		m_tpos[1] = (m_pos[1]+m_pos[3])/2.0f - g_font[m_font].gheight/2.0f;
	}
}

void Widget::changevalue(const char* newv)
{
	m_value = newv;
	if(m_caret > strlen(newv))
		m_caret = strlen(newv);
}

void Widget::placechar(int k)
{
	int len = m_value.length();

	//if(m_type == WIDGET_EDITBOX && len >= m_maxlen)
	//	return;

	char addchar = k;

	string before = m_value.substr(0, m_caret);
	string after = m_value.substr(m_caret, len-m_caret);
	m_value = before + addchar + after;
	m_caret ++;
}

void Widget::reframe()	//resized or moved
{
	if(reframefunc)
		reframefunc(this);

	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->reframe();
}

Widget* Widget::getsubwidg(const char* name, int type)
{
	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		if((*i)->m_type == type && _stricmp((*i)->m_name.c_str(), name) == 0)
			return *i;

	return NULL;
}

void CenterLabel(Widget* w)
{
	Font* f = &g_font[w->m_font];

	int texwidth = TextWidth(w->m_font, w->m_label.c_str());

	w->m_tpos[0] = (w->m_pos[2]+w->m_pos[0])/2 - texwidth/2;
	w->m_tpos[1] = (w->m_pos[3]+w->m_pos[1])/2 - f->gheight/2;
}