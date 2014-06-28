

#include "gui.h"
#include "../texture.h"
#include "../debug.h"

void View::draw()
{
	//if(_stricmp(name.c_str(), "brush edit") == 0)
	//	g_log<<"draw brush edit"<<endl;

	for(auto i=widget.begin(); i!=widget.end(); i++)
	{
		//g_log<<"\t"<<(*i)->m_name<<endl;
		//g_log.flush();
#ifdef DEBUG
		CheckGLError(__FILE__, __LINE__);
#endif
		(*i)->draw();
	}
}

void View::draw2()
{
	for(auto i=widget.rbegin(); i!=widget.rend(); i++)
	{
		//g_log<<"\t"<<(*i)->m_name<<endl;
		//g_log.flush();
#ifdef DEBUG
		CheckGLError(__FILE__, __LINE__);
#endif
		(*i)->draw2();
	}
}
