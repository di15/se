


#include "../draw/shader.h"
#include "gui.h"
#include "../texture.h"
#include "font.h"
#include "../math/3dmath.h"
#include "../platform.h"
#include "../window.h"
#include "draw2d.h"
#include "../draw/shadow.h"
#include "../debug.h"

GUI g_GUI;
int g_currw;
int g_currh;

void GUI::draw()
{
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	glClear(GL_DEPTH_BUFFER_BIT);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	Ortho(g_width, g_height, 1, 1, 1, 1);

#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
	LastNum(__FILE__, __LINE__);
#endif

	for(auto i=view.begin(); i!=view.end(); i++)
	{
		if(i->opened)
		{
			//LastNum(view[i].name.c_str());
			//g_log<<(i->name)<<endl;
			//g_log.flush();
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
			i->draw();
		}
	}

#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
	LastNum(__FILE__, __LINE__);
#endif

	for(auto i=view.begin(); i!=view.end(); i++)
		if(i->opened)
		{
			//g_log<<(i->name)<<endl;
			//g_log.flush();
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
			i->draw2();
		}

	//DrawImage(g_depth, g_width - 300, 0, g_width, 300, 0, 1, 1, 0);

#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
	LastNum(__FILE__, __LINE__);
#endif
	EndS();

	UseS(SHADER_COLOR2D);
    glUniform1f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_WIDTH], (float)g_width);
    glUniform1f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_HEIGHT], (float)g_height);
	//DrawMarquee();
	EndS();
}

void AssignKey(int i, void (*down)(), void (*up)())
{
	g_GUI.assignKey(i, down, up);
}

void AssignAnyKey(void (*down)(int k), void (*up)(int k))
{
	g_GUI.assignAnyKey(down, up);
}

void AssignMouseWheel(void (*wheel)(int delta))
{
	g_GUI.assignMouseWheel(wheel);
}

void AssignMouseMove(void (*mouse)())
{
	g_GUI.assignMouseMove(mouse);
}

void AssignLButton(void (*down)(), void (*up)())
{
	g_GUI.assignLButton(down, up);
}

void AssignRButton(void (*down)(), void (*up)())
{
	g_GUI.assignRButton(down, up);
}

void AssignMButton(void (*down)(), void (*up)())
{
	g_GUI.assignMButton(down, up);
}

View* AddView(const char* name, int page)
{
	View view(name, page);
	g_GUI.view.push_back(view);
	return &*g_GUI.view.rbegin();
}

bool ViewOpen(const char* name, int page)
{
	for(auto i=g_GUI.view.begin(); i!=g_GUI.view.end(); i++)
		if(_stricmp(i->name.c_str(), name) == 0 && i->page == page)
			return i->opened;

	return false;
}

void CloseView(const char* name)
{
	for(auto i=g_GUI.view.begin(); i!=g_GUI.view.end(); i++)
		if(_stricmp(i->name.c_str(), name) == 0)
			i->opened = false;
}

void OpenSoleView(const char* name, int page)
{
	for(auto i=g_GUI.view.begin(); i!=g_GUI.view.end(); i++)
	{
		if(_stricmp(i->name.c_str(), name) == 0 && i->page == page)
			i->opened = true;
		else
			i->opened = false;
	}
}

bool OpenAnotherView(const char* name, int page)
{
	for(auto i=g_GUI.view.begin(); i!=g_GUI.view.end(); i++)
	{
		if(_stricmp(i->name.c_str(), name) == 0 && i->page == page)
		{
			i->opened = true;
			return true;
		}
	}

	return false;
}

void NextPage(const char* name)
{
	int page = 0;

	for(auto i=g_GUI.view.begin(); i!=g_GUI.view.end(); i++)
	{
		if(_stricmp(i->name.c_str(), name) == 0 && i->opened)
		{
			page = i->page;
			i->opened = false;
			break;
		}
	}

	if(!OpenAnotherView(name, page+1))
		OpenAnotherView(name, 0);
}

void Status(const char* status, bool logthis)
{
	if(logthis)
	{
		g_log<<status<<endl;
		g_log.flush();
	}
	/*
	char upper[1024];
	int i;
	for(i=0; i<strlen(status); i++)
	{
		upper[i] = toupper(status[i]);
	}
	upper[i] = '\0';*/

	//g_GUI.getview("loading")->getwidget("status", WIDGET_TEXT)->m_text = upper;
	g_GUI.getview("loading")->getwidget("status", WIDGET_TEXT)->m_text = status;
}

bool MousePosition()
{
	Vec2i old = g_mouse;
	SDL_GetMouseState(&g_mouse.x, &g_mouse.y);

	if(g_mouse.x == old.x && g_mouse.y == old.y)
		return false;

	return true;
}

void CenterMouse()
{
	g_mouse.x = g_width/2;
	g_mouse.y = g_height/2;
	SDL_WarpMouseInWindow(g_window, g_mouse.x, g_mouse.y);
}

void Ortho(int width, int height, float r, float g, float b, float a)
{
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	UseS(SHADER_ORTHO);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	glUniform1f(g_shader[SHADER_ORTHO].m_slot[SSLOT_WIDTH], (float)width);
	glUniform1f(g_shader[SHADER_ORTHO].m_slot[SSLOT_HEIGHT], (float)height);
	glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], r, g, b, a);
	//glEnableVertexAttribArray(g_shader[SHADER_ORTHO].m_slot[SSLOT_POSITION]);
	//glEnableVertexAttribArray(g_shader[SHADER_ORTHO].m_slot[SSLOT_TEXCOORD0]);
	g_currw = width;
	g_currh = height;
}
