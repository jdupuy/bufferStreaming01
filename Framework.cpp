#include "Framework.hpp"
#include <fstream> // std::ifstream
#include <climits> // CHAR_BIT
#include <cstring> // memcpy
#include <sstream> // std::stringstream

#ifdef _WIN32
#	include <windows.h>
#	include <winbase.h>
#else
#	include <sys/time.h>
#endif // _WIN32

namespace fw
{
////////////////////////////////////////////////////////////////////////////////
// Exceptions
//
////////////////////////////////////////////////////////////////////////////////
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

class _InvalidViewportDimensionsException : public FWException
{
public:
	_InvalidViewportDimensionsException()
	{
		mMessage = "Invalid viewport dimensions.";
	}
};

////////////////////////////////////////////////////////////////////////////////
// Local functions
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// extract uint16 from two uint8
static GLushort _unpack_uint16(GLubyte msb, GLubyte lsb)
{
	return (static_cast<GLushort>(lsb) | static_cast<GLushort>(msb) << 8);
}


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
	const GLchar** string = &stringsrc;
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
// Functions implementation
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
GLvoid build_glsl_program( GLuint program, 
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
			GLchar logContent[512];
			glGetProgramInfoLog(program, 512, NULL, logContent);
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
			glDebugMessageCallbackARB(
				reinterpret_cast<GLDEBUGPROCARB>(&_gl_debug_message_callback),
				NULL );
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
// Take a screen shot
GLvoid save_gl_front_buffer( GLint x,
	                         GLint y,
	                         GLsizei width,
	                         GLsizei height) throw(FWException)
{
	static GLuint sShotCounter = 1;
	GLubyte *tgaPixels = NULL;
	GLint tgaWidth, tgaHeight;
	GLint pixelPackBufferBinding,
	      readBuffer,
	      packSwapBytes, packLsbFirst, packRowLength, packImageHeight,
	      packSkipRows, packSkipPixels, packSkipImages, packAlignment;

	// check values
	if(x>=width || y>=height || x<0 || y<0)
		throw _InvalidViewportDimensionsException();

	// compute final tga dimensions
	tgaWidth  = width  - x;
	tgaHeight = height - y;

	// save GL state
	glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &pixelPackBufferBinding);
	glGetIntegerv(GL_READ_BUFFER, &readBuffer);
	glGetIntegerv(GL_PACK_SWAP_BYTES, &packSwapBytes);
	glGetIntegerv(GL_PACK_LSB_FIRST, &packLsbFirst);
	glGetIntegerv(GL_PACK_ROW_LENGTH, &packRowLength);
	glGetIntegerv(GL_PACK_IMAGE_HEIGHT, &packImageHeight);
	glGetIntegerv(GL_PACK_SKIP_ROWS, &packSkipRows);
	glGetIntegerv(GL_PACK_SKIP_PIXELS, &packSkipPixels);
	glGetIntegerv(GL_PACK_SKIP_IMAGES, &packSkipImages);
	glGetIntegerv(GL_PACK_ALIGNMENT, &packAlignment);

	// push gl state
	glReadBuffer(GL_FRONT);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	glPixelStorei(GL_PACK_SWAP_BYTES, 0);
	glPixelStorei(GL_PACK_LSB_FIRST, 0);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_IMAGE_HEIGHT, 0);
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_PACK_SKIP_IMAGES, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);

	// allocate data and read mPixels frome framebuffer
	tgaPixels = new GLubyte[tgaWidth*tgaHeight*3];
	glReadPixels(x, y, width, height, GL_BGR, GL_UNSIGNED_BYTE, tgaPixels);

	// compute new filename
	std::stringstream ss;
	ss << "screenshot";
	if(sShotCounter < 10)
		ss << '0';
	if(sShotCounter < 100)
		ss << '0';
	ss << sShotCounter << ".tga";

	// open file
	std::ofstream fileStream( ss.str().c_str(),
	                          std::ifstream::out | std::ifstream::binary );
	// check opening
	if(!fileStream)
		throw _FileNotFoundException(ss.str().c_str());

	// create header
	GLchar tgaHeader[18]=
	{
		0,                                     // image identification field
		0,                                     // colormap type
		2,                                     // image type code
		0,0,0,0,0,                             // color map spec (ignored here)
		0,0,                                   // x origin of image
		0,0,                                   // y origin of image
		tgaWidth & 255,  tgaWidth >> 8 & 255,  // width of the image
		tgaHeight & 255, tgaHeight >> 8 & 255, // height of the image
		24,                                    // bits per pixel
		0                                      // image descriptor byte
	};

	// write header and pixel data
	fileStream.write(tgaHeader, 18);
	fileStream.write(reinterpret_cast<const GLchar*>(tgaPixels),
	                 tgaWidth*tgaHeight*3);
	fileStream.close();

	// restore GL state
	glReadBuffer(readBuffer);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pixelPackBufferBinding);
	glPixelStorei(GL_PACK_SWAP_BYTES, packSwapBytes);
	glPixelStorei(GL_PACK_LSB_FIRST, packLsbFirst);
	glPixelStorei(GL_PACK_ROW_LENGTH, packRowLength);
	glPixelStorei(GL_PACK_IMAGE_HEIGHT, packImageHeight);
	glPixelStorei(GL_PACK_SKIP_ROWS, packSkipRows);
	glPixelStorei(GL_PACK_SKIP_PIXELS, packSkipPixels);
	glPixelStorei(GL_PACK_SKIP_IMAGES, packSkipImages);
	glPixelStorei(GL_PACK_ALIGNMENT, packAlignment);

	// free memory
	delete[] tgaPixels;

	// increment screenshot counter
	++sShotCounter;
}


////////////////////////////////////////////////////////////////////////////////
// Timer implementation
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Timer Constructor
Timer::Timer() : 
	mStartTicks(0.0), mStopTicks(0.0), mIsTicking()
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


////////////////////////////////////////////////////////////////////////////////
// Tga local functions/constants
//
////////////////////////////////////////////////////////////////////////////////

// thanks to : http://paulbourke.net/dataformats/tga/
enum _TgaImageType
{
	_TGA_TYPE_CM            = 1,
	_TGA_TYPE_RGB           = 2,
	_TGA_TYPE_LUMINANCE     = 3,
	_TGA_TYPE_CM_RLE        = 9,
	_TGA_TYPE_RGB_RLE       = 10,
	_TGA_TYPE_LUMINANCE_RLE = 11
};


////////////////////////////////////////////////////////////////////////////////
// Tga specific exceptions
//
////////////////////////////////////////////////////////////////////////////////
class _TgaLoaderException : public FWException
{
public:
	_TgaLoaderException(const std::string& filename, const std::string& log)
	{
		mMessage = "In file "+filename+": "+log;
	}
};

class _TgaInvalidDescriptorException : public FWException
{
public:
	_TgaInvalidDescriptorException()
	{
		mMessage = "Invalid TGA image descriptor.";
	}
};

class _TgaInvalidBppValueException : public FWException
{
public:
	_TgaInvalidBppValueException()
	{
		mMessage = "Invalid TGA bits per pixel amount.";
	}
};

class _TgaInvalidCmSizeException : public FWException
{
public:
	_TgaInvalidCmSizeException()
	{
		mMessage = "Invalid TGA colour map size.";
	}
};

class _TgaInvalidImageDescriptorByteException : public FWException
{
public:
	_TgaInvalidImageDescriptorByteException()
	{
		mMessage = "Invalid TGA image descriptor byte.";
	}
};


////////////////////////////////////////////////////////////////////////////////
// Tga implementation
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Flip horizontally
void Tga::_Flip()
{
	GLubyte* flippedData = new GLubyte[mWidth*mHeight*mPixelFormat];
	for(GLushort y = 0; y<mHeight; ++y)
		for(GLushort x = 0; x<mWidth; ++x)
			memcpy( &flippedData[y*mWidth*mPixelFormat+x*mPixelFormat],
			       &mPixels[(mWidth-y)*mWidth*mPixelFormat+x*mPixelFormat],
			       mPixelFormat );
	delete[] mPixels;
	mPixels = flippedData;
}


////////////////////////////////////////////////////////////////////////////////
// colour mapped images
void Tga::_LoadColourMapped( std::ifstream& fileStream,
                             GLchar* header )  throw(FWException)
{
	// set offset
	GLushort offset = _unpack_uint16(header[4], header[3])+header[0];
	fileStream.seekg(offset, std::ifstream::cur);

	// check image descriptor byte
	if(header[17]!=0)
		throw _TgaInvalidDescriptorException();

	// get colourMap indexes
	GLuint indexSize = header[16];
	if(indexSize<1)
		throw _TgaInvalidBppValueException();

	// check cm size
	GLuint colourMapSize = _unpack_uint16(header[5], header[6]);
	if(colourMapSize < 1)
		throw _TgaInvalidCmSizeException();

	GLubyte* colourMap = NULL;
	mPixelFormat = header[7]>>3;
	if(mPixelFormat==4 || mPixelFormat==3)
	{
		GLuint colourMapByteCnt = colourMapSize * mPixelFormat;
		colourMap = new GLubyte[colourMapByteCnt];
		fileStream.read( reinterpret_cast<GLchar*>(colourMap),
		                 colourMapByteCnt );
	}
	else if(mPixelFormat==2)
	{
		colourMapSize = _unpack_uint16(header[5], header[6]);
		GLuint colourMapByteCnt = colourMapSize * 3;
		colourMap = new GLubyte[colourMapByteCnt];

		GLuint maxIter_cm 	= colourMapSize*2;
		GLushort rgb16 			= 0;
		for(GLuint i=0; i<maxIter_cm; ++i)
		{
			fileStream.read(reinterpret_cast<GLchar*>(&rgb16), 2);
			colourMap[i*3]   = static_cast<GLubyte>((rgb16 & 0x001F)<<3);
			colourMap[i*3+1] = static_cast<GLubyte>(((rgb16 & 0x03E0)>>5)<<3);
			colourMap[i*3+2] = static_cast<GLubyte>(((rgb16 & 0x7C00)>>10)<<3);
		}
		mPixelFormat = Tga::PIXEL_FORMAT_BGR; // convert to bgr
	}
	else
		throw _TgaInvalidBppValueException();

	// allocate pixel data
	mPixels = new GLubyte[mWidth*mHeight*mPixelFormat];

	// read data
	GLuint maxIter = static_cast<GLuint>(mWidth)*static_cast<GLuint>(mHeight);
	GLubyte bytesPerIndex = header[16]>>3;
	GLuint index = 0;
	for(GLuint i=0; i<maxIter; ++i)
	{
		// read index
		fileStream.read(reinterpret_cast<GLchar*>(&index), bytesPerIndex);
		// get color
		memcpy( &mPixels[i*mPixelFormat],
		        &colourMap[index*mPixelFormat],
		        mPixelFormat );
	}

	// flip if necessary
	if(1==(header[17]>>5 & 0x01))
		_Flip();

	// free memory
	delete[] colourMap;
}


////////////////////////////////////////////////////////////////////////////////
void Tga::_LoadLuminance( std::ifstream& fileStream,
                          GLchar* header ) throw(FWException)
{
	// set offset
	GLuint offset = header[0]
	              + header[1]
	              * ( _unpack_uint16(header[4], header[3])
	              + _unpack_uint16(header[6], header[5] )
	              * (header[7]>>3));
	fileStream.seekg(offset, std::ifstream::cur);

	// read data depending on bits per pixel
	if(header[16]==8 || header[16]==16)
	{
		mPixelFormat = header[16] >> 3;
		mPixels      = new GLubyte[mWidth*mHeight*mPixelFormat];
		fileStream.read( reinterpret_cast<GLchar*>(mPixels),
		                 mWidth*mHeight*mPixelFormat);
	}
	else
		throw _TgaInvalidBppValueException();

	// flip if necessary
	if(1==(header[17]>>5 & 0x01))
		_Flip();
}


////////////////////////////////////////////////////////////////////////////////
// unmapped
void Tga::_LoadUnmapped( std::ifstream& fileStream,
                         GLchar* header ) throw(FWException)
{
	// set offset
	GLuint offset = header[0]
	              + header[1]
	              * ( _unpack_uint16(header[4], header[3])
	              + _unpack_uint16(header[6], header[5] )
	              * (header[7]>>3));
	fileStream.seekg(offset, std::ifstream::cur);

	// read data depending on bits per pixel
	if(header[16]==16)
	{
		mPixelFormat = 3;
		mPixels = new GLubyte[mWidth*mHeight*mPixelFormat];
		// load variables
		GLuint maxIter  = static_cast<GLuint>(mWidth)
		                * static_cast<GLuint>(mHeight);
		GLushort rgb16  = 0;
		for(GLuint i=0; i<maxIter; ++i)
		{
			fileStream.read(reinterpret_cast<GLchar*>(&rgb16), 2);
			mPixels[i*3]     = static_cast<GLubyte>((rgb16 & 0x001F)<<3);
			mPixels[i*3+1]   = static_cast<GLubyte>(((rgb16 & 0x03E0)>>5)<<3);
			mPixels[i*3+2]   = static_cast<GLubyte>(((rgb16 & 0x7C00)>>10)<<3);
		}
	}
	else if(header[16]==24 || header[16]==32)
	{
		mPixelFormat = header[16] >> 3;
		mPixels      = new GLubyte[mWidth*mHeight*mPixelFormat];
		fileStream.read( reinterpret_cast<GLchar*>(mPixels),
		                 mWidth*mHeight*mPixelFormat );
	}
	else
		throw _TgaInvalidBppValueException();

	// flip if necessary
	if(1==(header[17]>>5 & 0x01))
		_Flip();
}


////////////////////////////////////////////////////////////////////////////////
// unmapped rle
void Tga::_LoadUnmappedRle( std::ifstream& fileStream,
                            GLchar* header ) throw(FWException)
{
	// set offset
	GLuint offset = header[0]
	              + header[1]
	              * ( _unpack_uint16(header[4], header[3])
	              + _unpack_uint16(header[6], header[5] )
	              * (header[7]>>3));
	fileStream.seekg(offset, std::ifstream::cur);

	// read data depending on bits per pixel
	if(header[16]==16)
	{
		mPixelFormat = 3;   // will be converted to bgr
		mPixels = new GLubyte[mWidth*mHeight*mPixelFormat];
		GLubyte* dataPtrMax = mPixels + (mWidth*mHeight*mPixelFormat);
		GLubyte* dataPtr = mPixels;
		GLint blockSize, blockIter;
		GLubyte packetHeader = 0;
		GLushort rgb16 = 0;  // compressed data
		while(dataPtr < dataPtrMax)
		{
			// reset data block iterator
			blockIter = 1;
			// read first byte
			fileStream.read( reinterpret_cast<GLchar*>(&packetHeader),
			                 sizeof(GLubyte));
			blockSize = 1 + (packetHeader & 0x7F);
			fileStream.read(reinterpret_cast<GLchar*>(&rgb16), 16);  // read 16 bits
			dataPtr[0] = static_cast<GLubyte>((rgb16 & 0x001F) << 3);
			dataPtr[1] = static_cast<GLubyte>(((rgb16 & 0x03E0) >> 5) << 3);
			dataPtr[2] = static_cast<GLubyte>(((rgb16 & 0x7C00) >> 10) << 3);
			if(packetHeader & 0x80)   // rle
			{
				for(; blockIter<blockSize; ++blockIter)
					memcpy( dataPtr + blockIter * mPixelFormat,
					        dataPtr,
					        mPixelFormat );
			}
			else
				for(; blockIter<blockSize; ++blockIter)
				{
					fileStream.read(reinterpret_cast<GLchar*>(&rgb16), 16);  // read 16 bits
					dataPtr[blockIter*mPixelFormat+0] = static_cast<GLubyte>((rgb16 & 0x001F) << 3);
					dataPtr[blockIter*mPixelFormat+1] = static_cast<GLubyte>(((rgb16 & 0x03E0) >> 5) << 3);
					dataPtr[blockIter*mPixelFormat+2] = static_cast<GLubyte>(((rgb16 & 0x7C00) >> 10) << 3);
				}
			// increment pixel ptr
			dataPtr+=mPixelFormat*blockSize;
		}
	}
	else if(header[16]==24 || header[16]==32)
	{
		mPixelFormat = header[16] >> 3;
		mPixels      = new GLubyte[mWidth*mHeight*mPixelFormat];

		// read rle
		GLuint blockSize, blockIter;
		GLubyte* dataPtrMax  = mPixels + mWidth*mHeight*mPixelFormat;
		GLubyte* dataPtr     = mPixels;
		GLubyte packetHeader = 0u;
		while(dataPtr < dataPtrMax)
		{
			// reset data block iterator
			blockIter = 1u;
			// read first byte
			fileStream.read(reinterpret_cast<GLchar*>(&packetHeader), 1);
			blockSize = 1u + (packetHeader & 0x7F);
			fileStream.read(reinterpret_cast<GLchar*>(dataPtr), mPixelFormat);
			if(packetHeader & 0x80u)   // rle
				for(; blockIter<blockSize; ++blockIter)
					memcpy(dataPtr+blockIter*mPixelFormat, dataPtr, mPixelFormat);
			else
				for(; blockIter<blockSize; ++blockIter)
					fileStream.read(reinterpret_cast<GLchar*>(dataPtr
					                                         +blockIter
					                                         *mPixelFormat),
					                mPixelFormat);
			// increment pixel ptr
			dataPtr+=mPixelFormat*blockSize;
		}
	}
	else
		throw _TgaInvalidBppValueException();


	// flip if necessary
	if(1==(header[17]>>5 & 0x01))
		_Flip();
}


////////////////////////////////////////////////////////////////////////////////
// colour mapped images rle
void Tga::_LoadColourMappedRle( std::ifstream& fileStream,
                                GLchar* header) throw(FWException)
{
	// set offset
	GLushort offset = _unpack_uint16(header[4], header[3])+header[0];
	fileStream.seekg(offset, std::ifstream::cur);

	// check image descriptor byte
	if(header[17]!=0)
		throw _TgaInvalidImageDescriptorByteException();

	// get colourMap indexes
	GLuint indexSize = header[16];
	if(indexSize<1)
		throw _TgaInvalidBppValueException();

	// check cm size
	GLuint colourMapSize = _unpack_uint16(header[5], header[6]);
	if(colourMapSize < 1)
		throw _TgaInvalidCmSizeException();

	GLubyte* colourMap = NULL;
	mPixelFormat = header[7]>>3;
	if(mPixelFormat==4 || mPixelFormat==3)
	{
		GLuint colourMapByteCnt = colourMapSize * mPixelFormat;
		colourMap = new GLubyte[colourMapByteCnt];
		fileStream.read(reinterpret_cast<GLchar*>(colourMap), colourMapByteCnt);
	}
	else if(mPixelFormat==2)
	{
		colourMapSize = _unpack_uint16(header[5], header[6]);
		GLuint colourMapByteCnt = colourMapSize * 3;
		colourMap = new GLubyte[colourMapByteCnt];

		GLuint maxIter_cm = colourMapSize*2;
		GLushort rgb16    = 0;
		for(GLuint i=0; i<maxIter_cm; ++i)
		{
			fileStream.read(reinterpret_cast<GLchar*>(&rgb16), 2);
			colourMap[i*3]   = static_cast<GLubyte>((rgb16 & 0x001F)<<3);
			colourMap[i*3+1] = static_cast<GLubyte>(((rgb16 & 0x03E0)>>5)<<3);
			colourMap[i*3+2] = static_cast<GLubyte>(((rgb16 & 0x7C00)>>10)<<3);
		}
		mPixelFormat = 3; // convert to bgr
	}
	else
		throw _TgaInvalidBppValueException();

	// allocate pixel data
	mPixels = new GLubyte[mWidth*mHeight*mPixelFormat];

	// read rle
	GLubyte* dataPtrMax = mPixels + mWidth*mHeight*mPixelFormat;
	GLubyte* dataPtr = mPixels;
	GLuint blockSize, blockIter;
	GLubyte packetHeader = 0u;
	GLubyte bytesPerIndex = header[16]>>3;
	GLuint index = 0;
	while(dataPtr < dataPtrMax)
	{
		// reset data block iterator
		blockIter = 1u;
		// read first byte
		fileStream.read(reinterpret_cast<GLchar*>(&packetHeader), 1);
		blockSize = 1u + (packetHeader & 0x7F);
		fileStream.read(reinterpret_cast<GLchar*>(&index), bytesPerIndex);
		memcpy(dataPtr, &colourMap[index*mPixelFormat], mPixelFormat);
		if(packetHeader & 0x80u)   // rle
			for(; blockIter<blockSize; ++blockIter)
				memcpy(dataPtr+blockIter*mPixelFormat, dataPtr, mPixelFormat);
		else // raw
			for(; blockIter<blockSize; ++blockIter)
			{
				fileStream.read(reinterpret_cast<GLchar*>(&index),
				                bytesPerIndex);
				memcpy( dataPtr+blockIter*mPixelFormat,
				        &colourMap[index*mPixelFormat], 
				        mPixelFormat );
			}
		// increment pixel ptr
		dataPtr+=blockSize*mPixelFormat;
	}

	// flip if necessary
	if(1==(header[17]>>5 & 0x01))
		_Flip();

	// free memory
	delete[] colourMap;
}

////////////////////////////////////////////////////////////////////////////////
// luminance images rle
void Tga::_LoadLuminanceRle( std::ifstream& fileStream,
                             GLchar* header) throw(FWException)
{
	// set offset
	GLuint offset = header[0]
	              + header[1]
	              *( _unpack_uint16(header[4], header[3])
	              + _unpack_uint16(header[6], header[5])
	              * (header[7]>>3) );
	fileStream.seekg(offset, std::ifstream::cur);

	// read data depending on bits per pixel
	if(header[16]==8 || header[16]==16)
	{
		mPixelFormat = header[16] >> 3;
		mPixels      = new GLubyte[mWidth*mHeight*mPixelFormat];

		// read rle
		GLubyte* dataPtrMax = mPixels + mWidth*mHeight*mPixelFormat;
		GLubyte* dataPtr = mPixels;
		GLuint blockSize, blockIter;
		GLubyte packetHeader = 0u;
		while(dataPtr < dataPtrMax)
		{
			// reset data block iterator
			blockIter = 1u;
			// read first byte
			fileStream.read(reinterpret_cast<GLchar*>(&packetHeader), 1);
			blockSize = 1u + (packetHeader & 0x7F);
			fileStream.read(reinterpret_cast<GLchar*>(dataPtr), mPixelFormat);
			if(packetHeader & 0x80u)   // rle
				for(; blockIter<blockSize; ++blockIter)
					memcpy( dataPtr+blockIter*mPixelFormat,
					        dataPtr,
					        mPixelFormat );
			else
				for(; blockIter<blockSize; ++blockIter)
					fileStream.read( reinterpret_cast<GLchar*>(dataPtr
					                                          +blockIter
					                                          *mPixelFormat),
					                 mPixelFormat );
			// increment pixel ptr
			dataPtr+=mPixelFormat*blockSize;
		}
	}
	else
		throw _TgaInvalidBppValueException();

	// flip if necessary
	if(1==(header[17]>>5 & 0x01))
		_Flip();
}


////////////////////////////////////////////////////////////////////////////////
// Default constructor
Tga::Tga(): 
mPixels(NULL),
mWidth(0), mHeight(0),
mPixelFormat(PIXEL_FORMAT_UNKNOWN)
{
}


////////////////////////////////////////////////////////////////////////////////
// Overloaded constructor
Tga::Tga(const std::string& filename) throw(FWException):
mPixels(NULL),
mWidth(0), mHeight(0),
mPixelFormat(PIXEL_FORMAT_UNKNOWN)
{
	Load(filename);
}


////////////////////////////////////////////////////////////////////////////////
// Destructor
Tga::~Tga()
{
	_Clear();
}


////////////////////////////////////////////////////////////////////////////////
// Load from file
void Tga::Load(const std::string& filename) throw(FWException)
{
	// Clear memory if necessary
	_Clear();

	std::ifstream fileStream( filename.c_str(),
	                          std::ifstream::in | std::ifstream::binary );
	if(!fileStream)
		throw _FileNotFoundException(filename);

	// read header
	GLchar header[18];   // header is 18 bytes
	fileStream.read(reinterpret_cast<char*>(header), 18);

	// get data
	mWidth  = _unpack_uint16(header[13],header[12]);
	mHeight = _unpack_uint16(header[15],header[14]);

	// check image dimensions and pixel data
	if(0 == mWidth * mHeight)
		throw _TgaLoaderException(filename, "Invalid TGA dimensions.");

	// load data according to image type code
	try
	{
		if(header[2]==_TGA_TYPE_RGB)
			_LoadUnmapped(fileStream, header);
		else if(header[2]==_TGA_TYPE_CM)
			_LoadColourMapped(fileStream, header);
		else if(header[2]==_TGA_TYPE_LUMINANCE)
			_LoadLuminance(fileStream, header);
		else if(header[2]==_TGA_TYPE_CM_RLE)
			_LoadColourMappedRle(fileStream, header);
		else if(header[2]==_TGA_TYPE_RGB_RLE)
			_LoadUnmappedRle(fileStream, header);
		else if(header[2]==_TGA_TYPE_LUMINANCE_RLE)
			_LoadLuminanceRle(fileStream, header);
		else
			throw _TgaLoaderException(filename, "Unknown TGA image type code.");
	}
	catch(FWException& e)
	{
		throw _TgaLoaderException(filename, e.what());
	}
	catch(...)
	{
		throw _TgaLoaderException(filename, "Unknown error occured.");
	}

	fileStream.close();
}


//////////////////////////////////////////////////////////////////////////////////
//// Save to file
//void Tga::Save(const std::string& filename) throw(FWException)
//{
//	std::ofstream fileStream( filename.c_str(),
//	                          std::ifstream::out | std::ifstream::binary );

//	if(!fileStream)
//		throw _TgaLoaderException(filename, "Could not open file.");

//	if(NULL == mPixels)
//		throw _TgaLoaderException(filename, "No pixel data in object.");

//	// build header
//	const GLchar header[18]=
//	{
//		0,                                 // image identification field
//		0,                                 // colormap type
//		mPixelFormat < 3 ? 3 : 2,          // image type code
//		0,0,0,0,0,                         // color map spec (ignored here)
//		0,0,                               // x origin of image
//		0,0,                               // y origin of image
//		mWidth & 255,  mWidth >> 8 & 255,  // width of the image
//		mHeight & 255, mHeight >> 8 & 255, // height of the image
//		mPixelFormat << 3,                 // bits per pixel
//		0                                  // image descriptor byte
//	};

//	// emit header, and pixel images
//	fileStream.write(header, 18);
//	fileStream.write(reinterpret_cast<const GLchar*>(mPixels),
//	                 mWidth*mHeight*mPixelFormat);
//	fileStream.close();
//}


////////////////////////////////////////////////////////////////////////////////
// Clear
void Tga::_Clear()
{
	delete[] mPixels;
	mPixels = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Accessors
GLushort Tga::Width()    const {return mWidth;}
GLushort Tga::Height()   const {return mHeight;}
GLint Tga::PixelFormat() const {return mPixelFormat;}
GLubyte* Tga::Pixels()   const {return mPixels;}

} // namespace fw

