
#ifndef CHOOSEFILE_H
#define CHOOSEFILE_H

#include "widget.h"

class ChooseFile : public Widget
{
public:
    Text m_title;
    ListBox m_listbox;
    EditBox m_filename;
    Button m_choosebutton;
    void (*callback)(const char* fullpath);

    ChooseFile();
    ChooseFile(Widget* parent, const char* name, void (*reframef)(Widget* thisw), void (*callback)(const char* fullpath));
};

#endif
