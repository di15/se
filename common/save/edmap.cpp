

#include "edmap.h"
#include "../draw/shader.h"
#include "../texture.h"
#include "../utils.h"
#include "../save/modelholder.h"

EdMap g_edmap;
vector<Brush*> g_selB;
Brush* g_sel1b = NULL;	//drag selected brush (brush being dragged or manipulated currently)
int g_dragV = -1;	//drag vertex of selected brush?
int g_dragS = -1;	//drag side of selected brush?
bool g_dragW = false;	//drag whole brush or model?
int g_dragD = -1;	// dragging DRAG_DOOR_POINT or DRAG_DOOR_AXIS ?
int g_dragM = -1;	//dragging model holder?
vector<ModelHolder*> g_selM;
ModelHolder* g_sel1m = NULL;	//drag selected model (model being dragged or manipulated currently)

void DrawEdMap(EdMap* map, bool showsky)
{
	//return;
	//glDisable(GL_CULL_FACE);
	//glEnable(GL_CULL_FACE);

	Shader* shader = &g_shader[g_curS];

	Matrix modelmat;
	modelmat.loadIdentity();
    glUniformMatrix4fv(shader->m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

	for(auto b=map->m_brush.begin(); b!=map->m_brush.end(); b++)
	{
		Texture* t = &g_texture[b->m_texture];

		if(t->sky && !showsky)
			continue;

		for(int i=0; i<b->m_nsides; i++)
		{
			BrushSide* side = &b->m_sides[i];
			/*
			CreateTexture(side->m_diffusem, "gui/frame.jpg", false);
			side->m_diffusem = g_texindex;*/
			side->usedifftex();
			side->usespectex();
			side->usenormtex();
			side->useteamtex();
			/*
			unsigned int atex;
			CreateTexture(atex, "gui/dmd.jpg", false);
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, atex);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_TEXTURE0], 0);
	*/
			VertexArray* va = &side->m_drawva;

			//glVertexAttribPointer(shader->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
			//glVertexAttribPointer(shader->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
			//glVertexAttribPointer(shader->m_slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, va->normals);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
            glVertexPointer(3, GL_FLOAT, 0, va->vertices);
            glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
            glNormalPointer(GL_FLOAT, 0, va->normals);
			//glVertexAttribPointer(shader->m_slot[SSLOT_TANGENT], 3, GL_FLOAT, GL_FALSE, 0, va->tangents);
			//glVertexAttribPointer(shader->m_slot[SSLOT_TANGENT], 3, GL_FLOAT, GL_FALSE, 0, va->normals);

			glDrawArrays(GL_TRIANGLES, 0, va->numverts);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
		}
	}
}

void DrawEdMapDepth(EdMap* map, bool showsky)
{
	//return;
	//glDisable(GL_CULL_FACE);
	//glEnable(GL_CULL_FACE);

	Shader* shader = &g_shader[g_curS];

	Matrix modelmat;
	modelmat.loadIdentity();
    glUniformMatrix4fv(shader->m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);

	for(auto b=map->m_brush.begin(); b!=map->m_brush.end(); b++)
	{
		Texture* t = &g_texture[b->m_texture];

		if(t->sky && !showsky)
			continue;

		for(int i=0; i<b->m_nsides; i++)
		{
			BrushSide* side = &b->m_sides[i];
			/*
			CreateTexture(side->m_diffusem, "gui/frame.jpg", false);
			side->m_diffusem = g_texindex;*/
			side->usedifftex();
			/*
			unsigned int atex;
			CreateTexture(atex, "gui/dmd.jpg", false);
	glActiveTextureARB(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, atex);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_TEXTURE0], 0);
	*/
			VertexArray* va = &side->m_drawva;

			//glVertexAttribPointer(shader->m_slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, va->vertices);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
			//glVertexAttribPointer(shader->m_slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, va->texcoords);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
            glVertexPointer(3, GL_FLOAT, 0, va->vertices);
            glTexCoordPointer(2, GL_FLOAT, 0, va->texcoords);
			//glVertexAttribPointer(shader->m_slot[SSLOT_TANGENT], 3, GL_FLOAT, GL_FALSE, 0, va->tangents);
			//glVertexAttribPointer(shader->m_slot[SSLOT_TANGENT], 3, GL_FLOAT, GL_FALSE, 0, va->normals);

			glDrawArrays(GL_TRIANGLES, 0, va->numverts);
#ifdef DEBUG
			CheckGLError(__FILE__, __LINE__);
#endif
		}
	}
}
