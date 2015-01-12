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
#include "truckmgr.h"
#include "blpreview.h"
#include "blview.h"
#include "../../icon.h"
#include "../../../math/fixmath.h"

#if 0
//resize bl view column item
void Resize_BV_Cl(Widget* thisw)
{
	Widget* parw = thisw->m_parent;
	int row;
	int col;
	sscanf(thisw->m_name.c_str(), "%d %d", &row, &col);
	
	thisw->m_pos[0] = parw->m_pos[0] + 64*(imax(col-1,0));
	thisw->m_pos[1] = parw->m_pos[1] + 20*row + 90;
	thisw->m_pos[2] = imin(parw->m_pos[0] + 150 + 64*(col+1), parw->m_pos[2]);
	thisw->m_pos[3] = imin(parw->m_pos[1] + 20*(row+1) + 90, parw->m_pos[3]);

	//first column is extra large (label text)
	if(col > 0)
		thisw->m_pos[0] += 200;
}

// change bl view production level
void Change_BV_PL(unsigned int key, unsigned int scancode, bool down, int parm)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	TruckMgr* bv = (TruckMgr*)gui->get("bl view");
	Selection* sel = &g_sel;

	Building* b = &g_building[ *sel->buildings.begin() ];
	BlType* bt = &g_bltype[ b->type ];

	int row = 0;

	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(bt->output[ri] <= 0)
			continue;
		row++;
	}

	if(bt->input[RES_LABOUR] > 0)
		row++;

	char wn[32];
	sprintf(wn, "%d 1", row);
	EditBox* cw = (EditBox*)bv->get(wn);
	std::string sval = cw->m_value.rawstr();

	if(sval.length() <= 0)
		return;

	int ival;
	sscanf(sval.c_str(), "%d", &ival);

	//TO DO: this change needs to happen at the next net turn (next 200 netframe interval).
	//Also needs to be sent to server and only executed when received back.
	ival = imin(ival, RATIO_DENOM);
	ival = imax(ival, 0);
	b->prodlevel = ival;
}
#endif

// change (truck mgr's) global driver wage
void Change_TM_DW(unsigned int key, unsigned int scancode, bool down, int parm)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	TruckMgr* tm = (TruckMgr*)gui->get("truck mgr");
	Selection* sel = &g_sel;

	int row = 0;

	char wn[32];
	sprintf(wn, "%d 1", row);
	EditBox* dw = (EditBox*)tm->get(wn);
	std::string sval = dw->m_value.rawstr();

	if(sval.length() <= 0)
		return;

	int ival;
	sscanf(sval.c_str(), "%d", &ival);

	//TO DO: this change needs to happen at the next net turn (next 200 netframe interval).
	//Also needs to be sent to server and only executed when received back.
	//ival = imin(ival, RATIO_DENOM);
	ival = imax(ival, 0);
	py->truckwage = ival;
}

// change (truck mgr's) global transport cost
void Change_TM_TC(unsigned int key, unsigned int scancode, bool down, int parm)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	TruckMgr* tm = (TruckMgr*)gui->get("truck mgr");
	Selection* sel = &g_sel;

	int row = 0;

	row++;

	char wn[32];
	sprintf(wn, "%d 1", row);
	EditBox* dw = (EditBox*)tm->get(wn);
	std::string sval = dw->m_value.rawstr();

	if(sval.length() <= 0)
		return;

	int ival;
	sscanf(sval.c_str(), "%d", &ival);

	//TO DO: this change needs to happen at the next net turn (next 200 netframe interval).
	//Also needs to be sent to server and only executed when received back.
	//ival = imin(ival, RATIO_DENOM);
	ival = imax(ival, 0);
	py->transpcost = ival;
}

TruckMgr::TruckMgr(Widget* parent, const char* n, void (*reframef)(Widget* thisw)) : WindowW(parent, n, reframef)
{
	m_parent = parent;
	m_type = WIDGET_TRUCKMGR;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;

	if(reframefunc)
		reframefunc(this);

	reframe();
}

void TruckMgr::regen(Selection* sel)
{
	RichText uname;
	int wage;
	int cost;

	Player* py = &g_player[g_localP];
	UType* ut;
	Unit* u;
	bool owned = false;	//owned by current player?
	Player* opy;

	if(sel->units.size() > 0)
	{
		int ui = *sel->units.begin();
		u = &g_unit[ui];
		ut = &g_utype[u->type];

		if(u->owner == g_localP)
			owned = true;

		//g_bptype = b->type;
		opy = &g_player[u->owner];
		wage = opy->truckwage;
		cost = opy->transpcost;

		uname = RichText(UString(ut->name));
	}

	freech();

	//m_subwidg.push_back(new Viewport(this, "viewport", Resize_BP_VP, &DrawViewport, NULL, NULL, NULL, NULL, NULL, NULL, VIEWPORT_ENTVIEW));
	m_subwidg.push_back(new Text(this, "owner", opy->name, FONT_EUROSTILE16, Resize_BP_Ow, true, opy->color[0], opy->color[1], opy->color[2], opy->color[3]));
	m_subwidg.push_back(new Text(this, "title", RichText("Truck Manager"), MAINFONT32, Resize_BP_Tl, true, 0.9f, 0.7f, 0.3f, 1));

	int row = 0;
	
	{
		Resource* r = &g_resource[RES_DOLLARS];

		char rowname[32];
		sprintf(rowname, "%d %d", row, 0);
		RichText label;

		m_subwidg.push_back(new Text(this, rowname, RichText(UString("Driver wage ")) + RichText(RichPart(RICHTEXT_ICON, r->icon)) + RichText(UString("/second:")), MAINFONT16, Resize_BV_Cl, true, 1.0f,1.0f,1.0f,1.0f /*, 0.7f, 0.9f, 0.3f, 1*/));

		sprintf(rowname, "%d %d", row, 1);
		char cwstr[32];
		sprintf(cwstr, "%d", opy->truckwage);

		if(owned)
			m_subwidg.push_back(new EditBox(this, rowname, RichText(UString(cwstr)), MAINFONT16, Resize_BV_Cl, false, 6, Change_TM_DW, NULL, -1));
		else
			m_subwidg.push_back(new Text(this, rowname, RichText(UString(cwstr)), MAINFONT16, Resize_BV_Cl));

		row++;
	}

	{
		Resource* r = &g_resource[RES_DOLLARS];

		char rowname[32];
		sprintf(rowname, "%d %d", row, 0);
		RichText label;

		m_subwidg.push_back(new Text(this, rowname, RichText(UString("Transport cost ")) + RichText(RichPart(RICHTEXT_ICON, r->icon)) + RichText(UString("/second:")), MAINFONT16, Resize_BV_Cl, true, 1.0f,1.0f,1.0f,1.0f /*, 0.7f, 0.9f, 0.3f, 1*/));

		sprintf(rowname, "%d %d", row, 1);
		char cwstr[32];
		sprintf(cwstr, "%d", opy->transpcost);

		if(owned)
			m_subwidg.push_back(new EditBox(this, rowname, RichText(UString(cwstr)), MAINFONT16, Resize_BV_Cl, false, 6, Change_TM_TC, NULL, -1));
		else
			m_subwidg.push_back(new Text(this, rowname, RichText(UString(cwstr)), MAINFONT16, Resize_BV_Cl));

		row++;
	}

	//TO DO

	reframe();
}
