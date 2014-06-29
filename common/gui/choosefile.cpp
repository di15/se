

#include "choosefile.h"
#include "font.h"

ChooseFile::ChooseFile() : Widget()
{
	m_parent = NULL;
	m_type = WIDGET_CHOOSEFILE;
	m_name = "";
	reframefunc = NULL;
	m_title = "Choose file";
	CreateTexture(m_bgtex, "gui/frame.jpg", true, false);
}

ChooseFile::ChooseFile(Widget* parent, const char* name, void (*reframef)(Widget* thisw), void (*callback)(const char* fullpath)) : Widget()
{
	m_parent = parent;
	m_type = WIDGET_CHOOSEFILE;
	m_name = name;
	reframefunc = reframef;
	m_title = "Choose file";
	CreateTexture(m_bgtex, "gui/frame.jpg", true, false);
	this->callback = callback;
	reframe();
}

void ChooseFile::reframe()	//resized or moved
{
	if(reframefunc)
		reframefunc(this);

	for(auto i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->reframe();

#if 0
    ListBox m_listbox;
    EditBox m_filename;
    Button m_choosebutton;
    Button m_cancelbutton;
#endif

	m_listbox.m_pos[0] = m_pos[0] + 15;
	m_listbox.m_pos[1] = m_pos[1] + 20;
	m_listbox.m_pos[2] = m_pos[2] - 15;
	m_listbox.m_pos[3] = m_pos[3] - 100;
}

//http://www.linuxquestions.org/questions/programming-9/c-list-files-in-directory-379323/

void ChooseFile::chdir(const char* fullpath)
{
	while(m_listbox.m_options.size() > 0)
		m_listbox.erase(0);


}

void ChooseFile::draw()
{
	DrawImage(g_texture[m_bgtex].texname, m_pos[0], m_pos[1], m_pos[2], m_pos[3]);

	DrawShadowedText(MAINFONT8, m_pos[0]+8, m_pos[1]+8, m_title.c_str());

	m_listbox.draw();
}
