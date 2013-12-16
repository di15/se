

#include "../platform.h"
#include "edmap.h"
#include "../math/triangle.h"

class CutBrushSide
{
public:
	unsigned int m_diffusem;
	unsigned int m_specularm;
	unsigned int m_normalm;
	Plane m_tceq[2];
	list<Triangle> m_frag;
};

class CutBrush
{
public:
	list<CutBrushSide> m_side;
};

extern float g_renderpitch;
extern float g_renderyaw;
extern int g_1tilewidth;

void ToCutSide(CutBrushSide* cuts, BrushSide* eds);
void CompileMap(const char* full, EdMap* map);
void ResetView();