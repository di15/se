

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
unsigned int g_renderdepthtex;
unsigned int g_renderrb;
unsigned int g_renderfb;
bool g_renderbs = false;

void MakeFBO()
{
	if(g_renderbs)
		return;

	if(g_mode == RENDERING || g_mode == PREREND_ADJFRAME)
	{
		g_renderbs = true;
#if 0   //OpenGL 3.0 way
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
#else   //OpenGL 1.4 way

        glGenTextures(1, &g_rendertex);
        glBindTexture(GL_TEXTURE_2D, g_rendertex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, g_deswidth, g_desheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        //g_applog<<"gge1 "<<glGetError()<<std::endl;

        glGenTextures(1, &g_renderdepthtex);
        glBindTexture(GL_TEXTURE_2D, g_renderdepthtex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, g_deswidth, g_desheight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        //g_applog<<"gge2 "<<glGetError()<<std::endl;
        //glDrawBuffer(GL_NONE); // No color buffer is drawn
        //glReadBuffer(GL_NONE);

        glGenFramebuffers(1, &g_renderfb);
        glBindFramebuffer(GL_FRAMEBUFFER, g_renderfb);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_rendertex, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_renderdepthtex, 0);

        //g_applog<<"gge3 "<<glGetError()<<std::endl;

        GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT_EXT};
        glDrawBuffers(1, DrawBuffers);

        //g_applog<<"gge4 "<<glGetError()<<std::endl;

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            ErrorMessage("Error", "Couldn't create framebuffer for render.");
            return;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

#ifdef DEBUG
		g_applog<<__FILE__<<":"<<__LINE__<<"create check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<" ok="<<(int)(GL_FRAMEBUFFER_COMPLETE)<<std::endl;
		g_applog<<__FILE__<<":"<<__LINE__<<"create check frame buf stat ext: "<<glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)<<" ok="<<(int)(GL_FRAMEBUFFER_COMPLETE)<<std::endl;
#endif
	}
}

void DelFBO()
{
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
	glDeleteTextures(1, &g_renderdepthtex);
	//glDeleteRenderbuffers(1, &g_renderrb);
	//Bind 0, which means render to back buffer, as a result, fb is unbound
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glDeleteFramebuffers(1, &g_renderfb);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
#endif
	
	g_renderbs = false;
}

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
	Resize(w, h);
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

#if 0
	g_applog<<"rf"<<g_renderframe<<" desired: "<<w<<","<<h<<", g_wh: "<<g_width<<","<<g_height<<" wr.rl: "<<(WindowRect.right-WindowRect.left)<<","<<(WindowRect.bottom-WindowRect.top)<<std::endl;
	g_applog.flush();
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
	g_applog<<"rf"<<g_renderframe<<" o des wh "<<g_deswidth<<","<<g_desheight<<"  xyextn:"<<xextent<<","<<yextent<<" vminmax:("<<vmin.x<<","<<vmin.y<<")->("<<vmax.x<<","<<vmax.y<<") gwh:"<<g_width<<","<<g_height<<" "<<std::endl;
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
		View* edview = g_GUI.get("editor");
		Widget* viewportsframe = edview->get("viewports frame", WIDGET_FRAME);
		Widget* toprightviewport = viewportsframe->get("top right viewport", WIDGET_VIEWPORT);
		width = toprightviewport->m_pos[2] - toprightviewport->m_pos[0];
		height = toprightviewport->m_pos[3] - toprightviewport->m_pos[1];
	}

	float aspect = fabsf((float)width / (float)height);
	Matrix projection;

	bool persp = false;

	if(g_mode == EDITOR && g_projtype == PROJ_PERSP)
	{
		projection = PerspProj(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
		persp = true;
	}
	else
	{
		projection = OrthoProj(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
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

    Matrix viewmat = LookAt(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);
	Matrix mvpmat;
	mvpmat.set(projection.m_matrix);
	mvpmat.postmult(viewmat);

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

	//g_applog<<"zoom" <<g_zoom<<","<<zoomscale<<","<<xrange<<","<<topleft4.x<<","<<topleft.x<<","<<width<<","<<height<<std::endl;

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
		projection = PerspProj(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
		persp = true;
	}
	else
	{
		projection = OrthoProj(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
	}

#ifdef DEBUG
	{

		g_applog<<"rf"<<g_renderframe<<" params:"<<aspect<<","<<width<<","<<height<<","<<g_zoom<<std::endl;
		g_applog<<"rf"<<g_renderframe<<" pmat0:"<<projection.m_matrix[0]<<","<<projection.m_matrix[1]<<","<<projection.m_matrix[2]<<","<<projection.m_matrix[3]<<std::endl;
		g_applog<<"rf"<<g_renderframe<<" pmat1:"<<projection.m_matrix[4]<<","<<projection.m_matrix[5]<<","<<projection.m_matrix[6]<<","<<projection.m_matrix[7]<<std::endl;
		g_applog<<"rf"<<g_renderframe<<" pmat2:"<<projection.m_matrix[8]<<","<<projection.m_matrix[9]<<","<<projection.m_matrix[10]<<","<<projection.m_matrix[11]<<std::endl;
		g_applog<<"rf"<<g_renderframe<<" pmat3:"<<projection.m_matrix[12]<<","<<projection.m_matrix[13]<<","<<projection.m_matrix[14]<<","<<projection.m_matrix[15]<<std::endl;
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

    Matrix viewmat = LookAt(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);
	Matrix mvpmat;
	mvpmat.set(projection.m_matrix);
	mvpmat.postmult(viewmat);


#ifdef DEBUG
	{
		g_applog<<"rf"<<g_renderframe<<" vmat0:"<<viewmat.m_matrix[0]<<","<<viewmat.m_matrix[1]<<","<<viewmat.m_matrix[2]<<","<<viewmat.m_matrix[3]<<std::endl;
		g_applog<<"rf"<<g_renderframe<<" vmat1:"<<viewmat.m_matrix[4]<<","<<viewmat.m_matrix[5]<<","<<viewmat.m_matrix[6]<<","<<viewmat.m_matrix[7]<<std::endl;
		g_applog<<"rf"<<g_renderframe<<" vmat2:"<<viewmat.m_matrix[8]<<","<<viewmat.m_matrix[9]<<","<<viewmat.m_matrix[10]<<","<<viewmat.m_matrix[11]<<std::endl;
		g_applog<<"rf"<<g_renderframe<<" vmat3:"<<viewmat.m_matrix[12]<<","<<viewmat.m_matrix[13]<<","<<viewmat.m_matrix[14]<<","<<viewmat.m_matrix[15]<<std::endl;
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
				g_applog<<"rf"<<g_renderframe<<" v4:"<<v4.x<<","<<v4.y<<","<<v4.z<<","<<v4.w<<std::endl;
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

				if(floor(v4.x+0.5f) < vmin->x || !setmm[0])
				{
					vmin->x = floor(v4.x+0.5f);
					setmm[0] = true;
				}
				if(floor(v4.y+0.5f) < vmin->y || !setmm[1])
				{
					vmin->y = floor(v4.y+0.5f);
					setmm[1] = true;
				}
				if(floor(v4.x+0.5f) > vmax->x || !setmm[2])
				{
					vmax->x = floor(v4.x+0.5f);
					setmm[2] = true;
				}
				if(floor(v4.y+0.5f) > vmax->y || !setmm[3])
				{
					vmax->y = floor(v4.y+0.5f);
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
		g_applog<<"rf"<<g_renderframe<<" mat0:"<<mvpmat.m_matrix[0]<<","<<mvpmat.m_matrix[1]<<","<<mvpmat.m_matrix[2]<<","<<mvpmat.m_matrix[3]<<std::endl;
		g_applog<<"rf"<<g_renderframe<<" mat1:"<<mvpmat.m_matrix[4]<<","<<mvpmat.m_matrix[5]<<","<<mvpmat.m_matrix[6]<<","<<mvpmat.m_matrix[7]<<std::endl;
		g_applog<<"rf"<<g_renderframe<<" mat2:"<<mvpmat.m_matrix[8]<<","<<mvpmat.m_matrix[9]<<","<<mvpmat.m_matrix[10]<<","<<mvpmat.m_matrix[11]<<std::endl;
		g_applog<<"rf"<<g_renderframe<<" mat3:"<<mvpmat.m_matrix[12]<<","<<mvpmat.m_matrix[13]<<","<<mvpmat.m_matrix[14]<<","<<mvpmat.m_matrix[15]<<std::endl;
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
			g_applog<<"rf"<<g_renderframe<<" mdl v:"<<v.x<<","<<v.y<<","<<v.z<<std::endl;
			g_applog<<"rf"<<g_renderframe<<" mdl v4:"<<v4.x<<","<<v4.y<<","<<v4.z<<","<<v4.w<<std::endl;
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

			if(floor(v4.x+0.5f) < vmin->x || !setmm[0])
			{
				vmin->x = floor(v4.x+0.5f);
				setmm[0] = true;
			}
			if(floor(v4.y+0.5f) < vmin->y || !setmm[1])
			{
				vmin->y = floor(v4.y+0.5f);
				setmm[1] = true;
			}
			if(floor(v4.x+0.5f) > vmax->x || !setmm[2])
			{
				vmax->x = floor(v4.x+0.5f);
				setmm[2] = true;
			}
			if(floor(v4.y+0.5f) > vmax->y || !setmm[3])
			{
				vmax->y = floor(v4.y+0.5f);
				setmm[3] = true;
			}
		}
	}

	{
		VertexArray* va = &g_tileva[g_currinc];

		for(int i=0; i<va->numverts; i++)
		{
			Vec3f v = va->vertices[i];
			Vec4f v4 = ScreenPos(&mvpmat, v, width, height, persp);

			if(floor(v4.x+0.5f) < vmin->x || !setmm[0])
			{
				vmin->x = floor(v4.x+0.5f);
				setmm[0] = true;
			}
			if(floor(v4.y+0.5f) < vmin->y || !setmm[1])
			{
				vmin->y = floor(v4.y+0.5f);
				setmm[1] = true;
			}
			if(floor(v4.x+0.5f) > vmax->x || !setmm[2])
			{
				vmax->x = floor(v4.x+0.5f);
				setmm[2] = true;
			}
			if(floor(v4.y+0.5f) > vmax->y || !setmm[3])
			{
				vmax->y = floor(v4.y+0.5f);
				setmm[3] = true;
			}
		}
	}
#ifdef DEBUG
	g_applog<<"rf"<<g_renderframe<<" setmm:"<<setmm[0]<<","<<setmm[1]<<","<<setmm[2]<<","<<setmm[3]<<std::endl;
#endif
#endif
}

void PrepareRender(const char* fullpath, int rendtype)
{
	//g_mode = PREREND_ADJFRAME;
	g_rendertype = rendtype;
	strcpy(g_renderbasename, fullpath);
	g_GUI.closeall();
	g_GUI.open("render");
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
	g_applog<<"rf"<<g_renderframe<<" adjf1 gwh:"<<g_width<<","<<g_height<<" vminmax:("<<vmin.x<<","<<vmin.y<<")->("<<vmax.x<<","<<vmax.y<<")"<<std::endl;
#endif

	AllScreenMinMax(&vmin, &vmax, g_width, g_height);

#ifdef DEBUG
	g_applog<<"rf"<<g_renderframe<<" asmm adjf2 gwh:"<<g_width<<","<<g_height<<" vminmax:("<<vmin.x<<","<<vmin.y<<")->("<<vmax.x<<","<<vmax.y<<")"<<std::endl;
#endif

	if(!FitFocus(vmin, vmax))
		g_mode = RENDERING;
	g_mode = RENDERING;
}

void SaveRender(int rendstage)
{
#if 0
	SaveScreenshot();

	g_applog<<"sv r"<<std::endl;
	g_applog.flush();
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
	g_applog<<"rf"<<g_renderframe<<" gwh"<<g_width<<","<<g_height<<" clipxymm "<<g_clipmin.x<<","<<g_clipmin.y<<"->"<<g_clipmax.x<<","<<g_clipmax.y<<" clipsz "<<clipsz.x<<","<<clipsz.y<<" imgwh "<<imagew<<","<<imageh<<std::endl;
	g_applog.flush();

	g_applog<<"sv r 4"<<std::endl;
	g_applog.flush();
#endif

	LoadedTex sprite;
	AllocTex(&sprite, imagew, imageh, 4);

	int transpkey[3] = {(int)(g_transpkey[0]*255), (int)(g_transpkey[1]*255), (int)(g_transpkey[2]*255)};

	int xoff = g_clipmin.x;
	int yoff = g_clipmin.y;

	//if(g_deswidth != g_width)	xoff += (g_width-g_deswidth)/2;
	//if(g_desheight != g_height)	yoff += (g_height-g_desheight)/2;

#ifdef DEBUG
	g_applog<<"des wh "<<g_deswidth<<","<<g_desheight<<" actl "<<g_width<<","<<g_height<<std::endl;
	g_applog<<"xy off "<<xoff<<","<<yoff<<std::endl;
	g_applog.flush();
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

#if 1
			if(screen.data[index2+0] == transpkey[0]
			&& screen.data[index2+1] == transpkey[1]
			&& screen.data[index2+2] == transpkey[2])
				continue;
#endif
			//g_applog<<" access "<<(x+xoff)<<","<<(y+yoff)<<" of "<<g_width<<","<<g_height<<" "<<std::endl;
			//g_applog.flush();

			sprite.data[index+0] = screen.data[index2+0];
			sprite.data[index+1] = screen.data[index2+1];
			sprite.data[index+2] = screen.data[index2+2];
			sprite.data[index+3] = screen.data[index2+3];
		}
#endif

	char fullpath[MAX_PATH+1];

	char frame[32];
	char side[32];
	strcpy(frame, "");
	strcpy(side, "");

	if(g_rendertype == RENDER_UNIT || g_rendertype == RENDER_BUILDING)
		sprintf(frame, "_fr%03d", g_renderframe);
	
	if(g_rendertype == RENDER_UNIT)
		sprintf(side, "_si%d", g_rendside);
	
	string incline = "";

	if(g_rendertype == RENDER_TERRTILE || g_rendertype == RENDER_ROAD)
	{
		if(g_currinc == INC_0000)	incline = "_inc0000";
		else if(g_currinc == INC_0001)	incline = "_inc0001";
		else if(g_currinc == INC_0010)	incline = "_inc0010";
		else if(g_currinc == INC_0011)	incline = "_inc0011";
		else if(g_currinc == INC_0100)	incline = "_inc0100";
		else if(g_currinc == INC_0101)	incline = "_inc0101";
		else if(g_currinc == INC_0110)	incline = "_inc0110";
		else if(g_currinc == INC_0111)	incline = "_inc0111";
		else if(g_currinc == INC_1000)	incline = "_inc1000";
		else if(g_currinc == INC_1001)	incline = "_inc1001";
		else if(g_currinc == INC_1010)	incline = "_inc1010";
		else if(g_currinc == INC_1011)	incline = "_inc1011";
		else if(g_currinc == INC_1100)	incline = "_inc1100";
		else if(g_currinc == INC_1101)	incline = "_inc1101";
		else if(g_currinc == INC_1110)	incline = "_inc1110";
	}

	string stage = "";

	if(rendstage == RENDSTAGE_TEAM)
		stage = "_team";

	sprintf(fullpath, "%s%s%s%s%s.png", g_renderbasename, side, frame, incline.c_str(), stage.c_str());
	SavePNG(fullpath, &sprite);
	//sprite.channels = 3;
	//sprintf(fullpath, "%s_si%d_fr%03d-rgb.png", g_renderbasename, g_rendside, g_renderframe);
	//SavePNG(fullpath, &sprite);

	sprintf(fullpath, "%s%s%s%s.txt", g_renderbasename, side, frame, incline.c_str());
	ofstream ofs(fullpath, ios_base::out);
	ofs<<g_spritecenter.x<<" "<<g_spritecenter.y<<std::endl<<imagew<<" "<<imageh<<std::endl<<clipsz.x<<" "<<clipsz.y;
}

void SpriteRender(int rendstage)
{
	glClearColor(g_transpkey[0],g_transpkey[1],g_transpkey[2],0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
	g_applog<<__FILE__<<":"<<__LINE__<<"check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<std::endl;
	g_applog<<__FILE__<<":"<<__LINE__<<"check frame buf stat ext: "<<glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)<<std::endl;
#endif

	{
		float aspect = fabsf((float)g_width / (float)g_height);
		Matrix projection;

		Viewport* v = &g_viewport[3];
		ViewportT* t = &g_viewportT[v->m_type];

		bool persp = false;

#if 0
		if(g_projtype == PROJ_PERSP && v->m_type == VIEWPORT_ANGLE45O)
		{
			projection = PerspProj(FIELD_OF_VIEW, aspect, MIN_DISTANCE, MAX_DISTANCE);
			persp = true;
		}
		else if(g_projtype == PROJ_ORTHO || v->m_type != VIEWPORT_ANGLE45O)
#endif
		{
			projection = OrthoProj(-PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT*aspect/g_zoom, PROJ_RIGHT/g_zoom, -PROJ_RIGHT/g_zoom, MIN_DISTANCE, MAX_DISTANCE);
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

		Matrix viewmat = LookAt(posvec.x, posvec.y, posvec.z, focusvec.x, focusvec.y, focusvec.z, upvec.x, upvec.y, upvec.z);

		Matrix modelview;
		Matrix modelmat;
		float translation[] = {0, 0, 0};
		modelview.setTranslation(translation);
		modelmat.setTranslation(translation);
		modelview.postmult(viewmat);

#ifdef DEBUG
		LastNum(__FILE__, __LINE__);
#endif

		Matrix mvpmat;
		mvpmat.set(projection.m_matrix);
		mvpmat.postmult2(viewmat);

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
			if(rendstage == RENDSTAGE_COLOR)
				RenderShadowedScene(projection, viewmat, modelmat, modelview, DrawScene);
			else if(rendstage == RENDSTAGE_TEAM)
				RenderShadowedScene(projection, viewmat, modelmat, modelview, DrawSceneTeam);
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
	glBindFramebuffer(GL_FRAMEBUFFER, g_renderfb);
#ifdef DEBUG
	g_applog<<__FILE__<<":"<<__LINE__<<"check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<std::endl;
	CheckGLError(__FILE__, __LINE__);
#endif
	glViewport(0, 0, g_width, g_height);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glEnable(GL_DEPTH_TEST);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif

	MakeFBO();
	SpriteRender(RENDSTAGE_COLOR);
	SaveRender(RENDSTAGE_COLOR);
	DelFBO();
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	if(g_rendertype != RENDER_TERRTILE)
	{
		MakeFBO();
		SpriteRender(RENDSTAGE_TEAM);
		SaveRender(RENDSTAGE_TEAM);
		DelFBO();
	}
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
#endif

	if(g_rendertype == RENDER_UNIT || g_rendertype == RENDER_BUILDING)
		g_renderframe++;
	else if(g_rendertype == RENDER_TERRTILE || g_rendertype == RENDER_ROAD)
		g_currinc++;

	if(((g_rendertype == RENDER_UNIT || g_rendertype == RENDER_BUILDING) && g_renderframe >= g_renderframes) ||
		((g_rendertype == RENDER_TERRTILE || g_rendertype == RENDER_ROAD) && g_currinc >= INCLINES))
	{
		if(g_rendside < 7 && g_rendertype == RENDER_UNIT)
		{
			g_renderframe = 0;
			g_rendside++;
			AdjustFrame();
		}
		else if(g_rendertype == RENDER_TERRTILE || g_rendertype == RENDER_ROAD)
		{
			g_currinc = 0;
			EndRender();
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
	g_GUI.closeall();
	g_GUI.open("editor");
	g_renderframe = 0;
	g_lightPos = g_origlightpos;
	g_camera = g_origcam;
	CallResize(g_origwidth, g_origheight);
	ResetView();
	SortEdB(&g_edmap, g_camera.m_view, g_camera.m_pos);
	g_GUI.reframe();
}
