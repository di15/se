

#include "utils.h"
#include "platform.h"

ofstream g_log;

const string DateTime() 
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

    return buf;
}

void OpenLog(const char* filename, float version)
{
	char fullpath[MAX_PATH+1];
	FullPath(filename, fullpath);
	g_log.open(fullpath, ios_base::out);
	g_log<<DateTime()<<endl;
	g_log<<"Version "<<version<<endl<<endl;
	g_log.flush();
}

string MakePathRelative(const char* full)
{
	char full2c[MAX_PATH+1];
	strcpy(full2c, full);
	CorrectSlashes(full2c);
	string full2(full2c);
	char exepath[MAX_PATH+1];
	ExePath(exepath);
	CorrectSlashes(exepath);

	//g_log<<"exepath: "<<exepath<<endl;
	//g_log<<"fulpath: "<<full<<endl;

	string::size_type pos = full2.find(exepath, 0);

	if(pos == string::npos)
	{
		return full2;
	}

	//g_log<<"posposp: "<<pos<<endl;

	string sub = string( full2 ).substr(strlen(exepath)+1, strlen(full)-strlen(exepath)-1);

	//g_log<<"subpath: "<<sub<<endl;

    return sub;
}

void ExePath(char* exepath) 
{
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    string::size_type pos = string( buffer ).find_last_of( "\\/" );
    string strexepath = string( buffer ).substr( 0, pos);
	strcpy(exepath, strexepath.c_str());
}

string StripFile(string filepath)
{
	int lastof = filepath.find_last_of("/\\");
	if(lastof < 0)
		lastof = strlen(filepath.c_str());
	else
		lastof += 1;

	string stripped = filepath.substr(0, lastof);
	return stripped;
}

void StripPath(char* filepath)
{
	string s0(filepath);
	size_t sep = s0.find_last_of("\\/");
	string s1;

    if (sep != std::string::npos)
        s1 = s0.substr(sep + 1, s0.size() - sep - 1);
	else
		s1 = s0;

	strcpy(filepath, s1.c_str());
}

void StripExtension(char* filepath)
{
	string s1(filepath);

	size_t dot = s1.find_last_of(".");
	string s2;

	if (dot != std::string::npos)
		s2 = s1.substr(0, dot);
	else
		s2 = s1;

	strcpy(filepath, s2.c_str());
}

void StripPathExtension(const char* n, char* o)
{
	string s0(n);
	size_t sep = s0.find_last_of("\\/");
	string s1;

    if (sep != std::string::npos)
        s1 = s0.substr(sep + 1, s0.size() - sep - 1);
	else
		s1 = s0;

	size_t dot = s1.find_last_of(".");
	string s2;

	if (dot != std::string::npos)
		s2 = s1.substr(0, dot);
	else
		s2 = s1;

	strcpy(o, s2.c_str());
}

void FullPath(const char* filename, char* full)
{
	char exepath[MAX_PATH+1];
	GetModuleFileName(NULL, exepath, MAX_PATH);
	string path = StripFile(exepath);

	//char full[MAX_PATH+1];
	sprintf(full, "%s", path.c_str());

	char c = full[ strlen(full)-1 ];
	if(c != '\\' && c != '/')
		strcat(full, "\\");
		//strcat(full, "/");

	strcat(full, filename);
	CorrectSlashes(full);
}

float StrToFloat(const char *s)
{
	if(s[0] == '\0')
		return 1.0f;

    float x;
    istringstream iss(s);
    iss >> x;
	
	if(_isnan(x))
		x = 1.0f;

    return x;
}

int HexToInt(const char* s)
{
    int x;
    stringstream ss;
    ss << std::hex << s;
    ss >> x;
    return x;
}

int StrToInt(const char *s)
{
    int x;
    istringstream iss(s);
    iss >> x;
    return x;
}

void CorrectSlashes(char* corrected)
{
	int strl = strlen(corrected);
	for(int i=0; i<strl; i++)
		if(corrected[i] == '/')
			corrected[i] = '\\';
}

void BackSlashes(char* corrected)
{
	int strl = strlen(corrected);
	for(int i=0; i<strl; i++)
		if(corrected[i] == '/')
			corrected[i] = '\\';
}

