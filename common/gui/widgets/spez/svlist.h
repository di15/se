#ifndef SVLISTVIEW_H
#define SVLISTVIEW_H

#include "../../../platform.h"
#include "../button.h"
#include "../image.h"
#include "../text.h"
#include "../editbox.h"
#include "../touchlistener.h"
#include "../../widget.h"
#include "../viewportw.h"
#include "../../../sim/selection.h"
#include "../windoww.h"

#define SVLISTBOT	150 //sv list view bottom space for buttons and address+port fields

class SvList : public WindowW
{
public:
	SvList(Widget* parent, const char* n, void (*reframef)(Widget* thisw));

	class SvInfo
	{
	public:
		bool replied;
		IPaddress addr;
		RichText name;
		short nplayers;
		RichText nplayersrt;
		std::list<RichText> players;
		int ping;
		RichText pingrt;
		RichText mapnamert;

		SvInfo()
		{
			replied = false;
		}
	};

	//when persistance of internal widget states is important between calls to "regen()",
	//then children widgets must be stored as members, rather than items in the m_subwidg list,
	//because the list is destroyed and regenerated.
	//in this case we need to keep the state of the scroll bar. the other widgets like the join button
	//probably don't need to be member variables.
	Image m_svlistbg;
	std::list<SvInfo> m_svlist;
	SvInfo* m_selected;
	VScroll m_vscroll;
	SvInfo* m_selsv;	//selected sv
	float m_listbot;	//bottom y screen coord of list items
	float m_listtop;
	Button m_joinbut;

	Text m_addrlab;
	Text m_portlab;
	EditBox m_addrbox;
	EditBox m_portbox;
	Button m_addbut;
	Button m_clearbut;
	Button m_qrefbut;
	Button m_refbut;

	//void draw();
	//void drawover();
	//void reframe();
	//void inev(InEv* ie);
	//void frameupd();
	//void regen(Selection* sel);
	void regen();
	virtual void subdraw();
	virtual void subdrawover();
	virtual void subreframe();
	virtual void subinev(InEv* ie);
	virtual void frameupd();
};

extern std::list<SvList::SvInfo*> g_togetsv;
extern bool g_reqsvlist;
extern bool g_reqdnexthost;

#endif
