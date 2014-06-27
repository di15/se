






#include "../utils.h"
#include "../draw/shader.h"
#include "../platform.h"
#include "../debug.h"

PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB = NULL;
PFNGLSHADERSOURCEARBPROC glShaderSourceARB = NULL;
PFNGLCOMPILESHADERARBPROC glCompileShaderARB = NULL;
PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB = NULL;
PFNGLATTACHOBJECTARBPROC glAttachObjectARB = NULL;
PFNGLLINKPROGRAMARBPROC glLinkProgramARB = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB = NULL;
PFNGLUNIFORM1IARBPROC glUniform1iARB = NULL;
PFNGLUNIFORM1FARBPROC glUniform1fARB = NULL;
PFNGLUNIFORM2FARBPROC glUniform2fARB = NULL;
PFNGLUNIFORM3FARBPROC glUniform3fARB = NULL;
PFNGLUNIFORM4FARBPROC glUniform4fARB = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB = NULL;
PFNGLDETACHOBJECTARBPROC glDetachObjectARB = NULL;
PFNGLDELETEOBJECTARBPROC glDeleteObjectARB = NULL;
PFNGLPROGRAMLOCALPARAMETER4FARBPROC glProgramLocalParameter4fARB;
PFNGLBINDPROGRAMARBPROC glBindProgramARB;
PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;
PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT;
PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT;
PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT;
PFNGLUNIFORMMATRIX4FVARBPROC glUniformMatrix4fvARB;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM1IVPROC glUniform1iv;
PFNGLUNIFORM2IVPROC glUniform2iv;
PFNGLUNIFORM3IVPROC glUniform3iv;
PFNGLUNIFORM4IVPROC glUniform4iv;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM1FVPROC glUniform1fv;
PFNGLUNIFORM2FVPROC glUniform2fv;
PFNGLUNIFORM3FVPROC glUniform3fv;
PFNGLUNIFORM4FVPROC glUniform4fv;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLUNIFORM4FPROC glUniform4f;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
PFNGLGETPROGRAMIVPROC glGetProgramiv = 0;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = 0;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLATTACHSHADERPROC glAttachShader = 0;
PFNGLCREATEPROGRAMPROC glCreateProgram = 0;
PFNGLLINKPROGRAMPROC glLinkProgram = 0;
PFNGLUSEPROGRAMPROC glUseProgram = 0;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = 0;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;

Shader g_shader[SHADERS];
int g_curS = 0;

GLint Shader::GetUniform(const char* strVariable)
{
	if(!m_hProgramObject)
		return -1;

	return glGetUniformLocationARB(m_hProgramObject, strVariable);
}

GLint Shader::GetAttrib(const char* strVariable)
{
	if(!m_hProgramObject)
		return -1;

	return glGetAttribLocation(m_hProgramObject, strVariable);
}

void Shader::MapUniform(int slot, const char* variable)
{
	m_slot[slot] = GetUniform(variable);
	//g_log<<"\tmap uniform "<<variable<<" = "<<(int)m_slot[slot]<<endl;
}

void Shader::MapAttrib(int slot, const char* variable)
{
	m_slot[slot] = GetAttrib(variable);
	//g_log<<"\tmap attrib "<<variable<<" = "<<(int)m_slot[slot]<<endl;
}

void GetGLVersion(int* major, int* minor)
{
	// for all versions
	char* ver = (char*)glGetString(GL_VERSION); // ver = "3.2.0"
	
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
	LastNum(__FILE__, __LINE__);
#endif

	char vermaj[4];

	for(int i=0; i<4; i++)
	{
		if(ver[i] != '.')
			vermaj[i] = ver[i];
		else
			vermaj[i] = '\0';
	}

	//*major = ver[0] - '0';
	*major = StrToInt(vermaj);
	if( *major >= 3)
	{
		// for GL 3.x
		glGetIntegerv(GL_MAJOR_VERSION, major); // major = 3
		glGetIntegerv(GL_MINOR_VERSION, minor); // minor = 2
	}
	else
	{
		*minor = ver[2] - '0';
	}

	// GLSL
	ver = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION); // ver = "1.50 NVIDIA via Cg compiler"
}

void InitGLSL()
{
	g_log<<"Renderer: "<<(char*)glGetString(GL_RENDERER)<<endl;
	
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
	LastNum(__FILE__, __LINE__);
#endif

#if 0
	char *szGLExtensions = (char*)glGetString(GL_EXTENSIONS);

	CheckGLError(__FILE__, __LINE__);
	LastNum(__FILE__, __LINE__);


	if(!strstr(szGLExtensions, "GL_ARB_shader_objects"))
	{
		MessageBox(g_hWnd, TEXT("GL_ARB_shader_objects extension not supported!"), TEXT("Error"), MB_OK);
		return;
	}

	if(!strstr(szGLExtensions, "GL_ARB_shading_language_100"))
	{
		MessageBox(g_hWnd, TEXT("GL_ARB_shading_language_100 extension not supported!"), TEXT("Error"), MB_OK);
		return;
	}
#endif

	int major, minor;
	GetGLVersion(&major, &minor);
	
	if(major < 3 || ( major == 3 && minor < 2 ))
	{
		MessageBox(g_hWnd, TEXT("OpenGL 3.2 is not supported!"), TEXT("Error"), MB_OK);
	}

	CheckGLError(__FILE__, __LINE__);
	
	glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)wglGetProcAddress("glCreateShaderObjectARB");
	glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)wglGetProcAddress("glShaderSourceARB");
	glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)wglGetProcAddress("glCompileShaderARB");
	glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)wglGetProcAddress("glCreateProgramObjectARB");
	glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)wglGetProcAddress("glAttachObjectARB");
	glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)wglGetProcAddress("glLinkProgramARB");
	glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)wglGetProcAddress("glUseProgramObjectARB");
	glUniform1iARB = (PFNGLUNIFORM1IARBPROC)wglGetProcAddress("glUniform1iARB");
	glUniform1fARB = (PFNGLUNIFORM1FARBPROC)wglGetProcAddress("glUniform1fARB");
	glUniform2fARB = (PFNGLUNIFORM2FARBPROC)wglGetProcAddress("glUniform2fARB");
	glUniform3fARB = (PFNGLUNIFORM3FARBPROC)wglGetProcAddress("glUniform3fARB");
	glUniform4fARB = (PFNGLUNIFORM4FARBPROC)wglGetProcAddress("glUniform4fARB");
	glGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC)wglGetProcAddress("glGetUniformLocationARB");
	glDetachObjectARB = (PFNGLDETACHOBJECTARBPROC)wglGetProcAddress("glDetachObjectARB");
	glDeleteObjectARB  = (PFNGLDELETEOBJECTARBPROC)wglGetProcAddress("glDeleteObjectARB");

	CheckGLError(__FILE__, __LINE__);

	glProgramLocalParameter4fARB = (PFNGLPROGRAMLOCALPARAMETER4FARBPROC)wglGetProcAddress("glProgramLocalParameter4fARB");
	glBindProgramARB = (PFNGLBINDPROGRAMARBPROC)wglGetProcAddress("glBindProgramARB");
	glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT"); 
	glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT");
	glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)wglGetProcAddress("glFramebufferTexture2DEXT");
	glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)wglGetProcAddress("glBindRenderbufferEXT");
	glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)wglGetProcAddress("glGenRenderbuffersEXT");
	glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)wglGetProcAddress("glRenderbufferStorageEXT");
	glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)wglGetProcAddress("glFramebufferRenderbufferEXT");
	glUniformMatrix4fvARB = (PFNGLUNIFORMMATRIX4FVARBPROC)wglGetProcAddress("glUniformMatrix4fvARB");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
	glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)wglGetProcAddress("glGetAttribLocation");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
	glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
	glUniform1iv = (PFNGLUNIFORM1IVPROC)wglGetProcAddress("glUniform1iv");
	glUniform2iv = (PFNGLUNIFORM2IVPROC)wglGetProcAddress("glUniform2iv");
	glUniform3iv = (PFNGLUNIFORM3IVPROC)wglGetProcAddress("glUniform3iv");
	glUniform4iv = (PFNGLUNIFORM4IVPROC)wglGetProcAddress("glUniform4iv");
	glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
	glUniform1fv = (PFNGLUNIFORM1FVPROC)wglGetProcAddress("glUniform1fv");
	glUniform2fv = (PFNGLUNIFORM2FVPROC)wglGetProcAddress("glUniform2fv");
	glUniform3fv = (PFNGLUNIFORM3FVPROC)wglGetProcAddress("glUniform3fv");
	glUniform4fv = (PFNGLUNIFORM4FVPROC)wglGetProcAddress("glUniform4fv");
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
	glUniform4f = (PFNGLUNIFORM4FPROC)wglGetProcAddress("glUniform4f");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
	glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
	glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) wglGetProcAddress("glDeleteFramebuffers");
	glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) wglGetProcAddress("glDeleteRenderbuffersEXT");
	glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");

	glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
	glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
	glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
	glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) wglGetProcAddress("glGenVertexArrays");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) wglGetProcAddress("glBindVertexArray");
	glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glDisableVertexAttribArray");
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) wglGetProcAddress("glCheckFramebufferStatus");
	glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) wglGetProcAddress("glCheckFramebufferStatusEXT");

	CheckGLError(__FILE__, __LINE__);

	LoadShader(SHADER_ORTHO, "shaders/ortho.vert", "shaders/ortho.frag");
	LoadShader(SHADER_COLOR2D, "shaders/color2d.vert", "shaders/color2d.frag");
	LoadShader(SHADER_MODEL, "shaders/model.vert", "shaders/model.frag");
	LoadShader(SHADER_COLOR3D, "shaders/color3d.vert", "shaders/color3d.frag");
	LoadShader(SHADER_COLOR3DPERSP, "shaders/color3dpersp.vert", "shaders/color3d.frag");
	LoadShader(SHADER_BILLBOARD, "shaders/billboard.vert", "shaders/billboard.frag");
	LoadShader(SHADER_DEPTH, "shaders/depth.vert", "shaders/depth.frag");
	//LoadShader(SHADER_SHADOW, "shaders/shadow.vert", "shaders/shadow.frag");
	LoadShader(SHADER_OWNED, "shaders/owned.vert", "shaders/owned.frag");
	LoadShader(SHADER_MAP, "shaders/map.vert", "shaders/map.frag");
	LoadShader(SHADER_WATER, "shaders/water.vert", "shaders/water.frag");
	LoadShader(SHADER_BILLBOARDPERSP, "shaders/billboardpersp.vert", "shaders/billboard.frag");
	
	CheckGLError(__FILE__, __LINE__);
}

string LoadTextFile(char* strFile)
{
	ifstream fin(strFile);

	if(!fin)
		return "";

	string strLine = "";
	string strText = "";

	while(getline(fin, strLine))
		strText = strText + "\n" + strLine;

	fin.close();

	return strText;
}

void LoadShader(int shader, char* strVertex, char* strFragment)
{
	Shader* s = &g_shader[shader];
	string strVShader, strFShader;

	if(s->m_hVertexShader || s->m_hFragmentShader || s->m_hProgramObject)
		s->release();
	
	//s->m_hVertexShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	//s->m_hFragmentShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	s->m_hVertexShader = glCreateShader(GL_VERTEX_SHADER);
	s->m_hFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	strVShader = LoadTextFile(strVertex);
	strFShader = LoadTextFile(strFragment);

	const char *szVShader = strVShader.c_str();
	const char *szFShader = strFShader.c_str();
	
	//glShaderSourceARB(s->m_hVertexShader, 1, &szVShader, NULL);
	//glShaderSourceARB(s->m_hFragmentShader, 1, &szFShader, NULL);
	glShaderSource(s->m_hVertexShader, 1, &szVShader, NULL);
	glShaderSource(s->m_hFragmentShader, 1, &szFShader, NULL);
	
	//glCompileShaderARB(s->m_hVertexShader);
	glCompileShader(s->m_hVertexShader);
	GLint logLength;
	glGetShaderiv(s->m_hVertexShader, GL_INFO_LOG_LENGTH, &logLength);
    if(logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
		if(!log) OutOfMem(__FILE__, __LINE__);
        glGetShaderInfoLog(s->m_hVertexShader, logLength, &logLength, log);
        g_log<<"Shader "<<strVertex<<" compile log: "<<endl<<log<<endl;
        free(log);
    }
	
	CheckGLError(__FILE__, __LINE__);
	
	//glCompileShaderARB(s->m_hFragmentShader);
	glCompileShader(s->m_hFragmentShader);
	glGetShaderiv(s->m_hFragmentShader, GL_INFO_LOG_LENGTH, &logLength);
    if(logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
		if(!log) OutOfMem(__FILE__, __LINE__);
        glGetShaderInfoLog(s->m_hFragmentShader, logLength, &logLength, log);
        g_log<<"Shader "<<strFragment<<" compile log: "<<endl<<log<<endl;
        free(log);
    }
	
	CheckGLError(__FILE__, __LINE__);
	
	//s->m_hProgramObject = glCreateProgramObjectARB();
	s->m_hProgramObject = glCreateProgram();
	//glAttachObjectARB(s->m_hProgramObject, s->m_hVertexShader);
	//glAttachObjectARB(s->m_hProgramObject, s->m_hFragmentShader);
	glAttachShader(s->m_hProgramObject, s->m_hVertexShader);
	glAttachShader(s->m_hProgramObject, s->m_hFragmentShader);
	//glLinkProgramARB(s->m_hProgramObject);
	glLinkProgram(s->m_hProgramObject);

	//glUseProgramObjectARB(s->m_hProgramObject);

	//g_log<<"shader "<<strVertex<<","<<strFragment<<endl;

	CheckGLError(__FILE__, __LINE__);

	g_log<<"Program "<<strVertex<<" / "<<strFragment<<" :";

	glGetProgramiv(s->m_hProgramObject, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0) {
		GLchar *log = (GLchar *)malloc(logLength);
		glGetProgramInfoLog(s->m_hProgramObject, logLength, &logLength, log);
		g_log<<"Program link log:"<<endl<<log<<endl;
		free(log);
	}

	GLint status;
	glGetProgramiv(s->m_hProgramObject, GL_LINK_STATUS, &status);
	if (status == 0)
	{
		g_log<<"link status 0"<<endl;
	}
	else
	{
		g_log<<"link status ok"<<endl;
	}

	g_log<<endl<<endl;

	CheckGLError(__FILE__, __LINE__);

    s->MapAttrib(SSLOT_POSITION, "position");
	CheckGLError(__FILE__, __LINE__);
    s->MapAttrib(SSLOT_NORMAL, "normalIn");
	CheckGLError(__FILE__, __LINE__);
    s->MapAttrib(SSLOT_TEXCOORD0, "texCoordIn0");
	CheckGLError(__FILE__, __LINE__);
    s->MapAttrib(SSLOT_TEXCOORD1, "texCoordIn1");
	CheckGLError(__FILE__, __LINE__);
    //s->MapAttrib(SSLOT_TANGENT, "tangent");
	s->MapUniform(SSLOT_SHADOWMAP, "shadowmap");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_LIGHTMATRIX, "lightMatrix");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_LIGHTPOS, "lightPos");
	CheckGLError(__FILE__, __LINE__);
	//s->MapUniform(SSLOT_LIGHTDIR, "lightDir");
	s->MapUniform(SSLOT_TEXTURE0, "texture0");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_TEXTURE1, "texture1");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_TEXTURE2, "texture2");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_TEXTURE3, "texture3");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_NORMALMAP, "normalmap");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_SPECULARMAP, "specularmap");
	CheckGLError(__FILE__, __LINE__);
    s->MapUniform(SSLOT_PROJECTION, "projection");
	CheckGLError(__FILE__, __LINE__);
    s->MapUniform(SSLOT_MODELMAT, "model");
	CheckGLError(__FILE__, __LINE__);
    s->MapUniform(SSLOT_VIEWMAT, "view");
	CheckGLError(__FILE__, __LINE__);
    s->MapUniform(SSLOT_MVPMAT, "mvpmat");
	CheckGLError(__FILE__, __LINE__);
	//s->MapUniform(SSLOT_NORMALMAT, "normalMat");
	//s->MapUniform(SSLOT_INVMODLVIEWMAT, "invModelView");
    s->MapUniform(SSLOT_COLOR, "color");
	CheckGLError(__FILE__, __LINE__);
    s->MapUniform(SSLOT_OWNCOLOR, "owncolor");
	CheckGLError(__FILE__, __LINE__);
    s->MapUniform(SSLOT_WIDTH, "width");
	CheckGLError(__FILE__, __LINE__);
    s->MapUniform(SSLOT_HEIGHT, "height");
	CheckGLError(__FILE__, __LINE__);
    s->MapUniform(SSLOT_MIND, "mind");
	CheckGLError(__FILE__, __LINE__);
    s->MapUniform(SSLOT_MAXD, "maxd");
	CheckGLError(__FILE__, __LINE__);
    s->MapUniform(SSLOT_CAMERAPOS, "cameraPos");
	CheckGLError(__FILE__, __LINE__);
    s->MapUniform(SSLOT_SCALE, "scale");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_MAXELEV, "maxelev");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_SANDONLYMAXY, "sandonlymaxy");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_SANDGRASSMAXY, "sandgrassmaxy");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_GRASSONLYMAXY, "grassonlymaxy");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_GRASSDIRTMAXY, "grassdirtmaxy");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_DIRTONLYMAXY, "dirtonlymaxy");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_DIRTROCKMAXY, "dirtrockmaxy");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_SANDGRAD, "sandgrad");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_SANDDET, "sanddet");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_GRASSGRAD, "grassgrad");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_GRASSDET, "grassdet");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_DIRTGRAD, "dirtgrad");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_DIRTDET, "dirtdet");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_ROCKGRAD, "rockgrad");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_ROCKGRAD, "rockdet");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_SUNDIRECTION, "sundirection");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_GRADIENTTEX, "gradienttex");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_DETAILTEX, "detailtex");
	CheckGLError(__FILE__, __LINE__);
	s->MapUniform(SSLOT_OWNERMAP, "ownermap");
	
	CheckGLError(__FILE__, __LINE__);
}

void UseS(int shader)		
{	
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	g_curS = shader;
	//glUseProgramObjectARB(g_shader[shader].m_hProgramObject); 
	glUseProgram(g_shader[shader].m_hProgramObject); 
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	Shader* s = &g_shader[g_curS];
	
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	if(s->m_slot[SSLOT_POSITION] != -1)	glEnableVertexAttribArray(s->m_slot[SSLOT_POSITION]);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	if(s->m_slot[SSLOT_TEXCOORD0] != -1) glEnableVertexAttribArray(s->m_slot[SSLOT_TEXCOORD0]);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	if(s->m_slot[SSLOT_NORMAL] != -1)	glEnableVertexAttribArray(s->m_slot[SSLOT_NORMAL]);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
}

void EndS()
{
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif

	if(g_curS < 0)
		return;

	Shader* s = &g_shader[g_curS];
	
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	if(s->m_slot[SSLOT_POSITION] != -1)	glDisableVertexAttribArray(s->m_slot[SSLOT_POSITION]);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	if(s->m_slot[SSLOT_TEXCOORD0] != -1) glDisableVertexAttribArray(s->m_slot[SSLOT_TEXCOORD0]);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	if(s->m_slot[SSLOT_NORMAL] != -1)	glDisableVertexAttribArray(s->m_slot[SSLOT_NORMAL]);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif

	glUseProgram(0); 

	g_curS = -1;
}

void Shader::release()
{
	if(m_hVertexShader)
	{
		glDetachObjectARB(m_hProgramObject, m_hVertexShader);
		glDeleteObjectARB(m_hVertexShader);
		m_hVertexShader = NULL;
	}

	if(m_hFragmentShader)
	{
		glDetachObjectARB(m_hProgramObject, m_hFragmentShader);
		glDeleteObjectARB(m_hFragmentShader);
		m_hFragmentShader = NULL;
	}

	if(m_hProgramObject)
	{
		glDeleteObjectARB(m_hProgramObject);
		m_hProgramObject = NULL;
	}
}

void TurnOffShader()
{
	glUseProgramObjectARB(0);
}

void ReleaseShaders()
{
	for(int i=0; i<SHADERS; i++)
		g_shader[i].release();
}

