////////////////////////////////////////////////////////////////////////////////
// \author J Dupuy
// \brief Utility functions and classes for simple OpenGL demos.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef FRAMEWORK_HPP
#define FRAMEWORK_HPP

#include <string>
#include "glew.hpp"

// offset for buffer objects
#define FW_BUFFER_OFFSET(i)    ((char*)NULL + (i))

namespace fw 
{
	// Framework exception
	class FWException : public std::exception
	{
	public:
		virtual ~FWException()   throw()   {}
		const char* what() const throw()   {return mMessage.c_str();}
	protected:
		std::string mMessage;
	};

	// Get next power of two
	GLuint next_power_of_two(GLuint number);

	// Build GLSL program
	GLvoid buid_glsl_program( GLuint program, 
	                          const std::string& srcfile,
	                          const std::string& options,
	                          GLboolean link ) throw(FWException);

	// Check OpenGL errors (uses ARB_debug_output if available)
	// (throws an exception if an error is detected)
	GLvoid check_gl_error() throw (FWException);

	// Basic timer class
	class Timer
	{
	public:
		// Constructors / Destructor
		Timer();

		// Manipulation
		void Start();
		void Stop() ;

		// Queries
		double Ticks()   const;

		// Members
	private:
		double mStartTicks;
		double mStopTicks;
		bool   mIsTicking;
	};

} // namespace fw

#endif

