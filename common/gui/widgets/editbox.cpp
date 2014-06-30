
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
#include "../gui.h"
#include "../../sys/unicode.h"
#include "../../debug.h"

EditBox::EditBox() : Widget()
{
	m_parent = NULL;
	m_type = WIDGET_EDITBOX;
	m_name = "";
	m_font = MAINFONT8;
	m_value = "";
	m_caret = m_value.length();
	m_opened = false;
	m_passw = false;
	m_maxlen = 0;
	reframefunc = NULL;
	m_scroll[0] = 0;
	m_highl[0] = 0;
	m_highl[1] = 0;
	CreateTexture(m_frametex, "gui/frame.jpg", true, false);
	m_param = -1;
	changefunc2 = NULL;
	//reframe();
}

EditBox::EditBox(Widget* parent, const char* n, const std::string t, int f, void (*reframef)(Widget* thisw), bool pw, int maxl, void (*change2)(int p), int parm) : Widget()
{
	m_parent = parent;
	m_type = WIDGET_EDITBOX;
	m_name = n;
	m_font = f;
	m_value = t;
	m_caret = m_value.length();
	m_opened = false;
	m_passw = pw;
	m_maxlen = maxl;
	reframefunc = reframef;
	m_scroll[0] = 0;
	m_highl[0] = 0;
	m_highl[1] = 0;
	CreateTexture(m_frametex, "gui/frame.jpg", true, false);
	m_param = parm;
	changefunc2 = change2;
	reframe();
}

std::string EditBox::drawvalue()
{
#if 1
	std::string val = m_value;

	if(m_passw)
	{
		val = "";
		for(int i=0; i<m_value.length(); i++)
			val.append("*");
	}

	return val;
#else
	if(!m_passw)
		return m_value;

	return m_value.pwver();
#endif
}

void EditBox::draw()
{
	//glColor4f(1, 1, 1, 1);
	glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 1, 1, 1, 1);

	DrawImage(g_texture[m_frametex].texname, m_pos[0], m_pos[1], m_pos[2], m_pos[3]);

	if(m_over)
		//glColor4f(1, 1, 1, 1);
		glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 1, 1, 1, 1);
	else
		//glColor4f(0.8f, 0.8f, 0.8f, 1);
		glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 0.8f, 0.8f, 0.8f, 1);

	std::string val = drawvalue();

	//if(m_opened)
	//	g_applog<<"op m_caret="<<m_caret<<std::endl;

	DrawShadowedTextF(m_font, m_pos[0]+m_scroll[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3], val.c_str(), NULL, m_opened ? m_caret : -1);

	//glColor4f(1, 1, 1, 1);
	//glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 1, 1, 1, 1);

	HighlightF(m_font, m_pos[0]+m_scroll[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3], val.c_str(), m_highl[0], m_highl[1]);
}

//#define MOUSESC_DEBUG

void EditBox::frameupd()
{
#ifdef MOUSESC_DEBUG
	g_applog<<"editbox frameup"<<std::endl;
	g_applog.flush();
#endif

	if(m_ldown)
	{
		bool movedcar = false;

#ifdef MOUSESC_DEBUG
		g_applog<<"ldown frameup"<<std::endl;
		g_applog.flush();
#endif

		if(g_mouse.x >= m_pos[2]-5)
		{
			m_scroll[0] -= fmax(1, g_font[m_font].gheight/4.0f);

			std::string val = drawvalue();
			int vallen = val.length();

			int endx = EndX(val.c_str(), vallen, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

			if(endx < m_pos[2])
				m_scroll[0] += m_pos[2] - endx;

			if(m_scroll[0] > 0.0f)
				m_scroll[0] = 0.0f;

			movedcar = true;
		}
		else if(g_mouse.x <= m_pos[0]+5)
		{
			m_scroll[0] += fmax(1, g_font[m_font].gheight/4.0f);

			if(m_scroll[0] > 0.0f)
				m_scroll[0] = 0.0f;

			movedcar = true;
		}

		if(movedcar)
		{
			std::string val = drawvalue();
			int newcaret = MatchGlyphF(val.c_str(), m_font, g_mouse.x, m_pos[0]+m_scroll[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3]);

			if(newcaret > m_caret)
			{
				m_highl[0] = m_caret;
				m_highl[1] = newcaret;
			}
			else if(newcaret < m_caret)
			{
				m_highl[0] = newcaret;
				m_highl[1] = m_caret;
			}
		}
	}
}

void EditBox::inev(InEv* ev)
{
//#ifdef MOUSESC_DEBUG
	//g_applog<<"editbox mousemove"<<std::endl;
	//g_applog.flush();
//#endif

	if(ev->type == INEV_MOUSEMOVE)
	{
		if(!ev->intercepted)
		{
			if(m_ldown)
			{
				std::string val = drawvalue();
				int newcaret = MatchGlyphF(val.c_str(), m_font, g_mouse.x, m_pos[0]+m_scroll[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3]);

				if(newcaret > m_caret)
				{
					m_highl[0] = m_caret;
					m_highl[1] = newcaret;
					//g_applog<<"hihgl "<<m_highl[0]<<"->"<<m_highl[1]<<std::endl;
					//g_applog.flush();
				}
				else
				{
					m_highl[0] = newcaret;
					m_highl[1] = m_caret;
					//g_applog<<"hihgl "<<m_highl[0]<<"->"<<m_highl[1]<<std::endl;
					//g_applog.flush();
				}

				ev->intercepted = true;
				return;
			}

			if(g_mouse.x >= m_pos[0] && g_mouse.x <= m_pos[2] && g_mouse.y >= m_pos[1] && g_mouse.y <= m_pos[3])
			{
				m_over = true;

				g_mouseoveraction = true;

				ev->intercepted = true;
				return;
			}
			else
			{
				m_over = false;

				return;
			}
		}
	}
	else if(ev->type == INEV_MOUSEDOWN && ev->key == MOUSE_LEFT)
	{
		if(m_opened)
		{
			m_opened = false;
			m_highl[0] = m_highl[1] = 0;
		}

		if(!ev->intercepted)
		{
			if(m_over)
			{
				m_ldown = true;

				std::string val = drawvalue();

				//m_highl[1] = MatchGlyphF(m_value.c_str(), m_font, m_pos[0]+m_scroll[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3]);
				//m_highl[0] = m_highl[1];
				//m_caret = m_highl[1];
				m_caret = MatchGlyphF(val.c_str(), m_font, g_mouse.x, m_pos[0]+m_scroll[0], m_pos[1], m_pos[0], m_pos[1], m_pos[2], m_pos[3]);

				m_highl[0] = 0;
				m_highl[1] = 0;

				ev->intercepted = true;
				return;
			}
		}
	}
	else if(ev->type == INEV_MOUSEUP && ev->key == MOUSE_LEFT && !ev->intercepted)
	{
		//if(m_over && m_ldown)
		if(m_ldown)
		{
			m_ldown = false;

			if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
			{
				m_caret = -1;
			}

			ev->intercepted = true;
			gainfocus();

			return;
		}

		m_ldown = false;

		if(m_opened)
		{
			ev->intercepted = true;
			return;
		}
	}
	else if(ev->type == INEV_KEYDOWN && !ev->intercepted)
	{
		if(!m_opened)
			return;

		int len = m_value.length();

		if(m_caret > len)
			m_caret = len;

		if(ev->key == SDLK_F1)
			return;

		if(ev->key == SDLK_LEFT)
		{
			if(m_highl[0] > 0 && m_highl[0] != m_highl[1])
			{
				m_caret = m_highl[0];
				m_highl[0] = m_highl[1] = 0;
			}
			else if(m_caret <= 0)
			{
				ev->intercepted = true;
				return;
			}
			else
				m_caret --;

			std::string val = drawvalue();
			int endx = EndX(val.c_str(), m_caret, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

			//g_applog<<"left endx = "<<endx<<"/"<<m_pos[0]<<std::endl;
			//g_applog.flush();

			if(endx <= m_pos[0])
				m_scroll[0] += m_pos[0] - endx + 1;
		}
		else if(ev->key == SDLK_RIGHT)
		{
			int len = m_value.length();

			if(m_highl[0] > 0 && m_highl[0] != m_highl[1])
			{
				m_caret = m_highl[1];
				m_highl[0] = m_highl[1] = 0;
			}
			else if(m_caret >= len)
			{
				ev->intercepted = true;
				return;
			}
			else
				m_caret ++;

			std::string val = drawvalue();
			int endx = EndX(val.c_str(), m_caret, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

			if(endx >= m_pos[2])
				m_scroll[0] -= endx - m_pos[2] + 1;
		}
		else if(ev->key == SDLK_DELETE)
		{
			len = m_value.length();

			//g_applog<<"vk del"<<std::endl;
			//g_applog.flush();

			if((m_highl[1] <= 0 || m_highl[0] == m_highl[1]) && m_caret >= len || len <= 0)
			{
				ev->intercepted = true;
				return;
			}

			delnext();

#if 0
			if(!m_passw)
				m_value = ParseTags(m_value, &m_caret);
#else
#endif
		}
		if(ev->key == SDLK_BACKSPACE)
		{
			len = m_value.length();

			if((m_highl[1] <= 0 || m_highl[0] == m_highl[1]) && len <= 0)
			{
				ev->intercepted = true;
				return;
			}

			delprev();

#if 0
			if(!m_passw)
				m_value = ParseTags(m_value, &m_caret);
#endif
		}/*
		 else if(ev->key == SDLK_DELETE)
		 {
		 len = m_value.texlen();

		 g_applog<<"vk del"<<std::endl;
		 g_applog.flush();

		 if((m_highl[1] <= 0 || m_highl[0] == m_highl[1]) && m_caret >= len || len <= 0)
		 return true;

		 delnext();

		 if(!m_passw)
		 m_value = ParseTags(m_value, &m_caret);
		 }*/
		else if(ev->key == SDLK_LSHIFT || ev->key == SDLK_RSHIFT)
		{
			ev->intercepted = true;
			return;
		}
		else if(ev->key == SDLK_CAPSLOCK)
		{
			ev->intercepted = true;
			return;
		}
		else if(ev->key == SDLK_RETURN)
		{
			ev->intercepted = true;
			return;
		}
#if 0
		else if(ev->key == 190 && !g_keys[SDLK_SHIFT])
		{
			//placechar('.');
		}
#endif

		if(changefunc2 != NULL)
			changefunc2(m_param);

		ev->intercepted = true;
	}
	else if(ev->type == INEV_KEYUP && !ev->intercepted)
	{
		if(!m_opened)
			return;

		ev->intercepted = true;
	}
	else if(ev->type == INEV_TEXTIN && !ev->intercepted)
	{
		if(!m_opened)
			return;

		ev->intercepted = true;

		int len = m_value.length();

		if(m_caret > len)
			m_caret = len;

		//g_applog<<"vk "<<ev->key<<std::endl;
		//g_applog.flush();


#if 0
		if(ev->key == SDLK_SPACE)
		{
			placechar(' ');
		}
		else
#endif
		{

#ifdef PASTE_DEBUG
			g_applog<<"charin "<<(char)ev->key<<" ("<<ev->key<<")"<<std::endl;
			g_applog.flush();
#endif

#if 0
			//if(ev->key == 'C' && g_keys[SDLK_CONTROL])
			if(ev->key == 3)	//copy
			{
				copyval();
			}
			//else if(ev->key == 'V' && g_keys[SDLK_CONTROL])
			else if(ev->key == 22)	//paste
			{
				pasteval();
			}
			//else if(ev->key == 'A' && g_keys[SDLK_CONTROL])
			else if(ev->key == 1)	//select all
			{
				selectall();
			}
			else
#endif

#if 0
			unsigned int* ustr = ToUTF32((const unsigned char*)ev->text.c_str(), ev->text.length());
			//std::string addstr(RichTextP(UString(ustr)));	//Why does MSVS2012 not accept this?
			std::string addstr = std::string(ustr);
			delete [] ustr;

			placestr(&addstr);
#else
			placestr(&ev->text);
#endif
		}

		if(changefunc != NULL)
			changefunc();

		if(changefunc2 != NULL)
			changefunc2(m_param);

		ev->intercepted = true;
	}
}

void EditBox::placestr(const std::string* str)
{
	int len = m_value.length();

	if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
	{
		len -= m_highl[1] - m_highl[0];
	}

	int addlen = str->length();
	if(addlen + len >= m_maxlen)
		addlen = m_maxlen - len;

	std::string addstr = str->substr(0, addlen);

	if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
	{
		std::string before = m_value.substr(0, m_highl[0]);
		std::string after = m_value.substr(m_highl[1]-1, m_value.length()-m_highl[1]);
		m_value = before + addstr + after;

		m_caret = m_highl[0] + addlen;
		m_highl[0] = m_highl[1] = 0;
	}
	else
	{
		if(len >= m_maxlen)
		{
			return;
		}

		std::string before = m_value.substr(0, m_caret);
		std::string after = m_value.substr(m_caret, m_value.length()-m_caret);
		m_value = before + addstr + after;
		m_caret += addlen;

		//LogRich(&m_value);
	}

	std::string val = drawvalue();
	int endx = EndX(val.c_str(), m_caret, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

	if(endx >= m_pos[2])
		m_scroll[0] -= endx - m_pos[2] + 1;
}

void EditBox::changevalue(const char* str)
{
	int len = m_value.length();

	if(len >= m_maxlen)
		return;

	int setlen = strlen(str);
	if(setlen >= m_maxlen)
		setlen = m_maxlen;

	char* setstr = new char[setlen+1];

	if(!setstr)
		OutOfMem(__FILE__, __LINE__);

	for(int i=0; i<setlen; i++)
		setstr[i] = str[i];
	setstr[setlen] = '\0';

	m_value = setstr;
	m_highl[0] = m_highl[1] = 0;
	m_caret = 0;

	delete [] setstr;
}

bool EditBox::delnext()
{
	int len = m_value.length();

	if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
	{
		std::string before = m_value.substr(0, m_highl[0]);
		std::string after = m_value.substr(m_highl[1], len-m_highl[1]);
		m_value = before + after;

		m_caret = m_highl[0];
		m_highl[0] = m_highl[1] = 0;
	}
	else if(m_caret >= len || len <= 0)
		return true;
	else
	{
		std::string before = m_value.substr(0, m_caret);
		std::string after = m_value.substr(m_caret+1, len-m_caret);
		m_value = before + after;
	}

	std::string val = drawvalue();
	int endx = EndX(val.c_str(), m_caret, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

	if(endx <= m_pos[0])
		m_scroll[0] += m_pos[0] - endx + 1;
	else if(endx >= m_pos[2])
		m_scroll[0] -= endx - m_pos[2] + 1;

	return true;
}

bool EditBox::delprev()
{
	int len = m_value.length();

	if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
	{
		std::string before = m_value.substr(0, m_highl[0]);
		std::string after = m_value.substr(m_highl[1], len-m_highl[1]);
		m_value = before + after;

		m_caret = m_highl[0];
		m_highl[0] = m_highl[1] = 0;
	}
	else if(m_caret <= 0 || len <= 0)
		return true;
	else
	{
		std::string before = m_value.substr(0, m_caret-1);
		std::string after = m_value.substr(m_caret, len-m_caret);
		m_value = before + after;

		//g_applog<<"before newval="<<before.rawstr()<<" texlen="<<before.texlen()<<std::endl;
		//g_applog<<"after="<<after.rawstr()<<" texlen="<<after.texlen()<<std::endl;
		//g_applog<<"ba newval="<<m_value.rawstr()<<" texlen="<<(before + after).texlen()<<std::endl;
		//g_applog<<"newval="<<m_value.rawstr()<<" texlen="<<m_value.texlen()<<std::endl;

		m_caret--;
	}

	std::string val = drawvalue();
	int endx = EndX(val.c_str(), m_caret, m_font, m_pos[0]+m_scroll[0], m_pos[1]);

	if(endx <= m_pos[0])
		m_scroll[0] += m_pos[0] - endx + 1;
	else if(endx >= m_pos[2])
		m_scroll[0] -= endx - m_pos[2] + 1;

	return true;
}

//#define PASTE_DEBUG

void EditBox::copyval()
{
#ifdef PASTE_DEBUG
	g_applog<<"copy vkc"<<std::endl;
	g_applog.flush();
#endif

#ifdef PLATFORM_WIN
	if(m_highl[1] > 0 && m_highl[0] != m_highl[1])
	{
		std::string highl = m_value.substr(m_highl[0], m_highl[1]-m_highl[0]);
		std::string rawhighl = highl.rawstr();
		const size_t len = strlen(rawhighl.c_str())+1;
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
		memcpy(GlobalLock(hMem), rawhighl.c_str(), len);
		GlobalUnlock(hMem);
		OpenClipboard(0);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hMem);
		CloseClipboard();
	}
	else
	{
		const char* output = "";
		const size_t len = strlen(output)+1;
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
		memcpy(GlobalLock(hMem), output, len);
		GlobalUnlock(hMem);
		OpenClipboard(0);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hMem);
		CloseClipboard();
	}
#endif // PLATFORM_WIN

	//return true;
}

void EditBox::pasteval()
{
#ifdef PLATFORM_WIN
#ifdef PASTE_DEBUG
	g_applog<<"paste"<<std::endl;
#endif
	OpenClipboard(NULL);

#ifdef PASTE_DEBUG
	g_applog<<"paste1"<<std::endl;
#endif
	HANDLE clip0 = GetClipboardData(CF_TEXT);

#ifdef PASTE_DEBUG
	g_applog<<"paste2"<<std::endl;
#endif
	//HANDLE h = GlobalLock(clip0);
	//placestr((char*)clip0);
	char* str = (char*)GlobalLock(clip0);
#ifdef PASTE_DEBUG
	g_applog<<"paste3"<<std::endl;
	g_applog<<str<<std::endl;
#endif

	//placestr(str);

#ifdef PASTE_DEBUG
	g_applog<<"place str ";
	g_applog<<str<<std::endl;
	g_applog.flush();
	g_applog.flush();
#endif

	if(!m_passw)
		m_value = ParseTags(m_value, &m_caret);

	GlobalUnlock(clip0);
	CloseClipboard();

	//return true;
#endif // PLATFORM_WIN
}

void EditBox::selectall()
{
	m_highl[0] = 0;
	m_highl[1] = m_value.length()+1;
	m_caret = -1;

	std::string val = drawvalue();
	int endx = EndX(val.c_str(), m_value.length(), m_font, m_pos[0]+m_scroll[0], m_pos[1]);

	if(endx <= m_pos[2])
		m_scroll[0] += m_pos[2] - endx - 1;

	if(m_scroll[0] >= 0)
		m_scroll[0] = 0;

	//return true;
}

void EditBox::close()
{
	losefocus();

	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->close();
}


void EditBox::gainfocus()
{
	if(!m_opened)
	{
		if(g_kbfocus > 0)
		{
			SDL_StopTextInput();
			g_kbfocus--;
		}

		m_opened = true;
		SDL_StartTextInput();
		SDL_Rect r;
		r.x = m_pos[0];
		r.y = m_pos[3];
		r.w = g_width - m_pos[0];
		r.h = g_height - m_pos[3];
		SDL_SetTextInputRect(&r);
		g_kbfocus++;
	}
}

void EditBox::losefocus()
{
	if(m_opened)
	{
		if(g_kbfocus > 0)
		{
			SDL_StopTextInput();
			g_kbfocus--;
		}

		m_opened = false;
	}
}
