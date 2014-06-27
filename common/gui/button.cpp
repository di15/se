
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


Button::Button(Widget* parent, const char* n, const char* filepath, const char* label, const char* t, int f, void (*reframef)(Widget* thisw), void (*click)(), void (*overf)(), void (*out)()) : Widget()
{
	m_parent = parent;
	m_type = WIDGET_BUTTON;
	m_name = n;
    m_text = t;
	m_label = label;
    m_font = f;
    //float length = 0;
    //for(int i=0; i<strlen(t); i++)
    //    length += g_font[f].glyph[t[i]].w;
	//m_tpos[0] = (left+right)/2.0f - length/2.0f;
	//m_tpos[1] = (top+bottom)/2.0f - g_font[f].gheight/2.0f;
	m_over = false;
	m_ldown = false;
	CreateTexture(m_tex, filepath, true, false);
	CreateTexture(m_bgtex, "gui/buttonbg.png", true, false);
	CreateTexture(m_bgovertex, "gui/buttonbgover.png", true, false);
	reframefunc = reframef;
	clickfunc = click;
	overfunc = overf;
	clickfunc2 = NULL;
	overfunc2 = NULL;
	outfunc = out;
	reframe();
}

Button::Button(Widget* parent, const char* filepath, const char* t, int f, void (*reframef)(Widget* thisw), void (*click)(), void (*overf)(), void (*out)()) : Widget()
{
	m_parent = parent;
	m_type = WIDGET_BUTTON;
    m_text = t;
    m_font = f;
   // float length = 0;
    //for(int i=0; i<strlen(t); i++)
    //    length += g_font[f].glyph[t[i]].w;
    //m_tpos[0] = (left+right)/2.0f - length/2.0f;
    //m_tpos[1] = (top+bottom)/2.0f - g_font[f].gheight/2.0f;
	m_over = false;
	m_ldown = false;
	CreateTexture(m_tex, filepath, true, false);
	CreateTexture(m_bgtex, "gui/buttonbg.png", true, false);
	CreateTexture(m_bgovertex, "gui/buttonbgover.png", true, false);
	reframefunc = reframef;
	clickfunc = click;
	overfunc = overf;
	clickfunc2 = NULL;
	overfunc2 = NULL;
	outfunc = out;
	reframe();
}

Button::Button(Widget* parent, const char* filepath, const char* t, int f, void (*reframef)(Widget* thisw), void (*click2)(int p), int parm) : Widget()
{
	m_parent = parent;
	m_type = WIDGET_BUTTON;
    m_text = t;
    m_font = f;
    //float length = 0;
    //for(int i=0; i<strlen(t); i++)
    //    length += g_font[f].glyph[t[i]].w;
    //m_tpos[0] = (left+right)/2.0f - length/2.0f;
    //m_tpos[1] = (top+bottom)/2.0f - g_font[f].gheight/2.0f;
	m_over = false;
	m_ldown = false;
	CreateTexture(m_tex, filepath, true, false);
	CreateTexture(m_bgtex, "gui/buttonbg.png", true, false);
	CreateTexture(m_bgovertex, "gui/buttonbgover.png", true, false);
	reframefunc = reframef;
	clickfunc = NULL;
	overfunc = NULL;
	clickfunc2 = click2;
	overfunc2 = NULL;
	outfunc = NULL;
	m_param = parm;
	reframe();
}

Button::Button(Widget* parent, const char* filepath, const char* t, int f, void (*reframef)(Widget* thisw), void (*click2)(int p), void (*overf2)(int p), void (*out)(), int parm) : Widget()
{
	m_parent = parent;
	m_type = WIDGET_BUTTON;
    m_text = t;
    m_font = f;
    float length = 0;
    for(int i=0; i<strlen(t); i++)
        length += g_font[f].glyph[t[i]].origsize[0];
    //m_tpos[0] = (left+right)/2.0f - length/2.0f;
    //m_tpos[1] = (top+bottom)/2.0f - g_font[f].gheight/2.0f;
	m_over = false;
	m_ldown = false;
	CreateTexture(m_tex, filepath, true, false);
	CreateTexture(m_bgtex, "gui/buttonbg.png", true, false);
	CreateTexture(m_bgovertex, "gui/buttonbgover.png", true, false);
	reframefunc = reframef;
	clickfunc = NULL;
	overfunc = NULL;
	clickfunc2 = click2;
	overfunc2 = overf2;
	outfunc = out;
	m_param = parm;
	reframe();
}

void Button::reframe()
{
#if 0
	m_pos[0].recalc(m_parent);
	m_pos[1].recalc(m_parent);
	m_pos[2].recalc(m_parent);
	m_pos[3].recalc(m_parent);
    float length = 0;
    for(int i=0; i<strlen(m_label.c_str()); i++)
	{
        length += g_font[m_font].glyph[m_label[i]].origsize[0];
	}

	m_tpos[0] = Margin(MARGIN_SOURCE_WIDTH, MARGIN_FUNC_PIXELS, (m_pos[2]-m_pos[0])/2.0f-length/2.0f);
	m_tpos[1] = Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, (m_pos[3]-m_pos[1])/2.0f-g_font[m_font].gheight/2.0f);
	m_tpos[0].recalc(this);
	m_tpos[1].recalc(this);
	//m_tpos[2].recalc(this);
	//m_tpos[3].recalc(this);
#endif

	if(reframefunc)
		reframefunc(this);
}

void Button::premousemove()
{
	if(g_mouse.x >= m_pos[0] && g_mouse.x <= m_pos[2] && g_mouse.y >= m_pos[1] && g_mouse.y <= m_pos[3])
	{
	}
	else
	{
		if(m_over && outfunc != NULL)
			outfunc();

		m_over = false;
	}
}

bool Button::mousemove()
{
	if(g_mouse.x >= m_pos[0] && g_mouse.x <= m_pos[2] && g_mouse.y >= m_pos[1] && g_mouse.y <= m_pos[3])
	{
		if(overfunc != NULL)
			overfunc();
		if(overfunc2 != NULL)
			overfunc2(m_param);

		m_over = true;

		return true;
	}

	return false;
}

void Button::draw()
{
	if(m_over)
		DrawImage(g_texture[m_bgovertex].texname, m_pos[0], m_pos[1], m_pos[2], m_pos[3]);
	else
		DrawImage(g_texture[m_bgtex].texname, m_pos[0], m_pos[1], m_pos[2], m_pos[3]);

	DrawImage(g_texture[m_tex].texname, m_pos[0], m_pos[1], m_pos[2], m_pos[3]);

	DrawShadowedText(m_font, m_tpos[0], m_tpos[1], m_label.c_str());

	//if(_stricmp(m_name.c_str(), "choose texture") == 0)
	//	g_log<<"draw choose texture"<<endl;
}

void Button::draw2()
{
	if(m_over)
		//DrawShadowedText(m_font, m_tpos[0], m_tpos[1], m_text.c_str());
		DrawShadowedText(m_font, g_mouse.x, g_mouse.y-g_font[m_font].gheight, m_text.c_str());
}

