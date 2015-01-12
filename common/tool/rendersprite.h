

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
extern unsigned int g_rendertex[4];
extern unsigned int g_renderrb[4];
extern unsigned int g_renderfb[4];

#define RENDER_BUILDING		0
#define RENDER_UNIT			1
#define RENDER_TERRTILE		2
#define RENDER_ROAD			3

#define RENDSTAGE_COLOR		0
#define RENDSTAGE_TEAM		1

extern bool g_antialias;

//should be a power of two,
//otherwise there's some space-saving optimizations i could've made
//in SaveRender();... checking if downsample size fits in a smaller image.
//edit: MUST be a power of two. until i fix it.
#define ANTIALIAS_UPSCALE	4

//Arbitrary max texture size, needed for determining upscale vertex screen coordinates.
//If it's too small for the upscale sprite, the coordinates won't fit and the sprite won't be rendered at those points.
#define MAX_TEXTURE		4096


void AdjustFrame();
void PrepareRender(const char* fullpath, int rendtype);
void UpdateRender();
void EndRender();
void AllScreenMinMax(Vec2i *vmin, Vec2i *vmax, int width, int height);
void SaveRender(int rendstage);

#endif
