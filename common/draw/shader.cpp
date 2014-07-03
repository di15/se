






#include "../utils.h"
#include "../draw/shader.h"
#include "../platform.h"
#include "../debug.h"
#include "../window.h"

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
	//g_applog<<"\tmap uniform "<<variable<<" = "<<(int)m_slot[slot]<<std::endl;
}

void Shader::MapAttrib(int slot, const char* variable)
{
	m_slot[slot] = GetAttrib(variable);
	//g_applog<<"\tmap attrib "<<variable<<" = "<<(int)m_slot[slot]<<std::endl;
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
	//strstr("abab", "ba");
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if( glewError != GLEW_OK )
	{
		ErrorMessage("Error initializing GLEW!", (const char*)glewGetErrorString( glewError ));
		return;
	}

	g_applog<<"Renderer: "<<(char*)glGetString(GL_RENDERER)<<std::endl;
	g_applog<<"GL_VERSION = "<<(char*)glGetString(GL_VERSION)<<std::endl;

#if 0
	//Make sure OpenGL 2.1 is supported
	if( !GLEW_VERSION_2_1 )
	{
		ErrorMessage("Error", "OpenGL 2.1 not supported!\n" );
		return;
	}
#elif 0
	if( !GLEW_VERSION_3_2 )
	{
		ErrorMessage("Error", "OpenGL 3.2 not supported!\n" );
		return;
	}
#else
	if( !GLEW_VERSION_1_4 )
	{
		ErrorMessage("Error", "OpenGL 1.4 not supported!\n" );
		g_quit = true;
		return;
	}
#endif

#if 1
	char* szGLExtensions = (char*)glGetString(GL_EXTENSIONS);

	g_applog<<szGLExtensions<<std::endl;
	g_applog.flush();

	if(!strstr(szGLExtensions, "GL_ARB_debug_output"))
	{
		//ErrorMessage("Error", "GL_ARB_debug_output extension not supported!");
		//g_quit = true;
		//return;
		g_applog<<"GL_ARB_debug_output extension not supported"<<std::endl;
	}
	else
	{
		g_applog<<"Reging debug handler"<<std::endl;
		g_applog.flush();
		glDebugMessageCallbackARB(&GLMessageHandler, 0);
		CheckGLError(__FILE__, __LINE__);
	}

	if(!strstr(szGLExtensions, "GL_ARB_framebuffer_object"))
	{
		ErrorMessage("Error", "GL_ARB_framebuffer_object extension not supported!");
		g_quit = true;
		return;
	}

	if(!strstr(szGLExtensions, "GL_ARB_shader_objects"))
	{
		ErrorMessage("Error", "GL_ARB_shader_objects extension not supported!");
		g_quit = true;
		return;
	}

	if(!strstr(szGLExtensions, "GL_ARB_shading_language_100"))
	{
		ErrorMessage("Error", "GL_ARB_shading_language_100 extension not supported!");
		g_quit = true;
		return;
	}
#endif

	int major, minor;
	GetGLVersion(&major, &minor);

	if(major < 1 || ( major == 1 && minor < 4 ))
	{
		ErrorMessage("Error", "OpenGL 1.4 is not supported!");
		g_quit = true;
	}

	CheckGLError(__FILE__, __LINE__);
	
	LoadShader(SHADER_ORTHO, "shaders/ortho.vert", "shaders/ortho.frag", true, true, false);
	LoadShader(SHADER_LIGHTTEST, "shaders/lighttest.vert", "shaders/lighttest.frag", true, true, false);
	LoadShader(SHADER_COLOR2D, "shaders/color2d.vert", "shaders/color2d.frag", true, false, false);
	LoadShader(SHADER_MODEL, "shaders/model.vert", "shaders/model.frag", true, true, true);
	LoadShader(SHADER_MODELPERSP, "shaders/modelpersp.vert", "shaders/model.frag", true, true, true);
	LoadShader(SHADER_COLOR3D, "shaders/color3d.vert", "shaders/color3d.frag", true, false, false);
	LoadShader(SHADER_COLOR3DPERSP, "shaders/color3dpersp.vert", "shaders/color3d.frag", true, false, false);
	LoadShader(SHADER_BILLBOARD, "shaders/billboard.vert", "shaders/billboard.frag", true, true, false);
	LoadShader(SHADER_DEPTH, "shaders/depth.vert", "shaders/depth.frag", true, true, false);
	//LoadShader(SHADER_SHADOW, "shaders/shadow.vert", "shaders/shadow.frag");
	LoadShader(SHADER_OWNED, "shaders/owned.vert", "shaders/owned.frag", true, true, true);
	LoadShader(SHADER_MAP, "shaders/map.vert", "shaders/map.frag", true, true, true);
	LoadShader(SHADER_MAPPERSP, "shaders/mappersp.vert", "shaders/map.frag", true, true, true);
	LoadShader(SHADER_WATER, "shaders/water.vert", "shaders/water.frag", true, true, true);
	LoadShader(SHADER_BILLBOARDPERSP, "shaders/billboardpersp.vert", "shaders/billboard.frag", true, true, false);
	LoadShader(SHADER_TEAM, "shaders/team.vert", "shaders/team.frag", true, true, true);

	CheckGLError(__FILE__, __LINE__);
}

std::string LoadTextFile(char* strFile)
{
	std::ifstream fin(strFile);

	if(!fin)
		return "";

	std::string strLine = "";
	std::string strText = "";

	while(getline(fin, strLine))
		strText = strText + "\n" + strLine;

	fin.close();

	return strText;
}

void LoadShader(int shader, char* strVertex, char* strFragment, bool hasverts, bool hastexcoords, bool hasnormals)
{
	Shader* s = &g_shader[shader];
	
	g_shader[shader].m_hasverts = hasverts;
	g_shader[shader].m_hastexcoords = hastexcoords;
	g_shader[shader].m_hasnormals = hasnormals;

	std::string strVShader, strFShader;

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
        g_applog<<"Shader "<<strVertex<<" compile log: "<<std::endl<<log<<std::endl;
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
        g_applog<<"Shader "<<strFragment<<" compile log: "<<std::endl<<log<<std::endl;
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

	//g_applog<<"shader "<<strVertex<<","<<strFragment<<std::endl;

	CheckGLError(__FILE__, __LINE__);

	g_applog<<"Program "<<strVertex<<" / "<<strFragment<<" :";

	glGetProgramiv(s->m_hProgramObject, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0) {
		GLchar *log = (GLchar *)malloc(logLength);
		glGetProgramInfoLog(s->m_hProgramObject, logLength, &logLength, log);
		g_applog<<"Program link log:"<<std::endl<<log<<std::endl;
		free(log);
	}

	GLint status;
	glGetProgramiv(s->m_hProgramObject, GL_LINK_STATUS, &status);
	if (status == 0)
	{
		g_applog<<"link status 0"<<std::endl;
	}
	else
	{
		g_applog<<"link status ok"<<std::endl;
	}

	g_applog<<std::endl<<std::endl;

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
    s->MapUniform(SSLOT_MVP, "mvp");
	CheckGLError(__FILE__, __LINE__);
    s->MapUniform(SSLOT_MODELVIEW, "modelview");
	s->MapUniform(SSLOT_NORMALMAT, "normalMat");
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
	glUseProgramObjectARB(g_shader[shader].m_hProgramObject);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	Shader* s = &g_shader[g_curS];
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif

#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	//if(s->m_slot[SSLOT_POSITION] != -1)	glEnableVertexAttribArray(s->m_slot[SSLOT_POSITION]);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	//if(s->m_slot[SSLOT_TEXCOORD0] != -1) glEnableVertexAttribArray(s->m_slot[SSLOT_TEXCOORD0]);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	//if(s->m_slot[SSLOT_NORMAL] != -1)	glEnableVertexAttribArray(s->m_slot[SSLOT_NORMAL]);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif

	if(s->m_hasverts)	glEnableClientState(GL_VERTEX_ARRAY);
	if(s->m_hastexcoords)	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if(s->m_hasnormals)	glEnableClientState(GL_NORMAL_ARRAY);
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
	//if(s->m_slot[SSLOT_POSITION] != -1)	glDisableVertexAttribArray(s->m_slot[SSLOT_POSITION]);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	//if(s->m_slot[SSLOT_TEXCOORD0] != -1) glDisableVertexAttribArray(s->m_slot[SSLOT_TEXCOORD0]);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif
	//if(s->m_slot[SSLOT_NORMAL] != -1)	glDisableVertexAttribArray(s->m_slot[SSLOT_NORMAL]);
#ifdef DEBUG
	CheckGLError(__FILE__, __LINE__);
#endif

	glUseProgramObjectARB(0);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

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

