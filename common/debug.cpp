



#include "debug.h"
#include "utils.h"

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