#ifndef JH_TEST_CASE_H_
#define JH_TEST_CASE_H_

#include "Thread.h"
#include <string>
#include <stdlib.h>
#include <stdarg.h>

class TestCase
{
public:
	TestCase( const char *thread_name ) : mThread( thread_name, this, &TestCase::Run ) 
	{
	}
	
	virtual ~TestCase() 
	{
	}
	
	bool Start() 
	{
		if ( mTestName == "" )
		{
			printf( "Bad Test Case, please call SetTestName in TestCase's constructor\n" );
			return false;
		}

		printf( "Running test %s:", mTestName.c_str() );
		fflush( stdout );
		
		// start thread to run test.
		mThread.Start();
		// wait for test to complete.
		mThread.Join();

		// ensure that it completed.		
		if ( not mTestComplete )
		{
			printf( " FAILED\nBad Test Case, must call TestPassed or TestFailed before returning\n" );
			return false;
		}

		// return test status.
		return mTestPassed;
	}

	const char *GetTestName()
	{
		return mTestName.c_str();
	}
	
protected:
	void SetTestName( const char *name )
	{
		mTestName = name;
	}

	void TestPassed()
	{
		printf( " PASSED\n" );
		mTestComplete = true;
		mTestPassed = true;
	}
	
	void TestFailed( const char *fmt, ... )
	{
		va_list params;

		va_start(params, fmt);
		
		printf( " FAILED\n" );
		vprintf( fmt, params );
		printf( "\n" );
		// when Test Fails we exit the thread.
		mTestComplete = true;
		mTestPassed = false;
		Thread::Exit();
	}
		
	virtual void Run() = 0;
	
private:
	bool		mTestComplete;
	bool		mTestPassed;
	std::string	mTestName;
	Runnable<TestCase>	mThread;
};

class TestRunner
{
public:
	TestRunner( const char *ts_name )
	{
		std::string filename( ts_name );
		std::string::size_type i = filename.rfind( "/" );
		filename.erase( 0, i + 1 );
		filename += ".log";	
		//printf( "logging to file %s\n", filename.c_str() );
		FILE *log_file = fopen( filename.c_str(), "w+" );
		logging_set_file( log_file );
	}
	
	void RunAll( TestCase **array, int numTests )
	{
		for( int i = 0; i < numTests; i++ )
		{
			LOG_NOTICE( "Running test %s", array[ i ]->GetTestName() );
			if ( array[ i ]->Start() == false )
			{
				exit( EXIT_FAILURE );
			}
			delete array[ i ];
		}
	}
};

#endif // JH_TEST_CASE_H_

