

#ifndef HSCROLLER_H
#define HSCROLLER_H

#include "widget.h"

class HScroller : public Widget
{
public:
	HScroller(Widget* parent, void (*reframef)(Widget* thisw), void (*click2)(int p), void (*out)());

	void draw();
	bool lbuttonup(bool moved);
	bool lbuttondown();
	bool mousemove();
	bool checkover();
	void movesub();
	void frameupd();
};

#endif