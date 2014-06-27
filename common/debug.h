


#ifndef DEBUG_H
#define DEBUG_H

#include <winsock2.h>	// winsock2 needs to be included before windows.h
#include <Windows.h>
#include <fstream>

using namespace std;

void LastNum(const char* l);
void LastNum(const char* f, const int line);

#endif