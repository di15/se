

#ifndef LOBBY_H
#define LOBBY_H

#include "../../../platform.h"
#include "../button.h"
#include "../image.h"
#include "../text.h"
#include "../editbox.h"
#include "../touchlistener.h"
#include "../../widget.h"
#include "../viewportw.h"
#include "../../viewlayer.h"
#include "../../../sim/selection.h"
#include "../windoww.h"

class Lobby : public ViewLayer
{
public:
	Lobby(Widget* parent, const char* n);
	
	Image m_bg;
	Image m_pybg;
	float m_pylpos[4];	//player list pos
	VScroll m_pylsc;	//player list scroller
	float m_chpos[4];	//chat box pos
	TextBlock m_chls;	//chat lines
	float m_chsl;	//chat start line number (scrolled)
	VScroll m_chsc;	//chat lines scroll
	Image m_chbg;
	Text m_svname;	//server name/title

	//Text m_pyline[PLAYERS];

	int njcl();	//number of joined players (including host)
	void regen();
	void drawpyl();
	virtual void frameupd();
	virtual void subreframe();
	virtual void subinev(InEv* ie);
	virtual void subdraw();
	virtual void subdrawover();
};

#endif