

#include "../platform.h"
#include "../save/savesprite.h"
#include "rendersprite.h"
#include "compilebl.h"
#include "../../spriteed/main.h"
#include "../gui/gui.h"
#include "../save/compilemap.h"
#include "../../spriteed/main.h"
#include "../../spriteed/segui.h"
#include "../sim/tile.h"
#include "../math/camera.h"
#include "../draw/shadow.h"
#include "../math/vec4f.h"
#include "../draw/screenshot.h"
#include "../draw/sortb.h"
#include "../../spriteed/seviewport.h"
#include "../debug.h"

char g_renderpath[MAX_PATH+1];
char g_renderbasename[MAX_PATH+1];
int g_rendertype = -1;
int g_renderframes;
int g_origwidth;
int g_origheight;
int g_deswidth;
int g_desheight;
Vec2i g_spritecenter;
Vec2i g_clipmin;
Vec2i g_clipmax;
float g_transpkey[3] = {255.0f/255.0f, 127.0f/255.0f, 255.0f/255.0f};

Vec3f g_origlightpos;
Camera g_origcam;
int g_rendside;

bool g_warned = false;

unsigned int g_rendertex;
unsigned int g_renderrb;
unsigned int g_renderfb;
bool g_renderbs = false;

bool CallResize(int w, int h)
{
	int maxsz[] = {0,0};
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, maxsz);

	if((w >= maxsz[0]*0.7f || h >= maxsz[0]*0.7f) && !g_warned)
	{
		g_warned = true;
		char msg[1024];
		sprintf(msg, "The required texture size of %dx%d exceeds or approaches your system's supported maximum of %d. You might not be able to finish the render. Reduce 1_tile_pixel_width in config.ini.", w, h, maxsz[0]);
		WarningMessage("Warning", msg);
	}

#if 1

	if(!g_renderbs && (g_mode == RENDERING || g_mode == PREREND_ADJFRAME))
	{
		g_renderbs = true;
#if 1
		glGenTextures(1, &g_rendertex);
		glBindTexture(GL_TEXTURE_2D, g_rendertex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		CheckGLError(__FILE__, __LINE__);
#if 0
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
#endif
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		CheckGLError(__FILE__, __LINE__);
		//glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenRenderbuffersEXT(1, &g_renderrb);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, g_renderrb);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, w, h);
		//glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
		CheckGLError(__FILE__, __LINE__);

		glGenFramebuffersEXT(1, &g_renderfb);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_renderfb);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, g_rendertex, 0);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, g_renderrb);
		//glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		CheckGLError(__FILE__, __LINE__);
#else
		//RGBA8 2D texture, 24 bit depth texture, 256x256
		glGenTextures(1, &g_rendertex);
		glBindTexture(GL_TEXTURE_2D, g_rendertex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//NULL means reserve texture memory, but texels are undefined
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		//-------------------------
		glGenFramebuffersEXT(1, &g_renderfb);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_renderfb);
		//Attach 2D texture to this FBO
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, g_rendertex, 0);
		//-------------------------
		glGenRenderbuffersEXT(1, &g_renderrb);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, g_renderrb);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, w, h);
		//-------------------------
		//Attach depth buffer to FBO
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, g_renderrb);
		//-------------------------
#endif

#ifdef DEBUG
		g_log<<__FILE__<<":"<<__LINE__<<"create check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<" ok="<<(int)(GL_FRAMEBUFFER_COMPLETE)<<endl;
		g_log<<__FILE__<<":"<<__LINE__<<"create check frame buf stat ext: "<<glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)<<" ok="<<(int)(GL_FRAMEBUFFER_COMPLETE)<<endl;
#endif
	}
	//w=2048;
	//h=2048;
#if 0
	DWORD dwExStyle;
	DWORD dwStyle;
	RECT WindowRect;
	WindowRect.left=(long)0;
	WindowRect.right=(long)w;
	WindowRect.top=(long)0;
	WindowRect.bottom=(long)h;
#else
    SDL_SetWindowSize(g_window, w, h);
#endif

	int startx = 0;
	int starty = 0;

#if 0
	if(g_fullscreen)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP;
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW;
		//startx = CW_USEDEFAULT;
		//starty = CW_USEDEFAULT
		startx = GetSystemMetrics(SM_CXSCREEN)/2 - g_selectedRes.width/2;
		starty = GetSystemMetrics(SM_CYSCREEN)/2 - g_selectedRes.height/2;
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);
#endif

#ifdef DEBUG
	g_log<<"rf"<<g_renderframe<<" desired: "<<w<<","<<h<<", g_wh: "<<g_width<<","<<g_height<<" wr.rl: "<<(WindowRect.right-WindowRect.left)<<","<<(WindowRect.bottom-WindowRect.top)<<endl;
	g_log.flush();
#endif

#if 0
	if(WindowRect.right-WindowRect.left == g_width
		&& WindowRect.bottom-WindowRect.top == g_height)
#endif
	{
		if(g_mode == RENDERING || g_mode == PREREND_ADJFRAME)
		{
			g_width = w;
			g_height = h;
		}

		return false;
	}

#if 0
	SetWindowPos(g_hWnd,0,0,0,WindowRect.right-WindowRect.left, WindowRect.bottom-WindowRect.top,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
#endif

	if(g_mode == RENDERING || g_mode == PREREND_ADJFRAME)
	{
		g_width = w;
		g_height = h;
	}

	return true;
#else

	return true;
#endif
}

bool FitFocus(Vec2i vmin, Vec2i vmax)
{
#if 0
	int xextent = max(abs(vmin.x-g_width/2), abs(vmax.x-g_width/2));
	int yextent = max(abs(vmin.y-g_height/2), abs(vmax.y-g_height/2));
#else
	int xextent = max(abs(vmin.x-g_width/2), abs(vmax.x-g_width/2));
	int yextent = max(abs(vmin.y-g_height/2), abs(vmax.y-g_height/2));
#endif

	//int width2 = Max2Pow(xextent*2);
	//int height2 = Max2Pow(yextent*2);

	int width = xextent*2;
	int height = yextent*2;

	//g_deswidth = width;
	//g_desheight = height;

	//g_spritecenter.x = xextent;
	//g_spritecenter.y = yextent;

	// size must be multiple of 32 or else glReadPixels will crash !!!!!
	Vec2i compatsz;
	//compatsz.x = Max2Pow32(g_width);
	//compatsz.y = Max2Pow32(g_height);
	compatsz.x = Max2Pow32(width);
	compatsz.y = Max2Pow32(height);

	g_deswidth = compatsz.x;
	g_desheight = compatsz.y;


#ifdef DEBUG
	g_log<<"rf"<<g_renderframe<<" o des wh "<<g_deswidth<<","<<g_desheight<<"  xyextn:"<<xextent<<","<<yextent<<" vminmax:("<<vmin.x<<","<<vmin.y<<")->("<<vmax.x<<","<<vmax.y<<") gwh:"<<g_width<<","<<g_height<<" "<<endl;
#endif

	//Sleep(10);

	return CallResize(compatsz.x, compatsz.y);

#if 0
	//g_camera.position(1000.0f/3, 1000.0f/3, 1000.0f/3, 0, 0, 0, 0, 1, 0);

	g_projtype = PROJ_ORTHO;
	g_camera.position(0, 0, 1000.0f, 0, 0, 0, 0, 1, 0);
	g_camera.rotateabout(Vec3f(0,0,0), -DEGTORAD(g_renderpitch), 1, 0, 0);
	g_camera.rotateabout(Vec3f(0,0,0), DEGTORAD(g_renderyaw), 0, 1, 0);

	g_zoom = 1;

	Vec3f topleft(-TILE_SIZE/2, 0, -TILE_SIZE/2);
	Vec3f bottomleft(-TILE_SIZE/2, 0, TILE_SIZE/2);
	Vec3f topright(TILE_SIZE/2, 0, -TILE_SIZE/2);
	Vec3f bottomright(TILE_SIZE/2, 0, TILE_SIZE/2);

	int width;
	int height;

	if(g_mode == RENDERING)
	{
		width = g_width;
		height = g_height;
	}
	//if(g_mode == EDITOR)
	else
	{
		View* edview = g_GUI.getview("editor");
		Widget* viewportsframe = edview->getwidget("viewports frame", WIDGET_FRAME);
		Widget* toprightviewport = viewportsframe->getsubwidg("top right viewport", WIDGET_VIEWPORT);
		width = toprightviewport->m_pos[2] - toprightviewport->m_pos[0];
		height = toprightviewport->m_pos[3] - toprightviewport->m_pos[1];
	}

	float aspect = fabsf((float)width / (float)height);
	Matrix projection;

	bool persp = false;

	if(g_mode == EDITOR && g_projtype == PROJ_PERSP)
	{
		projection = BuildPerspProjMat(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
		persp = true;
	}
	else
	{
		projection = setorthographicmat(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
	}

	//Viewport* v = &g_viewport[VIEWPORT_ANGLE45O];
	//Vec3f viewvec = g_focus; //g_camera.m_view;
	Vec3f viewvec = g_camera.m_view;
	//Vec3f focusvec = v->focus();
    //Vec3f posvec = g_focus + t->m_offset;
	Vec3f posvec = g_camera.m_pos;
	//Vec3f posvec = v->pos();

	//if(v->m_type != VIEWPORT_ANGLE45O)
	{
	//	posvec = g_camera.m_view + t->m_offset;
		//viewvec = posvec + Normalize(g_camera.m_view-posvec);
	}

	//viewvec = posvec + Normalize(viewvec-posvec);
    //Vec3f posvec2 = g_camera.lookpos() + t->m_offset;
    //Vec3f upvec = t->m_up;
    Vec3f upvec = g_camera.m_up;
	//Vec3f upvec = v->up();

	//if(v->m_type != VIEWPORT_ANGLE45O)
	//	upvec = t->m_up;

	Vec3f focusvec = viewvec;

    Matrix viewmat = gluLookAt3(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);
	Matrix mvpmat;
	mvpmat.set(projection.m_matrix);
	mvpmat.postMultiply(viewmat);

	persp = false;

	Vec4f topleft4 = ScreenPos(&mvpmat, topleft, width, height, persp);
	Vec4f topright4 = ScreenPos(&mvpmat, topright, width, height, persp);
	Vec4f bottomleft4 = ScreenPos(&mvpmat, bottomleft, width, height, persp);
	Vec4f bottomright4 = ScreenPos(&mvpmat, bottomright, width, height, persp);

	float minx = min(topleft4.x, min(topright4.x, min(bottomleft4.x, bottomright4.x)));
	float maxx = max(topleft4.x, max(topright4.x, max(bottomleft4.x, bottomright4.x)));
	//float miny = min(topleft4.y, min(topright4.y, min(bottomleft4.y, bottomright4.y)));
	//float maxy = max(topleft4.y, max(topright4.y, max(bottomleft4.y, bottomright4.y)));

	float xrange = (float)maxx - (float)minx;

	if(xrange <= 0.0f)
		xrange = g_1tilewidth;

	float zoomscale = (float)g_1tilewidth / xrange;

	g_zoom *= zoomscale;

	//g_log<<"zoom" <<g_zoom<<","<<zoomscale<<","<<xrange<<","<<topleft4.x<<","<<topleft.x<<","<<width<<","<<height<<endl;

	g_mode = PREREND_ADJFRAME;
#endif
}

void AllScreenMinMax(Vec2i *vmin, Vec2i *vmax, int width, int height)
{
	//int width = g_width;
	//int height = g_height;

	float aspect = fabsf((float)width / (float)height);
	Matrix projection;

	bool persp = false;

	if(g_mode == EDITOR && g_projtype == PROJ_PERSP)
	{
		projection = BuildPerspProjMat(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
		persp = true;
	}
	else
	{
		projection = setorthographicmat(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
	}

#ifdef DEBUG
	{

		g_log<<"rf"<<g_renderframe<<" params:"<<aspect<<","<<width<<","<<height<<","<<g_zoom<<endl;
		g_log<<"rf"<<g_renderframe<<" pmat0:"<<projection.m_matrix[0]<<","<<projection.m_matrix[1]<<","<<projection.m_matrix[2]<<","<<projection.m_matrix[3]<<endl;
		g_log<<"rf"<<g_renderframe<<" pmat1:"<<projection.m_matrix[4]<<","<<projection.m_matrix[5]<<","<<projection.m_matrix[6]<<","<<projection.m_matrix[7]<<endl;
		g_log<<"rf"<<g_renderframe<<" pmat2:"<<projection.m_matrix[8]<<","<<projection.m_matrix[9]<<","<<projection.m_matrix[10]<<","<<projection.m_matrix[11]<<endl;
		g_log<<"rf"<<g_renderframe<<" pmat3:"<<projection.m_matrix[12]<<","<<projection.m_matrix[13]<<","<<projection.m_matrix[14]<<","<<projection.m_matrix[15]<<endl;
	}
#endif

	//Viewport* v = &g_viewport[VIEWPORT_ANGLE45O];
	//Vec3f viewvec = g_focus; //g_camera.m_view;
	Vec3f viewvec = g_camera.m_view;
	//Vec3f focusvec = v->focus();
    //Vec3f posvec = g_focus + t->m_offset;
	//Vec3f posvec = g_camera.m_pos;

	Vec3f dir = Normalize( g_camera.m_view - g_camera.m_pos );
	Vec3f posvec = g_camera.m_view - dir * 100000.0f / g_zoom;

	//Vec3f posvec = v->pos();

	//if(v->m_type != VIEWPORT_ANGLE45O)
	{
	//	posvec = g_camera.m_view + t->m_offset;
		//viewvec = posvec + Normalize(g_camera.m_view-posvec);
	}

	//viewvec = posvec + Normalize(viewvec-posvec);
    //Vec3f posvec2 = g_camera.lookpos() + t->m_offset;
    //Vec3f upvec = t->m_up;
    Vec3f upvec = g_camera.m_up;
    //Vec3f upvec = g_camera.up2();
	//Vec3f upvec = v->up();

	//if(v->m_type != VIEWPORT_ANGLE45O)
	//	upvec = t->m_up;

	Vec3f focusvec = viewvec;

    Matrix viewmat = gluLookAt3(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);
	Matrix mvpmat;
	mvpmat.set(projection.m_matrix);
	mvpmat.postMultiply(viewmat);


#ifdef DEBUG
	{
		g_log<<"rf"<<g_renderframe<<" vmat0:"<<viewmat.m_matrix[0]<<","<<viewmat.m_matrix[1]<<","<<viewmat.m_matrix[2]<<","<<viewmat.m_matrix[3]<<endl;
		g_log<<"rf"<<g_renderframe<<" vmat1:"<<viewmat.m_matrix[4]<<","<<viewmat.m_matrix[5]<<","<<viewmat.m_matrix[6]<<","<<viewmat.m_matrix[7]<<endl;
		g_log<<"rf"<<g_renderframe<<" vmat2:"<<viewmat.m_matrix[8]<<","<<viewmat.m_matrix[9]<<","<<viewmat.m_matrix[10]<<","<<viewmat.m_matrix[11]<<endl;
		g_log<<"rf"<<g_renderframe<<" vmat3:"<<viewmat.m_matrix[12]<<","<<viewmat.m_matrix[13]<<","<<viewmat.m_matrix[14]<<","<<viewmat.m_matrix[15]<<endl;
	}
#endif

	persp = false;

	//Vec4f topleft4 = ScreenPos(&mvpmat, topleft, width, height, persp);
	//Vec4f topright4 = ScreenPos(&mvpmat, topright, width, height, persp);
	//Vec4f bottomleft4 = ScreenPos(&mvpmat, bottomleft, width, height, persp);
	//Vec4f bottomright4 = ScreenPos(&mvpmat, bottomright, width, height, persp);

	bool showsky = false;

	bool setmm[] = {false, false, false, false};

#if 1
	for(auto b=g_edmap.m_brush.begin(); b!=g_edmap.m_brush.end(); b++)
	{
		Texture* t = &g_texture[b->m_texture];

		if(t->sky && !showsky)
			continue;

		for(int i=0; i<b->m_nsides; i++)
		//for(int i=0; i<1; i++)
		{
			BrushSide* side = &b->m_sides[i];
			VertexArray* va = &side->m_drawva;

			for(int j=0; j<va->numverts; j++)
			{
				Vec3f v = va->vertices[j];
				Vec4f v4 = ScreenPos(&mvpmat, v, width, height, persp);

#ifdef DEBUG
				g_log<<"rf"<<g_renderframe<<" v4:"<<v4.x<<","<<v4.y<<","<<v4.z<<","<<v4.w<<endl;
#endif

#if 0
				if(floorf(v4.x) < vmin->x)
					vmin->x = floorf(v4.x);
				if(floorf(v4.y) < vmin->y)
					vmin->y = floorf(v4.y);
				if(ceilf(v4.x) > vmax->x)
					vmax->x = ceilf(v4.x);
				if(ceilf(v4.y) > vmax->y)
					vmax->y = ceilf(v4.y);
#endif

				if((v4.x) < vmin->x || !setmm[0])
				{
					vmin->x = (v4.x);
					setmm[0] = true;
				}
				if((v4.y) < vmin->y || !setmm[1])
				{
					vmin->y = (v4.y);
					setmm[1] = true;
				}
				if((v4.x) > vmax->x || !setmm[2])
				{
					vmax->x = (v4.x);
					setmm[2] = true;
				}
				if((v4.y) > vmax->y || !setmm[3])
				{
					vmax->y = (v4.y);
					setmm[3] = true;
				}
			}
		}

		//break;
	}
#endif

#ifdef DEBUG
	//for(int my=0; my<4; my++)
	{
		g_log<<"rf"<<g_renderframe<<" mat0:"<<mvpmat.m_matrix[0]<<","<<mvpmat.m_matrix[1]<<","<<mvpmat.m_matrix[2]<<","<<mvpmat.m_matrix[3]<<endl;
		g_log<<"rf"<<g_renderframe<<" mat1:"<<mvpmat.m_matrix[4]<<","<<mvpmat.m_matrix[5]<<","<<mvpmat.m_matrix[6]<<","<<mvpmat.m_matrix[7]<<endl;
		g_log<<"rf"<<g_renderframe<<" mat2:"<<mvpmat.m_matrix[8]<<","<<mvpmat.m_matrix[9]<<","<<mvpmat.m_matrix[10]<<","<<mvpmat.m_matrix[11]<<endl;
		g_log<<"rf"<<g_renderframe<<" mat3:"<<mvpmat.m_matrix[12]<<","<<mvpmat.m_matrix[13]<<","<<mvpmat.m_matrix[14]<<","<<mvpmat.m_matrix[15]<<endl;
	}
#endif

#if 1
	for(auto iter = g_modelholder.begin(); iter != g_modelholder.end(); iter++)
	{
		ModelHolder* h = &*iter;
		Model* m = &g_model[h->model];

		//m->usetex();
		//DrawVA(&h->frames[ g_renderframe % m->m_ms3d.m_totalFrames ], h->translation);

		VertexArray* va = &h->frames[ g_renderframe % m->m_ms3d.m_totalFrames ];

		for(int i=0; i<va->numverts; i++)
		{
			Vec3f v = va->vertices[i] + h->translation;
			Vec4f v4 = ScreenPos(&mvpmat, v, width, height, persp);

#ifdef DEBUG
			g_log<<"rf"<<g_renderframe<<" mdl v:"<<v.x<<","<<v.y<<","<<v.z<<endl;
			g_log<<"rf"<<g_renderframe<<" mdl v4:"<<v4.x<<","<<v4.y<<","<<v4.z<<","<<v4.w<<endl;
#endif

#if 0
			if(floorf(v4.x) < vmin->x)
				vmin->x = floorf(v4.x);
			if(floorf(v4.y) < vmin->y)
				vmin->y = floorf(v4.y);
			if(ceilf(v4.x) > vmax->x)
				vmax->x = ceilf(v4.x);
			if(ceilf(v4.y) > vmax->y)
				vmax->y = ceilf(v4.y);
#endif

			if((v4.x) < vmin->x || !setmm[0])
			{
				vmin->x = (v4.x);
				setmm[0] = true;
			}
			if((v4.y) < vmin->y || !setmm[1])
			{
				vmin->y = (v4.y);
				setmm[1] = true;
			}
			if((v4.x) > vmax->x || !setmm[2])
			{
				vmax->x = (v4.x);
				setmm[2] = true;
			}
			if((v4.y) > vmax->y || !setmm[3])
			{
				vmax->y = (v4.y);
				setmm[3] = true;
			}
		}
	}


#ifdef DEBUG
	g_log<<"rf"<<g_renderframe<<" setmm:"<<setmm[0]<<","<<setmm[1]<<","<<setmm[2]<<","<<setmm[3]<<endl;
#endif
#endif
}

void PrepareRender(const char* fullpath, int rendtype)
{
	//g_mode = PREREND_ADJFRAME;
	g_rendertype = rendtype;
	strcpy(g_renderbasename, fullpath);
	OpenSoleView("render");
	//glClearColor(255.0f/255.0f, 127.0f/255.0f, 255.0f/255.0f, 1.0f);
	g_renderframe = 0;
	g_origlightpos = g_lightPos;
	g_origcam = g_camera;
	g_rendside = 0;
	g_renderframes = GetNumFrames();
	g_origwidth = g_width;
	g_origheight = g_height;
	g_warned = false;
	ResetView();
	AdjustFrame();
}

void AdjustFrame()
{
	g_mode = PREREND_ADJFRAME;
	ResetView();
	Vec2i vmin(g_width/2, g_height/2);
	Vec2i vmax(g_width/2, g_height/2);

#ifdef DEBUG
	g_log<<"rf"<<g_renderframe<<" adjf1 gwh:"<<g_width<<","<<g_height<<" vminmax:("<<vmin.x<<","<<vmin.y<<")->("<<vmax.x<<","<<vmax.y<<")"<<endl;
#endif

	AllScreenMinMax(&vmin, &vmax, g_width, g_height);

#ifdef DEBUG
	g_log<<"rf"<<g_renderframe<<" asmm adjf2 gwh:"<<g_width<<","<<g_height<<" vminmax:("<<vmin.x<<","<<vmin.y<<")->("<<vmax.x<<","<<vmax.y<<")"<<endl;
#endif

	if(!FitFocus(vmin, vmax))
		g_mode = RENDERING;
	g_mode = RENDERING;
}

void SaveRender()
{
#if 0
	SaveScreenshot();

	g_log<<"sv r"<<endl;
	g_log.flush();
#endif

	LoadedTex screen;

#if 0
	AllocTex(&screen, g_width, g_height, 3);
	memset(screen.data, 0, g_width * g_height * 3);

	// size must be multiple of 32 or else this will crash !!!!!
	glReadPixels(0, 0, g_width, g_height, GL_RGB, GL_UNSIGNED_BYTE, screen.data);
	FlipImage(&screen);
#else
	AllocTex(&screen, g_width, g_height, 4);
	memset(screen.data, 0, g_width * g_height * 4);

	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	//glPixelStorei(GL_PACK_ALIGNMENT, 1);
	// size must be multiple of 32 or else this will crash !!!!!
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	glReadPixels(0, 0, g_width, g_height, GL_RGBA, GL_UNSIGNED_BYTE, screen.data);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	FlipImage(&screen);
#endif

	Vec2i clipsz;
	clipsz.x = g_clipmax.x - g_clipmin.x;
	clipsz.y = g_clipmax.y - g_clipmin.y;

	int imagew = Max2Pow(clipsz.x);
	int imageh = Max2Pow(clipsz.y);

#ifdef DEBUG
	g_log<<"rf"<<g_renderframe<<" gwh"<<g_width<<","<<g_height<<" clipxymm "<<g_clipmin.x<<","<<g_clipmin.y<<"->"<<g_clipmax.x<<","<<g_clipmax.y<<" clipsz "<<clipsz.x<<","<<clipsz.y<<" imgwh "<<imagew<<","<<imageh<<endl;
	g_log.flush();

	g_log<<"sv r 4"<<endl;
	g_log.flush();
#endif

	LoadedTex sprite;
	AllocTex(&sprite, imagew, imageh, 4);

	int transpkey[3] = {(int)(g_transpkey[0]*255), (int)(g_transpkey[1]*255), (int)(g_transpkey[2]*255)};

	int xoff = g_clipmin.x;
	int yoff = g_clipmin.y;

	//if(g_deswidth != g_width)	xoff += (g_width-g_deswidth)/2;
	//if(g_desheight != g_height)	yoff += (g_height-g_desheight)/2;

#ifdef DEBUG
	g_log<<"des wh "<<g_deswidth<<","<<g_desheight<<" actl "<<g_width<<","<<g_height<<endl;
	g_log<<"xy off "<<xoff<<","<<yoff<<endl;
	g_log.flush();
#endif

#if 1
	for(int x=0; x<imagew; x++)
		for(int y=0; y<imageh; y++)
		{
			int index = 4 * ( y * imagew + x );
			sprite.data[index + 0] = transpkey[0];
			sprite.data[index + 1] = transpkey[1];
			sprite.data[index + 2] = transpkey[2];
			sprite.data[index + 3] = 0;
		}
#endif

#if 1
	for(int x=0; x<clipsz.x; x++)
		for(int y=0; y<clipsz.y; y++)
		{
			int index = 4 * ( y * imagew + x );
			int index2 = 4 * ( (y+yoff) * g_width + (x+xoff) );

#if 0
			if(screen.data[index2+0] == transpkey[0]
			&& screen.data[index2+1] == transpkey[1]
			&& screen.data[index2+2] == transpkey[2])
				continue;
#endif
			//g_log<<" access "<<(x+xoff)<<","<<(y+yoff)<<" of "<<g_width<<","<<g_height<<" "<<endl;
			//g_log.flush();

			sprite.data[index+0] = screen.data[index2+0];
			sprite.data[index+1] = screen.data[index2+1];
			sprite.data[index+2] = screen.data[index2+2];
			sprite.data[index+3] = screen.data[index2+3];
		}
#endif

	char fullpath[MAX_PATH+1];
	sprintf(fullpath, "%s_si%d_fr%03d.png", g_renderbasename, g_rendside, g_renderframe);
	SavePNG(fullpath, &sprite);
	//sprite.channels = 3;
	//sprintf(fullpath, "%s_si%d_fr%03d-rgb.png", g_renderbasename, g_rendside, g_renderframe);
	//SavePNG(fullpath, &sprite);

	sprintf(fullpath, "%s_si%d_fr%03d.txt", g_renderbasename, g_rendside, g_renderframe);
	ofstream ofs(fullpath, ios_base::out);
	ofs<<g_spritecenter.x<<" "<<g_spritecenter.y<<endl<<imagew<<" "<<imageh<<endl<<clipsz.x<<" "<<clipsz.y;

#if 0
	CheckGLError(__FILE__, __LINE__);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	CheckGLError(__FILE__, __LINE__);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
	CheckGLError(__FILE__, __LINE__);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	CheckGLError(__FILE__, __LINE__);
	glDeleteFramebuffers(1, &g_renderfb);
	CheckGLError(__FILE__, __LINE__);
	glDeleteRenderbuffers(1, &g_renderrb);
	CheckGLError(__FILE__, __LINE__);
	glDeleteTextures(1, &g_rendertex);
	CheckGLError(__FILE__, __LINE__);
#else
	//Delete resources
	glDeleteTextures(1, &g_rendertex);
	glDeleteRenderbuffers(1, &g_renderrb);
	//Bind 0, which means render to back buffer, as a result, fb is unbound
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glDeleteFramebuffers(1, &g_renderfb);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
#endif
	g_renderbs = false;
}

void UpdateRender()
{
	ResetView();
	g_lightPos = Rotate(g_lightPos, g_rendside*DEGTORAD(45.0), 0, 1, 0);
	g_camera.rotateabout(Vec3f(0,0,0), g_rendside*DEGTORAD(45.0), 0, 1, 0);
	SortEdB(&g_edmap, g_camera.m_view, g_camera.m_pos);

	//AllScreenMinMax needs to be called again because the pixels center of rendering depends on the window width and height, influencing the clip min/max
	AllScreenMinMax(&g_clipmin, &g_clipmax, g_width, g_height);
	g_spritecenter.x = g_width/2 - g_clipmin.x;
	g_spritecenter.y = g_height/2 - g_clipmin.y;

	APPMODE oldmode = g_mode;
	g_mode = EDITOR;
	Draw();
	Draw();
	g_mode = oldmode;
#if 0
	Draw();
#elif 0
	Ortho(g_width, g_height, 1, 1, 1, 1);
	DrawViewport(3, 0, 0, g_width, g_height);
	EndS();
#else
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_renderfb);
#ifdef DEBUG
	g_log<<__FILE__<<":"<<__LINE__<<"check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<endl;
	CheckGLError(__FILE__, __LINE__);
#endif
	glViewport(0, 0, g_width, g_height);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	glClearColor(1.0, 1.0, 1.0, 0.0);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
	g_log<<__FILE__<<":"<<__LINE__<<"check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<endl;
	g_log<<__FILE__<<":"<<__LINE__<<"check frame buf stat ext: "<<glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)<<endl;
#endif
	glEnable(GL_DEPTH_TEST);

	{
		float aspect = fabsf((float)g_width / (float)g_height);
		Matrix projection;

		Viewport* v = &g_viewport[3];
		ViewportT* t = &g_viewportT[v->m_type];

		bool persp = false;

#if 0
		if(g_projtype == PROJ_PERSP && v->m_type == VIEWPORT_ANGLE45O)
		{
			projection = BuildPerspProjMat(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
			persp = true;
		}
		else if(g_projtype == PROJ_ORTHO || v->m_type != VIEWPORT_ANGLE45O)
#endif
		{
			projection = setorthographicmat(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
		}

		//Vec3f viewvec = g_focus;	//g_camera.m_view;
		//Vec3f viewvec = g_camera.m_view;
		Vec3f focusvec = v->focus();
		//Vec3f posvec = g_focus + t->m_offset;
		//Vec3f posvec = g_camera.m_pos;
		Vec3f posvec = v->pos();

		//if(v->m_type != VIEWPORT_ANGLE45O)
		//	posvec = g_camera.m_view + t->m_offset;

		//viewvec = posvec + Normalize(viewvec-posvec);
		//Vec3f posvec2 = g_camera.lookpos() + t->m_offset;
		//Vec3f upvec = t->m_up;
		//Vec3f upvec = g_camera.m_up;
		Vec3f upvec = v->up();

		//if(v->m_type != VIEWPORT_ANGLE45O)
		//	upvec = t->m_up;

		Matrix viewmat = gluLookAt3(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);

		Matrix modelview;
		Matrix modelmat;
		float translation[] = {0, 0, 0};
		modelview.setTranslation(translation);
		modelmat.setTranslation(translation);
		modelview.postMultiply(viewmat);

#ifdef DEBUG
		LastNum(__FILE__, __LINE__);
#endif

		Matrix mvpmat;
		mvpmat.set(projection.m_matrix);
		mvpmat.postMultiply(viewmat);

#if 1
		if(v->m_type == VIEWPORT_ANGLE45O)
		{
			//RenderToShadowMap(projection, viewmat, modelmat, g_focus);
#ifdef DEBUG
			LastNum(__FILE__, __LINE__);
#endif
			//RenderToShadowMap(projection, viewmat, modelmat, g_camera.m_view);
#ifdef DEBUG
			LastNum(__FILE__, __LINE__);
#endif
			RenderShadowedScene(projection, viewmat, modelmat, modelview);
		}
#endif
	}

#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	glFlush();
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	//glFinish();
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	SaveRender();
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
#endif

	g_renderframe++;

	if(g_renderframe >= g_renderframes)
	{
		if(g_rendside < 7 && g_rendertype == RENDER_UNIT)
		{
			g_renderframe = 0;
			g_rendside++;
			AdjustFrame();
		}
		else
		{
			EndRender();
		}
	}
	else
	{
		AdjustFrame();
	}
}

void EndRender()
{
	g_mode = EDITOR;
	OpenSoleView("editor");
	g_renderframe = 0;
	g_lightPos = g_origlightpos;
	g_camera = g_origcam;
	CallResize(g_origwidth, g_origheight);
	ResetView();
	SortEdB(&g_edmap, g_camera.m_view, g_camera.m_pos);
}
