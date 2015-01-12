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
#include "saveview.h"
#include "loadview.h"
#include "../../icon.h"
#include "../../../net/sendpackets.h"
#include "../../../save/savemap.h"

//not engine
#include "../../../../game/gui/ggui.h"

//save view save button
void Resize_SV_SaveBut(Widget* thisw)
{
	SaveView* parw = (SaveView*)thisw->m_parent;

	thisw->m_pos[0] = parw->m_pos[2] - 70;
	thisw->m_pos[1] = parw->m_pos[3] - 30;
	thisw->m_pos[2] = parw->m_pos[2];
	thisw->m_pos[3] = parw->m_pos[3];

	CenterLabel(thisw);
}

//save file current name text label
void Resize_SV_CurName(Widget* thisw)
{
	SaveView* parw = (SaveView*)thisw->m_parent;

	thisw->m_pos[0] = parw->m_pos[0] + 30;
	thisw->m_pos[1] = parw->m_pos[3] - 60;
	thisw->m_pos[2] = parw->m_pos[2];
	thisw->m_pos[3] = thisw->m_pos[1] + 16;

	CenterLabel(thisw);
}

//save view delete button
void Resize_SV_DelBut(Widget* thisw)
{
	SaveView* parw = (SaveView*)thisw->m_parent;

	thisw->m_pos[0] = parw->m_pos[0];
	thisw->m_pos[1] = parw->m_pos[3] - 30;
	thisw->m_pos[2] = parw->m_pos[0] + 70;
	thisw->m_pos[3] = parw->m_pos[3];

	CenterLabel(thisw);
}

void Click_SV_Save()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	SaveView* v = (SaveView*)gui->get("save");
	EditBox* curn = (EditBox*)&v->m_curname;

	std::string path = std::string("saves/") + curn->m_value.rawstr();
	SaveMap(path.c_str());

	strcpy(g_lastsave, path.c_str());
	
	gui->close("save");
	gui->close("load");
}


void Click_SV_Del()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	SaveView* v = (SaveView*)gui->get("save");

	if(!v->m_selfile)
		return;

	std::string path = std::string("saves/") + v->m_selfile->rawstr();
	char fullpath[MAX_PATH+1];
	FullPath(path.c_str(), fullpath);
	unlink(fullpath);

	v->m_selfile = NULL;
	v->regen();
}

SaveView::SaveView(Widget* parent, const char* n, void (*reframef)(Widget* thisw)) : WindowW(parent, n, reframef)
{
	m_parent = parent;
	m_type = WIDGET_SAVEVIEW;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;
	m_font = MAINFONT16;

	m_svlistbg = Image(this, "gui/backg/svlistbg.jpg", false, NULL, 1, 1, 1,0.2f, 0, 0, 1, 1);
	m_selected = NULL;
	m_scroll[1] = 0;
	m_selfile = NULL;

	m_vscroll = VScroll(this, "m_vscroll");
	m_savebut = Button(this, "save but", "gui/transp.png", RichText("Save"), RichText("Save to file name"), m_font, BUST_LINEBASED, Resize_SV_SaveBut, Click_SV_Save, NULL, NULL, NULL, NULL, -1);
	m_delbut = Button(this, "del but", "gui/transp.png", RichText("Delete"), RichText("Delete the saved game"), m_font, BUST_LINEBASED, Resize_SV_DelBut, Click_SV_Del, NULL, NULL, NULL, NULL, -1);

	if(reframefunc)
		reframefunc(this);

	reframe();
}

void SaveView::subinev(InEv* ie)
{
	m_vscroll.inev(ie);
	m_savebut.inev(ie);
	m_delbut.inev(ie);
	m_curname.inev(ie);

	Player* py = &g_player[g_localP];
	
	m_scroll[1] = m_vscroll.m_scroll[1] * m_files.size()  * (1.0f);

	if(!ie->intercepted)
	{
		if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_LEFT)
		{
			if(g_mouse.x >= m_pos[0] && g_mouse.x <= m_pos[2] && g_mouse.y >= m_listtop && g_mouse.y <= m_listbot)
			{
				m_ldown = true;
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
						m_curname.changevalue(m_selfile);
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

void SaveView::regen()
{
	m_selfile = NULL;
	freech();
	m_files.clear();

	char datetime[64];
	sprintf(datetime, "%s.sav", FileDateTime().c_str());
	m_curname = EditBox(this, "cur name", RichText(datetime), MAINFONT16, Resize_SV_CurName, false, 64, NULL, NULL, -1);

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

void SaveView::frameupd()
{
	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->frameupd();

	m_vscroll.frameupd();
	
	m_scroll[1] = m_vscroll.m_scroll[1] * m_files.size()  * (1.0f);
}

void SaveView::subreframe()
{
	Font* f = &g_font[m_font];
	m_listtop = m_pos[1] + f->gheight;
	m_listbot = m_pos[3] - SLLISTBOT;

	m_vscroll.m_pos[0] = m_pos[2] - 16;
	m_vscroll.m_pos[1] = m_listtop;
	m_vscroll.m_pos[2] = m_pos[2];
	m_vscroll.m_pos[3] = m_listbot;
	float scrollable = f->gheight * m_files.size();
	float viewable = m_listbot - m_listtop;
	m_vscroll.m_domain = fmin(1, viewable / scrollable);
	m_vscroll.reframe();
	
	m_savebut.reframe();
	m_delbut.reframe();
	m_curname.reframe();
}

void SaveView::subdraw()
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
	m_savebut.draw();
	m_delbut.draw();
}

void SaveView::subdrawover()
{
	m_vscroll.drawover();
	m_curname.drawover();
	m_savebut.drawover();
	m_delbut.drawover();
}