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
#include "blpreview.h"
#include "../../../platform.h"
#include "../viewportw.h"
#include "../../../../game/gui/gviewport.h"

//viewport
void Resize_BP_VP(Widget* thisw)
{
	Widget* parw = thisw->m_parent;

	thisw->m_pos[0] = parw->m_pos[0];
	thisw->m_pos[1] = parw->m_pos[1];
	thisw->m_pos[2] = parw->m_pos[2];
	thisw->m_pos[3] = parw->m_pos[3];
}

//title
void Resize_BP_Tl(Widget* thisw)
{
	Widget* parw = thisw->m_parent;

	thisw->m_pos[0] = parw->m_pos[0] + 10;
	thisw->m_pos[1] = parw->m_pos[1] + 10;
	thisw->m_pos[2] = parw->m_pos[2] - 10;
	thisw->m_pos[3] = parw->m_pos[3] - 10;
}

//owner name
void Resize_BP_Ow(Widget* thisw)
{
	Widget* parw = thisw->m_parent;

	thisw->m_pos[0] = parw->m_pos[0] + 15;
	thisw->m_pos[1] = parw->m_pos[1] + 39;
	thisw->m_pos[2] = parw->m_pos[2] - 10;
	thisw->m_pos[3] = parw->m_pos[3] - 10;
}

//params text block: construction material
void Resize_BP_PB_C(Widget* thisw)
{
	Widget* parw = thisw->m_parent;

	thisw->m_pos[0] = parw->m_pos[0] + 10;
	thisw->m_pos[1] = parw->m_pos[1] + 32 + 20;
	thisw->m_pos[2] = parw->m_pos[2] - 10;
	thisw->m_pos[3] = parw->m_pos[1] + 32 + 10 + 100;
}

//inputs
void Resize_BP_PB_I(Widget* thisw)
{
	Widget* parw = thisw->m_parent;

	thisw->m_pos[0] = parw->m_pos[0] + 10;
	thisw->m_pos[1] = parw->m_pos[1] + 32 + 10 + 100;
	thisw->m_pos[2] = (parw->m_pos[0]+parw->m_pos[2])/2;
	thisw->m_pos[3] = parw->m_pos[1] + 32 + 10 + 200;
}

//outputs
void Resize_BP_PB_O(Widget* thisw)
{
	Widget* parw = thisw->m_parent;

	thisw->m_pos[0] = (parw->m_pos[0]+parw->m_pos[2])/2;
	thisw->m_pos[1] = parw->m_pos[1] + 32 + 10 + 100;
	thisw->m_pos[2] = parw->m_pos[2] - 10;
	thisw->m_pos[3] = parw->m_pos[1] + 32 + 10 + 200;
}

//description block
void Resize_BP_Ds(Widget* thisw)
{
	Widget* parw = thisw->m_parent;

	thisw->m_pos[0] = parw->m_pos[0] + 10;
	thisw->m_pos[1] = parw->m_pos[1] + 32 + 10 + 200;
	thisw->m_pos[2] = parw->m_pos[2] - 10;
	thisw->m_pos[3] = parw->m_pos[3] - 10;
}

BuildPreview::BuildPreview(Widget* parent, const char* n, void (*reframef)(Widget* thisw)) : WindowW(parent, n, reframef)
{
	m_parent = parent;
	m_type = WIDGET_BUILDPREVIEW;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;

	m_subwidg.push_back(new Viewport(this, "viewport", Resize_BP_VP, &DrawViewport, NULL, NULL, NULL, NULL, NULL, NULL, VIEWPORT_ENTVIEW));
	m_subwidg.push_back(new Text(this, "title", RichText("Buildings"), MAINFONT32, Resize_BP_Tl, true, 0.9f, 0.7f, 0.3f, 1));
	m_subwidg.push_back(new TextBlock(this, "conmat block", RichText("conmat block \n1 \n2 \n3"), MAINFONT16, Resize_BP_PB_C, 1, 0, 0, 1));
	m_subwidg.push_back(new TextBlock(this, "input block", RichText("input block \n1 \n2 \n3"), MAINFONT16, Resize_BP_PB_I, 0, 1, 0, 1));
	m_subwidg.push_back(new TextBlock(this, "output block", RichText("output block \n1 \n2 \n3"), MAINFONT16, Resize_BP_PB_O, 1, 0, 1, 1));
	m_subwidg.push_back(new TextBlock(this, "desc block", RichText("description block \n1 \n2 \n3"), MAINFONT16, Resize_BP_Ds, 0, 1, 1, 1));
	
	if(reframefunc)
		reframefunc(this);

	reframe();
}
