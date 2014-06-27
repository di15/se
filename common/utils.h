


#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <vector>

using namespace std;

#define CORRECT_SLASH '\\'

extern ofstream g_log;

const string DateTime();
const string FileDateTime();
void FullPath(const char* filename, char* full);
string MakePathRelative(const char* full);
void ExePath(char* exepath);
string StripFile(string filepath);
void StripPathExtension(const char* n, char* o);
void StripExtension(char* filepath);
void StripPath(char* filepath);
void OpenLog(const char* filename, float version);
float StrToFloat(const char *s);
int HexToInt(const char* s);
int StrToInt(const char *s);
void CorrectSlashes(char* corrected);
void BackSlashes(char* corrected);
void OutOfMem(const char* file, int line);
void CheckGLError(const char* file, int line);