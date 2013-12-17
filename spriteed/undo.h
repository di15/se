

#include "../common/math/brush.h"
#include "../common/sim/map.h"
#include "../common/platform.h"
#include "../common/save/modelholder.h"

class UndoH	//undo history
{
public:
	list<Brush> brushes;
	list<ModelHolder> modelholders;

	UndoH(){}
	~UndoH();
};

#define MAX_UNDO		16

void LinkPrevUndo(UndoH* tosave=NULL);	//call this BEFORE the change is made
void LinkLatestUndo();	//called by undo itself
void WriteH(UndoH* writeto);
void Undo();
void Redo();
void ClearUndo();