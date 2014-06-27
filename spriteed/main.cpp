

#include "main.h"
#include "../common/gui/gui.h"
#include "../common/draw/shader.h"
#include "res.h"
#include "../common/gui/font.h"
#include "../common/texture.h"
#include "../common/draw/model.h"
#include "../common/math/frustum.h"
#include "../common/draw/billboard.h"
#include "segui.h"
#include "../common/draw/particle.h"
#include "../common/draw/shadow.h"
#include "../common/draw/particle.h"
#include "../common/platform.h"
#include "../common/utils.h"
#include "../common/window.h"
#include "../common/sim/sim.h"
#include "../common/math/matrix.h"
#include "../common/math/vec3f.h"
#include "../common/math/3dmath.h"
#include "../common/math/camera.h"
#include "sesim.h"
#include "../common/sim/entity.h"
#include "../common/save/compilemap.h"
#include "../common/save/modelholder.h"
#include "../common/tool/rendersprite.h"
#include "../common/debug.h"

APPMODE g_mode = LOADING;
bool g_mouseout = false;
bool g_gameover = false;
bool g_moved = false;

void Draw()
{
    CheckGLError(__FILE__, __LINE__);

    if(g_mode == LOADING)
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    else if(g_mode == EDITOR)
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    else if(g_mode == RENDERING)
        glClearColor(g_transpkey[0], g_transpkey[1], g_transpkey[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
    CheckGLError(__FILE__, __LINE__);
#endif

    g_GUI.frameupd();

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
    CheckGLError(__FILE__, __LINE__);
#endif

    g_GUI.draw();
    //DrawEdMap(&g_edmap);

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
    CheckGLError(__FILE__, __LINE__);
#endif

    if(g_mode == EDITOR)
    {
#ifdef DEBUG
        LastNum(__FILE__, __LINE__);
#endif
        Ortho(g_width, g_height, 1, 1, 1, 1);
        char dbgstr[128];
        sprintf(dbgstr, "b's:%d", (int)g_edmap.m_brush.size());
        DrawShadowedText(MAINFONT8, 0, g_height-16, dbgstr);
        EndS();
    }

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
#endif

    SDL_GL_SwapWindow(g_window);
}

void DrawScene(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3])
{
#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
#endif

#if 0
    if(g_mode == RENDERING)
    {
        //SwapBuffers(g_hDC);

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, g_renderfb);
        g_log<<__FILE__<<":"<<__LINE__<<"check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<endl;
        CheckGLError(__FILE__, __LINE__);
        glViewport(0, 0, g_width, g_height);
        CheckGLError(__FILE__, __LINE__);
        glClearColor(1.0, 1.0, 1.0, 0.0);
        CheckGLError(__FILE__, __LINE__);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CheckGLError(__FILE__, __LINE__);
        g_log<<__FILE__<<":"<<__LINE__<<"check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<endl;
        g_log<<__FILE__<<":"<<__LINE__<<"check frame buf stat ext: "<<glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)<<endl;
    }
#endif

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
#endif

    UseShadow(SHADER_MODEL, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
    glActiveTextureARB(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, g_depth);
    glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 4);
    /*
    glActiveTextureARB(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_transparency);
    glUniform1iARB(g_shader[SHADER_OWNED].m_slot[SSLOT_TEXTURE1], 1);
    glActiveTextureARB(GL_TEXTURE0);*/

    DrawModelHolders();
#ifdef DEBUG
    CheckGLError(__FILE__, __LINE__);
#endif
    EndS();

    UseShadow(SHADER_MAP, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);

    glActiveTextureARB(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, g_depth);
    glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 4);

    DrawEdMap(&g_edmap, g_showsky);
#ifdef DEBUG
    CheckGLError(__FILE__, __LINE__);
#endif
    EndS();

    if(g_mode == EDITOR)
    {

    }

#if 0
    if(g_mode == RENDERING)
    {
        CheckGLError(__FILE__, __LINE__);
        glFlush();
        CheckGLError(__FILE__, __LINE__);
        //glFinish();
        CheckGLError(__FILE__, __LINE__);
        SaveRender();
        CheckGLError(__FILE__, __LINE__);
    }
#endif
}

void DrawSceneDepth()
{
    //g_model[themodel].draw(0, Vec3f(0,0,0), 0);

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
#endif

    DrawModelHoldersDepth();

#ifdef DEBUG
    CheckGLError(__FILE__, __LINE__);
#endif
    //if(g_model[0].m_on)
    //	g_model[0].draw(0, Vec3f(0,0,0), 0);

    DrawEdMapDepth(&g_edmap, false);/*
	for(int i=0; i<10; i++)
		for(int j=0; j<5; j++)
			g_model[themodel].draw(0, Vec3f(-5*180 + 180*i,0,-2.5f*90 + j*90), 0);*/

#ifdef DEBUG
    CheckGLError(__FILE__, __LINE__);
#endif
}

void UpdateLoading()
{
    static int stage = 0;

    switch(stage)
    {
        //case 0: Status("Loading textures...", true); stage++; break;
        //case 1: LoadTiles(); LoadMap(); LoadTerrainTextures(); LoadHoverTex(); LoadSkyBox("defsky"); Status("Loading particles...", true); stage++; break;
        //case 2: LoadParticles(); Status("Loading projectiles...", true); stage++; break;
        //case 3: LoadProjectiles(); Status("Loading unit sprites...", true); stage++; break;
        //case 4: LoadUnitSprites(); Status("Loading sounds...", true); stage++; break;
        //case 5: LoadSounds(); Status("Loading Map sprites...", true); stage++; break;
        //case 6: BSprites(); Status("Loading models...", true); stage++; break;
    case 0:
        if(!Load1Model()) stage++;
        break;
    case 1:
        if(!Load1Texture()) stage++;
        break;
    case 2:
        //Status("logo");
        g_mode = EDITOR;
        OpenSoleView("editor");
        //g_mode = LOGO;
        //OpenSoleView("logo");
        break;
    }
}

int g_reStage = 0;
void UpdateReloading()
{
    switch(g_reStage)
    {
    case 0:
        if(Load1Texture())
        {
            g_mode = EDITOR;
            OpenSoleView("editor");
        }
        break;
    }
}

void UpdateGameState()
{
    //CalculateFrameRate();
    //Scroll();
    //LastNum("pre upd u");
    //UpdateUnits();
    //LastNum("pre upd b");
    //UpdateMaps();
    //LastNum("post upd b");
    //UpdateParticles();
    //LastNum("up pl");
    //UpdatePlayers();
    //LastNum("up ai");
    //UpdateAI();
    //ResourceTicker();
    //UpdateTimes();
    //UpdateFPS();
}

void UpdateEditor()
{
    if(g_keys[SDLK_w])
    {
        g_camera.accelerate(50.0f/g_zoom);
    }
    if(g_keys[SDLK_s])
    {
        g_camera.accelerate(-50.0f/g_zoom);
    }

    if(g_keys[SDLK_a])
    {
        g_camera.accelstrafe(-50.0f/g_zoom);
    }
    if(g_keys[SDLK_d])
    {
        g_camera.accelstrafe(50.0f/g_zoom);
    }

    if(g_keys[SDLK_r])
    {
        g_camera.accelrise(25.0f/g_zoom);
    }
    if(g_keys[SDLK_f])
    {
        g_camera.accelrise(-25.0f/g_zoom);
    }

    g_camera.frameupd();
    g_camera.friction2();
}

void Update()
{
    if(g_mode == LOADING)
        UpdateLoading();
    else if(g_mode == LOGO)
        UpdateLogo();
    //else if(g_mode == INTRO)
    //	UpdateIntro();
    //else if(g_mode == PLAY)
    //	UpdateGameState();
    else if(g_mode == EDITOR)
        UpdateEditor();
    else if(g_mode == RENDERING)
        UpdateRender();
}

void LoadConfig()
{
    char cfgfull[MAX_PATH+1];
    FullPath(CONFIGFILE, cfgfull);

#if 0
    ifstream config(cfgfull);

    int fulls;
    config>>fulls;

    if(fulls)
        g_fullscreen = true;
    else
        g_fullscreen = false;

    config>>g_selectedRes.width>>g_selectedRes.height;
    config>>g_bpp;

    g_width = g_selectedRes.width;
    g_height = g_selectedRes.height;
#endif

    ifstream f(cfgfull);
    string line;
    char keystr[32];
    char actstr[32];

    while(!f.eof())
    {

#if 0
        key = -1;
        down = NULL;
        up = NULL;
#endif
        strcpy(keystr, "");
        strcpy(actstr, "");

        getline(f, line);
        sscanf(line.c_str(), "%s %s", keystr, actstr);

        float valuef = StrToFloat(actstr);
        int valuei = StrToInt(actstr);
        bool valueb = (bool)valuei;

        if(stricmp(keystr, "fullscreen") == 0)					g_fullscreen = valueb;
        else if(stricmp(keystr, "work_width") == 0)				g_width = g_selectedRes.width = valuei;
        else if(stricmp(keystr, "work_height") == 0)			g_height = g_selectedRes.height = valuei;
        else if(stricmp(keystr, "work_bpp") == 0)				g_bpp = valuei;
        else if(stricmp(keystr, "render_pitch") == 0)			g_renderpitch = valuef;
        else if(stricmp(keystr, "render_yaw") == 0)				g_renderyaw = valuef;
        else if(stricmp(keystr, "1_tile_pixel_width") == 0)		g_1tilewidth = valuei;
        else if(stricmp(keystr, "sun_x") == 0)					g_lightOff.x = valuef;
        else if(stricmp(keystr, "sun_y") == 0)					g_lightOff.y = valuef;
        else if(stricmp(keystr, "sun_z") == 0)					g_lightOff.z = valuef;
    }
}

#if 0
void WriteConfig()
{
    ofstream config;
    config.open(CONFIGFILE, ios_base::out);

    int fulls;
    if(g_fullscreen)
        fulls = 1;
    else
        fulls = 0;

    config<<fulls<<endl;
    config<<g_selectedRes.width<<" "<<g_selectedRes.height<<endl;
    config<<g_bpp;
}
#endif

/*
void EnumerateMaps()
{
	WIN32_FIND_DATA ffd;
	string bldgPath = ExePath() + "\\bldgs\\*";
	HANDLE hFind = FindFirstFile(bldgPath.c_str(), &ffd);

	if(INVALID_HANDLE_VALUE != hFind)
	{
		do
		{
			if(!strstr(ffd.cFileName, ".bsp"))
				continue;

			int pos = string( ffd.cFileName ).find_last_of( ".bsp" );
			string name = string( ffd.cFileName ).substr(0, pos-3);

			g_bldgs.push_back(name);
		} while(FindNextFile(hFind, &ffd) != 0);
		FindClose(hFind);
	}
	else
	{
		// Failure
	}
}*/


void ScoreFPS()
{
    return;

    if(g_framesPerSecond == 0)
    {
        char msg[128];
        //sprintf(msg, "FPS: %f, %fs", (float)g_framesPerSecond, (float)(g_currentTime - g_lastTime)/(float)g_framesPerSecond);
        sprintf(msg, "FPS: %f, %fs", (float)g_instantFPS, (float)(1.0f/g_instantFPS));
        g_GUI.getview("editor")->getwidget("top panel", WIDGET_FRAME)->getsubwidg("fps", WIDGET_TEXT)->m_text = msg;
    }
}


void CalcDrawRate()
{
	static unsigned int frametime = 0.0f;				// This stores the last frame's time
	static int framecounter = 0;
	static unsigned int lasttime;

	// Get the current time in seconds
	unsigned int currtime = timeGetTime();

	// We added a small value to the frame interval to account for some video
	// cards (Radeon's) with fast computers falling through the floor without it.

	// Here we store the elapsed time between the current and last frame,
	// then keep the current frame in our static variable for the next frame.
	g_frameinterval = (currtime - frametime) / 1000.0f;	// + 0.005f;

	//g_instantdrawfps = 1.0f / (g_currentTime - frameTime);
	//g_instantdrawfps = 1.0f / g_drawfrinterval;

	frametime = currtime;

	// Increase the frame counter
	++framecounter;

	// Now we want to subtract the current time by the last time that was stored
	// to see if the time elapsed has been over a second, which means we found our FPS.
	if( currtime - lasttime > 1000 )
	{
		g_instantFPS = framecounter;

		// Here we set the lastTime to the currentTime
		lasttime = currtime;

		// Reset the frames per second
		framecounter = 0;
	}
}

bool DrawNextFrame(int desiredFrameRate)
{
	static long long lastTime = GetTickCount64();
	static long long elapsedTime = 0;

	long long currentTime = GetTickCount64(); // Get the time (milliseconds = seconds * .001)
	long long deltaTime = currentTime - lastTime; // Get the slice of time
	int desiredFPS = 1000 / (float)desiredFrameRate; // Store 1 / desiredFrameRate

	elapsedTime += deltaTime; // Add to the elapsed time
	lastTime = currentTime; // Update lastTime

	// Check if the time since we last checked is greater than our desiredFPS
	if( elapsedTime > desiredFPS )
	{
		elapsedTime -= desiredFPS; // Adjust the elapsed time

		// Return true, to animate the next frame of animation
		return true;
	}

	// We don't animate right now.
	return false;
	/*
	long long currentTime = GetTickCount();
	float desiredFPMS = 1000.0f/(float)desiredFrameRate;
	int deltaTime = currentTime - g_lasttime;

	if(deltaTime >= desiredFPMS)
	{
		g_lasttime = currentTime;
		return true;
	}

	return false;*/
}

void EventLoop()
{
#if 0
    key->keysym.scancode
    SDLMod  e.key.keysym.mod
    key->keysym.unicode

    if( mod & KMOD_NUM ) printf( "NUMLOCK " );
    if( mod & KMOD_CAPS ) printf( "CAPSLOCK " );
    if( mod & KMOD_LCTRL ) printf( "LCTRL " );
    if( mod & KMOD_RCTRL ) printf( "RCTRL " );
    if( mod & KMOD_RSHIFT ) printf( "RSHIFT " );
    if( mod & KMOD_LSHIFT ) printf( "LSHIFT " );
    if( mod & KMOD_RALT ) printf( "RALT " );
    if( mod & KMOD_LALT ) printf( "LALT " );
    if( mod & KMOD_CTRL ) printf( "CTRL " );
    if( mod & KMOD_SHIFT ) printf( "SHIFT " );
    if( mod & KMOD_ALT ) printf( "ALT " );
#endif

    //SDL_EnableUNICODE(SDL_ENABLE);

    GUI* gui = &g_GUI;

    while (!g_quit)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            switch(e.type)
            {
            case SDL_WINDOWEVENT:
                switch (e.window.event)
                {
                case SDL_WINDOWEVENT_RESIZED:
                    if(g_mode == PREREND_ADJFRAME)
                    {
#ifdef DEBUG
                        g_log<<"to r"<<endl;
                        g_log<<"rf"<<g_renderframe<<" rsz "<<g_deswidth<<","<<g_desheight<<endl;
#endif
                        Resize(g_deswidth, g_desheight);
                        g_mode = RENDERING;
                    }
                    else
                    {
#ifdef DEBUG
                        g_log<<"rf"<<g_renderframe<<" rsz "<<(LOWORD(lParam))<<","<<(HIWORD(lParam))<<endl;
#endif
                        Resize(e.window.data1, e.window.data2);
                    }
                    break;
                default:
                    break;
                }
                break;
            case SDL_QUIT:
                g_quit = true;
                break;
            case SDL_KEYDOWN:
                if(!g_GUI.keydown(e.key.keysym.scancode))
                    g_keys[e.key.keysym.scancode] = true;
                break;
            case SDL_KEYUP:
                if(!g_GUI.keyup(e.key.keysym.scancode))
                    g_keys[e.key.keysym.scancode] = false;
                break;
            case SDL_TEXTINPUT:
                g_GUI.charin((unsigned int)*e.text.text);
                break;
            case SDL_TEXTEDITING:
#if 0
                //g_GUI.charin(e.text.text);	//UTF8
                ev.type = INEV_TEXTED;
                ev.text = e.text.text;
                ev.cursor = e.edit.start;
                ev.sellen = e.edit.length;

                g_log<<"SDL_TEXTEDITING:";
                for(int i=0; i<strlen(e.text.text); i++)
                {
                    g_log<<"[#"<<(unsigned int)(unsigned char)e.text.text[i]<<"]";
                }
                g_log<<endl;
                g_log.flush();

                g_log<<"texted: cursor:"<<ev.cursor<<" sellen:"<<ev.sellen<<endl;
                g_log.flush();

                gui->inev(&ev);
#endif
                break;
            case SDL_MOUSEWHEEL:
                g_GUI.mousewheel(e.wheel.y);
                break;
            case SDL_MOUSEBUTTONDOWN:
                switch (e.button.button)
                {
                case SDL_BUTTON_LEFT:
                    g_mousekeys[0] = true;
                    g_moved = false;
                    g_GUI.lbuttondown();
                    break;
                case SDL_BUTTON_RIGHT:
                    g_mousekeys[2] = true;
                    g_moved = false;
                    g_GUI.rbuttondown();
                    break;
                case SDL_BUTTON_MIDDLE:
                    g_mousekeys[1] = true;
                    g_moved = false;
                    g_GUI.mbuttondown();
                    break;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                switch (e.button.button)
                {
                case SDL_BUTTON_LEFT:
                    g_mousekeys[0] = false;
                    g_GUI.lbuttonup(g_moved);
                    break;
                case SDL_BUTTON_RIGHT:
                    g_mousekeys[2] = false;
                    g_GUI.rbuttonup(g_moved);
                    break;
                case SDL_BUTTON_MIDDLE:
                    g_mousekeys[1] = false;
                    g_GUI.mbuttonup(g_moved);
                    break;
                }
                break;
            case SDL_MOUSEMOTION:
                //py->mouse.x = e.motion.x;
                //py->mouse.y = e.motion.y;

                if(g_mouseout)
                {
                    //TrackMouse();
                    g_mouseout = false;
                }
                if(MousePosition())
                {
                    g_moved = true;
                    g_GUI.mousemove();
                }
                break;
            }
        }

        if(g_mode == LOADING || g_mode == RENDERING || DrawNextFrame(FRAME_RATE))
        {
#ifdef DEBUG
            CheckGLError(__FILE__, __LINE__);
#endif
            CalcDrawRate();
#ifdef DEBUG
            CheckGLError(__FILE__, __LINE__);
#endif
            ScoreFPS();
#ifdef DEBUG
            LastNum(__FILE__, __LINE__);
            CheckGLError(__FILE__, __LINE__);
#endif
            Update();
#ifdef DEBUG
            LastNum(__FILE__, __LINE__);
            CheckGLError(__FILE__, __LINE__);
#endif
            Draw();
#ifdef DEBUG
            LastNum(__FILE__, __LINE__);
#endif
        }
    }
}

// Define the function to be called when ctrl-c (SIGINT) signal is sent to process
void SignalCallback(int signum)
{
    //printf("Caught signal %d\n",signum);
    // Cleanup and close up stuff here

    // Terminate program
    g_quit = true;
}

void Init()
{
#ifdef PLATFORM_LINUX
    signal(SIGINT, SignalCallback);
#endif

    SDL_Init(SDL_INIT_VIDEO);
    OpenLog("log.txt", VERSION);
    srand(GetTickCount64());
    g_lastsave[0] = '\0';
    LoadConfig();

    //EnumerateMaps();
    //EnumerateDisplay();
    //MapKeys();

    InitOnce();
}

void Deinit()
{
    DestroyEntities();
    DestroyWindow(TITLE);
    // Clean up
    SDL_Quit();
}

#ifdef PLATFORM_WIN
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char* argv[])
#endif
{
	g_log << "Log start"    << endl; /* TODO, include date */
	g_log << "Init: "       << endl;
	g_log.flush();

	Init();

	g_log << "MakeWindow: " << endl;
	g_log.flush();

	MakeWindow(TITLE);

	g_log << "Queue: "      << endl;
	g_log.flush();

	//SDL_ShowCursor(false);
	Queue();

	g_log << "FillGUI: "    << endl;
	g_log.flush();

	FillGUI();

	g_log << "EventLoop: "  << endl;
	g_log.flush();

	EventLoop();

	g_log << "Deinit: "     << endl;
	g_log.flush();

	Deinit();
	//SDL_ShowCursor(true);

	return 0;
}



