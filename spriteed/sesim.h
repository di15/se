

#include "../common/sim/map.h"
#include "../common/save/edmap.h"
#include "../common/save/modelholder.h"
#include "../common/math/physics.h"

#define MERGEV_D		1.5f

class Brush;

extern Brush g_copyB;
extern ModelHolder g_copyM;

#define EDTOOL_NONE			-1
#define EDTOOL_CUT			0
#define EDTOOL_EXPLOSION	1

extern int g_edtool;

void DrawFilled(EdMap* map, list<ModelHolder>& modelholder);
void DrawOutlines(EdMap* map, list<ModelHolder>& modelholder);
void DrawSelOutlines(EdMap* map, list<ModelHolder>& modelholder);
void DrawDrag(EdMap* map, Matrix* mvp, int w, int h, bool persp);
bool SelectDrag(EdMap* map, Matrix* mvp, int w, int h, int x, int y, Vec3f eye, bool persp);
void SelectBrush(EdMap* map, Vec3f line[], Vec3f vmin, Vec3f vmax);
bool PruneB(EdMap* map, Brush* b);
bool PruneB2(Brush* b, Plane* p, float epsilon=-CLOSE_EPSILON*2);