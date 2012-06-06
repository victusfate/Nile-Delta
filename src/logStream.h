#ifndef LOG_STREAM_H
#define LOG_STREAM_H


#include <string>
#include <iostream>
#include <sstream>
#include <syslog.h>

using namespace std;

// syslog(LOG_EMERG,"This is an emergency message\n")); 
// syslog(LOG_ALERT,"This is an alert message\n"); 
// syslog(LOG_CRIT,"This is a critical message\n"); 
// syslog(LOG_ERR,"This is an error message\n"); 
// syslog(LOG_WARNING,"This is a warning\n"); 
// syslog(LOG_NOTICE,"This is a notice\n"); 
// syslog(LOG_INFO,"This is an informal message\n"); 
// syslog(LOG_DEBUG,"This is a debug message\n");

//http://stackoverflow.com/questions/1134388/stdendl-is-of-unknown-type-when-overloading-operator/1134467#1134467
class LogStream 
{ 
    public: 
        LogStream(int logType,const char *action) : m_logType(logType) {
            setAction(string(action));
        }
        ~LogStream() {}

        void setLog(int logType) { m_logType = logType; }
        void reset() { m_oss.str(""); }
        
        void setAction(const string &action) {
            m_action = string("{\"action\":\"") + action + string("\",\"message\":\"");
        }

        template<typename T> 
        LogStream& operator<<(const T& rhs) {
            m_oss << rhs;
            return *this; 
        }
        
        typedef LogStream& (*MyStreamManipulator)(LogStream&);

        LogStream& operator<<(MyStreamManipulator manip)
        {
            return manip(*this);
        }        
        
        
        static LogStream& endl(LogStream& stream) {
            string slog = stream.m_action + stream.m_oss.str() + "\"}";
            syslog(stream.m_logType,"%s",slog.c_str());                
            std::cout << stream.m_oss.str() << std::endl;
            stream.m_oss.str("");
            return stream;
        }
        
        typedef std::basic_ostream<char, std::char_traits<char> > CoutType;

        // this is the function signature of std::endl
        typedef CoutType& (*StandardEndLine)(CoutType&);

        // define an operator<< to take in std::endl
        LogStream& operator<<(StandardEndLine manip) {
            manip(std::cout);
            return *this;
        }        

   protected:       
       string m_action;
       int m_logType;
       ostringstream m_oss; 
};

#endif