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
#include "svlist.h"
#include "blpreview.h"
#include "loadview.h"
#include "../../icon.h"
#include "../../../save/savemap.h"

//not engine
#include "../../../../game/gui/ggui.h"
#include "../../../../game/gmain.h"

//load view load button
void Resize_LV_LoadBut(Widget* thisw)
{
	LoadView* parw = (LoadView*)thisw->m_parent;

	thisw->m_pos[0] = parw->m_pos[2] - 70;
	thisw->m_pos[1] = parw->m_pos[3] - 30;
	thisw->m_pos[2] = parw->m_pos[2];
	thisw->m_pos[3] = parw->m_pos[3];

	CenterLabel(thisw);
}


//load view delete button
void Resize_LV_DelBut(Widget* thisw)
{
	LoadView* parw = (LoadView*)thisw->m_parent;
	
	thisw->m_pos[0] = parw->m_pos[0];
	thisw->m_pos[1] = parw->m_pos[3] - 30;
	thisw->m_pos[2] = parw->m_pos[0] + 70;
	thisw->m_pos[3] = parw->m_pos[3];

	CenterLabel(thisw);
}

void Click_LV_Load()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	LoadView* v = (LoadView*)gui->get("load");

	if(!v->m_selfile)
		return;

	std::string path = std::string("saves/") + v->m_selfile->rawstr();
	FreeMap();
	EndSess();
	BegSess();
	LoadMap(path.c_str());
	strcpy(g_lastsave, path.c_str());
	
	gui->close("save");
	gui->close("load");

	gui->closeall();
	gui->open("play");
	g_mode = APPMODE_PLAY;
}

void Click_LV_Del()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	LoadView* v = (LoadView*)gui->get("load");

	if(!v->m_selfile)
		return;

	std::string path = std::string("saves/") + v->m_selfile->rawstr();
	char fullpath[MAX_PATH+1];
	FullPath(path.c_str(), fullpath);
	unlink(fullpath);

	v->m_selfile = NULL;
	v->regen();
}

LoadView::LoadView(Widget* parent, const char* n, void (*reframef)(Widget* thisw)) : WindowW(parent, n, reframef)
{
	m_parent = parent;
	m_type = WIDGET_LOADVIEW;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;
	m_font = MAINFONT16;

	m_svlistbg = Image(this, "gui/backg/svlistbg.jpg", false, NULL, 1, 1, 1,0.2f, 0, 0, 1, 1);
	m_selected = NULL;
	m_scroll[1] = 0;
	m_selfile = NULL;

	m_vscroll = VScroll(this, "m_vscroll");
	m_loadbut = Button(this, "load but", "gui/transp.png", RichText("Load"), RichText("Load the selected game"), m_font, BUST_LINEBASED, Resize_LV_LoadBut, Click_LV_Load, NULL, NULL, NULL, NULL, -1);
	m_delbut = Button(this, "del but", "gui/transp.png", RichText("Delete"), RichText("Delete the selected game"), m_font, BUST_LINEBASED, Resize_LV_DelBut, Click_LV_Del, NULL, NULL, NULL, NULL, -1);

	if(reframefunc)
		reframefunc(this);

	reframe();
}

void LoadView::subinev(InEv* ie)
{
	m_vscroll.inev(ie);
	m_loadbut.inev(ie);
	m_delbut.inev(ie);
	
	m_scroll[1] = m_vscroll.m_scroll[1] * m_files.size()  * (1.0f);

	Player* py = &g_player[g_localP];

	if(!ie->intercepted)
	{
		if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_LEFT)
		{
			if(g_mouse.x >= m_pos[0] && g_mouse.x <= m_pos[2] && g_mouse.y >= m_listtop && g_mouse.y <= m_listbot)
			{
				m_ldown = true;
				ie->intercepted = true;
				int sin = 0;
				Font* f = &g_font[m_font];
				float scrollspace = m_listbot - m_listtop;
				int viewable = scrollspace / f->gheight;
				float scroll = m_scroll[1];
				int viewi = 0;

				for(auto sit=m_files.begin(); sit!=m_files.end(); sit++, sin++)
				{
					if(sin < scroll)
						continue;

					if(sin - scroll > viewable)
						break;

					if(g_mouse.y >= m_listtop + (sin-scroll)*f->gheight &&
						g_mouse.y <= m_listtop + (sin-scroll+1.0f)*f->gheight)
					{
						m_selfile = &*sit;
						m_curname.m_text = *m_selfile;
						ie->intercepted = true;
						break;
					}

					viewi++;
				}
			}
		}
		else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT)
		{
			if(m_ldown)
				ie->intercepted = true;
		}
	}

	if(ie->type == INEV_MOUSEMOVE)
	{
	}
}

void LoadView::regen()
{
	m_selfile = NULL;

	freech();

	if(m_files.size() > 0)
		m_files.clear();

	std::list<std::string> files;
	char fullpath[MAX_PATH+1];
	FullPath("saves/", fullpath);
	ListFiles(fullpath, files);

	for(auto fi=files.begin(); fi!=files.end(); fi++)
	{
		const char* fs = fi->c_str();
		m_files.push_back(RichText(UString(fs)));
	}

	reframe();
}

void LoadView::frameupd()
{
	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->frameupd();

	m_vscroll.frameupd();
	
	m_scroll[1] = m_vscroll.m_scroll[1] * m_files.size()  * (1.0f);
}

void LoadView::subreframe()
{
	Font* f = &g_font[m_font];
	m_listtop = m_pos[1] + f->gheight;
	m_listbot = m_pos[3] - SLLISTBOT;

	Texture* im = &g_texture[ m_svlistbg.m_tex ];
	m_svlistbg.m_pos[0] = m_pos[0];
	m_svlistbg.m_pos[1] = m_pos[1];
	m_svlistbg.m_pos[2] = m_pos[2];
	m_svlistbg.m_pos[3] = m_pos[3];
	m_svlistbg.m_texc[2] = (m_pos[2]-m_pos[0])/im->width;
	m_svlistbg.m_texc[3] = (m_pos[3]-m_pos[1])/im->height;

	m_vscroll.m_pos[0] = m_pos[2] - 16;
	m_vscroll.m_pos[1] = m_listtop;
	m_vscroll.m_pos[2] = m_pos[2];
	m_vscroll.m_pos[3] = m_listbot;
	//scrollable is the total area that can be scrolled through
	float scrollable = f->gheight * m_files.size();
	//viewable is the subset of the scrollable area that is seen once at a time.
	//in this case viewable is the area on the screen within which the scrollable area is viewed.
	float viewable = m_listbot - m_listtop;
	m_vscroll.m_domain = fmin(1, viewable / scrollable);
	m_vscroll.reframe();

	m_loadbut.reframe();
	m_delbut.reframe();
}

void LoadView::subdraw()
{
	//m_svlistbg.draw();

	int sin = 0;
	Font* f = &g_font[m_font];
	float scrollspace = m_listbot - m_listtop;
	int viewable = scrollspace / f->gheight;
	float scroll = m_scroll[1];
	int viewi = 0;

	for(auto sit=m_files.begin(); sit!=m_files.end(); sit++, sin++)
	{
		if(sin < scroll)
			continue;

		if(sin - scroll > viewable)
			break;

		if(&*sit == m_selfile)
		{
			UseS(SHADER_COLOR2D);
			Player* py = &g_player[g_localP];
			
			Shader* s = &g_shader[g_curS];
			glUniform1f(g_shader[SHADER_ORTHO].m_slot[SSLOT_WIDTH], (float)g_width);
			glUniform1f(g_shader[SHADER_ORTHO].m_slot[SSLOT_HEIGHT], (float)g_height);
			//glUniform4f(g_shader[SHADER_ORTHO].m_slot[SSLOT_COLOR], 0.2f, 1.0f, 0.2f, 0.6f);

			DrawSquare(0.2f, 1.0f, 0.2f, 0.6f, m_pos[0], m_listtop + (sin-scroll)*f->gheight, m_pos[2], m_listtop + (sin-scroll+1.0f)*f->gheight);

			Ortho(g_width, g_height, 1, 1, 1, 1);
		}

		float namecolor[4] = {0.9f, 0.6f, 0.2f, 0.9f};
		DrawShadowedTextF(m_font, m_pos[0], (int)(m_listtop + (sin-scroll)*f->gheight), m_pos[0], m_pos[1], m_pos[2], m_listbot, &*sit, namecolor);

		viewi++;
	}

	m_vscroll.draw();
	m_curname.draw();
	m_loadbut.draw();
	m_delbut.draw();
}

void LoadView::subdrawover()
{
	m_vscroll.drawover();
	m_curname.drawover();
	m_loadbut.drawover();
	m_delbut.drawover();
}