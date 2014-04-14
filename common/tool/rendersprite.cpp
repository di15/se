

#include "../platform.h"
#include "../save/savesprite.h"
#include "rendersprite.h"
#include "../../spriteed/main.h"
#include "../gui/gui.h"
#include "../save/compilemap.h"
#include "../../spriteed/main.h"

char g_renderpath[MAX_PATH+1];
char g_renderbasename[MAX_PATH+1];

void PrepareRender(const char* fullpath)
{
	g_mode = RENDERING;
	OpenSoleView("render");
	//glClearColor(255.0f/255.0f, 127.0f/255.0f, 255.0f/255.0f, 1.0f);
	g_renderframe = 0;
	ResetView();
}

void UpdateRender()
{
	ResetView();
	Draw();
	Draw();

}

void EndRender()
{
	g_mode = EDITOR;
	OpenSoleView("editor");
	g_renderframe = 0;
	ResetView();
}