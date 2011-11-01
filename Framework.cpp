#include "Framework.hpp"
#include <fstream> // std::ifstream
#include <climits> // CHAR_BIT
#ifdef _WIN32
#	include <windows.h>
#	include <winbase.h>
#else
#	include <sys/time.h>
#endif // _WIN32

namespace fw
{
////////////////////////////////////////////////////////////////////////////////
// Local functions
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Exceptions
class _ShaderCompilationFailedException : public FWException
{
public:
	_ShaderCompilationFailedException(const std::string& log)
	{
		mMessage = log;
	}
};

class _ProgramLinkFailException : public FWException
{
public:
	_ProgramLinkFailException(const std::string& file, const std::string& log)
	{
		mMessage = "GLSL link error in " + file + ":\n" + log;
	}
};

class _ProgramBuildFailException : public FWException
{
public:
	_ProgramBuildFailException(const std::string& file, const std::string& log)
	{
		mMessage = "GLSL build error in " + file + ":\n" + log;
	}
};

class _FileNotFoundException : public FWException
{
public:
	_FileNotFoundException(const std::string& file)
	{
		mMessage = "File " + file + " not found.";
	}
};

class _ProgramInvalidFirstLineException : public FWException
{
public:
	_ProgramInvalidFirstLineException(const std::string& file)
	{
		mMessage = "First line must be GLSL version specification (in "+file+").";
	}
};

class _GLErrorException : public FWException
{
public:
	_GLErrorException(const std::string& log)
	{
		mMessage = log;
	}
};

////////////////////////////////////////////////////////////////////////////////
// Get time
static GLdouble _get_ticks()
{
#ifdef _WIN32
	__int64 time;
	__int64 cpuFrequency;
	QueryPerformanceCounter((LARGE_INTEGER*) &time);
	QueryPerformanceFrequency((LARGE_INTEGER*) &cpuFrequency);
	return time / static_cast<GLdouble>(cpuFrequency);
#else
	static GLdouble t0 = 0.0;
	timeval tv;
	gettimeofday(&tv, 0);
	if (!t0) 
		t0 = tv.tv_sec;

	return   static_cast<GLdouble>(tv.tv_sec-t0)
	       + static_cast<GLdouble>(tv.tv_usec) / 1e6;
#endif
}


////////////////////////////////////////////////////////////////////////////////
// Attach shader
static void _attach_shader( GLuint program,
                            GLenum shaderType,
                            const GLchar* stringsrc) throw(FWException)
{
	const GLchar** string =  &stringsrc;
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, string, NULL);
	glCompileShader(shader);

	// check compilation
	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE)
	{
		GLchar logContent[256];
		glGetShaderInfoLog(shader, 256, NULL, logContent);
		throw _ShaderCompilationFailedException(std::string(logContent));
	}

	// attach shader and flag for deletion
	glAttachShader(program, shader);
	glDeleteShader(shader);
}


////////////////////////////////////////////////////////////////////////////////
// Convert GL error code to string
static const std::string _gl_error_to_string(GLenum error)
{
	switch(error)
	{
	case GL_NO_ERROR:
		return "GL_NO_ERROR";
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return "GL_INVALID_FRAMEBUFFER_OPERATION";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	default:
		return "unknown code";
	}
}


////////////////////////////////////////////////////////////////////////////////
// OpenGL Debug ARB callback
static GLvoid _gl_debug_message_callback( GLenum source,
                                          GLenum type,
                                          GLuint id,
                                          GLenum severity,
                                          GLsizei length,
                                          const GLchar* message,
                                          GLvoid* userParam )
{
	throw _GLErrorException(message);
}

////////////////////////////////////////////////////////////////////////////////
// Implementation
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Next power of two
GLuint next_power_of_two(GLuint number)
{
	if (number == 0)
		return 1;
	--number;
	for (GLuint i=1; i<sizeof(GLuint)*CHAR_BIT; i<<=1)
		number = number | number >> i;
	return number+1;
}


////////////////////////////////////////////////////////////////////////////////
// build glsl program
GLvoid buid_glsl_program( GLuint program, 
                          const std::string& srcfile,
                          const std::string& options,
                          GLboolean link ) throw(FWException)
{
	// open source file
	std::ifstream file(srcfile.c_str());
	if(file.fail())
		throw _FileNotFoundException(srcfile);

	// check first line (must be the version specification)
	std::string source;
	getline(file, source);
	if(source.find("#version") == std::string::npos)
		throw _ProgramInvalidFirstLineException(srcfile);

	// add the endline character
	source += '\n';

	// add options (if any)
	if(!options.empty())
		source += options + '\n';

	// backup position
	const size_t posbu = source.length();

	// recover whole source
	std::string line;
	while(getline(file, line))
		source += line + '\n';

	try
	{
		// find different stages and build shaders
		if(source.find("_VERTEX_") != std::string::npos)
		{
			std::string vsource = source;
			vsource.insert(posbu, "#define _VERTEX_\n");
			_attach_shader(program, GL_VERTEX_SHADER, vsource.data());
		}
		if(source.find("_TESS_CONTROL_") != std::string::npos)
		{
			std::string csource = source;
			csource.insert(posbu, "#define _TESS_CONTROL_\n");
			_attach_shader(program, GL_TESS_CONTROL_SHADER, csource.data());
		}
		if(source.find("_TESS_EVALUATION_") != std::string::npos)
		{
			std::string esource = source;
			esource.insert(posbu, "#define _TESS_EVALUATION_\n");
			_attach_shader(program, GL_TESS_EVALUATION_SHADER, esource.data());
		}
		if(source.find("_GEOMETRY_") != std::string::npos)
		{
			std::string gsource = source;
			gsource.insert(posbu, "#define _GEOMETRY_\n");
			_attach_shader(program, GL_GEOMETRY_SHADER, gsource.data());
		}
		if(source.find("_FRAGMENT_") != std::string::npos)
		{
			std::string fsource = source;
			fsource.insert(posbu, "#define _FRAGMENT_\n");
			_attach_shader(program, GL_FRAGMENT_SHADER, fsource.data());
		}
	}
	catch(FWException& e)
	{
		throw _ProgramBuildFailException(srcfile, e.what());
	}

	// Link program if asked
	if(GL_TRUE == link)
	{
		glLinkProgram(program);
		// check link
		GLint linkStatus = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
		if(GL_FALSE == linkStatus)
		{
			GLchar logContent[256];
			glGetProgramInfoLog(program, 256, NULL, logContent);
			throw _ProgramLinkFailException(srcfile, logContent);
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// Check OpenGL error
GLvoid check_gl_error() throw (FWException)
{
	static bool isArbDebugOutputConfigured = false;
	if(GLEW_ARB_debug_output)
	{
		if(!isArbDebugOutputConfigured)
		{
			glDebugMessageCallbackARB(&_gl_debug_message_callback, NULL);
			isArbDebugOutputConfigured = true;
		}
	}
	else
	{
		GLenum error = glGetError();
		if(GL_NO_ERROR != error)
		{
			throw _GLErrorException(_gl_error_to_string(error));
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// Timer Constructor
Timer::Timer() : 
	mStartTicks(0.0), mStopTicks(0.0), mIsTicking(false)
{}


////////////////////////////////////////////////////////////////////////////////
// Timer::Start 
void Timer::Start()
{
	if(false == mIsTicking)
	{
		mIsTicking  = true;
		mStartTicks = _get_ticks();
	}
}


////////////////////////////////////////////////////////////////////////////////
// Timer::Stop()
void Timer::Stop()
{
	if(true == mIsTicking)
	{
		mIsTicking = false;
		mStopTicks = _get_ticks();
	}
}


////////////////////////////////////////////////////////////////////////////////
// Timer::Ticks()
double Timer::Ticks() const
{
	return true==mIsTicking ? _get_ticks() - mStartTicks : mStopTicks - mStartTicks;
}


} // namespace fw

