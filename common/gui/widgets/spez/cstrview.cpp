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
#include "../../../sim/player.h"
#include "blpreview.h"
#include "../../icon.h"

//resize construction view column item
void Resize_CV_Cl(Widget* thisw)
{
	Widget* parw = thisw->m_parent;
	int row;
	int col;
	sscanf(thisw->m_name.c_str(), "%d %d", &row, &col);
	
	thisw->m_pos[0] = parw->m_pos[0] + 150*col;
	thisw->m_pos[1] = parw->m_pos[1] + 20*row + 90;
	thisw->m_pos[2] = imin(parw->m_pos[0] + 150*(col+1), parw->m_pos[2]);
	thisw->m_pos[3] = imin(parw->m_pos[1] + 20*(row+1) + 90, parw->m_pos[3]);
}

//resize construction view's requisite list
void Resize_CV_RL(Widget* thisw)
{
	Widget* parw = thisw->m_parent;
	int row = 1;
	int col = 0; 
	
	thisw->m_pos[0] = parw->m_pos[0];
	thisw->m_pos[1] = parw->m_pos[1] + 20*row + 90;
	thisw->m_pos[2] = parw->m_pos[2];
	thisw->m_pos[3] = parw->m_pos[3];
}

// change construction view construction wage
void Change_CV_CW(unsigned int key, unsigned int scancode, bool down, int parm)
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	CstrView* cv = (CstrView*)gui->get("cstr view");
	EditBox* cw = (EditBox*)cv->get("0 1");
	std::string sval = cw->m_value.rawstr();
	int ival;
	sscanf(sval.c_str(), "%d", &ival);

	Selection* sel = &g_sel;

	//TO DO: this change needs to happen at the next net turn (next 200 netframe interval).
	//Also needs to be sent to server and only executed when received back.
	if(sel->buildings.size() > 0)
	{
		int bi = *sel->buildings.begin();
		Building* b = &g_building[bi];
		BlType* t = &g_bltype[b->type];
		b->conwage = ival;
	}
#if 1
	else if(sel->roads.size() > 0)
	{
		g_bptype = BL_ROAD;
		CdType* cot = &g_cdtype[CONDUIT_ROAD];
		for(auto coi=sel->roads.begin(); coi!=sel->roads.end(); coi++)
		{
			Vec2i xz = *coi;
			CdTile* co = GetCd(CONDUIT_ROAD, xz.x, xz.y, false);
			co->conwage = ival;
		}
	}
	else if(sel->powls.size() > 0)
	{
		g_bptype = BL_POWL;
		CdType* cot = &g_cdtype[CONDUIT_POWL];
		for(auto coi=sel->powls.begin(); coi!=sel->powls.end(); coi++)
		{
			Vec2i xz = *coi;
			CdTile* co = GetCd(CONDUIT_POWL, xz.x, xz.y, false);
			co->conwage = ival;
		}
	}
	else if(sel->crpipes.size() > 0)
	{
		g_bptype = BL_CRPIPE;
		CdType* cot = &g_cdtype[CONDUIT_CRPIPE];
		for(auto coi=sel->crpipes.begin(); coi!=sel->crpipes.end(); coi++)
		{
			Vec2i xz = *coi;
			CdTile* co = GetCd(CONDUIT_CRPIPE, xz.x, xz.y, false);
			co->conwage = ival;
		}
	}
#endif
}

CstrView::CstrView(Widget* parent, const char* n, void (*reframef)(Widget* thisw), void (*movefunc)(), void (*cancelfunc)(), void (*proceedfunc)(), void (*estimatefunc)()) : WindowW(parent, n, reframef)
{
	m_parent = parent;
	m_type = WIDGET_CONSTRUCTIONVIEW;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;
	this->movefunc = movefunc;
	this->cancelfunc = cancelfunc;
	this->proceedfunc = proceedfunc;
	this->estimatefunc = estimatefunc;

	if(reframefunc)
		reframefunc(this);

	reframe();
}

void CstrView::regen(Selection* sel)
{
	int* conmat = NULL;
	int qty = -1;
	RichText bname;
	int conwage;
	CdType* cot;

	Player* py = &g_player[g_localP];
	Player* opy;	//owner player
	
	//numerator,denominator for amount of requisite met
	int num[RESOURCES];
	int denom[RESOURCES];
	Zero(num);
	Zero(denom);

	if(sel->buildings.size() > 0)
	{
		int bi = *sel->buildings.begin();
		Building* b = &g_building[bi];
		BlType* t = &g_bltype[b->type];

		conmat = t->conmat;
		g_bptype = b->type;
		bname = RichText(UString(t->name));
		conwage = b->conwage;
		opy = &g_player[b->owner];

		for(int ri=0; ri<RESOURCES; ri++)
		{
			Resource* r = &g_resource[ri];

			if(t->conmat[ri] <= 0)
				continue;

			denom[ri] += t->conmat[ri];
			num[ri] += b->conmat[ri];
		}
	}
#if 1
	else if(sel->roads.size() > 0)
	{
		g_bptype = BL_ROAD;
		cot = &g_cdtype[CONDUIT_ROAD];
		conmat = cot->conmat;
		qty = sel->roads.size();
		Vec2i xz = *sel->roads.begin();
		CdTile* co = GetCd(CONDUIT_ROAD, xz.x, xz.y, false);
		conwage = co->conwage;
		opy = &g_player[co->owner];
		bname = RichText(UString(cot->name));

		for(auto cit=sel->roads.begin(); cit!=sel->roads.end(); cit++)
		{
			CdTile* ctile = GetCd(CONDUIT_ROAD, cit->x, cit->y, false);

			for(int ri=0; ri<RESOURCES; ri++)
			{
				Resource* r = &g_resource[ri];

				if(cot->conmat[ri] <= 0)
					continue;
				
				denom[ri] += cot->conmat[ri];
				num[ri] += ctile->conmat[ri];
			}
		}
	}
	else if(sel->powls.size() > 0)
	{
		g_bptype = BL_POWL;
		cot = &g_cdtype[CONDUIT_POWL];
		conmat = cot->conmat;
		qty = sel->powls.size();
		Vec2i xz = *sel->powls.begin();
		CdTile* co = GetCd(CONDUIT_POWL, xz.x, xz.y, false);
		conwage = co->conwage;
		opy = &g_player[co->owner];
		bname = RichText(UString(cot->name));

		for(auto cit=sel->roads.begin(); cit!=sel->roads.end(); cit++)
		{
			CdTile* ctile = GetCd(CONDUIT_POWL, cit->x, cit->y, false);

			for(int ri=0; ri<RESOURCES; ri++)
			{
				Resource* r = &g_resource[ri];

				if(cot->conmat[ri] <= 0)
					continue;
				
				denom[ri] += cot->conmat[ri];
				num[ri] += ctile->conmat[ri];
			}
		}
	}
	else if(sel->crpipes.size() > 0)
	{
		g_bptype = BL_CRPIPE;
		qty = sel->crpipes.size();
		cot = &g_cdtype[CONDUIT_CRPIPE];
		conmat = cot->conmat;
		Vec2i xz = *sel->crpipes.begin();
		CdTile* co = GetCd(CONDUIT_CRPIPE, xz.x, xz.y, false);
		conwage = co->conwage;
		opy = &g_player[co->owner];
		bname = RichText(UString(cot->name));

		for(auto cit=sel->roads.begin(); cit!=sel->roads.end(); cit++)
		{
			CdTile* ctile = GetCd(CONDUIT_CRPIPE, cit->x, cit->y, false);

			for(int ri=0; ri<RESOURCES; ri++)
			{
				Resource* r = &g_resource[ri];

				if(cot->conmat[ri] <= 0)
					continue;

				denom[ri] += cot->conmat[ri];
				num[ri] += ctile->conmat[ri];
			}
		}
	}
#endif

	freech();

	m_subwidg.push_back(new Viewport(this, "viewport", Resize_BP_VP, &DrawViewport, NULL, NULL, NULL, NULL, NULL, NULL, VIEWPORT_ENTVIEW));
	m_subwidg.push_back(new Text(this, "owner", opy->name, FONT_EUROSTILE16, Resize_BP_Ow, true, opy->color[0], opy->color[1], opy->color[2], opy->color[3]));
	m_subwidg.push_back(new Text(this, "title", bname, MAINFONT32, Resize_BP_Tl, true, 0.9f, 0.7f, 0.3f, 1));

	int row = 0;

	char rowname[32];
	sprintf(rowname, "%d %d", row, 0);
	m_subwidg.push_back(new Text(this, rowname, RichText(UString("Construction Wage")) + RichText(RichPart(RICHTEXT_ICON, ICON_LABOUR)) + RichText(UString(":")), MAINFONT16, Resize_CV_Cl, true, 1.0f,1.0f,1.0f,1.0f /*, 0.7f, 0.9f, 0.3f, 1*/));

	sprintf(rowname, "%d %d", row, 1);
	char cwstr[32];
	sprintf(cwstr, "%d", conwage);
	m_subwidg.push_back(new EditBox(this, rowname, RichText(UString(cwstr)), MAINFONT16, Resize_CV_Cl, false, 6, Change_CV_CW, NULL, -1));

	//requisite list
	RichText rl;
	rl.m_part.push_back(RichPart("Construction requisites:\n"));

	for(int ri=0; ri<RESOURCES; ri++)
	{
		if(denom[ri] <= 0)
			continue;

		char numdenom[32];
		sprintf(numdenom, "%d/%d\n", num[ri], denom[ri]);

		Resource* r = &g_resource[ri];
		
		rl.m_part.push_back(RichPart("      "));
		rl.m_part.push_back(RichPart(r->name.c_str()));
		rl.m_part.push_back(RichPart(" "));
		rl.m_part.push_back(RichPart(RICHTEXT_ICON, r->icon));
		rl.m_part.push_back(RichPart(": "));
		rl.m_part.push_back(RichPart(numdenom));
	}

	m_subwidg.push_back(new TextBlock(this, "req list", rl, MAINFONT16, Resize_CV_RL, 1.0f,1.0f,1.0f,1.0f /*, 0.7f, 0.9f, 0.3f, 1*/));

	reframe();
}
