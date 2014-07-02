

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

void Text::draw()
{
	//glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
	//glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 0.8f, 0.8f, 0.8f, 1.0f);
	//float color[] = {0.8f, 0.8f, 0.8f, 1.0f};
	//DrawShadowedText(font, pos[0], pos[1], text.c_str(), color);

#ifdef DEBUG
	g_applog<<"draw text "<<m_text.c_str()<<" (shadow: "<<m_shadow<<")"<<std::endl;
	g_applog.flush();
#endif

#ifdef DEBUG
	g_applog<<"text "<<__FILE__<<" "<<__LINE__<<std::endl;
	g_applog.flush();
#endif

	if(m_shadow)
	{
#ifdef DEBUG
		g_applog<<"text "<<__FILE__<<" "<<__LINE__<<std::endl;
		g_applog.flush();
#endif
#if 0
		DrawShadowedTextF(m_font, m_pos[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3], m_text.c_str(), m_rgba);
#else
		DrawShadowedText(m_font, m_pos[0], m_pos[1], m_text.c_str(), m_rgba);
#endif
	}
	else
	{
#ifdef DEBUG
		g_applog<<"text "<<__FILE__<<" "<<__LINE__<<std::endl;
		g_applog.flush();
#endif
#if 0
		DrawLineF(m_font, m_pos[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3], m_text.c_str(), m_rgba);
#else
		DrawLine(m_font, m_pos[0], m_pos[1], m_text.c_str(), m_rgba);
#endif
	}
	
#ifdef DEBUG
	g_applog<<"text "<<__FILE__<<" "<<__LINE__<<std::endl;
	g_applog.flush();
#endif

	//glColor4f(1, 1, 1, 1);
	glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 1, 1, 1, 1);
}

