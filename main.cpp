////////////////////////////////////////////////////////////////////////////////
// \author   Jonathan Dupuy
//
////////////////////////////////////////////////////////////////////////////////

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
#include "gmath_ext.hpp"    // basic math library extensions

// Standard librabries
#include <iostream>
#include <sstream>


////////////////////////////////////////////////////////////////////////////////
// Global variables
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Callbacks
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// on init cb
void on_init()
{
	glClearColor(0.1,0.2,0.3,1.0);

	fw::check_gl_error();

#ifdef _ANT_ENABLE
	// start ant
	TwInit(TW_OPENGL, NULL);
	// send the ''glutGetModifers'' function pointer to AntTweakBar
	TwGLUTModifiersFunc(glutGetModifiers);
#endif // _ANT_ENABLE
}


////////////////////////////////////////////////////////////////////////////////
// on clean cb
void on_clean()
{

#ifdef _ANT_ENABLE
	TwTerminate();
#endif // _ANT_ENABLE
	fw::check_gl_error();
}


////////////////////////////////////////////////////////////////////////////////
// on update cb
void on_update()
{
	glClear(GL_COLOR_BUFFER_BIT);

#ifdef _ANT_ENABLE
	TwDraw();
#endif // _ANT_ENABLE
	fw::check_gl_error();

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
}


////////////////////////////////////////////////////////////////////////////////
// on key up cb
void on_key_up(GLubyte key, GLint x, GLint y)
{

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
	glutInitWindowPosition(10, 10);
	glutCreateWindow("OpenGL");

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
	glutKeyboardUpFunc(&on_key_up);
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



