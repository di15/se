
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
#include "viewportw.h"
#include "../platform.h"
#include "../window.h"
#include "../draw/shader.h"
#include "gui.h"

ViewportW::ViewportW(Widget* parent, const char* n, void (*reframef)(Widget* thisw),
					 void (*drawf)(int p, int x, int y, int w, int h),
					 bool (*ldownf)(int p, int x, int y, int w, int h),
					 bool (*lupf)(int p, int x, int y, int w, int h),
					 bool (*mousemovef)(int p, int x, int y, int w, int h),
					bool (*rdownf)(int p, int relx, int rely, int w, int h),
					bool (*rupf)(int p, int relx, int rely, int w, int h),
					bool (*mousewf)(int p, int d),
					bool (*mdownf)(int p, int relx, int rely, int w, int h),
					bool (*mupf)(int p, int relx, int rely, int w, int h),
					 int parm)
{
	m_parent = parent;
	m_type = WIDGET_VIEWPORT;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;
	m_param = parm;
	drawfunc = drawf;
	ldownfunc = ldownf;
	lupfunc = lupf;
	mdownfunc = mdownf;
	mupfunc = mupf;
	mousemovefunc = mousemovef;
	rdownfunc = rdownf;
	rupfunc = rupf;
	mousewfunc = mousewf;
	reframe();
}

void ViewportW::draw()
{
	//g_log<<"vpw "<<m_pos[0]<<","<<m_pos[1]<<","<<m_pos[2]<<","<<m_pos[3]<<endl;

	int w = m_pos[2] - m_pos[0];
	int h = m_pos[3] - m_pos[1];

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glViewport(m_pos[0], g_height-m_pos[3], w, h);
    glUniform1f(g_shader[SHADER_ORTHO].m_slot[SSLOT_WIDTH], (float)w);
    glUniform1f(g_shader[SHADER_ORTHO].m_slot[SSLOT_HEIGHT], (float)h);

	if(drawfunc != NULL)
		drawfunc(m_param,m_pos[0], m_pos[1], w, h);

	//glViewport(0, 0, g_width, g_height);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	EndS();
    Ortho(g_width, g_height, 1, 1, 1, 1);
}

void ViewportW::premousemove()
{
	if(g_mouse.x >= m_pos[0] && g_mouse.x <= m_pos[2] && g_mouse.y >= m_pos[1] && g_mouse.y <= m_pos[3])
	{}
	else
		m_over = false;
}

bool ViewportW::mousemove()
{
	if(g_mouse.x >= m_pos[0] && g_mouse.x <= m_pos[2] && g_mouse.y >= m_pos[1] && g_mouse.y <= m_pos[3])
		m_over = true;

	if(mousemovefunc != NULL)
	{
		int relx = g_mouse.x - m_pos[0];
		int rely = g_mouse.y - m_pos[1];
		int w = m_pos[2] - m_pos[0];
		int h = m_pos[3] - m_pos[1];
		return mousemovefunc(m_param, relx, rely, w, h);
	}

	return false;
}

bool ViewportW::lbuttondown()
{
	if(!m_over)
		return false;

	if(ldownfunc != NULL)
	{
		int relx = g_mouse.x - m_pos[0];
		int rely = g_mouse.y - m_pos[1];
		int w = m_pos[2] - m_pos[0];
		int h = m_pos[3] - m_pos[1];
		return ldownfunc(m_param, relx, rely, w, h);
	}

	return false;
}

bool ViewportW::lbuttonup(bool moved)
{
	if(lupfunc != NULL)
	{
		int relx = g_mouse.x - m_pos[0];
		int rely = g_mouse.y - m_pos[1];
		int w = m_pos[2] - m_pos[0];
		int h = m_pos[3] - m_pos[1];
		return lupfunc(m_param, relx, rely, w, h);
	}

	return false;
}

bool ViewportW::rbuttondown()
{
	//g_log<<"r down vp"<<endl;
	//g_log.flush();

	if(!m_over)
		return false;

	if(rdownfunc != NULL)
	{
		int relx = g_mouse.x - m_pos[0];
		int rely = g_mouse.y - m_pos[1];
		int w = m_pos[2] - m_pos[0];
		int h = m_pos[3] - m_pos[1];
		return rdownfunc(m_param, relx, rely, w, h);
	}

	return false;
}

bool ViewportW::rbuttonup(bool moved)
{
	if(rupfunc != NULL)
	{
		int relx = g_mouse.x - m_pos[0];
		int rely = g_mouse.y - m_pos[1];
		int w = m_pos[2] - m_pos[0];
		int h = m_pos[3] - m_pos[1];
		return rupfunc(m_param, relx, rely, w, h);
	}

	return false;
}

bool ViewportW::mbuttondown()
{
	//g_log<<"r down vp"<<endl;
	//g_log.flush();

	if(!m_over)
		return false;

	if(mdownfunc != NULL)
	{
		int relx = g_mouse.x - m_pos[0];
		int rely = g_mouse.y - m_pos[1];
		int w = m_pos[2] - m_pos[0];
		int h = m_pos[3] - m_pos[1];
		return mdownfunc(m_param, relx, rely, w, h);
	}

	return false;
}

bool ViewportW::mbuttonup(bool moved)
{
	if(mupfunc != NULL)
	{
		int relx = g_mouse.x - m_pos[0];
		int rely = g_mouse.y - m_pos[1];
		int w = m_pos[2] - m_pos[0];
		int h = m_pos[3] - m_pos[1];
		return mupfunc(m_param, relx, rely, w, h);
	}

	return false;
}

bool ViewportW::mousewheel(int delta)
{
	if(mousewfunc != NULL)
	{
		return mousewfunc(m_param, delta);
	}

	return false;
}
