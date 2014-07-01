

#ifndef RENDERSPRITE_H
#define RENDERSPRITE_H

#include "../platform.h"
#include "../math/vec2i.h"

extern char g_renderpath[MAX_PATH+1];
extern char g_renderbasename[MAX_PATH+1];
extern int g_rendertype;
extern int g_renderframes;
extern float g_transpkey[3];

extern int g_deswidth;
extern int g_desheight;
extern unsigned int g_rendertex;
extern unsigned int g_renderrb;
extern unsigned int g_renderfb;

#define RENDER_BUILDING		0
#define RENDER_UNIT			1

#define RENDSTAGE_COLOR		0
#define RENDSTAGE_TEAM		1

void AdjustFrame();
void PrepareRender(const char* fullpath, int rendtype);
void UpdateRender();
void EndRender();
void AllScreenMinMax(Vec2i *vmin, Vec2i *vmax, int width, int height);
void SaveRender(int rendstage);

#endif
