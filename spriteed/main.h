

#ifndef MAIN_H
#define MAIN_H

#include "../common/platform.h"

#define VERSION				3
#define TITLE				"DMD Sprite Editor"
#define CONFIGFILE			"config.ini"

extern bool g_mouseout;
extern bool g_gameover;

enum APPMODE{LOADING, LOGO, EDITOR, RENDERING, PREREND_ADJFRAME};
extern APPMODE g_mode;
extern int g_reStage;

class Matrix;

void WriteConfig();
void DrawScene(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3]);
void DrawSceneTeam(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float mvLightPos[3], float lightDir[3]);
void DrawSceneDepth();
void Draw();

#endif
