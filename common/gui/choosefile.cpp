

#include "choosefile.h"

ChooseFile::ChooseFile()
{
	m_parent = parent;
	m_type = WIDGET_CHOOSEFILE;
	m_name = n;
	m_font = f;
	m_value = t;
	m_caret = strlen(t);
	m_opened = false;
	m_passw = pw;
	m_maxlen = maxl;
	reframefunc = reframef;
	m_scroll[0] = 0;
	m_highl[0] = 0;
	m_highl[1] = 0;
	CreateTexture(m_frametex, "gui/frame.jpg", true, false);
	m_param = parm;
	changefunc2 = change2;
	reframe();
}

ChooseFile::ChooseFile(Widget* parent, const char* name, void (*reframef)(Widget* thisw), void (*callback)(const char* fullpath))
{
}

//http://www.linuxquestions.org/questions/programming-9/c-list-files-in-directory-379323/

