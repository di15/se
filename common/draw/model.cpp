
#include "../platform.h"
#include "../math/3dmath.h"
#include "model.h"
#include "../texture.h"
#include "../utils.h"
#include "../gui/gui.h"
#include "shader.h"
#include "../debug.h"
#include "vertexarray.h"

Model g_model[MODELS];
vector<ModelToLoad> g_modelsToLoad;

Model::~Model()
{
	destroy();
}

void Model::destroy()
{
	if(m_va == NULL)
		return;

	delete [] m_va;
	m_va = NULL;

	m_ms3d.destroy();

	m_on = false;
}

int NewModel()
{
	for(int i=0; i<MODELS; i++)
		if(!g_model[i].m_on)
			return i;

	return -1;
}

void QueueModel(int* id, const char* relative, Vec3f scale, Vec3f translate)
{
	ModelToLoad toLoad;
	toLoad.id = id;
	strcpy(toLoad.filepath, relative);
	toLoad.scale = scale;
	toLoad.translate = translate;

	g_modelsToLoad.push_back(toLoad);
}

bool Load1Model()
{
	static int last = -1;

	if(last+1 < g_modelsToLoad.size())
		Status(g_modelsToLoad[last+1].filepath);

	if(last >= 0)
	{
		int id = NewModel();
		g_model[id].load(g_modelsToLoad[last].filepath, g_modelsToLoad[last].scale, g_modelsToLoad[last].translate, false);
		(*g_modelsToLoad[last].id) = id;
	}

	last ++;

	if(last >= g_modelsToLoad.size())
	{
		g_modelsToLoad.clear();
		return false;	// Done loading all models
	}

	return true;	// Not finished loading models
}

void DrawVA(VertexArray* va, Vec3f pos)
{
	Shader* s = &g_shader[g_curS];

	Matrix modelmat;
    modelmat.setTranslation((const float*)&pos);
    glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

    //glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
    //if(s->m_slot[SSLOT_TEXCOORD0] != -1)	glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
    //if(s->m_slot[SSLOT_NORMAL] != -1)	glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
    glVertexPointer(3, GL_FLOAT, 0, va->vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
    glNormalPointer(GL_FLOAT, 0, va->normals);

	//if(s->m_slot[SSLOT_TEXCOORD0] == -1) g_log<<"s->m_slot[SSLOT_TEXCOORD0] = -1"<<endl;
	//if(s->m_slot[SSLOT_NORMAL] == -1) g_log<<"s->m_slot[SSLOT_NORMAL] = -1"<<endl;
	//g_log.flush();

#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	glDrawArrays(GL_TRIANGLES, 0, va->numverts);
}

void DrawVADepth(VertexArray* va, Vec3f pos)
{
	Shader* s = &g_shader[g_curS];

	Matrix modelmat;
    modelmat.setTranslation((const float*)&pos);
    glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

    //glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
    //glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
    glVertexPointer(3, GL_FLOAT, 0, va->vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
    //glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	glDrawArrays(GL_TRIANGLES, 0, va->numverts);
}

void Model::usedifftex()
{
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ m_diffusem ].texname);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_TEXTURE0], 0);
}


void Model::usespectex()
{
	glActiveTextureARB(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_texture[ m_specularm ].texname);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_SPECULARMAP], 1);
}

void Model::usenormtex()
{
	glActiveTextureARB(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_texture[ m_normalm ].texname);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_NORMALMAP], 2);
}

void Model::useteamtex()
{
	glActiveTextureARB(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, g_texture[ m_ownerm ].texname);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_OWNERMAP], 3);
}

void Model::draw(int frame, Vec3f pos, float yaw)
{
	Shader* s = &g_shader[g_curS];

	float pitch = 0;
	Matrix modelmat;
    float radians[] = {DEGTORAD(pitch), DEGTORAD(yaw), 0};
    modelmat.setTranslation((const float*)&pos);
    Matrix rotation;
    rotation.setRotationRadians(radians);
    modelmat.postMultiply(rotation);
    glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

	VertexArray* va = &m_va[frame];

	usedifftex();
	usespectex();
	usenormtex();
	useteamtex();

    //glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
    //glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
    //glVertexAttribPointer(s->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
    glVertexPointer(3, GL_FLOAT, 0, va->vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
    glNormalPointer(GL_FLOAT, 0, va->normals);

#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	glDrawArrays(GL_TRIANGLES, 0, va->numverts);
}

void Model::drawdepth(int frame, Vec3f pos, float yaw)
{
	Shader* s = &g_shader[g_curS];

	float pitch = 0;
	Matrix modelmat;
    float radians[] = {DEGTORAD(pitch), DEGTORAD(yaw), 0};
    modelmat.setTranslation((const float*)&pos);
    Matrix rotation;
    rotation.setRotationRadians(radians);
    modelmat.postMultiply(rotation);
    glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

	VertexArray* va = &m_va[frame];

	usedifftex();

    //glVertexAttribPointer(s->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
    //glVertexAttribPointer(s->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
    glVertexPointer(3, GL_FLOAT, 0, va->vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);

#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	glDrawArrays(GL_TRIANGLES, 0, va->numverts);
}

int FindModel(const char* relative)
{
	char full[MAX_PATH+1];
	FullPath(relative, full);
	char corrected[MAX_PATH+1];
	strcpy(corrected, full);
	CorrectSlashes(corrected);

	for(int i=0; i<MODELS; i++)
	{
		Model* m = &g_model[i];

		if(!m->m_on)
			continue;

		if(stricmp(m->m_fullpath.c_str(), corrected) == 0)
			return i;
	}

	return -1;
}


int LoadModel(const char* relative, Vec3f scale, Vec3f translate, bool dontqueue)
{
	int i = FindModel(relative);

	if(i >= 0)
		return i;

	i = NewModel();

	if(i < 0)
		return i;

	if(g_model[i].load(relative, scale, translate, dontqueue))
		return i;

	return -1;
}

bool Model::load(const char* relative, Vec3f scale, Vec3f translate, bool dontqueue)
{
	m_diffusem = 0;
	m_specularm = 0;
	m_normalm = 0;
	m_ownerm = 0;

	bool result = m_ms3d.load(relative, m_diffusem, m_specularm, m_normalm, m_ownerm, dontqueue);

	if(result)
	{
		m_on = true;
		m_ms3d.genva(&m_va, scale, translate, relative);
		char full[MAX_PATH+1];
		FullPath(relative, full);
		char corrected[MAX_PATH+1];
		strcpy(corrected, full);
		CorrectSlashes(corrected);
		m_fullpath = corrected;
	}

	/*
	if(result)
	{
		//CreateTexture(spectex, specfile);
		//QueueTexture(&spectex, specfile, true);
		CorrectNormals();
	}*/

	return result;
}

bool PlayAnimation(float& frame, int first, int last, bool loop, float rate)
{
    if(frame < first || frame > last+1)
    {
        frame = first;
        return false;
    }

    frame += rate;

    if(frame > last)
    {
        if(loop)
            frame = first;
		else
			frame = last;

        return true;
    }

    return false;
}

//Play animation backwards
bool PlayAnimationB(float& frame, int first, int last, bool loop, float rate)
{
    if(frame < first-1 || frame > last)
    {
        frame = last;
        return false;
    }

    frame -= rate;

    if(frame < first)
    {
        if(loop)
            frame = last;
		else
			frame = first;

        return true;
    }

    return false;
}

void FreeModels()
{
	for(int i=0; i<MODELS; i++)
	{
		Model* m = &g_model[i];

		if(!m->m_on)
			continue;

		m->destroy();
	}
}
