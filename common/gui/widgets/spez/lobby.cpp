#include "../../widget.h"
#include "../barbutton.h"
#include "../button.h"
#include "../checkbox.h"
#include "../editbox.h"
#include "../droplist.h"
#include "../image.h"
#include "../insdraw.h"
#include "../link.h"
#include "../listbox.h"
#include "../text.h"
#include "../textarea.h"
#include "../textblock.h"
#include "../touchlistener.h"
#include "../frame.h"
#include "cstrview.h"
#include "../../../platform.h"
#include "../viewportw.h"
#include "../../../../game/gui/gviewport.h"
#include "../../../sim/building.h"
#include "../../../sim/bltype.h"
#include "../../../sim/road.h"
#include "../../../sim/crpipe.h"
#include "../../../sim/powl.h"
#include "../../../sim/unit.h"
#include "../../../sim/utype.h"
#include "../../../sim/player.h"
#include "newhost.h"
#include "blpreview.h"
#include "../../icon.h"
#include "../../../net/sendpackets.h"
#include "lobby.h"
#include "../../../sys/unicode.h"
#include "../../../sys/utf8.h"
#include "../../../net/client.h"

//not engine
#include "../../../../game/gui/ggui.h"
#include "../../../../game/gui/chattext.h"

//player lines top
#define PYLINESTOP	200
//player lines left start
#define PYLINESLEFT	100
//player lines seperator width
#define PYSEPW		110

//lobby submit chat message
void Submit_L_Chat()
{
}

//lobby add chat message - local from user or remote from other players
void L_AddCh()
{
}

Lobby::Lobby(Widget* parent, const char* n) : ViewLayer(parent, n)
{
	m_parent = parent;
	m_type = WIDGET_LOBBY;
	m_name = n;
	reframefunc = NULL;
	m_ldown = false;
	m_font = MAINFONT16;

	m_chsl = 0;
	
	m_bg = Image(this, "gui/mmbg.jpg", true, Resize_Fullscreen);
	m_pybg = Image(this, "gui/backg/white.jpg", true, NULL, 1.0f, 1.0f, 1.0f, 0.6f);
	m_chbg = Image(this, "gui/backg/white.jpg", true, NULL, 1.0f, 1.0f, 1.0f, 0.6f);
	m_svname = Text(this, "sv name", RichText(""), MAINFONT16, NULL);
	m_chls = TextBlock(this, "ch lines", RichText("polyfrag: LAJSJAKLSD\npolyfrag: dkfp9fw9jfwo43\npolyfrag: f9wj30wj09fjfw\npolyfrag: pjfp9sjfs**(score#8\n...."), MAINFONT16, NULL, 0.8f, 0.8f, 0.8f, 1.0f);

#if 0
	for(int i=0; i<PLAYERS; i++)
	{
		char wname[32];
		sprintf(wname, "%d player", i);
		const unsigned char* uc = g_pycols[i].color;
		const float c[3] = { uc[0] / 255.0f, uc[1] / 255.0f, uc[2] / 255.0f };
		m_pyline[i] = Text(this, wname, g_player[i].name, MAINFONT16, Resize_L_PyL, true, c[0], c[1], c[2], 1.0f);
		//m_pyline[i] = Text(this, wname, RichText(wname), MAINFONT16, Resize_L_PyL, true, c[0], c[1], c[2], 1.0f);
	}
#endif

	if(reframefunc)
		reframefunc(this);

	reframe();
}

void Lobby::frameupd()
{
	m_pylsc.frameupd();
}

void Lobby::subreframe()
{
	Player* py = &g_player[g_localP];

	m_pylpos[0] = g_width - 220;
	m_pylpos[1] = 20;
	m_pylpos[2] = g_width - 20;
	m_pylpos[3] = 420;

	Font* f = &g_font[m_font];

	m_chpos[0] = 20;
	m_chpos[1] = g_height - 20 - f->gheight * (CHAT_LINES+1);
	m_chpos[2] = m_pylpos[0] - 20;
	m_chpos[3] = g_height - 20;

	m_svname.m_pos[0] = 20;
	m_svname.m_pos[1] = 20;
	m_svname.m_pos[2] = g_width - 20;
	m_svname.m_pos[3] = g_height - 20;
	m_svname.m_tpos[0] = 20;
	m_svname.m_tpos[1] = 20;
	
	m_chls.m_pos[0] = m_chpos[0];
	m_chls.m_pos[1] = m_chpos[1];
	m_chls.m_pos[2] = m_chpos[2];
	m_chls.m_pos[3] = m_chpos[3];
	m_chls.reframe();

	//TextBlock m_chls;	//chat lines
	//VScroll m_chsc;	//chat lines scroll

	//DrawBoxShadTextF(
	//TO DO chatlines

	m_bg.reframe();
	//m_pybg.reframe();
	m_svname.reframe();

	//for(int i=0; i<PLAYERS; i++)
	//	m_pyline[i].reframe();
	
	m_pybg.m_pos[0] = m_pylpos[0];
	m_pybg.m_pos[1] = m_pylpos[1];
	m_pybg.m_pos[2] = m_pylpos[2];
	m_pybg.m_pos[3] = m_pylpos[3];
	
	m_chbg.m_pos[0] = m_chpos[0];
	m_chbg.m_pos[1] = m_chpos[1];
	m_chbg.m_pos[2] = m_chpos[2];
	m_chbg.m_pos[3] = m_chpos[3];

	m_pylsc.m_pos[0] = m_pylpos[2] - 16;
	m_pylsc.m_pos[1] = m_pylpos[1];
	m_pylsc.m_pos[2] = m_pylpos[2];
	m_pylsc.m_pos[3] = m_pylpos[3];
	
	//jcls = joined players count
	int jcls = njcl();
#if 0
	//scrollspace is the area on the screen within which the scrollable area is viewed
	float scrollspace = m_pylpos[3] - m_pylpos[1];
	//scrollable is the total area that can be scrolled through
	float scrollable = f->gheight * jcls;
	//viewable is the subset of the scrollable area that is seen once at a time 
	//(that subset of list items that is seen at a screenful).
	int viewable = scrollspace / f->gheight;
#else
	//scrollable is the total area that can be scrolled through
	float scrollable = f->gheight * jcls;
	//viewable is the subset of the scrollable area that is seen once at a time.
	//in this case viewable is the area on the screen within which the scrollable area is viewed.
	float viewable = m_pylpos[3] - m_pylpos[1];
#endif
	m_pylsc.m_domain = fmin(1, viewable / scrollable);
	m_pylsc.reframe();
}

void Lobby::regen()
{
#if 0
	for(int i=0; i<PLAYERS; i++)
	{
		char wname[32];
		sprintf(wname, "%d player", i);
		const unsigned char* uc = g_pycols[i].color;
		const float c[3] = { uc[0] / 255.0f, uc[1] / 255.0f, uc[2] / 255.0f };
		m_pyline[i] = Text(this, wname, g_player[i].name, MAINFONT16, Resize_L_PyL, true, c[0], c[1], c[2], 1.0f);
		//m_pyline[i] = Text(this, wname, RichText(wname), MAINFONT16, Resize_L_PyL, true, c[0], c[1], c[2], 1.0f);
	}
#endif

	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;

	if(g_netmode == NETM_HOST)
	{
		NewHost* h = (NewHost*)gui->get("new host");
		EditBox* n = (EditBox*)&h->m_gamename;

		if(n->m_value.m_part.size() > 0)
		{
			//g_log<<" n->m_value.m_part.begin()->m_text.m_length = "<<n->m_value.m_part.begin()->m_text.m_length<<std::endl;
			//g_log<<" n->m_value.rawlen() = "<<n->m_value.rawlen()<<std::endl;
		
			//unsigned char* utf8 = ToUTF8(n->m_value.m_part.begin()->m_text.m_data);
			//unsigned char* utf8 = ToUTF8((unsigned char*)n->m_value.rawstr().c_str());
			std::string rawstr = n->m_value.rawstr();
			unsigned char* utf8 = new unsigned char[ rawstr.length() + 1 ];
			strcpy((char*)utf8, rawstr.c_str());
			//unsigned char* utf8 = new unsigned char [ n->m_value.rawlen() * 4 + 1 ];
			//int utf8len = wchar_to_utf8(n->m_value.m_part.begin()->m_text.m_data, n->m_value.rawlen(), (char*)utf8, n->m_value.rawlen() * 4, 0);
			//utf8[utf8len] = 0;
			//int rawlen = n->m_value.rawlen();

			//ValidateUTF8(utf8, strlen((char*)utf8));

			//g_log<<"n->m_value.m_part.begin()->m_text[0] = "<<n->m_value.m_part.begin()->m_text[0]<<std::endl;
		
			//g_log<<" strlen((char*)utf8) = "<<strlen((char*)utf8)<<std::endl;

			if(strlen((char*)utf8) > SVNAME_LEN)
				utf8[SVNAME_LEN] = 0;

			//g_log<<"utf8[0,1,2,3] = "<<(int)utf8[0]<<","<<(int)utf8[1]<<","<<(int)utf8[2]<<","<<(int)utf8[3]<<" = "<<(char)utf8[0]<<std::endl;

			strcpy(g_svname, (char*)utf8);
		
			//g_log<<" strlen(g_svname) = "<<strlen(g_svname)<<std::endl;
		
			//g_log<<"g_svname[0,1,2,3] = "<<(int)g_svname[0]<<","<<(int)g_svname[1]<<","<<(int)g_svname[2]<<","<<(int)g_svname[3]<<" = "<<(char)g_svname[0]<<std::endl;
		
			//unsigned int* utf32 = ToUTF32(utf8, strlen((char*)utf8));
			//unsigned int* utf32 = ToUTF32(utf8, rawlen);
			//unsigned int* utf32 = new unsigned int [ strlen((char*)utf8) + 1 ];
			//int utf32len = utf8_to_wchar((char*)utf8, strlen((char*)utf8), utf32, strlen((char*)utf8), 0);
			//utf32[utf32len] = 0;
			unsigned int* utf32 = ToUTF32(utf8);
			delete [] utf8;

			m_svname.m_text = RichText(UString(utf32));
			m_svname.m_text = ParseTags(m_svname.m_text, NULL);
			delete [] utf32;

			//g_log<<" m_svname.m_text.rawlen() = "<<m_svname.m_text.rawlen()<<std::endl;
		}
		else
		{
			strcpy(g_svname, "");
			m_svname.m_text = RichText(UString(g_svname));
		}
	}
	else if(g_netmode == NETM_CLIENT)
	{
		unsigned int* utf32 = ToUTF32((unsigned char*)g_svname);
		m_svname.m_text = RichText(UString(utf32));
		m_svname.m_text = ParseTags(m_svname.m_text, NULL);
		delete [] utf32;
	}

	reframe();
}

void Lobby::subinev(InEv* ie)
{
	//m_bg.inev(ie);
	//m_pybg.inev(ie);
	m_pylsc.inev(ie);

	//for(int i=0; i<PLAYERS; i++)
	//	m_pyline[i].inev(ie);
}

int Lobby::njcl()
{
	//jcls = joined clients count
	int jcls = 0;

#if 0
	for(int tcin=0; tcin<PLAYERS; tcin++)
	{
		Player* py = &g_player[tcin];
		if(!py->on)
			continue;
		jcls++;
	}
#else
	for(int tcin=0; tcin<CLIENTS; tcin++)
	{
		Client* c = &g_client[tcin];
		if(!c->on)
			continue;
		jcls++;
	}
#endif

	return jcls;
}

//draw player list
void Lobby::drawpyl()
{
	int jcin = 0;
	Font* f = &g_font[m_font];
	float scrollspace = m_pylpos[3] - m_pylpos[1];
	int viewable = scrollspace / f->gheight;

	//jcls = joined clients count
	int jcls = njcl();

	float scroll = m_pylsc.m_scroll[1] * jcls;
	int viewi = 0;

	//tcin = total players' index (out of all players that can exist)
	//jcin = joined players' index (subset that is actually listed/drawn)
	for(int tcin=0; tcin<CLIENTS; tcin++)
	{
		Client* c = &g_client[tcin];

		if(!c->on)
			continue;
		
		if(jcin < scroll)
		{
			jcin++;
			continue;
		}

		if(jcin - scroll > viewable)
		{
			jcin++;
			break;
		}

		float color[4] = {0.9f, 0.6f, 0.2f, 0.9f};

		char numcs[16];
		sprintf(numcs, "%d.", jcin+1);
		RichText numrt = RichText(UString(numcs));
		DrawShadowedTextF(m_font, m_pylpos[0], (int)(m_pylpos[1] + (jcin-scroll)*f->gheight), m_pylpos[0], m_pylpos[1], m_pylpos[2], m_pylpos[3], &numrt, color);

		DrawShadowedTextF(m_font, m_pylpos[0] + f->gheight*1.5f, (int)(m_pylpos[1] + (jcin-scroll)*f->gheight), m_pylpos[0], m_pylpos[1], m_pylpos[2], m_pylpos[3], &c->name, color);

		viewi++;
		jcin++;
	}
}

void Lobby::subdraw()
{
	m_bg.draw();
	m_pybg.draw();
	m_chbg.draw();
	drawpyl();
	m_chls.draw();
	m_pylsc.draw();
	m_svname.draw();

	//for(int i=0; i<PLAYERS; i++)
	//	m_pyline[i].draw();
}
void Lobby::subdrawover()
{
	//m_bg.drawover();
	//m_pybg.drawover();
	m_pylsc.drawover();

	//for(int i=0; i<PLAYERS; i++)
	//	m_pyline[i].drawover();
}