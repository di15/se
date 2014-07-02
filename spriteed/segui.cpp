


#include "main.h"
#include "../common/math/3dmath.h"
#include "../common/gui/gui.h"
#include "../common/texture.h"
#include "segui.h"
#include "../common/window.h"
#include "seviewport.h"
#include "sesim.h"
#include "../common/save/save.h"
#include "../common/save/saveedm.h"
#include "../common/sim/sim.h"
#include "../common/draw/sortb.h"
#include "undo.h"
#include "../common/save/compilemap.h"
#include "../common/draw/shadow.h"
#include "../common/sim/tile.h"
#include "../common/save/modelholder.h"
#include "../common/tool/compilebl.h"
#include "../common/save/savesprite.h"
#include "../common/tool/rendersprite.h"
#include "../common/draw/screenshot.h"
#include "../common/gui/widgets/choosefile.h"

int g_projtype = PROJ_ORTHO;
bool g_showsky = false;
float g_snapgrid = 25;
char g_lastsave[MAX_PATH+1];

void AnyKeyDown(int k)
{
	SkipLogo();
}

void AnyKeyUp(int k)
{
}

void Escape()
{
	if(g_mode != EDITOR)
		return;
}

void Delete()
{
	if(g_mode != EDITOR)
		return;

	if(g_selB.size() > 0 || g_selM.size() > 0)
		LinkPrevUndo();

	for(int i=0; i<g_selB.size(); i++)
	{
		for(auto j=g_edmap.m_brush.begin(); j!=g_edmap.m_brush.end(); j++)
		{
			if(g_selB[i] == &*j)
			{
				g_edmap.m_brush.erase(j);
				break;
			}
		}
	}

	g_selB.clear();
	g_sel1b = NULL;

	CloseSideView();

	for(int i=0; i<g_selM.size(); i++)
	{
		for(auto j=g_modelholder.begin(); j!=g_modelholder.end(); j++)
		{
			if(g_selM[i] == &*j)
			{
				g_modelholder.erase(j);
				break;
			}
		}
	}

	g_selM.clear();
	g_sel1m = NULL;

	//LinkLatestUndo();
	//g_applog<<"delete b"<<std::endl;
	//g_applog.flush();
}

void MouseLeftButtonDown()
{
	if(g_mode == LOGO)
		SkipLogo();
	else if(g_mode == EDITOR)
	{

	}
}

void MouseLeftButtonUp()
{
}

void Change_RotDeg(int dummy)
{
}

void Change_Zoom(int dummy)
{
	Widget* zoombox = g_GUI.get("editor")->get("top panel")->get("zoom");

	//if(zoombox->m_value.c_str()[0] == '\0')
	//	return;

	g_zoom = StrToFloat(zoombox->m_value.c_str());

	if(g_zoom <= 0.0f)
		g_zoom = 1.0f;
}

//void Change_SnapGrid(int dummy)
void Change_SnapGrid()
{
	Widget* snapgbox = g_GUI.get("editor")->get("top panel")->get("snapgrid");

	//if(snapgbox->m_value.c_str()[0] == '\0')
	//	return;

	//g_snapgrid = StrToFloat(snapgbox->m_value.c_str());
	//int power = snapgbox->m_selected;
	//g_snapgrid = 400 / pow(2, power);

	float cm_scales[] = CM_SCALES;

	g_snapgrid = cm_scales[ snapgbox->m_selected ];

	if(g_snapgrid <= 0.0f)
		g_snapgrid = 1;
}

void Change_MaxElev(int dummy)
{
	Widget* maxelevbox = g_GUI.get("editor")->get("top panel")->get("maxelev");

	g_maxelev = StrToFloat(maxelevbox->m_value.c_str());
}

void SkipLogo()
{
	if(g_mode != LOGO)
		return;

	//g_mode = LOADING;
	//OpenSoleView("loading");
	g_mode = EDITOR;
	g_GUI.closeall();
	g_GUI.open("editor");
	//OpenAnotherView("brush edit");
}

void UpdateLogo()
{
	static int stage = 0;
	//g_applog<<"update logo "<<stage<<std::endl;
	//g_applog.flush();

	if(stage < 60)
	{
		float a = (float)stage / 60.0f;
		g_GUI.get("logo")->get("logo")->m_rgba[3] = a;
	}
	else if(stage < 120)
	{
		float a = 1.0f - (float)(stage-60) / 60.0f;
		g_GUI.get("logo")->get("logo")->m_rgba[3] = a;
	}
	else
		SkipLogo();

	stage++;
}

void Click_NewBrush()
{
	LinkPrevUndo();
	//Vec3f pos = g_focus;
	Vec3f pos = g_camera.m_view;
	pos.x = Snap(g_snapgrid, pos.x);
	pos.y = Snap(g_snapgrid, pos.y);
	pos.z = Snap(g_snapgrid, pos.z);
	Brush b;
	BrushSide top(Vec3f(0,1,0), pos + Vec3f(0,STOREY_HEIGHT,0));
	BrushSide bottom(Vec3f(0,-1,0), pos + Vec3f(0,0,0));
	BrushSide left(Vec3f(-1,0,0), pos + Vec3f(-TILE_SIZE/2.0f,0,0));
	BrushSide right(Vec3f(1,0,0), pos + Vec3f(TILE_SIZE/2.0f,0,0));
	BrushSide front(Vec3f(0,0,1), pos + Vec3f(0,0,TILE_SIZE/2.0f));
	BrushSide back(Vec3f(0,0,-1), pos + Vec3f(0,0,-TILE_SIZE/2.0f));
	b.add(top);		//0
	b.add(bottom);	//1
	b.add(left);	//2
	b.add(right);	//3
	b.add(front);	//4
	b.add(back);	//5
	b.collapse();
	b.remaptex();
	//g_applog<<"---------push back brush-----------"<<std::endl;
	g_edmap.m_brush.push_back(b);
	//g_selB.clear();
	//auto i = g_edmap.m_brush.rbegin();
	//g_selB.push_back(&*i);

	ViewportT* t = &g_viewportT[VIEWPORT_ANGLE45O];
	//SortEdB(&g_edmap, g_focus, g_focus + t->m_offset);
	SortEdB(&g_edmap, g_camera.m_view, g_camera.m_pos);
	//LinkLatestUndo();
}

void Click_LoadEdMap()
{
	char filepath[MAX_PATH+1];
	char initdir[MAX_PATH+1];
	FullPath("projects\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("projects\\project", filepath);
	CorrectSlashes(filepath);

	if(!OpenFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	FreeEdMap(&g_edmap);

	if(LoadEdMap(filepath, &g_edmap))
		strcpy(g_lastsave, filepath);

	ViewportT* t = &g_viewportT[VIEWPORT_ANGLE45O];
	//SortEdB(&g_edmap, g_focus, g_focus + t->m_offset);
	SortEdB(&g_edmap, g_camera.m_view, g_camera.m_pos);
	ClearUndo();
}

void Click_SaveEdMap()
{
	char filepath[MAX_PATH+1];
	char initdir[MAX_PATH+1];
	FullPath("projects\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("projects\\project", filepath);
	CorrectSlashes(filepath);

	if(!SaveFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	SaveEdMap(filepath, &g_edmap);
	strcpy(g_lastsave, filepath);
}

void Click_QSaveEdMap()
{
	if(g_lastsave[0] == '\0')
	{
		Click_SaveEdMap();
		return;
	}

	SaveEdMap(g_lastsave, &g_edmap);
}

void Click_CompileMap()
{
	char filepath[MAX_PATH+1];
	char initdir[MAX_PATH+1];
	FullPath("maps\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("maps\\map", filepath);
	CorrectSlashes(filepath);

	if(!SaveFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	CompileMap(filepath, &g_edmap);
}

void Click_ExportBuildingSprite()
{
#ifdef DEMO
	MessageBox(g_hWnd, "feature disabled ;)", "demo", NULL);
#else
	char filepath[MAX_PATH+1];
	char initdir[MAX_PATH+1];
	FullPath("renders\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("renders\\building base name", filepath);
	CorrectSlashes(filepath);

	if(!SaveFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	//CompileMap(filepath, &g_edmap);
	PrepareRender(filepath, RENDER_BUILDING);
#endif
}

void Click_ExportUnitSprites()
{
#ifdef DEMO
	MessageBox(g_hWnd, "feature disabled ;)", "demo", NULL);
#else
	char filepath[MAX_PATH+1];
	char initdir[MAX_PATH+1];
	FullPath("renders\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("renders\\unit base name", filepath);
	CorrectSlashes(filepath);

	if(!SaveFileDialog(initdir, filepath))
		return;

	//CorrectSlashes(filepath);
	//CompileMap(filepath, &g_edmap);
	PrepareRender(filepath, RENDER_UNIT);
#endif
}

void RunMap(const char* full)
{
	// TO DO: launch game

	char relativeexe[] = "BattleComputer.exe";
	char fullexe[1024];
	FullPath(relativeexe, fullexe);
	char relativemap[] = "temp/tempmap";
	char fullmap[1024];
	FullPath(relativemap, fullmap);

	char cmd[2048];
	sprintf(cmd, "%s +devmap %s", fullexe, fullmap);
	system(cmd);
}

void Click_CompileRunMap()
{
	/*
	OPENFILENAME ofn;

	char filepath[MAX_PATH+1];

	ZeroMemory( &ofn , sizeof( ofn));

	char initdir[MAX_PATH+1];
	FullPath("maps\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("maps\\map", filepath);
	CorrectSlashes(filepath);

	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL  ;
	ofn.lpstrInitialDir = initdir;
	ofn.lpstrFile = filepath;
	//ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( filepath );
	//ofn.lpstrFilter = "Save\0*.map\0All\0*.*\0";
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = MAX_PATH;	//0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_OVERWRITEPROMPT;

	if(!GetSaveFileName(&ofn))
		return;

	//CorrectSlashes(filepath);
	CompileMap(filepath, &g_edmap);
	*/

	char filepath[MAX_PATH+1];
	FullPath("temp/tempmap", filepath);
	CompileMap(filepath, &g_edmap);
	RunMap(filepath);
}

void Click_BChooseTex()
{
	char filepath[MAX_PATH+1];
	char initdir[MAX_PATH+1];
	FullPath("textures\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("textures\\texture", filepath);
	CorrectSlashes(filepath);

	if(!OpenFileDialog(initdir, filepath))
		return;

	LinkPrevUndo();

	unsigned int diffuseindex;
	string relativepath = MakeRelative(filepath);
	CreateTexture(diffuseindex, relativepath.c_str(), false, true);
	unsigned int texname = g_texture[diffuseindex].texname;

	if(diffuseindex == 0)
	{
		char msg[MAX_PATH+1];
		sprintf(msg, "Couldn't load diffuse texture %s", relativepath.c_str());

		ErrorMessage("Error", msg);
	}

	char specpath[MAX_PATH+1];
	strcpy(specpath, relativepath.c_str());
	StripExtension(specpath);
	strcat(specpath, ".spec.jpg");

	unsigned int specindex;
	CreateTexture(specindex, specpath, false, true);

	if(specindex == 0)
	{
		char msg[MAX_PATH+1];
		sprintf(msg, "Couldn't load specular texture %s", specpath);

		ErrorMessage("Error", msg);
	}

	char normpath[MAX_PATH+1];
	strcpy(normpath, relativepath.c_str());
	StripExtension(normpath);
	strcat(normpath, ".norm.jpg");

	unsigned int normindex;
	CreateTexture(normindex, normpath, false, true);

	if(normindex == 0)
	{
		char msg[MAX_PATH+1];
		sprintf(msg, "Couldn't load normal texture %s", normpath);

		ErrorMessage("Error", msg);
	}

	char ownpath[MAX_PATH+1];
	strcpy(ownpath, relativepath.c_str());
	StripExtension(ownpath);
	strcat(ownpath, ".team.png");

	unsigned int ownindex;
	CreateTexture(ownindex, ownpath, false, true);

	if(ownindex == 0)
	{
		char msg[MAX_PATH+1];
		sprintf(msg, "Couldn't load team color texture %s", normpath);

		ErrorMessage("Error", msg);
	}

	if(g_sel1b == NULL)
	{
		for(auto i=g_selB.begin(); i!=g_selB.end(); i++)
		{
			Brush* b = *i;

			b->m_texture = diffuseindex;

			for(int j=0; j<b->m_nsides; j++)
			{
				BrushSide* s = &b->m_sides[j];
				s->m_diffusem = diffuseindex;
				s->m_specularm = specindex;
				s->m_normalm = normindex;
				s->m_ownerm = ownindex;
			}
		}
	}
	else
	{
		Brush* b = g_sel1b;

		b->m_texture = diffuseindex;

		if(g_dragS >= 0)
		{
			BrushSide* s = &b->m_sides[g_dragS];
			s->m_diffusem = diffuseindex;
			s->m_specularm = specindex;
			s->m_normalm = normindex;
			s->m_ownerm = ownindex;
		}
		else
		{
			for(int i=0; i<b->m_nsides; i++)
			{
				BrushSide* s = &b->m_sides[i];
				s->m_diffusem = diffuseindex;
				s->m_specularm = specindex;
				s->m_normalm = normindex;
				s->m_ownerm = ownindex;
			}
		}
	}
}

void Click_FitToFace()
{
	LinkPrevUndo();

	if(g_sel1b == NULL)
	{
		for(auto i=g_selB.begin(); i!=g_selB.end(); i++)
		{
			Brush* b = *i;

			for(int j=0; j<b->m_nsides; j++)
				b->m_sides[j].fittex();
		}
	}
	else
	{
		Brush* b = g_sel1b;

		if(g_dragS >= 0)
			b->m_sides[g_dragS].fittex();
		else
		{
			for(int i=0; i<b->m_nsides; i++)
				b->m_sides[i].fittex();
		}
	}

	RedoBSideGUI();
}

void CloseSideView()
{
	g_GUI.close("brush side edit");
	g_GUI.close("brush edit");
	g_GUI.close("door edit");
}

void Click_DoorView()
{
	CloseSideView();
#if 0
	OpenAnotherView("door edit");
#endif
}

void CopyBrush()
{
	if(g_selB.size() <= 0 && g_selM.size() <= 0)
		return;

	if(g_selB.size() > 0)
	{
		g_copyB = *g_selB[0];
		g_copyM.model = -1;
	}
	else if(g_selM.size() > 0)
	{
		g_copyM = *g_selM[0];
		g_copyB.m_nsides = 0;
	}

	//g_applog<<"copy brush"<<std::endl;
	//g_applog.flush();
}

void PasteBrush()
{
	//g_applog<<"paste brush?"<<std::endl;
	//g_applog.flush();

	if(g_copyB.m_nsides > 0)
	{
		LinkPrevUndo();

		//Vec3f pos = g_focus;
		Vec3f pos = g_camera.m_view;
		pos.x = Snap(g_snapgrid, pos.x);
		pos.y = Snap(g_snapgrid, pos.y);
		pos.z = Snap(g_snapgrid, pos.z);
		Brush b = g_copyB;
		//b.moveto(pos);
	/*
		for(int i=0; i<b.m_nsides; i++)
		{
			BrushSide* s = &b.m_sides[i];

			g_applog<<"side"<<i<<" plane="<<s->m_plane.m_normal.x<<","<<s->m_plane.m_normal.y<<","<<s->m_plane.m_normal.z<<","<<s->m_plane.m_d<<std::endl;
		}
		*/
		g_edmap.m_brush.push_back(b);

		//g_applog<<"paste brush"<<std::endl;
		//g_applog.flush();
	}
	else if(g_copyM.model >= 0)
	{
		LinkPrevUndo();

		g_modelholder.push_back(g_copyM);
	}
}

void Down_C()
{
	if(g_mode != EDITOR)
		return;

	if(g_keys[SDL_SCANCODE_LCTRL] || g_keys[SDL_SCANCODE_RCTRL])
		CopyBrush();
}

void Down_V()
{
	if(g_mode != EDITOR)
		return;

	if(g_keys[SDL_SCANCODE_LCTRL] || g_keys[SDL_SCANCODE_RCTRL])
		PasteBrush();
}

void Click_CutBrush()
{
	g_edtool = EDTOOL_CUT;
}

bool OpenFileDialog(char* initdir, char* filepath)
{
#ifdef PLATFORM_WIN
	OPENFILENAME ofn;
	ZeroMemory( &ofn , sizeof( ofn));

	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL;
	ofn.lpstrInitialDir = initdir;
	ofn.lpstrFile = filepath;
	//ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = MAX_PATH;
	//ofn.lpstrFilter = "Save\0*.map\0All\0*.*\0";
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = MAX_PATH;	//0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

	if(!GetOpenFileName(&ofn))
		return false;

	return true;
#elif defined( PLATFORM_LINUX )
    GtkWidget *dialog;

    dialog = gtk_file_chooser_dialog_new ("Open File",
    NULL,
    GTK_FILE_CHOOSER_ACTION_OPEN,
    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
    NULL);

    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), initdir);

    char initfile[MAX_PATH+1];
    strcpy(initfile, filepath);
    StripPath(initfile);

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), initfile);

    //gtk_widget_show_all (dialog);
    //gtk_dialog_run (GTK_DIALOG (dialog));

#if 1
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename;

        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        //open_file (filename);

        strcpy(filepath, filename);

        g_free (filename);

		gtk_widget_destroy (dialog);

		while (gtk_events_pending ())
			gtk_main_iteration ();

        return true;
    }
#endif
    //gtk_widget_hide(dialog);
    gtk_widget_destroy (dialog);

    while (gtk_events_pending ())
        gtk_main_iteration ();

	return false;
#endif
}

bool SaveFileDialog(char* initdir, char* filepath)
{
#ifdef PLATFORM_WIN
	OPENFILENAME ofn;
	ZeroMemory( &ofn , sizeof( ofn));

	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL  ;
	ofn.lpstrInitialDir = initdir;
	ofn.lpstrFile = filepath;
	//ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = MAX_PATH;
	//ofn.lpstrFilter = "Save\0*.map\0All\0*.*\0";
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = MAX_PATH;	//0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_OVERWRITEPROMPT;

	if(!GetSaveFileName(&ofn))
		return false;
	
	return true;
#elif defined( PLATFORM_LINUX )
    GtkWidget *dialog;

    dialog = gtk_file_chooser_dialog_new ("Save File",
    NULL,
    GTK_FILE_CHOOSER_ACTION_OPEN,
    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
    GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
    NULL);

	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER(dialog), TRUE);

    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), initdir);

    char initfile[MAX_PATH+1];
    strcpy(initfile, filepath);
    StripPath(initfile);

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), initfile);

    //gtk_widget_show_all (dialog);
    //gtk_dialog_run (GTK_DIALOG (dialog));

#if 1
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename;

        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        //open_file (filename);

        strcpy(filepath, filename);

        g_free (filename);

		gtk_widget_destroy (dialog);

		while (gtk_events_pending ())
			gtk_main_iteration ();

        return true;
    }
#endif
    //gtk_widget_hide(dialog);
    gtk_widget_destroy (dialog);

    while (gtk_events_pending ())
        gtk_main_iteration ();

	return false;
#endif
}

void Click_AddMS3D()
{
	char filepath[MAX_PATH+1];

	char initdir[MAX_PATH+1];
	FullPath("models\\", initdir);
	CorrectSlashes(initdir);
	//strcpy(filepath, initdir);
	FullPath("models\\model", filepath);
	CorrectSlashes(filepath);

	if(!OpenFileDialog(initdir, filepath))
		return;

	string relative = MakeRelative(filepath);

	g_applog<<"relative "<<relative<<std::endl;

	int modelid = LoadModel(relative.c_str(), Vec3f(1,1,1), Vec3f(0,0,0), true);

	if(modelid < 0)
	{
		char msg[MAX_PATH+1];
		sprintf(msg, "Couldn't load model %s", relative.c_str());

		ErrorMessage("Error", msg);

		return;
	}

	LinkPrevUndo();

	ModelHolder mh(modelid, Vec3f(0,0,0));
	g_modelholder.push_back(mh);
}

void RotateModels(float degrees, float xaxis, float yaxis, float zaxis)
{
	LinkPrevUndo();

	for(auto mhiter = g_selM.begin(); mhiter != g_selM.end(); mhiter++)
	{
		ModelHolder* pmh = *mhiter;

		pmh->rotdegrees = pmh->rotdegrees + Vec3f( degrees*xaxis, degrees*yaxis, degrees*zaxis );
		pmh->retransform();
	}
}

void RotateBrushes(float radians, Vec3f axis)
{
	LinkPrevUndo();
	//EdMap* m = &g_edmap;

	//for(auto i=map->m_brush.begin(); i!=map->m_brush.end(); i++)
	for(auto i=g_selB.begin(); i!=g_selB.end(); i++)
	{
		Brush* b = *i;
		Vec3f centroid(0,0,0);

		for(int j=0; j<b->m_nsides; j++)
		{
			BrushSide* s = &b->m_sides[j];

			centroid = centroid + s->m_centroid;
		}

		centroid = centroid / (float)b->m_nsides;

		list<float> oldus;
		list<float> oldvs;

		for(int j=0; j<b->m_nsides; j++)
		{
			BrushSide* s = &b->m_sides[j];

			RotatePlane(s->m_plane, centroid, radians, axis);
			//RotatePlane(s->m_tceq[0], centroid, radians, axis);
			//RotatePlane(s->m_tceq[1], centroid, radians, axis);
			//s->m_tceq[0].m_normal = Rotate(s->m_tceq[0].m_normal, radians, axis.x, axis.y, axis.z);
			//s->m_tceq[1].m_normal = Rotate(s->m_tceq[1].m_normal, radians, axis.x, axis.y, axis.z);
			//RotatePlane(s->m_tceq[0], s->m_centroid, radians, axis);
			//RotatePlane(s->m_tceq[1], s->m_centroid, radians, axis);
			//Vec3f sharedv = b->m_sharedv[ s->m_vindices[0] ];
			//s->m_tceq[0].m_normal = RotateAround(s->m_tceq[0].m_normal, sharedv, radians, axis.x, axis.y, axis.z);
			//s->m_tceq[1].m_normal = RotateAround(s->m_tceq[1].m_normal, sharedv, radians, axis.x, axis.y, axis.z);
			//Vec3f sharedv = b->m_sharedv[ s->m_vindices[0] ];
			//Vec3f offcenter = sharedv - centroid;
			//Vec3f newoffcenter = Rotate(offcenter, radians, axis.x, axis.y, axis.z);
			//float uminusd = sharedv.x*s->m_tceq[0].m_normal.x + sharedv.y*s->m_tceq[0].m_normal.y + sharedv.z*s->m_tceq[0].m_normal.z;
			//float vminusd = sharedv.x*s->m_tceq[1].m_normal.x + sharedv.y*s->m_tceq[1].m_normal.y + sharedv.z*s->m_tceq[1].m_normal.z;
			//s->m_tceq[0].m_normal = Rotate(s->m_tceq[0].m_normal - offcenter, radians, axis.x, axis.y, axis.z) + newoffcenter;
			//s->m_tceq[1].m_normal = Rotate(s->m_tceq[1].m_normal - offcenter, radians, axis.x, axis.y, axis.z) + newoffcenter;
			//s->m_tceq[0].m_normal = Rotate(s->m_tceq[0].m_normal, radians, axis.x, axis.y, axis.z);
			//s->m_tceq[1].m_normal = Rotate(s->m_tceq[1].m_normal, radians, axis.x, axis.y, axis.z);
			//float newuminusd = sharedv.x*s->m_tceq[0].m_normal.x + sharedv.y*s->m_tceq[0].m_normal.y + sharedv.z*s->m_tceq[0].m_normal.z;
			//float newvminusd = sharedv.x*s->m_tceq[1].m_normal.x + sharedv.y*s->m_tceq[1].m_normal.y + sharedv.z*s->m_tceq[1].m_normal.z;
			//float uscale = uminusd / newuminusd;
			//float vscale = vminusd / newvminusd;
			//s->m_tceq[0].m_normal = s->m_tceq[0].m_normal * uscale;
			//s->m_tceq[1].m_normal = s->m_tceq[1].m_normal * vscale;
			/*
			Vec3f sharedv = b->m_sharedv[ s->m_vindices[0] ];
			float oldu = sharedv.x*s->m_tceq[0].m_normal.x + sharedv.y*s->m_tceq[0].m_normal.y + sharedv.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
			float oldv = sharedv.x*s->m_tceq[1].m_normal.x + sharedv.y*s->m_tceq[1].m_normal.y + sharedv.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
			s->m_tceq[0].m_normal = Rotate(s->m_tceq[0].m_normal, radians, axis.x, axis.y, axis.z);
			s->m_tceq[1].m_normal = Rotate(s->m_tceq[1].m_normal, radians, axis.x, axis.y, axis.z);
			float newu = sharedv.x*s->m_tceq[0].m_normal.x + sharedv.y*s->m_tceq[0].m_normal.y + sharedv.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
			float newv = sharedv.x*s->m_tceq[1].m_normal.x + sharedv.y*s->m_tceq[1].m_normal.y + sharedv.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
			float changeu = newu - oldu;
			float changev = newv - oldv;
			s->m_tceq[0].m_d += changeu;
			s->m_tceq[1].m_d += changev;
			*/
			Vec3f sharedv = b->m_sharedv[ s->m_vindices[0] ];
			float u = sharedv.x*s->m_tceq[0].m_normal.x + sharedv.y*s->m_tceq[0].m_normal.y + sharedv.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
			float v = sharedv.x*s->m_tceq[1].m_normal.x + sharedv.y*s->m_tceq[1].m_normal.y + sharedv.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
			oldus.push_back(u);
			oldvs.push_back(v);
			s->m_tceq[0].m_normal = Rotate(s->m_tceq[0].m_normal, radians, axis.x, axis.y, axis.z);
			s->m_tceq[1].m_normal = Rotate(s->m_tceq[1].m_normal, radians, axis.x, axis.y, axis.z);
		}

		b->collapse();

		auto oldu = oldus.begin();
		auto oldv = oldvs.begin();

		for(int j=0; j<b->m_nsides; j++, oldu++, oldv++)
		{
			BrushSide* s = &b->m_sides[j];

			Vec3f newsharedv = b->m_sharedv[ s->m_vindices[0] ];

			float newu = newsharedv.x*s->m_tceq[0].m_normal.x + newsharedv.y*s->m_tceq[0].m_normal.y + newsharedv.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
			float newv = newsharedv.x*s->m_tceq[1].m_normal.x + newsharedv.y*s->m_tceq[1].m_normal.y + newsharedv.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
			float changeu = newu - *oldu;
			float changev = newv - *oldv;
			s->m_tceq[0].m_d -= changeu;
			s->m_tceq[1].m_d -= changev;
		}

		b->remaptex();
	}
}

void Click_RotXCCW()
{
	float degrees = StrToFloat(g_GUI.get("editor")->get("top panel")->get("rotdeg")->m_value.c_str());

	if(g_selB.size() > 0)
		RotateBrushes(DEGTORAD(degrees), Vec3f(1, 0, 0));
	else if(g_selM.size() > 0)
		RotateModels(degrees, 1, 0, 0);
}

void Click_RotXCW()
{
	float degrees = StrToFloat(g_GUI.get("editor")->get("top panel")->get("rotdeg")->m_value.c_str());

	if(g_selB.size() > 0)
		RotateBrushes(DEGTORAD(-degrees), Vec3f(1, 0, 0));
	else if(g_selM.size() > 0)
		RotateModels(-degrees, 1, 0, 0);
}

void Click_RotYCCW()
{
	float degrees = StrToFloat(g_GUI.get("editor")->get("top panel")->get("rotdeg")->m_value.c_str());

	if(g_selB.size() > 0)
		RotateBrushes(DEGTORAD(degrees), Vec3f(0, 1, 0));
	else if(g_selM.size() > 0)
		RotateModels(degrees, 0, 1, 0);
}

void Click_RotYCW()
{
	float degrees = StrToFloat(g_GUI.get("editor")->get("top panel")->get("rotdeg")->m_value.c_str());

	if(g_selB.size() > 0)
		RotateBrushes(DEGTORAD(-degrees), Vec3f(0, 1, 0));
	else if(g_selM.size() > 0)
		RotateModels(-degrees, 0, 1, 0);
}

void Click_RotZCCW()
{
	float degrees = StrToFloat(g_GUI.get("editor")->get("top panel")->get("rotdeg")->m_value.c_str());

	if(g_selB.size() > 0)
		RotateBrushes(DEGTORAD(degrees), Vec3f(0, 0, 1));
	else if(g_selM.size() > 0)
		RotateModels(degrees, 0, 0, 1);
}

void Click_RotZCW()
{
	float degrees = StrToFloat(g_GUI.get("editor")->get("top panel")->get("rotdeg")->m_value.c_str());

	if(g_selB.size() > 0)
		RotateBrushes(DEGTORAD(-degrees), Vec3f(0, 0, 1));
	else if(g_selM.size() > 0)
		RotateModels(-degrees, 0, 0, 1);
}

void Click_RotateTex()
{
	LinkPrevUndo();

	float degrees = StrToFloat(g_GUI.get("editor")->get("top panel")->get("rotdeg")->m_value.c_str());

	Brush* b = g_sel1b;
	BrushSide* s = &b->m_sides[g_dragS];

	//Vec3f sharedv = b->m_sharedv[ s->m_vindices[0] ];
	float oldu = s->m_centroid.x*s->m_tceq[0].m_normal.x + s->m_centroid.y*s->m_tceq[0].m_normal.y + s->m_centroid.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
	float oldv = s->m_centroid.x*s->m_tceq[1].m_normal.x + s->m_centroid.y*s->m_tceq[1].m_normal.y + s->m_centroid.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
	Vec3f axis = s->m_plane.m_normal;
	float radians = DEGTORAD(degrees);
	s->m_tceq[0].m_normal = Rotate(s->m_tceq[0].m_normal, radians, axis.x, axis.y, axis.z);
	s->m_tceq[1].m_normal = Rotate(s->m_tceq[1].m_normal, radians, axis.x, axis.y, axis.z);

	//Vec3f newsharedv = b->m_sharedv[ s->m_vindices[0] ];
	float newu = s->m_centroid.x*s->m_tceq[0].m_normal.x + s->m_centroid.y*s->m_tceq[0].m_normal.y + s->m_centroid.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
	float newv = s->m_centroid.x*s->m_tceq[1].m_normal.x + s->m_centroid.y*s->m_tceq[1].m_normal.y + s->m_centroid.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
	float changeu = newu - oldu;
	float changev = newv - oldv;
	s->m_tceq[0].m_d -= changeu;
	s->m_tceq[1].m_d -= changev;

	s->remaptex();
}

void RedoBSideGUI()
{
	if(g_sel1b == NULL)
		return;

	if(g_dragS < 0)
		return;
	
	if(!g_GUI.get("brush side edit"))	InfoMessage("e", "no bse");
	if(!g_GUI.get("brush side edit")->get("left panel"))	InfoMessage("e", "no lp");
	if(!g_GUI.get("brush side edit")->get("left panel")->get("u equation"))	InfoMessage("e", "no ue");
	if(!g_GUI.get("brush side edit")->get("left panel")->get("v equation"))	InfoMessage("e", "no ve");

	EditBox* uwidg = (EditBox*)g_GUI.get("brush side edit")->get("left panel")->get("u equation");
	EditBox* vwidg = (EditBox*)g_GUI.get("brush side edit")->get("left panel")->get("v equation");

	Plane* tceq = g_sel1b->m_sides[g_dragS].m_tceq;

	char tceqstr[256];
	sprintf(tceqstr, "%f %f %f %f", tceq[0].m_normal.x, tceq[0].m_normal.y, tceq[0].m_normal.z, tceq[0].m_d);
	uwidg->changevalue(tceqstr);
	sprintf(tceqstr, "%f %f %f %f", tceq[1].m_normal.x, tceq[1].m_normal.y, tceq[1].m_normal.z, tceq[1].m_d);
	vwidg->changevalue(tceqstr);

	uwidg->m_scroll[0] = 0;
	vwidg->m_scroll[0] = 0;
}

void Change_TexEq(int c)
{
	if(g_sel1b == NULL)
		return;

	if(g_dragS < 0)
		return;

	EditBox* uwidg = (EditBox*)g_GUI.get("brush side edit")->get("left panel")->get("u equation");
	EditBox* vwidg = (EditBox*)g_GUI.get("brush side edit")->get("left panel")->get("v equation");

	float A = 0;
	float B = 0;
	float C = 0;
	float D = 0;

	sscanf(uwidg->m_value.c_str(), "%f %f %f %f", &A, &B, &C, &D);

	Plane* tceq = g_sel1b->m_sides[g_dragS].m_tceq;
	tceq[0].m_normal.x = A;
	tceq[0].m_normal.y = B;
	tceq[0].m_normal.z = C;
	tceq[0].m_d = D;

	A = 0;
	B = 0;
	C = 0;
	D = 0;

	sscanf(vwidg->m_value.c_str(), "%f %f %f %f", &A, &B, &C, &D);

	tceq[1].m_normal.x = A;
	tceq[1].m_normal.y = B;
	tceq[1].m_normal.z = C;
	tceq[1].m_d = D;

	g_sel1b->m_sides[g_dragS].remaptex();
}

void Change_SelComp()
{
	//RedoBSideGUI();
}

int GetComponent()
{
	int c = g_GUI.get("brush side edit")->get("left panel")->get("select component")->m_selected;

	if(c != 0 && c != 1)
		c = 0;

	return c;
}

void Click_ScaleTex()
{
	if(g_sel1b == NULL)
		return;

	if(g_dragS < 0)
		return;

	int c = GetComponent();

	float scale = 1.0f/StrToFloat(g_GUI.get("brush side edit")->get("left panel")->get("texture scale")->m_value.c_str());

	Brush* b = g_sel1b;
	BrushSide* s = &b->m_sides[g_dragS];

	float oldcomp = s->m_centroid.x*s->m_tceq[c].m_normal.x + s->m_centroid.y*s->m_tceq[c].m_normal.y + s->m_centroid.z*s->m_tceq[c].m_normal.z + s->m_tceq[c].m_d;
	s->m_tceq[c].m_normal = s->m_tceq[c].m_normal * scale;

	//Vec3f newsharedv = b->m_sharedv[ s->m_vindices[0] ];
	float newcomp = s->m_centroid.x*s->m_tceq[c].m_normal.x + s->m_centroid.y*s->m_tceq[c].m_normal.y + s->m_centroid.z*s->m_tceq[c].m_normal.z + s->m_tceq[c].m_d;
	float changecomp = newcomp - oldcomp;
	s->m_tceq[c].m_d -= changecomp;

	s->remaptex();

	RedoBSideGUI();
}

void Click_ShiftTex()
{
	if(g_sel1b == NULL)
		return;

	if(g_dragS < 0)
		return;

	int c = GetComponent();

	float shift = StrToFloat(g_GUI.get("brush side edit")->get("left panel")->get("texture shift")->m_value.c_str());

	Brush* b = g_sel1b;
	BrushSide* s = &b->m_sides[g_dragS];

	s->m_tceq[c].m_d += shift;

	s->remaptex();

	RedoBSideGUI();
}

void Change_ShowSky()
{
	Widget* showskchbox = g_GUI.get("editor")->get("top panel")->get("showsky");

	g_showsky = (bool)showskchbox->m_selected;
}

void Click_ProjPersp()
{
	g_projtype = PROJ_PERSP;
}

void Click_ProjOrtho()
{
	g_projtype = PROJ_ORTHO;
}

void Click_ResetView()
{
	ResetView();
}

void Click_Explode()
{
	g_edtool = EDTOOL_EXPLOSION;
}

void Click_SetDoor()
{
	if(g_selB.size() <= 0)
	{
		//MessageBox(g_hWnd, "NULL 1b", "asdasd", NULL);
		return;
	}

	LinkPrevUndo();

	Brush* b = *g_selB.begin();
	EdDoor* door = b->m_door;

	if(!b->m_door)
	{
		b->m_door = new EdDoor();
		door = b->m_door;

		door->axis = Vec3f(0,200,0);
		door->point = b->m_sides[2].m_centroid;
		door->startopen = false;
	}

	float opendeg = 90;

	ViewLayer* dooredview = (ViewLayer*)g_GUI.get("door edit");
	Frame* leftpanel = (Frame*)dooredview->get("left panel");
	EditBox* opendegedit = (EditBox*)leftpanel->get("opendeg");
	opendeg = StrToFloat(opendegedit->m_value.c_str());

	door->opendeg = opendeg;

	b->getsides(&door->m_nsides, &door->m_sides);

#if 0
	g_applog<<"set ed door"<<std::endl;
	for(int i=0; i<door->m_nsides; i++)
	{
		g_applog<<"side "<<i<<std::endl;
		Plane* p = &door->m_sides[i].m_plane;

		g_applog<<"plane = "<<p->m_normal.x<<","<<p->m_normal.y<<","<<p->m_normal.z<<",d="<<p->m_d<<std::endl;
	}
#endif
}

void Click_UnmakeDoor()
{
	if(g_selB.size() <= 0)
	{
		//MessageBox(g_hWnd, "NULL 1b", "asdasd", NULL);
		return;
	}

	Brush* b = *g_selB.begin();

	if(b->m_door)
	{
		LinkPrevUndo();
		delete [] b->m_door;
		b->m_door = NULL;
	}
}

void Click_OpenCloseDoor()
{
	if(g_selB.size() <= 0)
		return;

	Brush* b = *g_selB.begin();
	EdDoor* door = b->m_door;

	if(!b->m_door)
		return;

	LinkPrevUndo();

	Brush transformed;
	transformed.setsides(door->m_nsides, door->m_sides);
	transformed.collapse();
	transformed.remaptex();

	door->startopen = !door->startopen;

	if(door->startopen)
	{
		//transform

		Vec3f centroid = door->point;
		Vec3f axis = Normalize(door->axis);
		float radians = DEGTORAD(door->opendeg);

		//char msg[128];
		//sprintf(msg, "point(%f,%f,%f) axis(%f,%f,%f)

		list<float> oldus;
		list<float> oldvs;

		for(int j=0; j<transformed.m_nsides; j++)
		{
			BrushSide* s = &transformed.m_sides[j];
			RotatePlane(s->m_plane, centroid, radians, axis);

			Vec3f sharedv = transformed.m_sharedv[ s->m_vindices[0] ];
			float u = sharedv.x*s->m_tceq[0].m_normal.x + sharedv.y*s->m_tceq[0].m_normal.y + sharedv.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
			float v = sharedv.x*s->m_tceq[1].m_normal.x + sharedv.y*s->m_tceq[1].m_normal.y + sharedv.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
			oldus.push_back(u);
			oldvs.push_back(v);
			s->m_tceq[0].m_normal = Rotate(s->m_tceq[0].m_normal, radians, axis.x, axis.y, axis.z);
			s->m_tceq[1].m_normal = Rotate(s->m_tceq[1].m_normal, radians, axis.x, axis.y, axis.z);
		}

		//transformed.collapse();

		auto oldu = oldus.begin();
		auto oldv = oldvs.begin();

		for(int j=0; j<transformed.m_nsides; j++, oldu++, oldv++)
		{
			BrushSide* s = &transformed.m_sides[j];

			Vec3f newsharedv = transformed.m_sharedv[ s->m_vindices[0] ];

			float newu = newsharedv.x*s->m_tceq[0].m_normal.x + newsharedv.y*s->m_tceq[0].m_normal.y + newsharedv.z*s->m_tceq[0].m_normal.z + s->m_tceq[0].m_d;
			float newv = newsharedv.x*s->m_tceq[1].m_normal.x + newsharedv.y*s->m_tceq[1].m_normal.y + newsharedv.z*s->m_tceq[1].m_normal.z + s->m_tceq[1].m_d;
			float changeu = newu - *oldu;
			float changev = newv - *oldv;
			s->m_tceq[0].m_d -= changeu;
			s->m_tceq[1].m_d -= changev;
		}

		//transformed.remaptex();
	}

	b->setsides(transformed.m_nsides, transformed.m_sides);
	b->collapse();
	b->remaptex();
}

void Resize_Logo(Widget* thisw)
{
}

void Resize_LoadingText(Widget* thisw)
{
	thisw->m_pos[0] = g_width * 0.4f;
	thisw->m_pos[1] = g_height * 0.5f;
}

void Resize_TopPanel(Widget* thisw)
{
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = TOP_PANEL_HEIGHT;
}

void Resize_LeftPanel(Widget* thisw)
{
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = TOP_PANEL_HEIGHT;
	thisw->m_pos[2] = LEFT_PANEL_WIDTH;
	thisw->m_pos[3] = g_height;
}

void Resize_LoadButton(Widget* thisw)
{
	int i = 0;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;

#if 0
	char msg[128];
	sprintf(msg, "pos:%f,%f,%f,%f", thisw->m_pos[0], thisw->m_pos[1], thisw->m_pos[2], thisw->m_pos[3]);
	MessageBox(g_hWnd, msg, "asd", NULL);
#endif
}

void Resize_SaveButton(Widget* thisw)
{
	int i = 1;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_QSaveButton(Widget* thisw)
{
	int i = 2;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_CompileMapButton(Widget* thisw)
{
	int i = 3;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_ExportBldgButton(Widget* thisw)
{
	int i = 4;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_ExportUnitButton(Widget* thisw)
{
	int i = 5;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_ExportTileButton(Widget* thisw)
{
	int i = 6;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_ExportRoadButton(Widget* thisw)
{
	int i = 7;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_ExportRidgeButton(Widget* thisw)
{
	int i = 8;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_ExportWaterButton(Widget* thisw)
{
	int i = 9;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_UndoButton(Widget* thisw)
{
	int i = 10;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_RedoButton(Widget* thisw)
{
	int i = 11;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_NewBrushButton(Widget* thisw)
{
	int i = 12;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_CutBrushButton(Widget* thisw)
{
	int i = 13;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_NewEntityButton(Widget* thisw)
{
	int i = 14;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_RotXCCWButton(Widget* thisw)
{
	int i = 15;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_RotXCWButton(Widget* thisw)
{
	int i = 16;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_RotYCCWButton(Widget* thisw)
{
	int i = 17;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_RotYCWButton(Widget* thisw)
{
	int i = 18;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_RotZCCWButton(Widget* thisw)
{
	int i = 19;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_RotZCWButton(Widget* thisw)
{
	int i = 20;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32;
}

void Resize_ViewportsFrame(Widget* thisw)
{
	thisw->m_pos[0] = LEFT_PANEL_WIDTH;
	thisw->m_pos[1] = TOP_PANEL_HEIGHT;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_height;
}

void Resize_TopLeftViewport(Widget* thisw)
{
	Widget* parentw = thisw->m_parent;
	thisw->m_pos[0] = parentw->m_pos[0];
	thisw->m_pos[1] = parentw->m_pos[1];
	thisw->m_pos[2] = (parentw->m_pos[2]+parentw->m_pos[0])/2;
	thisw->m_pos[3] = (parentw->m_pos[3]+parentw->m_pos[1])/2;
}

void Resize_BottomLeftViewport(Widget* thisw)
{
	Widget* parentw = thisw->m_parent;
	thisw->m_pos[0] = parentw->m_pos[0];
	thisw->m_pos[1] = (parentw->m_pos[3]+parentw->m_pos[1])/2;
	thisw->m_pos[2] = (parentw->m_pos[2]+parentw->m_pos[0])/2;
	thisw->m_pos[3] = parentw->m_pos[3];
}

void Resize_TopRightViewport(Widget* thisw)
{
	Widget* parentw = thisw->m_parent;
	thisw->m_pos[0] = (parentw->m_pos[2]+parentw->m_pos[0])/2;
	thisw->m_pos[1] = parentw->m_pos[1];
	thisw->m_pos[2] = parentw->m_pos[2];
	thisw->m_pos[3] = (parentw->m_pos[3]+parentw->m_pos[1])/2;
}

void Resize_BottomRightViewport(Widget* thisw)
{
	Widget* parentw = thisw->m_parent;
	thisw->m_pos[0] = (parentw->m_pos[2]+parentw->m_pos[0])/2;
	thisw->m_pos[1] = (parentw->m_pos[3]+parentw->m_pos[1])/2;
	thisw->m_pos[2] = parentw->m_pos[2];
	thisw->m_pos[3] = parentw->m_pos[3];
}

void Resize_FullViewport(Widget* thisw)
{
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = 0;
	thisw->m_pos[2] = g_width;
	thisw->m_pos[3] = g_height;
}

void Resize_HDivider(Widget* thisw)
{
	Widget* parentw = thisw->m_parent;
	thisw->m_pos[0] = parentw->m_pos[0];
	thisw->m_pos[1] = (parentw->m_pos[3]+parentw->m_pos[1])/2;
	thisw->m_pos[2] = parentw->m_pos[2];
	thisw->m_pos[3] = (parentw->m_pos[3]+parentw->m_pos[1])/2 + 1;
}

void Resize_VDivider(Widget* thisw)
{
	Widget* parentw = thisw->m_parent;
	thisw->m_pos[0] = (parentw->m_pos[2]+parentw->m_pos[0])/2;
	thisw->m_pos[1] = parentw->m_pos[1];
	thisw->m_pos[2] = (parentw->m_pos[2]+parentw->m_pos[0])/2 + 1;
	thisw->m_pos[3] = parentw->m_pos[3];
}

void Resize_RotDegEditBox(Widget* thisw)
{
	int i = 0;
	//Margin(0+32*i++), Margin(32), Margin(32+32*i++), Margin(32+16)
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32+16;
}

void Resize_RotDegText(Widget* thisw)
{
	//Margin(0+32*(i-2)), Margin(32+16)
	int i = 2;
	thisw->m_pos[0] = 0+32*(i-2);
	thisw->m_pos[1] = 32+16;
}

void Resize_ZoomEditBox(Widget* thisw)
{
	//Margin(0+32*i++), Margin(32), Margin(32+32*i++), Margin(32+16)
	int i = 2;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32+16;
}

void Resize_ZoomText(Widget* thisw)
{
	//Margin(0+32*(i-2)), Margin(32+16)
	int i = 4;
	thisw->m_pos[0] = 0+32*(i-2);
	thisw->m_pos[1] = 32+16;
}

void Resize_SnapGridEditBox(Widget* thisw)
{
	//Margin(0+32*i++), Margin(32), Margin(50+32*i++), Margin(32+16)
	int i = 4;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 50+32*i;
	thisw->m_pos[3] = 32+16;
}

void Resize_SnapGridText(Widget* thisw)
{
	//Margin(0+32*(i-2)), Margin(32+16)
	int i = 6;
	thisw->m_pos[0] = 0+32*(i-2);
	thisw->m_pos[1] = 32+16;
}

void Resize_MaxElevEditBox(Widget* thisw)
{
	//Margin(0+32*i++), Margin(32), Margin(32+32*i++), Margin(32+16)
	int i = 6;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32+16;
}

void Resize_MaxElevText(Widget* thisw)
{
	//Margin(0+32*(i-2)), Margin(32+16)
	int i = 8;
	thisw->m_pos[0] = 0+32*(i-2);
	thisw->m_pos[1] = 32+16;
}

void Resize_ShowSkyCheckBox(Widget* thisw)
{
	//Margin(0+32*i++), Margin(32), Margin(32+32*i++), Margin(32+16)
	int i = 8;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32+16;
}

void Resize_PerspProjButton(Widget* thisw)
{
	//Margin(0+32*i), Margin(32), Margin(32+32*i++), Margin(32*2)
	int i = 10;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32*2;
	CenterLabel(thisw);
}

void Resize_OrthoProjButton(Widget* thisw)
{
	//Margin(0+32*i), Margin(32), Margin(32+32*i++), Margin(32*2)
	int i = 11;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32*2;
	CenterLabel(thisw);
}

void Resize_ResetViewButton(Widget* thisw)
{
	//Margin(0+32*i), Margin(32), Margin(32+32*i++), Margin(32*2)
	int i = 12;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32*2;
	CenterLabel(thisw);
}

void Resize_FramesText(Widget* thisw)
{
	int i = 13;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32 + 16;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32*2;
}

void Resize_FramesEditBox(Widget* thisw)
{
	int i = 13;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32 + 16;
}

void Resize_ExplodeButton(Widget* thisw)
{
	int i = 14;
	thisw->m_pos[0] = 0+32*i;
	thisw->m_pos[1] = 32;
	thisw->m_pos[2] = 32+32*i;
	thisw->m_pos[3] = 32 + 32;
	CenterLabel(thisw);
}

void Change_Frames(int dummy)
{
}

void Resize_BrushEditFrame(Widget* thisw)
{
	//Margin(MARGIN_SOURCE_WIDTH, MARGIN_FUNC_PIXELS, 0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, TOP_PANEL_HEIGHT), Margin(MARGIN_SOURCE_WIDTH, MARGIN_FUNC_PIXELS, LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_RATIO, 1)
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = TOP_PANEL_HEIGHT;
	thisw->m_pos[2] = LEFT_PANEL_WIDTH;
	thisw->m_pos[3] = g_height;
}

void Resize_ChooseTexButton(Widget* thisw)
{
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*(i+1))
	int i=1;
	Font* f = &g_font[thisw->m_font];
	Widget* parentw = thisw->m_parent;
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = parentw->m_pos[1] + f->gheight*i;
	thisw->m_pos[2] = LEFT_PANEL_WIDTH;
	thisw->m_pos[3] = parentw->m_pos[1] + f->gheight*(i+1);
	CenterLabel(thisw);
}

void Resize_FitToFaceButton(Widget* thisw)
{
	//Margin(0+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(32+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i+32)
	int i=2;
	int j=0;
	Font* f = &g_font[thisw->m_font];
	Widget* parentw = thisw->m_parent;
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = parentw->m_pos[1] + f->gheight*i;
	thisw->m_pos[2] = 32+32*j;
	thisw->m_pos[3] = parentw->m_pos[1] + f->gheight*i+32;
	CenterLabel(thisw);
}

void Resize_BrushSideEditFrame(Widget* thisw)
{
	int j=1;
	//leftpanel->add(new Button(leftpanel, "door view", "gui/door.png", "", "Door view",	MAINFONT8, Margin(0+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(32+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i+32), Click_DoorView, NULL, NULL));
	int i=4;
	//Margin(MARGIN_SOURCE_WIDTH, MARGIN_FUNC_PIXELS, 0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, TOP_PANEL_HEIGHT), Margin(MARGIN_SOURCE_WIDTH, MARGIN_FUNC_PIXELS, LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_RATIO, 1)
	thisw->m_pos[0] = 0;
	thisw->m_pos[1] = TOP_PANEL_HEIGHT;
	thisw->m_pos[2] = LEFT_PANEL_WIDTH;
	thisw->m_pos[3] = g_height;
}

void Resize_RotateTexButton(Widget* thisw)
{
	int i = 4;
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = thisw->m_parent;
	Font* f = &g_font[thisw->m_font];
	thisw->m_pos[0] = parentw->m_pos[0] + 0;
	thisw->m_pos[1] = parentw->m_pos[1] + 32+f->gheight*i;
	thisw->m_pos[2] = parentw->m_pos[0] + LEFT_PANEL_WIDTH;
	thisw->m_pos[3] = parentw->m_pos[1] + 32+f->gheight*(i+1);
	CenterLabel(thisw);
}

void Resize_TexEqEditBox1(Widget* thisw)
{
	int i = 6;
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = thisw->m_parent;
	Font* f = &g_font[thisw->m_font];
	thisw->m_pos[0] = parentw->m_pos[0] + 0;
	thisw->m_pos[1] = parentw->m_pos[1] + 32+f->gheight*i;
	thisw->m_pos[2] = parentw->m_pos[0] + LEFT_PANEL_WIDTH;
	thisw->m_pos[3] = parentw->m_pos[1] + 32+f->gheight*(i+1);
}

void Resize_TexEqEditBox2(Widget* thisw)
{
	int i = 7;
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = thisw->m_parent;
	Font* f = &g_font[thisw->m_font];
	thisw->m_pos[0] = parentw->m_pos[0] + 0;
	thisw->m_pos[1] = parentw->m_pos[1] + 32+f->gheight*i;
	thisw->m_pos[2] = parentw->m_pos[0] + LEFT_PANEL_WIDTH;
	thisw->m_pos[3] = parentw->m_pos[1] + 32+f->gheight*(i+1);
}

void Resize_SelComponentDropDownS(Widget* thisw)
{
	int i = 8;
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = thisw->m_parent;
	Font* f = &g_font[thisw->m_font];
	thisw->m_pos[0] = parentw->m_pos[0] + 0;
	thisw->m_pos[1] = parentw->m_pos[1] + 32+f->gheight*i;
	thisw->m_pos[2] = parentw->m_pos[0] + LEFT_PANEL_WIDTH;
	thisw->m_pos[3] = parentw->m_pos[1] + 32+f->gheight*(i+1);
}

void Resize_TexScaleEditBox(Widget* thisw)
{
	int i = 9;
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH/2), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = thisw->m_parent;
	Font* f = &g_font[thisw->m_font];
	thisw->m_pos[0] = parentw->m_pos[0] + 0;
	thisw->m_pos[1] = parentw->m_pos[1] + 32+f->gheight*i;
	thisw->m_pos[2] = parentw->m_pos[0] + LEFT_PANEL_WIDTH/2;
	thisw->m_pos[3] = parentw->m_pos[1] + 32+f->gheight*(i+1);
}

void Resize_ScaleTexButton(Widget* thisw)
{
	int i = 9;
	//MAINFONT8, Margin(LEFT_PANEL_WIDTH/2), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = thisw->m_parent;
	Font* f = &g_font[thisw->m_font];
	thisw->m_pos[0] = parentw->m_pos[0] + LEFT_PANEL_WIDTH/2;
	thisw->m_pos[1] = parentw->m_pos[1] + 32+f->gheight*i;
	thisw->m_pos[2] = parentw->m_pos[0] + LEFT_PANEL_WIDTH;
	thisw->m_pos[3] = parentw->m_pos[1] + 32+f->gheight*(i+1);
	CenterLabel(thisw);
}

void Resize_TexShiftEditBox(Widget* thisw)
{
	int i = 10;
	//Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH/2), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = thisw->m_parent;
	Font* f = &g_font[thisw->m_font];
	thisw->m_pos[0] = parentw->m_pos[0] + 0;
	thisw->m_pos[1] = parentw->m_pos[1] + 32+f->gheight*i;
	thisw->m_pos[2] = parentw->m_pos[0] + LEFT_PANEL_WIDTH/2;
	thisw->m_pos[3] = parentw->m_pos[1] + 32+f->gheight*(i+1);
}

void Resize_TexShiftButton(Widget* thisw)
{
	int i = 10;
	//Margin(LEFT_PANEL_WIDTH/2), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 32+f->gheight*(i+1))
	Widget* parentw = thisw->m_parent;
	Font* f = &g_font[thisw->m_font];
	thisw->m_pos[0] = parentw->m_pos[0] + LEFT_PANEL_WIDTH/2;
	thisw->m_pos[1] = parentw->m_pos[1] + 32+f->gheight*i;
	thisw->m_pos[2] = parentw->m_pos[0] + LEFT_PANEL_WIDTH;
	thisw->m_pos[3] = parentw->m_pos[1] + 32+f->gheight*(i+1);
	CenterLabel(thisw);
}

void Click_CompileModel()
{
#ifdef DEMO
	MessageBox(g_hWnd, "feature disabled ;)", "demo", NULL);
#else
#ifdef PLATFORM_WIN
        OPENFILENAME ofn;

        char filepath[MAX_PATH+1];

        ZeroMemory( &ofn , sizeof( ofn));

        char initdir[MAX_PATH+1];
        FullPath("export models\\", initdir);
        CorrectSlashes(initdir);
        //strcpy(filepath, initdir);
        FullPath("export models\\export", filepath);
        CorrectSlashes(filepath);

        ofn.lStructSize = sizeof ( ofn );
        ofn.hwndOwner = NULL  ;
        ofn.lpstrInitialDir = initdir;
        ofn.lpstrFile = filepath;
        //ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof( filepath );
        //ofn.lpstrFilter = "Save\0*.bl\0All\0*.*\0";
        ofn.lpstrFilter = "All\0*.*\0";
        ofn.nFilterIndex =1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = MAX_PATH;        //0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_OVERWRITEPROMPT;

        if(!GetSaveFileName(&ofn))
                return;

        //CorrectSlashes(filepath);
        //SaveEdBuilding(filepath, &g_edbldg);
		CompileModel(filepath, &g_edmap, g_modelholder);
#elif defined( PLATFORM_LINUX )
	GtkWidget *dialog;

     dialog = gtk_file_chooser_dialog_new ("Open File",
     				      NULL,
     				      GTK_FILE_CHOOSER_ACTION_OPEN,
     				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
     				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
     				      NULL);

     if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
       {
         char *filename;

         filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
         //open_file (filename);
         g_free (filename);
       }

     gtk_widget_destroy (dialog);
#endif
#endif
}

void Resize_ChooseFile(Widget *thisw)
{
	thisw->m_pos[0] = g_width/2 - 200;
	thisw->m_pos[1] = g_height/2 - 200;
	thisw->m_pos[2] = g_width/2 + 200;
	thisw->m_pos[3] = g_height/2 + 200;
}

void Callback_ChooseFile(const char* fullpath)
{
}

void FillGUI()
{
	g_applog<<"assign keys"<<std::endl;
	g_applog.flush();

	g_GUI.assignanykey(&AnyKeyDown, &AnyKeyUp);

	g_applog<<"1,";
	g_applog.flush();

	g_GUI.assignkey(SDL_SCANCODE_F1, SaveScreenshot, NULL);
	g_applog<<"2,";
	g_applog.flush();
	g_GUI.assignkey(SDL_SCANCODE_ESCAPE, &Escape, NULL);
	g_applog<<"3,";
	g_applog.flush();
	g_GUI.assignkey(SDL_SCANCODE_DELETE, &Delete, NULL);
	g_applog<<"4,";
	g_applog.flush();
	g_GUI.assignkey(SDL_SCANCODE_C, Down_C, NULL);
	g_applog<<"5,";
	g_applog.flush();
	g_GUI.assignkey(SDL_SCANCODE_V, Down_V, NULL);
	g_applog<<"5,";
	g_applog.flush();
	//AssignMouseWheel(&MouseWheel);
	//AssignMouseMove(&MouseMove);
	g_GUI.assignlbutton(&MouseLeftButtonDown, &MouseLeftButtonUp);
	//AssignRButton(NULL, &MouseRightButtonUp);
	g_applog<<"6,"<<std::endl;
	g_applog.flush();

	Font* f = &g_font[MAINFONT8];

	g_applog<<"logo..."<<std::endl;
	g_applog.flush();

	g_GUI.add(new ViewLayer(&g_GUI, "logo"));
	ViewLayer* logoview = (ViewLayer*)g_GUI.get("logo");
	//Image::Image(Widget* parent, const char* nm, const char* filepath, bool clamp, void (*reframef)(Widget* thisw), float r, float g, float b, float a, float texleft, float textop, float texright, float texbottom) : Widget()

	logoview->add(new Image(NULL, "logo", "gui/dmd.jpg", true, Resize_Logo, 1,1,1,0));

	g_applog<<"loading..."<<std::endl;
	g_applog.flush();

	g_GUI.add(new ViewLayer(&g_GUI, "loading"));
	ViewLayer* loadingview = (ViewLayer*)g_GUI.get("loading");
	loadingview->add(new Text(NULL, "status", "Loading...", MAINFONT8, Resize_LoadingText));

	g_GUI.add(new ViewLayer(&g_GUI, "editor"));
	ViewLayer* edview = (ViewLayer*)g_GUI.get("editor");

	edview->add(new Frame(NULL, "top panel", Resize_TopPanel));
	edview->add(new Frame(NULL, "left panel", Resize_LeftPanel));

	Widget* toppanel = edview->get("top panel");
	Widget* leftpanel = edview->get("left panel");

	toppanel->add(new Image(toppanel, "top panel bg", "gui/filled.jpg", true, Resize_TopPanel));
	leftpanel->add(new Image(leftpanel, "left panel bg", "gui/filled.jpg", true, Resize_LeftPanel));

//Button(Widget* parent, const char* name, const char* filepath, const std::string label, const std::string tooltip, int f, int style, void (*reframef)(Widget* thisw), void (*click)(), void (*click2)(int p), void (*overf)(), void (*overf2)(int p), void (*out)(), int parm) : Widget()

	toppanel->add(new Button(toppanel, "load", "gui/load.png", "", "Load",												MAINFONT8, BUTTON_BGIMAGE, Resize_LoadButton, Click_LoadEdMap, NULL, NULL, NULL, NULL, -1));
	toppanel->add(new Button(toppanel, "save", "gui/save.png", "", "Save",												MAINFONT8, BUTTON_BGIMAGE, Resize_SaveButton, Click_SaveEdMap, NULL, NULL, NULL, NULL, -1));
	toppanel->add(new Button(toppanel, "qsave", "gui/qsave.png", "", "Quick save",										MAINFONT8, BUTTON_BGIMAGE, Resize_QSaveButton, Click_QSaveEdMap, NULL, NULL, NULL, NULL, -1));
#if 1
	toppanel->add(new Button(toppanel, "build", "gui/build.png", "", "Export model",									MAINFONT8, BUTTON_BGIMAGE, Resize_CompileMapButton, Click_CompileModel, NULL, NULL, NULL, NULL, -1));
#endif
	toppanel->add(new Button(toppanel, "build", "gui/buildbuilding.png", "", "Export building/tree/animation sprites",	MAINFONT8, BUTTON_BGIMAGE, Resize_ExportBldgButton, Click_ExportBuildingSprite, NULL, NULL, NULL, NULL, -1));
	toppanel->add(new Button(toppanel, "build", "gui/buildunit.png", "", "Export unit/animation sprites from 8 sides",	MAINFONT8, BUTTON_BGIMAGE, Resize_ExportUnitButton, Click_ExportUnitSprites, NULL, NULL, NULL, NULL, -1));
#if 0
	toppanel->add(new Button(toppanel, "build", "gui/buildtile.png", "", "Export tile with inclines",					MAINFONT8, BUTTON_BGIMAGE, Resize_ExportTileButton, Click_CompileMap, NULL, NULL));
	toppanel->add(new Button(toppanel, "build", "gui/buildroad.png", "", "Export road tile with inclines from 4 sides",	MAINFONT8, BUTTON_BGIMAGE, Resize_ExportRoadButton, Click_CompileMap, NULL, NULL));
	toppanel->add(new Button(toppanel, "build", "gui/buildridge.png", "", "Export ridge with inclines from 4 sides",	MAINFONT8, BUTTON_BGIMAGE, Resize_ExportRidgeButton, Click_CompileMap, NULL, NULL));
	toppanel->add(new Button(toppanel, "build", "gui/buildwater.png", "", "Export water tile with inclines",			MAINFONT8, BUTTON_BGIMAGE, Resize_ExportWaterButton, Click_CompileMap, NULL, NULL));
	//toppanel->add(new Button(toppanel, "run", "gui/run.png", "", "Compile and run",									MAINFONT8, BUTTON_BGIMAGE, Resize_CompileRunButton, Click_CompileRunMap, NULL, NULL));
#endif
	toppanel->add(new Button(toppanel, "undo", "gui/undo.png", "", "Undo",												MAINFONT8, BUTTON_BGIMAGE, Resize_UndoButton, Undo, NULL, NULL, NULL, NULL, -1));
	toppanel->add(new Button(toppanel, "redo", "gui/redo.png", "", "Redo",												MAINFONT8, BUTTON_BGIMAGE, Resize_RedoButton, Redo, NULL, NULL, NULL, NULL, -1));
	toppanel->add(new Button(toppanel, "newbrush", "gui/newbrush.png", "", "New brush",									MAINFONT8, BUTTON_BGIMAGE, Resize_NewBrushButton, &Click_NewBrush, NULL, NULL, NULL, NULL, -1));
	toppanel->add(new Button(toppanel, "cutbrush", "gui/cutbrush.png", "", "Cut brush",									MAINFONT8, BUTTON_BGIMAGE, Resize_CutBrushButton, Click_CutBrush, NULL, NULL, NULL, NULL, -1));
	toppanel->add(new Button(toppanel, "newent", "gui/newent.png", "", "Add milkshape3d model",							MAINFONT8, BUTTON_BGIMAGE, Resize_NewEntityButton, Click_AddMS3D, NULL, NULL, NULL, NULL, -1));
	//toppanel->add(new Button(toppanel, "selent", "gui/selent.png", "", "Select entities only",						MAINFONT8, BUTTON_BGIMAGE, Resize_SelEntButton, NULL, NULL, NULL));
	//toppanel->add(new Button(toppanel, "selbrush", "gui/selbrush.png", "", "Select brushes only",						MAINFONT8, BUTTON_BGIMAGE, Resize_SelBrushButton, NULL, NULL, NULL));
	//toppanel->add(new Button(toppanel, "selentbrush", "gui/selentbrush.png", "", "Select entities and brushes",		MAINFONT8, BUTTON_BGIMAGE, Resize_SelEntBrushButton, NULL, NULL, NULL));
	toppanel->add(new Button(toppanel, "rotxccw", "gui/rotxccw.png", "", "Rotate counter-clockwise on x axis",			MAINFONT8, BUTTON_BGIMAGE, Resize_RotXCCWButton, Click_RotXCCW, NULL, NULL, NULL, NULL, -1));
	toppanel->add(new Button(toppanel, "rotxcw", "gui/rotxcw.png", "", "Rotate clockwise on x axis",					MAINFONT8, BUTTON_BGIMAGE, Resize_RotXCWButton, Click_RotXCW, NULL, NULL, NULL, NULL, -1));
	toppanel->add(new Button(toppanel, "rotyccw", "gui/rotyccw.png", "", "Rotate counter-clockwise on y axis",			MAINFONT8, BUTTON_BGIMAGE, Resize_RotYCCWButton, Click_RotYCCW, NULL, NULL, NULL, NULL, -1));
	toppanel->add(new Button(toppanel, "rotycw", "gui/rotycw.png", "", "Rotate clockwise on y axis",					MAINFONT8, BUTTON_BGIMAGE, Resize_RotYCWButton, Click_RotYCW, NULL, NULL, NULL, NULL, -1));
	toppanel->add(new Button(toppanel, "rotzccw", "gui/rotzccw.png", "", "Rotate counter-clockwise on z axis",			MAINFONT8, BUTTON_BGIMAGE, Resize_RotZCCWButton, Click_RotZCCW, NULL, NULL, NULL, NULL, -1));
	toppanel->add(new Button(toppanel, "rotzcw", "gui/rotzcw.png", "", "Rotate clockwise on z axis",					MAINFONT8, BUTTON_BGIMAGE, Resize_RotZCWButton, Click_RotZCW, NULL, NULL, NULL, NULL, -1));

//EditBox::EditBox(Widget* parent, const char* n, const std::string t, int f, void (*reframef)(Widget* thisw), bool pw, int maxl, void (*change2)(int p), int parm) : Widget()

	toppanel->add(new EditBox(toppanel, "rotdeg", "15",															MAINFONT8, Resize_RotDegEditBox, false, 6, &Change_RotDeg, 0));
	toppanel->add(new Text(toppanel, "rotdegtext", "degrees",													MAINFONT8, Resize_RotDegText));
	toppanel->add(new EditBox(toppanel, "zoom", "1",															MAINFONT8, Resize_ZoomEditBox, false, 6, &Change_Zoom, 0));
	toppanel->add(new Text(toppanel, "zoomtext", "zoom",														MAINFONT8, Resize_ZoomText));
	toppanel->add(new DropDownS(toppanel, "snapgrid",															MAINFONT8, Resize_SnapGridEditBox, Change_SnapGrid));
	DropDownS* snapgs = (DropDownS*)toppanel->get("snapgrid");
	//snapgs->m_options.push_back("4 m");	//0
	//snapgs->m_options.push_back("2 m");	//1
	//snapgs->m_options.push_back("1 m");	//2
	//snapgs->m_options.push_back("50 cm");	//3
	//snapgs->m_options.push_back("25 cm");	//4
	//snapgs->m_options.push_back("12.5 cm");	//5
	//snapgs->m_options.push_back("6.25 cm");	//6
	//snapgs->m_options.push_back("3.125 cm");	//7
	//snapgs->m_options.push_back("1.5625 cm");	//8
	//snapgs->select(4);

	float cm_scales[] = CM_SCALES;
	string cm_scales_txt[] = CM_SCALES_TXT;

	for(int j=0; j<sizeof(cm_scales)/sizeof(float); j++)
	{
		snapgs->m_options.push_back(cm_scales_txt[j].c_str());
	}

	//snapgs->select(6);
	snapgs->m_selected = 6;

	//toppanel->add(new EditBox(toppanel, "snapgrid", "25",														MAINFONT8, Margin(0+32*i++), Margin(32), Margin(32+32*i++), Margin(32+16), false, 6, &Change_SnapGrid, 0));
	toppanel->add(new Text(toppanel, "snapgrid text", "snap grid",													MAINFONT8, Resize_SnapGridText));
	toppanel->add(new EditBox(toppanel, "maxelev", "10000",														MAINFONT8, Resize_MaxElevEditBox, false, 6, &Change_MaxElev, 0));
	toppanel->add(new Text(toppanel, "maxelev text", "max elev.",													MAINFONT8, Resize_MaxElevText));
	toppanel->add(new CheckBox(toppanel, "showsky", "show sky",													MAINFONT8, Resize_ShowSkyCheckBox, 0, 1.0f, 1.0f, 1.0f, 1.0f, &Change_ShowSky));

	toppanel->add(new Button(toppanel, "persp", "gui/projpersp.png", "", "Perspective projection",				MAINFONT8, BUTTON_BGIMAGE, Resize_PerspProjButton, Click_ProjPersp, NULL, NULL, NULL, NULL, -1));
	toppanel->add(new Button(toppanel, "ortho", "gui/projortho.png", "", "Orthographic projection",				MAINFONT8, BUTTON_BGIMAGE, Resize_OrthoProjButton, Click_ProjOrtho, NULL, NULL, NULL, NULL, -1));
	toppanel->add(new Button(toppanel, "resetview", "gui/resetview.png", "", "Reset view",						MAINFONT8, BUTTON_BGIMAGE, Resize_ResetViewButton, Click_ResetView, NULL, NULL, NULL, NULL, -1));

	toppanel->add(new Text(toppanel, "frames text", "frames",														MAINFONT8, Resize_FramesText));
	toppanel->add(new EditBox(toppanel, "frames", "1",															MAINFONT8, Resize_FramesEditBox, false, 6, &Change_Frames, 0));

#if 0
	toppanel->add(new Button(toppanel, "explosion", "gui/explosion.png", "", "Explode crater",					MAINFONT8, BUTTON_BGIMAGE, Resize_ExplodeButton, Click_Explode, NULL, NULL));
#endif

	//toppanel->add(new Text(toppanel, "fps", "fps: 0", MAINFONT8, Margin(MARGIN_SOURCE_WIDTH, MARGIN_FUNC_PIXELS, 10), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 70), true));

	edview->add(new Frame(NULL, "viewports frame", Resize_ViewportsFrame));
	Widget* viewportsframe = edview->get("viewports frame");

#if 0
ViewportW::ViewportW(Widget* parent, const char* n, void (*reframef)(Widget* thisw),
					 void (*drawf)(int p, int x, int y, int w, int h),
					 bool (*ldownf)(int p, int x, int y, int w, int h),
					 bool (*lupf)(int p, int x, int y, int w, int h),
					 bool (*mousemovef)(int p, int x, int y, int w, int h),
					 bool (*rdownf)(int p, int relx, int rely, int w, int h),
					 bool (*rupf)(int p, int relx, int rely, int w, int h),
					 bool (*mousewf)(int p, int d),
					 int parm)
#endif

	viewportsframe->add(new ViewportW(viewportsframe, "bottom left viewport",	Resize_BottomLeftViewport,	&DrawViewport, &ViewportLDown, &ViewportLUp, &ViewportMousemove, &ViewportRDown, &ViewportRUp, ViewportMousewheel, NULL, NULL, 0));
	viewportsframe->add(new ViewportW(viewportsframe, "top left viewport",		Resize_TopLeftViewport,		&DrawViewport, &ViewportLDown, &ViewportLUp, &ViewportMousemove, &ViewportRDown, &ViewportRUp, ViewportMousewheel, NULL, NULL, 1));
	viewportsframe->add(new ViewportW(viewportsframe, "bottom right viewport",	Resize_BottomRightViewport,	&DrawViewport, &ViewportLDown, &ViewportLUp, &ViewportMousemove, &ViewportRDown, &ViewportRUp, ViewportMousewheel, NULL, NULL, 2));
	viewportsframe->add(new ViewportW(viewportsframe, "top right viewport",		Resize_TopRightViewport,	&DrawViewport, &ViewportLDown, &ViewportLUp, &ViewportMousemove, &ViewportRDown, &ViewportRUp, ViewportMousewheel, NULL, NULL, 3));

	g_viewportT[VIEWPORT_FRONT] = ViewportT(Vec3f(0, 0, MAX_DISTANCE/2), Vec3f(0, 1, 0), "Front");
	g_viewportT[VIEWPORT_TOP] = ViewportT(Vec3f(0, MAX_DISTANCE/2, 0), Vec3f(0, 0, -1), "Top");
	g_viewportT[VIEWPORT_LEFT] = ViewportT(Vec3f(MAX_DISTANCE/2, 0, 0), Vec3f(0, 1, 0), "Left");
	//g_viewportT[VIEWPORT_ANGLE45O] = ViewportT(Vec3f(MAX_DISTANCE/3, MAX_DISTANCE/3, MAX_DISTANCE/3), Vec3f(0, 1, 0), "Angle");
	g_viewportT[VIEWPORT_ANGLE45O] = ViewportT(Vec3f(1000.0f/3, 1000.0f/3, 1000.0f/3), Vec3f(0, 1, 0), "Angle");
	//g_camera.position(1000.0f/3, 1000.0f/3, 1000.0f/3, 0, 0, 0, 0, 1, 0);
	ResetView();

	g_viewport[0] = Viewport(VIEWPORT_FRONT);
	g_viewport[1] = Viewport(VIEWPORT_TOP);
	g_viewport[2] = Viewport(VIEWPORT_LEFT);
	g_viewport[3] = Viewport(VIEWPORT_ANGLE45O);

	viewportsframe->add(new Image(viewportsframe, "h divider", "gui/filled.jpg", true, Resize_HDivider));
	viewportsframe->add(new Image(viewportsframe, "v divider", "gui/filled.jpg", true, Resize_VDivider));

	g_GUI.add(new ViewLayer(&g_GUI, "brush edit"));
	ViewLayer* brusheditview = (ViewLayer*)g_GUI.get("brush edit");
	brusheditview->add(new Frame(NULL, "left panel", Resize_BrushEditFrame));

	leftpanel = brusheditview->get("left panel");
	//leftpanel->add(new EditBox(leftpanel, "texture path", "no texture", MAINFONT8, Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, 0), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), false, 64, NULL, -1));
	//i++;
	leftpanel->add(new Button(leftpanel, "choose texture", "gui/transp.png", "Choose Texture", "Choose texture", MAINFONT8, BUTTON_BGIMAGE, Resize_ChooseTexButton, Click_BChooseTex, NULL, NULL, NULL, NULL, -1));
	leftpanel->add(new Button(leftpanel, "fit to face", "gui/fittoface.png", "", "Fit to face",	MAINFONT8, BUTTON_BGIMAGE, Resize_FitToFaceButton, Click_FitToFace, NULL, NULL, NULL, NULL, -1));
	//leftpanel->add(new Button(leftpanel, "door view", "gui/door.png", "", "Door view",	MAINFONT8, Margin(0+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(32+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i+32), Click_DoorView, NULL, NULL));

	g_GUI.add(new ViewLayer(&g_GUI, "brush side edit"));
	ViewLayer* brushsideeditview = (ViewLayer*)g_GUI.get("brush side edit");
	brushsideeditview->add(new Frame(brushsideeditview, "left panel", Resize_BrushSideEditFrame));

	leftpanel = brushsideeditview->get("left panel");
	leftpanel->add(new Button(leftpanel, "rotate texture", "gui/transp.png", "Rotate Texture", "Rotate texture", MAINFONT8, BUTTON_BGIMAGE, Resize_RotateTexButton, Click_RotateTex, NULL, NULL, NULL, NULL, -1));
	leftpanel->add(new EditBox(leftpanel, "u equation", "A B C D", MAINFONT8, Resize_TexEqEditBox1, false, 256, Change_TexEq, 0));
	leftpanel->add(new EditBox(leftpanel, "v equation", "A B C D", MAINFONT8, Resize_TexEqEditBox2, false, 256, Change_TexEq, 1));
	leftpanel->add(new DropDownS(leftpanel, "select component", MAINFONT8, Resize_SelComponentDropDownS, Change_SelComp));
	Widget* selcompwidg = leftpanel->get("select component");
	selcompwidg->m_options.push_back("u component");
	selcompwidg->m_options.push_back("v component");
	leftpanel->add(new EditBox(leftpanel, "texture scale", "1", MAINFONT8, Resize_TexScaleEditBox, false, 10, NULL, 0));
	leftpanel->add(new Button(leftpanel, "texture scale button", "gui/transp.png", "Scale", "Scale texture component", MAINFONT8, BUTTON_BGIMAGE, Resize_ScaleTexButton, Click_ScaleTex, NULL, NULL, NULL, NULL, -1));
	leftpanel->add(new EditBox(leftpanel, "texture shift", "0.05", MAINFONT8, Resize_TexShiftEditBox, false, 10, NULL, 0));
	leftpanel->add(new Button(leftpanel, "texture shift button", "gui/transp.png", "Shift", "Shift texture component", MAINFONT8, BUTTON_BGIMAGE, Resize_TexShiftButton, Click_ShiftTex, NULL, NULL, NULL, NULL, -1));

	g_GUI.add(new ViewLayer(&g_GUI, "choose file"));
	ViewLayer* choosefileview = (ViewLayer*)g_GUI.get("choose file");
	choosefileview->add(new ChooseFile(choosefileview, "choose file", Resize_ChooseFile, Callback_ChooseFile));

#if 1
	g_GUI.add(new ViewLayer(&g_GUI, "door edit"));
	ViewLayer* dooreditview = (ViewLayer*)g_GUI.get("door edit");
#if 0
	dooreditview->add(new Frame(NULL, "left panel", Margin(MARGIN_SOURCE_WIDTH, MARGIN_FUNC_PIXELS, 0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, TOP_PANEL_HEIGHT), Margin(MARGIN_SOURCE_WIDTH, MARGIN_FUNC_PIXELS, LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_RATIO, 1)));

	leftpanel = dooreditview->get("left panel", WIDGET_FRAME);
	i=1;
	j=0;
	leftpanel->add(new Button(leftpanel, "set door", "gui/yesdoor.png", "", "Make door and set properties",	MAINFONT8, Margin(0+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(32+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i+32), Click_SetDoor, NULL, NULL));
	j++;
	leftpanel->add(new Button(leftpanel, "unmake door", "gui/nodoor.png", "", "Unmake door",	MAINFONT8, Margin(0+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(32+32*j), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i+32), Click_UnmakeDoor, NULL, NULL));
	j=0;
	i+=3;
	leftpanel->add(new Button(leftpanel, "open/close", "gui/transp.png", "Open / Close", "Open / Close door", MAINFONT8, Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(LEFT_PANEL_WIDTH), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*(i+1)), Click_OpenCloseDoor, NULL, NULL));
	i++;
	leftpanel->add(new Text(leftpanel, "opendeg label", "Open degrees:", MAINFONT8, Margin(0), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i)));
	leftpanel->add(new EditBox(leftpanel, "opendeg", "90", MAINFONT8, Margin(MARGIN_SOURCE_WIDTH, MARGIN_FUNC_PIXELS, 70), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*i), Margin(LEFT_PANEL_WIDTH-10), Margin(MARGIN_SOURCE_HEIGHT, MARGIN_FUNC_PIXELS, f->gheight*(i+1)), false, 10, NULL, 0));
#endif
#endif

	g_GUI.add(new ViewLayer(&g_GUI, "render"));
	ViewLayer* renderview = (ViewLayer*)g_GUI.get("render");

	renderview->add(new ViewportW(NULL, "render viewport",	Resize_FullViewport, &DrawViewport, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 3));

	g_GUI.closeall();
	g_GUI.open("loading");
	//OpenAnotherView("brush edit view");
}

int GetNumFrames()
{
	ViewLayer* edview = (ViewLayer*)g_GUI.get("editor");

	Widget* toppanel = edview->get("top panel");

	Widget* frameseditbox = toppanel->get("frames");

	int nframes = StrToInt(frameseditbox->m_value.c_str());

	return nframes;
}

void SetNumFrames(int nframes)
{
	ViewLayer* edview = (ViewLayer*)g_GUI.get("editor");

	if(!edview)  ErrorMessage("Error", "edview not found");

	Widget* toppanel = edview->get("top panel");

	if(!toppanel)  ErrorMessage("Error", "toppanel not found");

	EditBox* frameseditbox = (EditBox*)toppanel->get("frames");

	if(!frameseditbox)  ErrorMessage("Error", "frameseditbox not found");

	char nframesstr[128];
	sprintf(nframesstr, "%d", nframes);

	frameseditbox->changevalue(nframesstr);
}
