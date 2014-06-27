

#include "modelholder.h"
#include "../draw/vertexarray.h"
#include "../math/brush.h"
#include "../math/quaternion.h"
#include "../math/vec4f.h"
#include "compilemap.h"
#include "../platform.h"
#include "../utils.h"

list<ModelHolder> g_modelholder;

ModelHolder::ModelHolder()
{
	nframes = 0;
	frames = NULL;
	model = -1;
}

void VAsMinMax(VertexArray** frames, int nframes, Vec3f* pvmin, Vec3f* pvmax)
{
	Vec3f vmin(0,0,0);
	Vec3f vmax(0,0,0);

	for(int i=0; i<nframes; i++)
	{
		VertexArray* frame = &(*frames)[i];

		for(int vertidx = 0; vertidx < frame->numverts; vertidx++)
		{
			Vec3f v = frame->vertices[vertidx];

			if(v.x < vmin.x)
				vmin.x = v.x;
			if(v.y < vmin.y)
				vmin.y = v.y;
			if(v.z < vmin.z)
				vmin.z = v.z;
			if(v.x > vmax.x)
				vmax.x = v.x;
			if(v.y > vmax.y)
				vmax.y = v.y;
			if(v.z > vmax.z)
				vmax.z = v.z;
		}
	}

	*pvmin = vmin;
	*pvmax = vmax;
}

ModelHolder::ModelHolder(int model, Vec3f pos)
{
	nframes = 0;
	frames = NULL;

	this->model = model;
	translation = pos;
	rotdegrees = Vec3f(0,0,0);
	scale = Vec3f(1,1,1);

	retransform();
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
	rotationmat = original.rotationmat;
	CopyVAs(&frames, &nframes, &original.frames, original.nframes);

	return *this;
}

void ModelHolder::retransform()
{
	destroy();

	Model* m = &g_model[model];
	
	CopyVAs(&frames, &nframes, &m->m_va, m->m_ms3d.m_totalFrames);

	//Quaternion rotquat;
	Vec3f rotrads;
	rotrads.x = DEGTORAD(rotdegrees.x);
	rotrads.y = DEGTORAD(rotdegrees.y);
	rotrads.z = DEGTORAD(rotdegrees.z);
	//rotquat.fromAngles((float*)&rotrads);
	rotationmat.loadIdentity();
	rotationmat.setRotationRadians((float*)&rotrads);

	for(int frameidx = 0; frameidx < nframes; frameidx++)
	{
		VertexArray* pframe = &frames[frameidx];
		Vec3f normal;

		for(int vertidx = 0; vertidx < pframe->numverts; vertidx++)
		{
			pframe->vertices[vertidx].transform(rotationmat);
			pframe->vertices[vertidx] = pframe->vertices[vertidx] * scale;
		}
	}

	regennormals();

	VAsMinMax(&frames, nframes, &absmin, &absmax);
	absmin = absmin + translation;
	absmax = absmax + translation;
}

void ModelHolder::regennormals()
{
	Model* m = &g_model[model];
	MS3DModel* ms3d = &m->m_ms3d;

	vector<Vec3f>* normalweights;

	normalweights = new vector<Vec3f>[ms3d->m_numVertices];
	
	for(int f = 0; f < nframes; f++)
	{
		for(int index = 0; index < ms3d->m_numVertices; index++)
		{
			normalweights[index].clear();
		}
		
		Vec3f* vertices = frames[f].vertices;
		//Vec2f* texcoords = frames[f].texcoords;
		Vec3f* normals = frames[f].normals;
  
		int vert = 0;

		for(int i = 0; i < ms3d->m_numMeshes; i++)
		{
			for(int j = 0; j < ms3d->m_pMeshes[i].m_numTriangles; j++)
			{
				int triangleIndex = ms3d->m_pMeshes[i].m_pTriangleIndices[j];
				const MS3DModel::Triangle* pTri = &ms3d->m_pTriangles[triangleIndex];

				Vec3f normal;
				Vec3f tri[3];
				tri[0] = vertices[vert+0];
				tri[1] = vertices[vert+1];
				tri[2] = vertices[vert+2];
				//normal = Normal2(tri);
				normal = Normal(tri);	//Reverse order
				//normals[i] = normal;
				//normals[i+1] = normal;
				//normals[i+2] = normal;

				for(int k = 0; k < 3; k++)
				{
					int index = pTri->m_vertexIndices[k];
					normalweights[index].push_back(normal);
					
					// Reverse vertex order
					//0=>2=>1

					if(vert % 3 == 0)
						vert += 2;
					else if(vert % 3 == 2)
						vert --;
					else if(vert % 3 == 1)
						vert += 2;
				}
			}
		}

		vert = 0;

		for(int i = 0; i < ms3d->m_numMeshes; i++)
		{
			for(int j = 0; j < ms3d->m_pMeshes[i].m_numTriangles; j++)
			{
				int triangleIndex = ms3d->m_pMeshes[i].m_pTriangleIndices[j];
				const MS3DModel::Triangle* pTri = &ms3d->m_pTriangles[triangleIndex];

				for(int k = 0; k < 3; k++)
				{
					int index = pTri->m_vertexIndices[k];

					Vec3f weighsum(0, 0, 0);

					for(int l=0; l<normalweights[index].size(); l++)
					{
						weighsum = weighsum + normalweights[index][l] / (float)normalweights[index].size();
					}

					normals[vert] = weighsum;
					
					// Reverse vertex order
					//0=>2=>1

					if(vert % 3 == 0)
						vert += 2;
					else if(vert % 3 == 2)
						vert --;
					else if(vert % 3 == 1)
						vert += 2;
				}
			}
		}
	}

	delete [] normalweights;
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

Vec3f ModelHolder::traceray(Vec3f line[])
{
	Vec3f planenorms[6];
	float planedists[6];
	MakeHull(planenorms, planedists, Vec3f(0,0,0), absmin, absmax);

#if 0
	for(int i=0; i<6; i++)
	{
		g_log<<"mh pl ("<<planenorms[i].x<<","<<planenorms[i].y<<","<<planenorms[i].z<<"),"<<planedists[i]<<endl;
	}
#endif

	if(LineInterHull(line, planenorms, planedists, 6))
	{
		return (absmin+absmax)/2.0f;
	}

	return line[1];
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
		
#ifdef DEBUG
		CheckGLError(__FILE__, __LINE__);
#endif
		m->usedifftex();
#ifdef DEBUG
		CheckGLError(__FILE__, __LINE__);
#endif
		m->usespectex();
#ifdef DEBUG
		CheckGLError(__FILE__, __LINE__);
#endif
		m->usenormtex();
#ifdef DEBUG
		CheckGLError(__FILE__, __LINE__);
#endif
		m->useteamtex();
#ifdef DEBUG
		CheckGLError(__FILE__, __LINE__);
#endif
		//DrawVA(&m->m_va[rand()%10], h->translation);
		DrawVA(&h->frames[ g_renderframe % m->m_ms3d.m_totalFrames ], h->translation);
#ifdef DEBUG
		CheckGLError(__FILE__, __LINE__);
#endif
	}
}

void DrawModelHoldersDepth()
{
	for(auto iter = g_modelholder.begin(); iter != g_modelholder.end(); iter++)
	{
		ModelHolder* h = &*iter;
		Model* m = &g_model[h->model];

		m->usedifftex();
		//DrawVA(&m->m_va[rand()%10], h->translation);
		DrawVADepth(&h->frames[ g_renderframe % m->m_ms3d.m_totalFrames ], h->translation);
	}
}