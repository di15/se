

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

APPMODE g_mode = LOADING;
bool g_mouseout = false;
bool g_gameover = false;
bool g_moved = false;

void TrackMouse()
{
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(tme);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.dwHoverTime = HOVER_DEFAULT;	//100;
	tme.hwndTrack = g_hWnd;
	TrackMouseEvent(&tme);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_SYSCOMMAND:
		{
			switch (wParam)
			{
				case SC_SCREENSAVE: // Screensaver trying to start?
				case SC_MONITORPOWER: // Monitor trying to enter powersave?
				return 0; // Prevent from happening
			}
			break;
		}
			
		case WM_CREATE:
		{
			if(g_fullscreen)
				CenterMoUseS();
		}break;

		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}

		case WM_MOUSELEAVE:
		{
			//TrackMouse();
			g_mouseout = true;
		}break;

		case WM_MOUSEHOVER:
		{
			TrackMouse();
			g_mouseout = false;
		}break;

		case WM_LBUTTONDOWN:
		{
			g_mousekeys[0] = true;
			g_moved = false;
			g_GUI.lbuttondown();
		}break;
			
		case WM_LBUTTONUP:
		{
			g_mousekeys[0] = false;
			g_GUI.lbuttonup(g_moved);
		}break;
		
		case WM_RBUTTONDOWN:
		{
			g_mousekeys[2] = true;
			g_moved = false;
			g_GUI.rbuttondown();
		}break;
			
		case WM_RBUTTONUP:
		{
			g_mousekeys[2] = false;
			g_GUI.rbuttonup(g_moved);
		}break;
		
		case WM_MBUTTONDOWN:
		{
			g_mousekeys[1] = true;
			g_moved = false;
			g_GUI.mbuttondown();
		}break;
			
		case WM_MBUTTONUP:
		{
			g_mousekeys[1] = false;
			g_GUI.mbuttonup(g_moved);
		}break;

		case WM_MOUSEMOVE:
		{
			if(g_mouseout)
			{
				TrackMouse();
				g_mouseout = false;
			}
			if(MousePosition())
			{
				g_moved = true;
				g_GUI.mousemove();
			}
		}break;

		case WM_MOUSEWHEEL:
		{
			g_GUI.mousewheel(GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
		}break;
		
		case WM_KEYDOWN:
		{
			if(!g_GUI.keydown(wParam))
				g_keys[wParam] = true;
			return 0;
		}
		
		case WM_KEYUP:
		{
			if(!g_GUI.keyup(wParam))
			g_keys[wParam] = false;	
			return 0;
		}
		
		case WM_CHAR:
		{
			g_GUI.charin(wParam);
			return 0;
		}

		case WM_SIZE:
		{
			Resize(LOWORD(lParam), HIWORD(lParam)); 
			//g_selectedRes.width = g_width;
			//g_selectedRes.height = g_height;
			//WriteConfig();
			return 0;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void AfterDraw(Matrix projection, Matrix viewmat, Matrix modelmat)
{
	//g_log<<"sizeof(g_scenery) = "<<sizeof(g_scenery)<<endl;

	Matrix mvp = projection;
	mvp.postMultiply(viewmat);
	mvp.postMultiply(modelmat);
	
	//DrawSelection(projection, viewmat, modelmat);
	
	UseS(SHADER_COLOR3D);
    glUniformMatrix4fv(g_shader[SHADER_COLOR3D].m_slot[SSLOT_PROJECTION], 1, 0, projection.m_matrix);
	glUniformMatrix4fv(g_shader[SHADER_COLOR3D].m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);
	glUniformMatrix4fv(g_shader[SHADER_COLOR3D].m_slot[SSLOT_VIEWMAT], 1, 0, viewmat.m_matrix);
	glEnableVertexAttribArray(g_shader[SHADER_COLOR3D].m_slot[SSLOT_POSITION]);
	glUniform4f(g_shader[SHADER_COLOR3D].m_slot[SSLOT_COLOR], 1, 1, 1, 1);
	//DrawGrid();
	//DrawUnitSquares();
	//DrawPaths();
	//DrawVelocities();

	UseS(SHADER_BILLBOARD);
    glUniformMatrix4fv(g_shader[SHADER_BILLBOARD].m_slot[SSLOT_PROJECTION], 1, 0, projection.m_matrix);
	glUniformMatrix4fv(g_shader[SHADER_BILLBOARD].m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);
	glUniformMatrix4fv(g_shader[SHADER_BILLBOARD].m_slot[SSLOT_VIEWMAT], 1, 0, viewmat.m_matrix);
	glUniform4f(g_shader[SHADER_BILLBOARD].m_slot[SSLOT_COLOR], 1, 1, 1, 1);
	glEnableVertexAttribArray(g_shader[SHADER_BILLBOARD].m_slot[SSLOT_POSITION]);
	glEnableVertexAttribArray(g_shader[SHADER_BILLBOARD].m_slot[SSLOT_TEXCOORD0]);
	//glEnableVertexAttribArray(g_shader[SHADER_BILLBOARD].m_slot[SSLOT_NORMAL]);
    glActiveTextureARB(GL_TEXTURE0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//DrawOrders();
	//DrawProjectiles();
	//SortParticles();
	//DrawParticles();
	//BeginVertexArrays();
	SortBillboards();
	DrawBillboards();
	//EndVertexArrays();
	
	Ortho(g_width, g_height, 1, 1, 1, 1);
	
	//DrawUnitStatus(mvp);
	//DrawBStatus(mvp);
	//DrawTransactions(mvp);
	
	UseS(SHADER_COLOR2D);
    glUniform1f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_WIDTH], (float)g_width);
    glUniform1f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_HEIGHT], (float)g_height);
    glUniform4f(g_shader[SHADER_COLOR2D].m_slot[SSLOT_COLOR], 1, 1, 1, STATUS_ALPHA);
    glEnableVertexAttribArray(g_shader[SHADER_COLOR2D].m_slot[SSLOT_POSITION]);

	//DrawUnitStats(mvp);
	//DrawBStats(mvp);
}

void Draw()
{
	if(g_mode != EDITOR)
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	else
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	g_GUI.frameupd();
	g_GUI.draw();
	//DrawEdMap(&g_edmap);

	SwapBuffers(g_hDC);
}

void DrawScene(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3])
{
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
	
	UseShadow(SHADER_MAP, projection, viewmat, modelmat, modelviewinv, mvLightPos, lightDir);
	
	glActiveTextureARB(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, g_depth);
	glUniform1iARB(g_shader[g_curS].m_slot[SSLOT_SHADOWMAP], 4);

	//if(g_model[0].m_on)
	//	g_model[0].draw(0, Vec3f(0,0,0), 0);

	DrawEdMap(&g_edmap, g_showsky);/*
	for(int i=0; i<10; i++)
		for(int j=0; j<5; j++)
			g_model[themodel].draw(0, Vec3f(-5*180 + 180*i,0,-2.5f*90 + j*90), 0);*/

	if(g_mode == EDITOR)
	{

	}
}

void DrawSceneDepth()
{
	//g_model[themodel].draw(0, Vec3f(0,0,0), 0);
	
	DrawModelHolders();
	
	//if(g_model[0].m_on)
	//	g_model[0].draw(0, Vec3f(0,0,0), 0);

	DrawEdMap(&g_edmap, false);/*
	for(int i=0; i<10; i++)
		for(int j=0; j<5; j++)
			g_model[themodel].draw(0, Vec3f(-5*180 + 180*i,0,-2.5f*90 + j*90), 0);*/
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
	case 0: if(!Load1Model()) stage++; break;
	case 1: if(!Load1Texture()) stage++; break;
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
	if(g_keys['W'])
	{
		g_camera.accelerate(50.0f/g_zoom);
	}
	if(g_keys['S'])
	{
		g_camera.accelerate(-50.0f/g_zoom);
	}
	
	if(g_keys['A'])
	{
		g_camera.accelstrafe(-50.0f/g_zoom);
	}
	if(g_keys['D'])
	{
		g_camera.accelstrafe(50.0f/g_zoom);
	}
	
	if(g_keys['R'])
	{
		g_camera.accelrise(25.0f/g_zoom);
	}
	if(g_keys['F'])
	{
		g_camera.accelrise(-25.0f/g_zoom);
	}
	
	g_camera.frameupd();
	g_camera.friction2();
}

void UpdateRender()
{

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

void Init()
{
	// Check note in Init() in game/gmain.cpp - EDIT: removed
	char exepath[MAX_PATH+1];
	ExePath(exepath);
	SetCurrentDirectory(exepath);

	OpenLog("log.txt", VERSION);

	srand(GetTickCount());

	//if(SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, true) == false)
    //{
    //    g_log<<"Unable to install handler!"<<endl;
    //}

	g_lastsave[0] = '\0';
	
	LoadConfig();
	//EnumerateMaps();
	EnumerateDisplay();
	//MapKeys();
	//InitPlayers();
	//InitResources();
	//InitScenery();
	//InitMaps();
	//InitUnits();
	//InitRoads();
	//InitPowerlines();
	//InitPipelines();
	//InitProfiles();

	InitOnce();
}

void Deinit()
{
	//FreeScript();
	DestroyEntities();
}

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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{	
	MSG msg;
	g_hInstance = hInstance;

	Init();
	
	if(!MakeWindow(TEXT(TITLE), LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_HAMMER)), &WndProc))
		return 0;
	
	Queue();
	FillGUI();
	//Click_NewBrush();

	while(!g_quit)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
				g_quit = true;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if(g_mode == LOADING || g_mode == RENDERING || AnimateNextFrame(FRAME_RATE))
			{
				CalculateFrameRate();
				ScoreFPS();
				Update();
				Draw();
			}
			else
			{
				Sleep(1);
			}
		}
	}

	DestroyWindow(TEXT(TITLE));
	Deinit();

	return msg.wParam;
}


