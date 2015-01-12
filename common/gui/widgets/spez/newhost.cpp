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
#include "../../../net/client.h"

//not engine
#include "../../../../game/gui/ggui.h"

//new host create button
void Resize_NH_Create(Widget* thisw)
{
	NewHost* parw = (NewHost*)thisw->m_parent;

	thisw->m_pos[0] = parw->m_pos[2] - 70;
	thisw->m_pos[1] = parw->m_pos[1] + 30;
	thisw->m_pos[2] = parw->m_pos[2];
	thisw->m_pos[3] = parw->m_pos[1] + 30*2;

	CenterLabel(thisw);
}

//new host game name edit box
void Resize_NH_GameName(Widget* thisw)
{
	NewHost* parw = (NewHost*)thisw->m_parent;

	thisw->m_pos[0] = parw->m_pos[0] + 110;
	thisw->m_pos[1] = parw->m_pos[1];
	thisw->m_pos[2] = parw->m_pos[2];
	thisw->m_pos[3] = parw->m_pos[1] + 30 - 10;

	CenterLabel(thisw);
}

//new host game name label
void Resize_NH_GNLab(Widget* thisw)
{
	NewHost* parw = (NewHost*)thisw->m_parent;

	thisw->m_pos[0] = parw->m_pos[0];
	thisw->m_pos[1] = parw->m_pos[1];
	thisw->m_pos[2] = parw->m_pos[0] + 110;
	thisw->m_pos[3] = parw->m_pos[1] + 30;

	CenterLabel(thisw);
}

void Click_NH_Create()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	NewHost* v = (NewHost*)gui->get("new host");

	//g_mode = APPMODE_PLAY;
	//Click_NewGame();

	g_netmode = NETM_HOST;

	OpenSock();

	g_sentsvinfo = false;

	CheckAddSv();

	gui->closeall();
	gui->open("lobby");
	Lobby* l = (Lobby*)gui->get("lobby");
	l->regen();
	
	//BegSess();
	ResetClients();
	AddClient(NULL, g_name, &g_localC);

	//contact matcher TO DO
}

NewHost::NewHost(Widget* parent, const char* n, void (*reframef)(Widget* thisw)) : WindowW(parent, n, reframef)
{
	m_parent = parent;
	m_type = WIDGET_NEWHOST;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;
	m_font = MAINFONT16;

#if 0
	m_svlistbg = Image(this, "gui/backg/svlistbg.jpg", false, NULL, 1, 1, 1,0.2f, 0, 0, 1, 1);
	m_selected = NULL;
	m_scroll[1] = 0;
	m_selsv = NULL;

	m_vscroll = VScroll(this, "m_vscroll");
	m_joinbut = Button(this, "join but", "gui/transp.png", RichText("Join"), RichText("Join the currently selected game"), m_font, BUST_LINEBASED, Resize_SL_JoinBut, Click_SL_Join, NULL, NULL, NULL, NULL, -1);

	SvInfo sv;
	sv.name = RichText(UString("asdjasld9a0f09230jf3"));
	sv.pingrt = RichText(UString("124"));
	sv.nplayersrt = RichText(UString("3/32"));
	sv.mapnamert = RichText(UString("wr_fluffy"));
	m_svlist.push_back(sv);
	sv.name = RichText(UString("Joe's"));
	sv.pingrt = RichText(UString("340"));
	sv.nplayersrt = RichText(UString("4/48"));
	sv.mapnamert = RichText(UString("bz_dunes"));
	m_svlist.push_back(sv);
	//m_selsv = &*m_svlist.rbegin();
	sv.name = RichText(UString("Bob's"));
	sv.pingrt = RichText(UString("15"));
	sv.nplayersrt = RichText(UString("2/12"));
	sv.mapnamert = RichText(UString("bz_hills"));
	m_svlist.push_back(sv);
#endif
	
	m_create = Button(this, "create", "gui/transp.png", RichText("Create"), RichText("Make the game publicly listed"), m_font, BUST_LINEBASED, Resize_NH_Create, Click_NH_Create, NULL, NULL, NULL, NULL, -1);
	m_gamename = EditBox(this, "game name", RichText("Game Room"), MAINFONT16, Resize_NH_GameName, false, SVNAME_LEN, NULL, NULL, -1);
	m_gnlab = Text(this, "gnlab", RichText("Game Name:"), MAINFONT16, Resize_NH_GNLab);

	if(reframefunc)
		reframefunc(this);

	reframe();
}

void NewHost::subinev(InEv* ie)
{
	//m_vscroll.inev(ie);

	Player* py = &g_player[g_localP];

	m_create.inev(ie);
	m_gamename.inev(ie);
	m_gnlab.inev(ie);
}

void NewHost::regen()
{
	RichText bname;
	int* price;

	Player* py = &g_player[g_localP];

#if 0
	if(b->type == BL_FACTORY)
	{
		char rowname[32];
		sprintf(rowname, "%d %d", col, 0);
		RichText label;

		Resource* r = &g_resource[RES_DOLLARS];
#if 0
		Button(Widget* parent, const char* name, const char* filepath, const RichText label, const RichText tooltip,int f, int style, void (*reframef)(Widget* thisw), void (*click)(), void (*click2)(int p), void (*overf)(), void (*overf2)(int p), void (*out)(), int parm);
#endif
		m_subwidg.push_back(new Button(this, rowname, "gui/transp.png", RichText(UString("Order Truck for ")) + RichText(RichPart(RICHTEXT_ICON, r->icon)) + RichText(UString(":")), RichText(), m_font, BUST_LINEBASED, Resize_BV_Cl, NULL, NULL, NULL, NULL, NULL, UNIT_TRUCK));
	
		sprintf(rowname, "%d %d", col, 1);
		char clabel[64];
		sprintf(clabel, "%d", b->manufprc[UNIT_TRUCK]);

		if(owned)
			m_subwidg.push_back(new EditBox(this, rowname, RichText(UString(clabel)), m_font, Resize_BV_Cl, false, 6, Change_BV_MP, NULL, UNIT_TRUCK));
		else
			m_subwidg.push_back(new Text(this, rowname, RichText(UString(clabel)), m_font, Resize_BV_Cl));

		col++;
	}
#endif

	freech();

	reframe();
}

void NewHost::subreframe()
{
	Font* f = &g_font[m_font];

	m_create.reframe();
	m_gamename.reframe();
	m_gnlab.reframe();
}

void NewHost::subdraw()
{
	m_create.draw();
	m_gamename.draw();
	m_gnlab.draw();
}

void NewHost::subdrawover()
{
	m_create.drawover();
	m_gamename.drawover();
	m_gnlab.drawover();
}