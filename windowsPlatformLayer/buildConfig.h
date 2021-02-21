#pragma once

#define INTERNAL_BUILD 1	//this includes things like debugg checking replay and hot code reloading
							//when this is 0 the code will build for shipping mode, it will disable
							//debugging tools like input record, custom memory allocation adress space
							//and the option to ignore an assert

#define ENABLE_CONSOLE 1	//this is the default windows console

#define ALLOW_ONLY_ONE_INSTANCE 1

#define NOT_RECORD_DELTATIME 0

#define USE_GPU_ENGINE 1


//todo move
#include <signal.h>
#include <Windows.h>
#include <string.h>
#include <stdio.h>

inline void assertFuncProduction(
	const char* expression,
	const char* file_name,
	unsigned const line_number,
	const char* comment = "---")
{

	char c[1024] = {};

	sprintf(c,
		"Assertion failed\n\n"
		"File:\n"
		"%s\n\n"
		"Line:\n"
		"%u\n\n"
		"Expresion:\n"
		"%s\n\n"
		"Comment:\n"
		"%s"
		"\n\nPlease report this error to the developer.",
		file_name,
		line_number,
		expression,
		comment
	);

	int const action = MessageBox(0, c, "Platform Layer", MB_TASKMODAL
		| MB_ICONHAND | MB_OK | MB_SETFOREGROUND);

	switch (action)
	{
		case IDOK: // Abort the program:
		{
			raise(SIGABRT);

			// We won't usually get here, but it's possible that a user-registered
			// abort handler returns, so exit the program immediately.  Note that
			// even though we are "aborting," we do not call abort() because we do
			// not want to invoke Watson (the user has already had an opportunity
			// to debug the error and chose not to).
			_exit(3);
		}
		default:
		{
			_exit(3);
		}
	}

}


inline void assertFuncInternal(
	const char* expression,
	const char* file_name,
	unsigned const line_number,
	const char* comment = "---")
{

	char c[1024] = {};
	
	sprintf(c, 
		"Assertion failed\n\n"
		"File:\n"
		"%s\n\n"
		"Line:\n"
		"%u\n\n"
		"Expresion:\n"
		"%s\n\n"
		"Comment:\n"
		"%s",
		file_name,
		line_number,
		expression,
		comment
	);

	int const action = MessageBox(0, c, "Platform Layer", MB_TASKMODAL 
		| MB_ICONHAND | MB_ABORTRETRYIGNORE | MB_SETFOREGROUND);

	switch (action)
	{
		case IDABORT: // Abort the program:
		{
			raise(SIGABRT);

			// We won't usually get here, but it's possible that a user-registered
			// abort handler returns, so exit the program immediately.  Note that
			// even though we are "aborting," we do not call abort() because we do
			// not want to invoke Watson (the user has already had an opportunity
			// to debug the error and chose not to).
			_exit(3);
		}
		case IDRETRY: // Break into the debugger then return control to caller
		{
			__debugbreak();
			return;
		}
		case IDIGNORE: // Return control to caller
		{
			return;
		}
		default: // This should not happen; treat as fatal error:
		{
			abort();
		}
	}

}

#if INTERNAL_BUILD == 1

#define winAssert(expression) (void)(											\
			(!!(expression)) ||												\
			(assertFuncInternal(#expression, __FILE__, (unsigned)(__LINE__)), 0)	\
		)

#define winAssertComment(expression, comment) (void)(								\
			(!!(expression)) ||														\
			(assertFuncInternal(#expression, __FILE__, (unsigned)(__LINE__), comment), 1)	\
		)

#else

#define winAssert(expression) (void)(											\
			(!!(expression)) ||												\
			(assertFuncProduction(#expression, __FILE__, (unsigned)(__LINE__)), 0)	\
		)

#define winAssertComment(expression, comment) (void)(								\
			(!!(expression)) ||														\
			(assertFuncProduction(#expression, __FILE__, (unsigned)(__LINE__), comment), 1)	\

		)

#endif
