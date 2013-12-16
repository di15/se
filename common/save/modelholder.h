

#include "../draw/model.h"
#include "../platform.h"

class ModelHolder
{
public:
	int model;
	Vec3f rotdegrees;
	Vec3f translation;
	Vec3f scale;
	Vec3f absmin;
	Vec3f absmax;
	Matrix transform;
	VertexArray* frames;
	int nframes;

	ModelHolder();
	ModelHolder(int model, Vec3f pos);
	~ModelHolder();
	ModelHolder(const ModelHolder& original);
	ModelHolder& operator=(const ModelHolder &original);

	void retransform();
	void regenva();
	void destroy();
};

extern list<ModelHolder> g_modelholder;

void FreeModelHolders();
void DrawModelHolders();