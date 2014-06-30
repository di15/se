
#ifndef EDITBOX_H
#define EDITBOX_H

#include "../widget.h"

class EditBox : public Widget
{
public:
	int m_highl[2];	// highlighted (selected) text
	UString m_compos;	//composition for unicode text

	EditBox();
	EditBox(Widget* parent, const char* n, const std::string t, int f, void (*reframef)(Widget* thisw), bool pw, int maxl, void (*change2)(int p), int parm);

	void draw();
	std::string drawvalue();
	void frameupd();
	void inev(InEv* ev);
	void placestr(const std::string* str);
	void changevalue(const char* str);
	bool delnext();
	bool delprev();
	void copyval();
	void pasteval();
	void selectall();
	void close();
	void gainfocus();
	void losefocus();
};

#endif
