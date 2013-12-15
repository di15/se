

#include "../common/sim/map.h"
#include "../common/save/edmap.h"

#define MERGEV_D		1.5f

class Brush;

extern Brush g_copyB;

#define EDTOOL_NONE		-1
#define EDTOOL_CUT		0

extern int g_edtool;

void DrawFilled(EdMap* map);
void DrawOutlines(EdMap* map);
void DrawSelOutlines(EdMap* map);
void DrawDrag(EdMap* map, Matrix* mvp, int w, int h, bool persp);
bool SelectDrag(EdMap* map, Matrix* mvp, int w, int h, int x, int y, Vec3f eye, bool persp);
void SelectBrush(EdMap* map, Vec3f line[], Vec3f vmin, Vec3f vmax);
bool PruneB(EdMap* map, Brush* b);