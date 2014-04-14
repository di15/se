

#ifndef RENDERSPRITE_H
#define RENDERSPRITE_H

#include "../platform.h"

extern char g_renderpath[MAX_PATH+1];
extern char g_renderbasename[MAX_PATH+1];

void PrepareRender(const char* fullpath);
void UpdateRender();

#endif