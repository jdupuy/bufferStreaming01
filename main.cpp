////////////////////////////////////////////////////////////////////////////////
// \author   Jonathan Dupuy
//
////////////////////////////////////////////////////////////////////////////////

// enable gui
#define _ANT_ENABLE

// GL libraries
#include "glew.hpp"
#include "GL/freeglut.h"

#ifdef _ANT_ENABLE
#	include "AntTweakBar.h"
#endif // _ANT_ENABLE

// Custom libraries
#include "Algebra.hpp"      // Basic algebra library
#include "Transform.hpp"    // Basic transformations
#include "Framework.hpp"    // utility classes/functions
#include "Md2.hpp"          // MD2 model loader/player

// Standard librabries
#include <iostream>
#include <sstream>
#include <vector>
#include <stdexcept>


////////////////////////////////////////////////////////////////////////////////
// Global variables
//
////////////////////////////////////////////////////////////////////////////////

// Constants
const GLuint STREAM_BUFFER_CAPACITY = 8192*1024; // 8MBytes
enum // OpenGLNames
{
	// buffers
	BUFFER_VERTEX_MD2 = 0,
	BUFFER_COUNT,

	// vertex arrays
	VERTEX_ARRAY_MD2 = 0,
	VERTEX_ARRAY_COUNT,

	// textures
	TEXTURE_SKIN_MD2 = 0,
	TEXTURE_COUNT,

	// programs
	PROGRAM_RENDER_MD2 = 0,
	PROGRAM_COUNT
};

// OpenGL objects
GLuint *buffers      = NULL;
GLuint *vertexArrays = NULL;
GLuint *textures     = NULL;
GLuint *programs     = NULL;

// Resources
Md2* md2 = NULL; // md2 model and texture

// Tools
Affine model          = Affine::Translation(Vector3(0,0,-400));
Projection projection = Projection::Perspective(50.0f, 1.0f, 10.0f, 4000.0f);
bool mouseLeft  = false;
bool mouseRight = false;

#ifdef _ANT_ENABLE
std::string activeAnimation;  // active animation name
double streamingTime   = 0.0; // streaming time, in ms
double framesPerSecond = 0.0; // fps
#endif

////////////////////////////////////////////////////////////////////////////////
// Functions
//
////////////////////////////////////////////////////////////////////////////////


#ifdef _ANT_ENABLE
static void TW_CALL play_next_animation(void *data)
{
	md2->NextAnimation();
}

static void TW_CALL toggle_fullscreen(void *data)
{
	// toggle fullscreen
	glutFullScreenToggle();
}

static void TW_CALL play_pause(void* data)
{
	if(md2->IsPlaying())
		md2->Pause();
	else
		md2->Play();
}

#endif

////////////////////////////////////////////////////////////////////////////////
// on init cb
void on_init()
{
	// load Md2 model
	md2 = new Md2("droid.md2");

	// alloc names
	buffers      = new GLuint[BUFFER_COUNT];
	vertexArrays = new GLuint[VERTEX_ARRAY_COUNT];
	textures     = new GLuint[TEXTURE_COUNT];
	programs     = new GLuint[PROGRAM_COUNT];

	// gen names
	glGenBuffers(BUFFER_COUNT, buffers);
	glGenVertexArrays(VERTEX_ARRAY_COUNT, vertexArrays);
	glGenTextures(TEXTURE_COUNT, textures);
	for(GLuint i=0; i<PROGRAM_COUNT;++i)
		programs[i] = glCreateProgram();

	// configure texture
	fw::Tga tga("droid.tga");
	glActiveTexture(GL_TEXTURE0+TEXTURE_SKIN_MD2);
	glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_SKIN_MD2]);
		glTexParameteri( GL_TEXTURE_2D,
		                 GL_TEXTURE_MAG_FILTER,
		                 GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D,
		                 GL_TEXTURE_MIN_FILTER,
		                 GL_LINEAR_MIPMAP_LINEAR );
	if(tga.PixelFormat() == fw::Tga::PIXEL_FORMAT_LUMINANCE)
		glTexImage2D( GL_TEXTURE_2D,
		              0,
		              GL_RED,
		              tga.Width(),
		              tga.Height(),
		              0,
		              GL_RED,
		              GL_UNSIGNED_BYTE,
		              tga.Pixels() );
	else if(tga.PixelFormat() == fw::Tga::PIXEL_FORMAT_LUMINANCE_ALPHA)
		glTexImage2D( GL_TEXTURE_2D,
		              0,
		              GL_RG,
		              tga.Width(),
		              tga.Height(),
		              0,
		              GL_RG,
		              GL_UNSIGNED_BYTE,
		              tga.Pixels() );
	else if(tga.PixelFormat() == fw::Tga::PIXEL_FORMAT_BGR)
		glTexImage2D( GL_TEXTURE_2D,
		              0,
		              GL_RGB,
		              tga.Width(),
		              tga.Height(),
		              0,
		              GL_BGR,
		              GL_UNSIGNED_BYTE,
		              tga.Pixels() );
	else if(tga.PixelFormat() == fw::Tga::PIXEL_FORMAT_BGRA)
		glTexImage2D( GL_TEXTURE_2D,
		              0,
		              GL_RGBA,
		              tga.Width(),
		              tga.Height(),
		              0,
		              GL_BGRA,
		              GL_UNSIGNED_BYTE,
		              tga.Pixels() );
	glGenerateMipmap(GL_TEXTURE_2D);


	// configure buffer objects
	glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_VERTEX_MD2]);
		glBufferData(GL_ARRAY_BUFFER, 
		             STREAM_BUFFER_CAPACITY,
		             NULL,
		             GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// configure vertex arrays
	glBindVertexArray(vertexArrays[VERTEX_ARRAY_MD2]);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_VERTEX_MD2]);
		glVertexAttribPointer( 0, 3, GL_FLOAT, 0, sizeof(Md2::Vertex),
		                       FW_BUFFER_OFFSET(0) );
		glVertexAttribPointer( 1, 3, GL_FLOAT, 0, sizeof(Md2::Vertex),
		                       FW_BUFFER_OFFSET(3*sizeof(GLfloat)));
		glVertexAttribPointer( 2, 2, GL_FLOAT, 0, sizeof(Md2::Vertex),
		                       FW_BUFFER_OFFSET(6*sizeof(GLfloat)));
	glBindVertexArray(0);

	// configure programs
	fw::build_glsl_program(programs[PROGRAM_RENDER_MD2],
	                       "md2.glsl",
	                       "",
	                       GL_TRUE);
	glProgramUniform1i( programs[PROGRAM_RENDER_MD2], 
	                    glGetUniformLocation( programs[PROGRAM_RENDER_MD2], 
	                                          "sSkin"),
	                    TEXTURE_SKIN_MD2 );

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(0.13,0.13,0.15,1.0);

#ifdef _ANT_ENABLE
	// start ant
	TwInit(TW_OPENGL, NULL);
	// send the ''glutGetModifers'' function pointer to AntTweakBar
	TwGLUTModifiersFunc(glutGetModifiers);

	// Create a new bar
	TwBar* menuBar = TwNewBar("menu");
	TwDefine("menu size='250 150'");
	TwAddButton( menuBar,
	             "fullscreen",
	             &toggle_fullscreen,
	             NULL,
	             "label='toggle fullscreen'");
	TwAddButton( menuBar,
	             "nextAnim",
	             &play_next_animation,
	             NULL,
	             "label='next animation'");
	TwAddButton( menuBar,
	             "playPause",
	             &play_pause,
	             NULL,
	             "label='play/pause animation'");
	TwAddVarRO( menuBar,
	            "speed",
	            TW_TYPE_DOUBLE,
	            &streamingTime,
	            "label='streaming speed (ms)'");
	TwAddVarRO( menuBar,
	            "fps",
	            TW_TYPE_DOUBLE,
	            &framesPerSecond,
	            "label='frames per second'");
#endif // _ANT_ENABLE

	fw::check_gl_error();
}


////////////////////////////////////////////////////////////////////////////////
// on clean cb
void on_clean()
{
	delete md2;

	// delete objects
	glDeleteBuffers(BUFFER_COUNT, buffers);
	glDeleteVertexArrays(VERTEX_ARRAY_COUNT, vertexArrays);
	glDeleteTextures(TEXTURE_COUNT, textures);
	for(GLuint i=0; i<PROGRAM_COUNT;++i)
		glDeleteProgram(programs[i]);

	// release memory
	delete[] buffers;
	delete[] vertexArrays;
	delete[] textures;
	delete[] programs;

#ifdef _ANT_ENABLE
	TwTerminate();
#endif // _ANT_ENABLE

	fw::check_gl_error();
}


////////////////////////////////////////////////////////////////////////////////
// on update cb
void on_update()
{
	// Global variable
	static fw::Timer deltaTimer;
	GLint windowWidth  = glutGet(GLUT_WINDOW_WIDTH);
	GLint windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

	// stop the timer during update
	deltaTimer.Stop();

	// set viewport
	glViewport(0,0,windowWidth, windowHeight);

	// clear back buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// update md2 animation
	md2->Update(deltaTimer.Ticks());

#ifdef _ANT_ENABLE
	// Bench stream
	fw::Timer streamTimer;
	streamTimer.Start();
	// Compute fps
	framesPerSecond = 1.0/deltaTimer.Ticks();
#endif // _ANT_ENABLE

	// stream vertices (if necessary)
	static GLuint streamOffset = 0;
	static GLuint drawOffset   = 0;
	if(md2->IsPlaying())
	{
		// bind the buffer
		glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_VERTEX_MD2]);
		// orphan the buffer if full
		GLuint streamDataSize = fw::next_power_of_two(md2->TriangleCount()
		                                              *3*sizeof(Md2::Vertex));
		if(streamOffset + streamDataSize > STREAM_BUFFER_CAPACITY)
		{
			// allocate new space and reset the vao
			glBufferData( GL_ARRAY_BUFFER,
			              STREAM_BUFFER_CAPACITY,
			              NULL,
			              GL_STREAM_DRAW );
			glBindVertexArray(vertexArrays[VERTEX_ARRAY_MD2]);
				glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_VERTEX_MD2]);
				glVertexAttribPointer( 0, 3, GL_FLOAT, 0, sizeof(Md2::Vertex),
				                       FW_BUFFER_OFFSET(0) );
				glVertexAttribPointer( 1, 3, GL_FLOAT, 0, sizeof(Md2::Vertex),
				                       FW_BUFFER_OFFSET(3*sizeof(GLfloat)));
				glVertexAttribPointer( 2, 2, GL_FLOAT, 0, sizeof(Md2::Vertex),
				                       FW_BUFFER_OFFSET(6*sizeof(GLfloat)));
			glBindVertexArray(0);
			// reset offset
			streamOffset = 0;
		}

		// get memory safely
		Md2::Vertex* vertices = (Md2::Vertex*)
		                        (glMapBufferRange(GL_ARRAY_BUFFER, 
		                                          streamOffset, 
		                                          streamDataSize, 
		                                          GL_MAP_WRITE_BIT
		                                          |GL_MAP_UNSYNCHRONIZED_BIT));

		// make sure memory is mapped
		if(NULL == vertices)
			throw std::runtime_error("Failed to map buffer.");

		// set final data
		md2->GenVertices(vertices);

		// unmap buffer
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// compute draw offset
		drawOffset = streamOffset/sizeof(Md2::Vertex);

		// increment offset
		streamOffset += streamDataSize;
	}

#ifdef _ANT_ENABLE
	// End bench
	streamTimer.Stop();
	streamingTime = streamTimer.Ticks()*1000.0; // convert to milliseconds
#endif // _ANT_ENABLE

	// update transformations
	projection.FitHeightToAspect(float(windowWidth)/float(windowHeight));

	Matrix4x4 mvp = projection.ExtractTransformMatrix()
	              * model.ExtractTransformMatrix()
	              * Matrix4x4(0, 1, 0, 0,
	                          0, 0,-1, 0,
	                          1, 0, 0, 0,
	                          0, 0, 0, 1);

	glProgramUniformMatrix4fv(programs[PROGRAM_RENDER_MD2],
	                          glGetUniformLocation(programs[PROGRAM_RENDER_MD2],
	                                         "uModelViewProjection"),
	                          1,
	                          0,
	                          reinterpret_cast<float*>(&mvp));

	// render the model
	glUseProgram(programs[PROGRAM_RENDER_MD2]);
	glBindVertexArray(vertexArrays[VERTEX_ARRAY_MD2]);
	glDrawArrays( GL_TRIANGLES,
	              drawOffset,
	              md2->TriangleCount()*3);

	// back to default vertex array
	glBindVertexArray(0);

#ifdef _ANT_ENABLE
	TwDraw();
#endif // _ANT_ENABLE

	fw::check_gl_error();

	// restart timer
	deltaTimer.Start();

	glutSwapBuffers();
	glutPostRedisplay();
}


////////////////////////////////////////////////////////////////////////////////
// on resize cb
void on_resize(GLint w, GLint h)
{
#ifdef _ANT_ENABLE
	TwWindowSize(w, h);
#endif
}


////////////////////////////////////////////////////////////////////////////////
// on key down cb
void on_key_down(GLubyte key, GLint x, GLint y)
{
#ifdef _ANT_ENABLE
	if(1==TwEventKeyboardGLUT(key, x, y))
		return;
#endif
	if (key==27) // escape
		glutLeaveMainLoop();
	if(key=='f')
		glutFullScreenToggle();
	if(key=='p')
		fw::save_gl_front_buffer(0,
		                         0,
		                         glutGet(GLUT_WINDOW_WIDTH),
		                         glutGet(GLUT_WINDOW_HEIGHT));

}


////////////////////////////////////////////////////////////////////////////////
// on mouse button cb
void on_mouse_button(GLint button, GLint state, GLint x, GLint y)
{
#ifdef _ANT_ENABLE
	if(1 == TwEventMouseButtonGLUT(button, state, x, y))
		return;
#endif // _ANT_ENABLE

	if(state==GLUT_DOWN)
	{
		mouseLeft  |= button == GLUT_LEFT_BUTTON;
		mouseRight |= button == GLUT_RIGHT_BUTTON;
	}
	else
	{
		mouseLeft  &= button == GLUT_LEFT_BUTTON  ? false : mouseLeft;
		mouseRight &= button == GLUT_RIGHT_BUTTON ? false : mouseRight;
	}
	if(button == 3)
		model.TranslateWorld(Vector3(0,0,1.0f));
	if(button == 4)
		model.TranslateWorld(Vector3(0,0,-1.0f));
}


////////////////////////////////////////////////////////////////////////////////
// on mouse motion cb
void on_mouse_motion(GLint x, GLint y)
{
#ifdef _ANT_ENABLE
	if(1 == TwEventMouseMotionGLUT(x,y))
		return;
#endif // _ANT_ENABLE

	static GLint sMousePreviousX = 0;
	static GLint sMousePreviousY = 0;
	const GLint MOUSE_XREL = x-sMousePreviousX;
	const GLint MOUSE_YREL = y-sMousePreviousY;
	sMousePreviousX = x;
	sMousePreviousY = y;

	if(mouseLeft)
	{
		model.RotateAboutWorldX(-0.01f*MOUSE_YREL);
		model.RotateAboutLocalY(-0.01f*MOUSE_XREL);
	}

}


////////////////////////////////////////////////////////////////////////////////
// on mouse wheel cb
void on_mouse_wheel(GLint wheel, GLint direction, GLint x, GLint y)
{
#ifdef _ANT_ENABLE
	if(1 == TwMouseWheel(wheel))
		return;
#endif // _ANT_ENABLE
	model.TranslateWorld(Vector3(0,0,direction));
}


////////////////////////////////////////////////////////////////////////////////
// Main
//
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	const GLuint CONTEXT_MAJOR = 4;
	const GLuint CONTEXT_MINOR = 2;

	// init glut
	glutInit(&argc, argv);
	glutInitContextVersion(CONTEXT_MAJOR ,CONTEXT_MINOR);
#ifdef _ANT_ENABLE
	glutInitContextFlags(GLUT_DEBUG);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
#else
	glutInitContextFlags(GLUT_DEBUG | GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);
#endif

	// build window
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("OpenGLBufferStreaming");

	// init glew
	GLenum err = glewInit();
	if(GLEW_OK != err)
	{
		std::stringstream ss;
		ss << err;
		std::cerr << "glewInit() gave error " << ss.str() << std::endl;
		return 1;
	}

	// glewInit generates an INVALID_ENUM error for some reason...
	glGetError();

	// set callbacks
	glutCloseFunc(&on_clean);
	glutReshapeFunc(&on_resize);
	glutDisplayFunc(&on_update);
	glutKeyboardFunc(&on_key_down);
	glutMouseFunc(&on_mouse_button);
	glutPassiveMotionFunc(&on_mouse_motion);
	glutMotionFunc(&on_mouse_motion);
	glutMouseWheelFunc(&on_mouse_wheel);

	// run
	try
	{
		// run demo
		on_init();
		glutMainLoop();
	}
	catch(std::exception& e)
	{
		std::cerr << "Fatal exception: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}

