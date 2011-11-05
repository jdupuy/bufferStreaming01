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
#include "Framework.hpp"    // utility classes/functions
#include "gmath.hpp"        // basic math library
#include "Md2.hpp"

// Standard librabries
#include <iostream>
#include <sstream>
#include <vector>


////////////////////////////////////////////////////////////////////////////////
// Global variables
//
////////////////////////////////////////////////////////////////////////////////

// Frame object
struct Frame
{
	vec3f translation;
	vec3f rotation;
};

// Constants
const GLsizei STREAM_BUFFER_CAPACITY = 8192*1024; // 8MBytes
enum // OpenGLNames
{
	// buffers
	BUFFER_VERTEX_MD2 = 0, BUFFER_COUNT,
	// vertex arrays
	VERTEX_ARRAY_MD2 = 0, VERTEX_ARRAY_COUNT,
	// textures
	TEXTURE_SKIN_MD2 = 0, TEXTURE_COUNT,
	// programs
	PROGRAM_RENDER_MD2 = 0, PROGRAM_COUNT
};

// OpenGL objects
GLuint *buffers      = NULL;
GLuint *vertexArrays = NULL;
GLuint *textures     = NULL;
GLuint *programs     = NULL;

// Resources
// md2 model and texture
Md2* md2 = NULL;
Frame frameMd2;

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
//	activeAnimation = md2->ActiveAnimationName();
}

static void TW_CALL toggle_fullscreen(void *data)
{
	// toggle fullscreen
	glutFullScreenToggle();
}

#endif

////////////////////////////////////////////////////////////////////////////////
// on init cb
void on_init()
{
	// load Md2 model
	md2 = new Md2("knight.md2");
	frameMd2.translation = vec3f(-50,0,0);
	frameMd2.rotation    = vec3f(0,0,0);

	// alloc names
	buffers      = new GLuint[BUFFER_COUNT];
	vertexArrays = new GLuint[VERTEX_ARRAY_COUNT];
	textures     = new GLuint[TEXTURE_COUNT];
	programs     = new GLuint[PROGRAM_COUNT];

	// gen names
	glGenBuffers(BUFFER_COUNT, buffers);
	glGenVertexArrays(BUFFER_COUNT, vertexArrays);
	glGenTextures(TEXTURE_COUNT, textures);
	for(GLuint i=0; i<PROGRAM_COUNT;++i)
		programs[i] = glCreateProgram();

	// configure texture
	fw::Tga tga("knight.tga");
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
		glBufferData(GL_ARRAY_BUFFER, STREAM_BUFFER_CAPACITY, NULL, GL_STREAM_DRAW);
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
	fw::build_glsl_program(programs[PROGRAM_RENDER_MD2], "md2.glsl", "", GL_TRUE);
	glProgramUniform1i( programs[PROGRAM_RENDER_MD2], 
	                    glGetUniformLocation( programs[PROGRAM_RENDER_MD2], 
	                                          "sSkin"),
	                    TEXTURE_SKIN_MD2 );

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0,0.0,0.0,1.0);

#ifdef _ANT_ENABLE
	// start ant
	TwInit(TW_OPENGL, NULL);
	// send the ''glutGetModifers'' function pointer to AntTweakBar
	TwGLUTModifiersFunc(glutGetModifiers);

	// Create a new bar
	TwBar* menuBar = TwNewBar("menu");
	TwDefine("menu size='250 100'");
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
	glDeleteVertexArrays(BUFFER_COUNT, vertexArrays);
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

	// update md2 animation
	md2->Update(deltaTimer.Ticks());

#ifdef _ANT_ENABLE
	// Bench stream
	fw::Timer streamTimer;
	streamTimer.Start();
	// Compute fps
	framesPerSecond = 1.0/deltaTimer.Ticks();
#endif // _ANT_ENABLE
	// stream vertices
	static GLuint streamOffset = 0;
	// bind the buffer
	glBindBuffer(GL_ARRAY_BUFFER, buffers[BUFFER_VERTEX_MD2]);
	// orphan if full
	GLuint streamDataSize = fw::next_power_of_two(md2->TriangleCount()*3*sizeof(Md2::Vertex));
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

	// get memory asynchronously AND safely
	Md2::Vertex* vertices = (Md2::Vertex*)
	                        (glMapBufferRange( GL_ARRAY_BUFFER, 
	                                           streamOffset, 
	                                           streamDataSize, 
	                                           GL_MAP_WRITE_BIT
	                                           |GL_MAP_UNSYNCHRONIZED_BIT ));

	// set final data
	md2->GenVertices(vertices);

	// unmap buffer
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

#ifdef _ANT_ENABLE
	// End bench
	streamTimer.Stop();
	streamingTime = streamTimer.Ticks()*1000.0; // convert to milliseconds
#endif // _ANT_ENABLE


	// set viewport
	glViewport(0,0,windowWidth, windowHeight);

	// clear back buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render the model
	glUseProgram(programs[PROGRAM_RENDER_MD2]);
	// build and upload mvp
	mat4f projection = perspective( 80.0f,
	                                float(windowWidth)/windowHeight,
	                                0.01f,
	                                1000.0f);
	mat4f model      = mat4f(0,1,0,frameMd2.translation.y,
	                         0,0,1,frameMd2.translation.z,
	                         1,0,0,frameMd2.translation.x,
	                         0,0,0,1);
	mat3f faxis = rotatey(frameMd2.rotation.y) * rotatez(frameMd2.rotation.z);
	model *= mat4f( faxis[0][0], faxis[1][0], faxis[2][0], 0,
	                faxis[0][1], faxis[1][1], faxis[2][1], 0,
	                faxis[0][2], faxis[1][2], faxis[2][2], 0,
	                0, 0, 0, 1 );
	mat4f modelViewProjection = projection * model;

	static GLint mvpLocation = glGetUniformLocation(programs[PROGRAM_RENDER_MD2],
	                                         "uModelViewProjection");
	glUniformMatrix4fv(mvpLocation, 1, 0, &modelViewProjection[0][0]);

	glBindVertexArray(vertexArrays[VERTEX_ARRAY_MD2]);
	glDrawArrays( GL_TRIANGLES,
	              streamOffset/sizeof(Md2::Vertex),
	              md2->TriangleCount()*3);

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
}


////////////////////////////////////////////////////////////////////////////////
// on mouse motion cb
void on_mouse_motion(GLint x, GLint y)
{
#ifdef _ANT_ENABLE
	if(1 == TwEventMouseMotionGLUT(x,y))
		return;
#endif // _ANT_ENABLE
}


////////////////////////////////////////////////////////////////////////////////
// on mouse wheel cb
void on_mouse_wheel(GLint wheel, GLint direction, GLint x, GLint y)
{
#ifdef _ANT_ENABLE
	if(1 == TwMouseWheel(wheel))
		return;
#endif // _ANT_ENABLE
}


////////////////////////////////////////////////////////////////////////////////
// Main
//
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	const GLuint CONTEXT_MAJOR = 4;
	const GLuint CONTEXT_MINOR = 1;

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
		return 0;
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
		return 0;
	}

	return 1;
}



