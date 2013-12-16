

#include "modelholder.h"
#include "../draw/vertexarray.h"

list<ModelHolder> g_modelholder;

ModelHolder::ModelHolder()
{
	nframes = 0;
	frames = NULL;
	model = -1;
}

ModelHolder::ModelHolder(int model, Vec3f pos)
{
	nframes = 0;
	frames = NULL;

	this->model = model;
	translation = pos;

	Model* m = &g_model[model];
	
	CopyVAs(&frames, &nframes, &m->m_va, m->m_ms3d.m_totalFrames);
}

ModelHolder::~ModelHolder()
{
	destroy();
}


ModelHolder::ModelHolder(const ModelHolder& original)
{
	nframes = 0;
	frames = NULL;
	*this = original;
}

ModelHolder& ModelHolder::operator=(const ModelHolder &original)
{
#if 0
	int model;
	Vec3f rotdegrees;
	Vec3f translation;
	Vec3f scale;
	Vec3f absmin;
	Vec3f absmax;
	Matrix transform;
	VertexArray* frames;
	int nframes;
#endif

	destroy();
	model = original.model;
	rotdegrees = original.rotdegrees;
	translation = original.translation;
	scale = original.scale;
	absmin = original.absmin;
	absmax = original.absmax;
	transform = original.transform;
	CopyVAs(&frames, &nframes, &original.frames, original.nframes);

	return *this;
}

void ModelHolder::retransform()
{
}

void ModelHolder::regenva()
{
}

void ModelHolder::destroy()
{
	nframes = 0;

	if(frames)
	{
		delete [] frames;
		frames = NULL;
	}
}

void FreeModelHolders()
{
	g_modelholder.clear();
}

void DrawModelHolders()
{
	for(auto iter = g_modelholder.begin(); iter != g_modelholder.end(); iter++)
	{
		ModelHolder* h = &*iter;
		Model* m = &g_model[h->model];

		m->usetex();
		DrawVA(&h->frames[0], Vec3f(0,0,0));
	}
}