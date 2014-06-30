#include "platform.h"

#define CORRECT_SLASH '/'

#define ARRSZ(a)	(sizeof(a)/sizeof(a[0]))

extern std::ofstream g_applog;

const std::string DateTime();
const std::string FileDateTime();
void FullPath(const char* filename, char* full);
std::string MakeRelative(const char* full);
void ExePath(char* exepath);
std::string StripFile(std::string filepath);
void StripPathExtension(const char* n, char* o);
void StripExtension(char* filepath);
void StripPath(char* filepath);
void OpenLog(const char* filename, int version);
float StrToFloat(const char *s);
int HexToInt(const char* s);
int StrToInt(const char *s);
void CorrectSlashes(char* corrected);
void BackSlashes(char* corrected);
void ErrorMessage(const char* title, const char* message);
void InfoMessage(const char* title, const char* message);
void WarningMessage(const char* title, const char* message);
void OutOfMem(const char* file, int line);

#ifndef PLATFORM_WIN

long timeGetTime();
long GetTickCount();
long long GetTickCount64();
void Sleep(int ms);

#endif

#ifdef PLATFORM_WIN

float fmax(float a, float b);
float fmin(float a, float b);

#endif

