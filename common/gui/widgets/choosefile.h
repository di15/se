
#ifndef CHOOSEFILE_H
#define CHOOSEFILE_H

#include "../widget.h"
#include "text.h"
#include "listbox.h"
#include "button.h"
#include "editbox.h"

class ChooseFile : public Widget
{
public:
    string m_title;
    ListBox m_listbox;
    EditBox m_filename;
    Button m_choosebutton;
    Button m_cancelbutton;
    void (*callback)(const char* fullpath);

    ChooseFile();
    ChooseFile(Widget* parent, const char* name, void (*reframef)(Widget* thisw), void (*callback)(const char* fullpath));

    void chdir(const char* fullpath);
    void draw();
    void reframe();
};

#endif
