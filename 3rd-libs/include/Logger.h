/*
 * =====================================================================================
 *
 *       Filename:  VSLogger.h
 *
 *    Description:  A simple wrapper for log4cxx.
 *
 *        Version:  1.0
 *        Created:  05/02/2012 11:24:52 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  sam wang (), gotostudy@gmail.com
 *        Company:  www.taotaosou.com
 *
 * =====================================================================================
 */

#pragma once
#include <string>
#include <iostream>

#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>

using namespace log4cxx;
using namespace std;

#define LOG_TRACE(...) (LOG4CXX_TRACE((VSLogger::getLogger()), __VA_ARGS__))
#define LOG_DEBUG(...) (LOG4CXX_DEBUG((VSLogger::getLogger()), __VA_ARGS__))
#define LOG_INFO(...) (LOG4CXX_INFO((VSLogger::getLogger()), __VA_ARGS__))
#define LOG_WARN(...) (LOG4CXX_WARN((VSLogger::getLogger()), __VA_ARGS__))
#define LOG_ERROR(...) (LOG4CXX_ERROR((VSLogger::getLogger()), __VA_ARGS__))
#define LOG_FATAL(...) (LOG4CXX_FATAL((VSLogger::getLogger()), __VA_ARGS__))

/*
 * =====================================================================================
 *        Class:  VSLogger
 *  Description:  
 * =====================================================================================
 */
class VSLogger
{
    public:
        /* ====================  ACCESSORS     ======================================= */
        static LoggerPtr& getLogger();

        /* ====================  MUTATORS      ======================================= */

        /* ====================  OPERATORS     ======================================= */
        static int Init(const string&, const string&);
        static int Init(const char*, const char*);

    protected:
        /* ====================  DATA MEMBERS  ======================================= */
    private:
        /* ====================  LIFECYCLE     ======================================= */
        VSLogger ();                             /* constructor      */
        VSLogger ( const VSLogger &other );   /* copy constructor */
        ~VSLogger ();                            /* destructor       */
        VSLogger& operator = ( const VSLogger &other ); /* assignment operator */

        /* ====================  DATA MEMBERS  ======================================= */
        static LoggerPtr logger;
        static bool isInited;
}; /* -----  end of class VSLogger  ----- */
