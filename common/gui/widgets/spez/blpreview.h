#ifndef BUILDPREVIEW_H
#define BUILDPREVIEW_H

#include "../../widget.h"
#include "../viewportw.h"
#include "../windoww.h"

class BuildPreview : public WindowW
{
public:
	BuildPreview(Widget* parent, const char* n, void (*reframef)(Widget* thisw));
};


void Resize_BP_VP(Widget* thisw);
void Resize_BP_Tl(Widget* thisw);
void Resize_BP_Ow(Widget* thisw);

#endif
