#ifndef LOG_STREAM_H
#define LOG_STREAM_H

#include <cstdio>
#include <string>
#include <iostream>
#include <sstream>
#include <syslog.h>
#include <typeinfo>
#include <vector>

#include <sys/types.h>
#include <sys/time.h>


#include "v8.h"
#include "node.h"

using namespace std;
using namespace v8;

Handle<Value>           parseJson(Handle<Value> jsonString);
Handle<Value>           toJson(Handle<Value> object);
string                  ObjectToString(Local<Value> value);
Local<Object>           kvPair(const string &key, double val);
Local<Object>           kvPair(const string &key, int val);
Local<Object>           kvPair(const string &key, const string &val);
Local<Object>           kvPair(const string &key, Local<Object> rObj);

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
            m_action = action;
        }

        template<typename T> 
        LogStream& operator<<(const T& rhs) {
            m_oss << rhs;
            return *this; 
        }

        LogStream& operator+(Local<Object> rhs) {
            HandleScope scope;
            m_objects.push_back(rhs);
            return *this; 
        }

        
        typedef LogStream& (*MyStreamManipulator)(LogStream&);

        LogStream& operator<<(MyStreamManipulator manip)
        {
            return manip(*this);
        }        
        
        
        static LogStream& endl(LogStream& stream) {
            uv_mutex_t mutex;
            int r;
            r = uv_mutex_init(&mutex);
            if (r != 0) {
                string emsg("UV MUTEX INIT FAILED");
                syslog(stream.m_logType,"%s",emsg.c_str());                
                std::cout << emsg << std::endl;                
            }
            uv_mutex_lock(&mutex);
            Local<Object> act = kvPair("action",stream.m_action);
            Local<Object> msg = kvPair("message",stream.m_oss.str());
            string slog = ObjectToString(*toJson(act)) + ObjectToString(*toJson(msg));
            for (unsigned long i=0;i < stream.m_objects.size();i++) {
                slog += ObjectToString(*toJson(stream.m_objects[i]));
            }
            syslog(stream.m_logType,"%s",slog.c_str());                
            std::cout << slog << std::endl;
            stream.m_oss.str("");
            stream.m_objects.resize(0);

            uv_mutex_unlock(&mutex);
            uv_mutex_destroy(&mutex);

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
       vector<Local<Object> > m_objects;
};

//could use CLOCK_MONOTONIC if -std=gnu99, clock_gettime or chrono with c+x11

class RunTime {
    public:
        RunTime() {
            gettimeofday(&m_first,NULL); 
        };
        string TimeFormat(const double NumSeconds) const;
        double TotalSeconds() const;
        double TotalMilliSeconds() const;
        double TotalMicroSeconds() const;

        friend ostream& operator<<(ostream& ros, const RunTime &rTime);
    protected:
        timeval m_first;
};

// will aid in silencing warnings about ignored return types
template<class T>
inline void ignore_return(T s){};

#endif
