

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
        g_applog<<__FILE__<<":"<<__LINE__<<"check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<std::endl;
        CheckGLError(__FILE__, __LINE__);
        glViewport(0, 0, g_width, g_height);
        CheckGLError(__FILE__, __LINE__);
        glClearColor(1.0, 1.0, 1.0, 0.0);
        CheckGLError(__FILE__, __LINE__);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CheckGLError(__FILE__, __LINE__);
        g_applog<<__FILE__<<":"<<__LINE__<<"check frame buf stat: "<<glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)<<std::endl;
        g_applog<<__FILE__<<":"<<__LINE__<<"check frame buf stat ext: "<<glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)<<std::endl;
    }
#endif

#ifdef DEBUG
    LastNum(__FILE__, __LINE__);
#endif

#if 1
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
#endif

    UseShadow(SHADER_MAP, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
    CheckGLError(__FILE__, __LINE__);
    glActiveTextureARB(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, g_depth);
    //glBindTexture(GL_TEXTURE_2D, g_texture[0].texname);
    glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 4);
    CheckGLError(__FILE__, __LINE__);
    DrawEdMap(&g_edmap, g_showsky);
    CheckGLError(__FILE__, __LINE__);
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
        g_GUI.closeall();
        g_GUI.open("editor");
        //g_mode = LOGO;
        //OpenSoleView("logo");
        //g_GUI.open("choose file");
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
            g_GUI.closeall();
            g_GUI.open("editor");
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
    if(g_keys[SDL_SCANCODE_W])
    {
        g_camera.accelerate(50.0f/g_zoom);
    }
    if(g_keys[SDL_SCANCODE_S])
    {
        g_camera.accelerate(-50.0f/g_zoom);
    }

    if(g_keys[SDL_SCANCODE_A])
    {
        g_camera.accelstrafe(-50.0f/g_zoom);
    }
    if(g_keys[SDL_SCANCODE_D])
    {
        g_camera.accelstrafe(50.0f/g_zoom);
    }

    if(g_keys[SDL_SCANCODE_R])
    {
        g_camera.accelrise(25.0f/g_zoom);
    }
    if(g_keys[SDL_SCANCODE_F])
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

    config<<fulls<<std::endl;
    config<<g_selectedRes.width<<" "<<g_selectedRes.height<<std::endl;
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
        g_GUI.get("editor")->get("top panel")->get("fps")->m_text = msg;
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
            InEv ev;
            ev.intercepted = false;

            switch(e.type)
            {
            case SDL_WINDOWEVENT:
                switch (e.window.event)
                {
                case SDL_WINDOWEVENT_RESIZED:
                    if(g_mode == PREREND_ADJFRAME)
                    {
#ifdef DEBUG
                        g_applog<<"to r"<<std::endl;
                        g_applog<<"rf"<<g_renderframe<<" rsz "<<g_deswidth<<","<<g_desheight<<std::endl;
#endif
                        Resize(g_deswidth, g_desheight);
                        g_mode = RENDERING;
                    }
                    else
                    {
#if 0
                        g_applog<<"rf"<<g_renderframe<<" rsz "<<(LOWORD(lParam))<<","<<(HIWORD(lParam))<<std::endl;
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
                ev.type = INEV_KEYDOWN;
                ev.key = e.key.keysym.sym;
                ev.scancode = e.key.keysym.scancode;

                gui->inev(&ev);

                if(!ev.intercepted)
                    g_keys[e.key.keysym.scancode] = true;

                g_keyintercepted = ev.intercepted;
                break;
            case SDL_KEYUP:
                ev.type = INEV_KEYUP;
                ev.key = e.key.keysym.sym;
                ev.scancode = e.key.keysym.scancode;

                gui->inev(&ev);

                if(!ev.intercepted)
                    g_keys[e.key.keysym.scancode] = false;

                g_keyintercepted = ev.intercepted;
                break;
            case SDL_TEXTINPUT:
                //g_GUI.charin(e.text.text);	//UTF8
                ev.type = INEV_TEXTIN;
                ev.text = e.text.text;

                g_applog<<"SDL_TEXTINPUT:";
                for(int i=0; i<strlen(e.text.text); i++)
                {
                    g_applog<<"[#"<<(unsigned int)(unsigned char)e.text.text[i]<<"]";
                }
                g_applog<<std::endl;
                g_applog.flush();

                gui->inev(&ev);
                break;
            case SDL_TEXTEDITING:
                //g_GUI.charin(e.text.text);	//UTF8
                ev.type = INEV_TEXTED;
                ev.text = e.text.text;
                ev.cursor = e.edit.start;
                ev.sellen = e.edit.length;

                g_applog<<"SDL_TEXTEDITING:";
                for(int i=0; i<strlen(e.text.text); i++)
                {
                    g_applog<<"[#"<<(unsigned int)(unsigned char)e.text.text[i]<<"]";
                }
                g_applog<<std::endl;
                g_applog.flush();

                g_applog<<"texted: cursor:"<<ev.cursor<<" sellen:"<<ev.sellen<<std::endl;
                g_applog.flush();

                gui->inev(&ev);
#if 0
                ev.intercepted = false;
                ev.type = INEV_TEXTIN;
                ev.text = e.text.text;

                gui->inev(&ev);
#endif
                break;
#if 0
            case SDL_TEXTINPUT:
                /* Add new text onto the end of our text */
                strcat(text, event.text.text);
#if 0
                ev.type = INEV_CHARIN;
                ev.key = wParam;
                ev.scancode = 0;

                gui->inev(&ev);
#endif
                break;
            case SDL_TEXTEDITING:
                /*
                   Update the composition text.
                   Update the cursor position.
                   Update the selection length (if any).
                 */
                composition = event.edit.text;
                cursor = event.edit.start;
                selection_len = event.edit.length;
                break;
#endif
                //else if(e.type == SDL_BUTTONDOWN)
                //{
                //}
            case SDL_MOUSEWHEEL:
                ev.type = INEV_MOUSEWHEEL;
                ev.amount = e.wheel.y;

                gui->inev(&ev);
            case SDL_MOUSEBUTTONDOWN:
                switch (e.button.button)
                {
                case SDL_BUTTON_LEFT:
                    g_mousekeys[MOUSE_LEFT] = true;
                    g_moved = false;

                    ev.type = INEV_MOUSEDOWN;
                    ev.key = MOUSE_LEFT;
                    ev.amount = 1;
                    ev.x = g_mouse.x;
                    ev.y = g_mouse.y;

                    gui->inev(&ev);

                    g_keyintercepted = ev.intercepted;
                    break;
                case SDL_BUTTON_RIGHT:
                    g_mousekeys[MOUSE_RIGHT] = true;

                    ev.type = INEV_MOUSEDOWN;
                    ev.key = MOUSE_RIGHT;
                    ev.amount = 1;
                    ev.x = g_mouse.x;
                    ev.y = g_mouse.y;

                    gui->inev(&ev);
                    break;
                case SDL_BUTTON_MIDDLE:
                    g_mousekeys[MOUSE_MIDDLE] = true;

                    ev.type = INEV_MOUSEDOWN;
                    ev.key = MOUSE_MIDDLE;
                    ev.amount = 1;
                    ev.x = g_mouse.x;
                    ev.y = g_mouse.y;

                    gui->inev(&ev);
                    break;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                switch (e.button.button)
                {
                case SDL_BUTTON_LEFT:
                    g_mousekeys[MOUSE_LEFT] = false;

                    ev.type = INEV_MOUSEUP;
                    ev.key = MOUSE_LEFT;
                    ev.amount = 1;
                    ev.x = g_mouse.x;
                    ev.y = g_mouse.y;

                    gui->inev(&ev);
                    break;
                case SDL_BUTTON_RIGHT:
                    g_mousekeys[MOUSE_RIGHT] = false;

                    ev.type = INEV_MOUSEUP;
                    ev.key = MOUSE_RIGHT;
                    ev.amount = 1;
                    ev.x = g_mouse.x;
                    ev.y = g_mouse.y;

                    gui->inev(&ev);
                    break;
                case SDL_BUTTON_MIDDLE:
                    g_mousekeys[MOUSE_MIDDLE] = false;

                    ev.type = INEV_MOUSEUP;
                    ev.key = MOUSE_MIDDLE;
                    ev.amount = 1;
                    ev.x = g_mouse.x;
                    ev.y = g_mouse.y;

                    gui->inev(&ev);
                    break;
                }
                break;
            case SDL_MOUSEMOTION:
                //g_mouse.x = e.motion.x;
                //g_mouse.y = e.motion.y;

                if(g_mouseout)
                {
                    //TrackMouse();
                    g_mouseout = false;
                }
                if(MousePosition())
                {
                    g_moved = true;

                    ev.type = INEV_MOUSEMOVE;
                    ev.x = g_mouse.x;
                    ev.y = g_mouse.y;

                    gui->inev(&ev);
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
	g_applog << "Log start"    << std::endl; /* TODO, include date */
	g_applog << "Init: "       << std::endl;
	g_applog.flush();

	Init();

#ifdef PLATFORM_LINUX
	gtk_init(&argc, &argv);	//after sdl init
#endif

	g_applog << "MakeWindow: " << std::endl;
	g_applog.flush();

	MakeWindow(TITLE);

	g_applog << "Queue: "      << std::endl;
	g_applog.flush();

	//SDL_ShowCursor(false);
	Queue();

	g_applog << "FillGUI: "    << std::endl;
	g_applog.flush();

	FillGUI();

	g_applog << "EventLoop: "  << std::endl;
	g_applog.flush();

	EventLoop();

	g_applog << "Deinit: "     << std::endl;
	g_applog.flush();

	Deinit();
	//SDL_ShowCursor(true);

	return 0;
}



