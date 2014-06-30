

#include "../widget.h"
#include "barbutton.h"
#include "button.h"
#include "checkbox.h"
#include "editbox.h"
#include "dropdowns.h"
#include "image.h"
#include "insdraw.h"
#include "link.h"
#include "listbox.h"
#include "text.h"
#include "textarea.h"
#include "textblock.h"
#include "touchlistener.h"

void Link::draw()
{
	glDisable(GL_TEXTURE_2D);

	float color[] = {1, 1, 1, 1};

	if(m_over)
	{
		//glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 1, 1, 1, 1);
		//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		//glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
		//glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 0.8f, 0.8f, 0.8f, 1.0f);
		color[0] = 0.8f;
		color[1] = 0.8f;
		color[2] = 0.8f;
	}

	DrawShadowedText(m_font, m_pos[0], m_pos[1], m_text.c_str(), color);

	//glColor4f(1, 1, 1, 1);
	glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 1, 1, 1, 1);

	//glEnable(GL_TEXTURE_2D);
}

void Link::inev(InEv* ev)
{
	if(ev->type == INEV_MOUSEUP && ev->key == MOUSE_LEFT && !ev->intercepted)
	{
		//mousemove();

		if(m_over && m_ldown)
		{
			if(clickfunc != NULL)
				clickfunc();

			m_over = false;
			m_ldown = false;

			ev->intercepted = true;
			return;	// intercept mouse event
		}

		m_over = false;
		m_ldown = false;
	}
	else if(ev->type == INEV_MOUSEDOWN && ev->key == MOUSE_LEFT && !ev->intercepted)
	{
		//mousemove();

		if(m_over)
		{
			m_ldown = true;
			ev->intercepted = true;
			return;	// intercept mouse event
		}
	}
	else if(ev->type == INEV_MOUSEMOVE)
	{
		int texlen = m_text.length();
		if(g_mouse.x >= m_pos[0] && g_mouse.y >= m_pos[1] &&
				g_mouse.x <= m_pos[0]+texlen*g_font[m_font].gheight/2 &&
				g_mouse.y <= m_pos[1]+g_font[m_font].gheight)
		{
		}
		else
		{
			m_over = false;
		}

		if(!ev->intercepted)
		{
			if(g_mouse.x >= m_pos[0] && g_mouse.y >= m_pos[1] &&
					g_mouse.x <= m_pos[0]+texlen*g_font[m_font].gheight/2 &&
					g_mouse.y <= m_pos[1]+g_font[m_font].gheight)
			{
				m_over = true;

				ev->intercepted = true;
			}
			else
			{
				m_over = false;
			}
		}
	}
}

