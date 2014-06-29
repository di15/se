



#include "debug.h"
#include "utils.h"
#include "platform.h"
#include "draw/shader.h"

void LastNum(const char* l)
{
	//return;
#if 1
	ofstream last;
	char filepath[MAX_PATH+1];
	FullPath("last.txt", filepath);
	last.open(filepath, ios_base::out);
	last<<l;
	last.flush();
#else
	g_log<<l<<endl;
	g_log.flush();
#endif
}

void LastNum(const char* f, const int line)
{
	return;
	g_log<<f<<":"<<line<<endl;
	g_log.flush();
}

#ifdef GLDEBUG
void CheckGLError(const char* file, int line)
{
	//char msg[2048];
	//sprintf(msg, "Failed to allocate memory in %s on line %d.", file, line);
	//ErrorMessage("Out of memory", msg);
	int error = glGetError();

	if(error == GL_NO_ERROR)
		return;

	g_log<<"GL Error #"<<error<<" in "<<file<<" on line "<<line<<" using shader #"<<g_curS<<endl;
}
#endif

GLvoid APIENTRY GLMessageHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
   //ErrorMessage("GL Error", message);
   g_log<<"GL Error: "<<message<<endl;
}
