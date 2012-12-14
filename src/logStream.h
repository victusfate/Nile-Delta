#ifndef LOG_STREAM_H
#define LOG_STREAM_H

#include <cstdio>
#include <string>
#include <iostream>
#include <sstream>
#include <syslog.h>
#include <typeinfo>
#include <vector>
#include <tr1/unordered_map>
#include <iterator>

#include <sys/types.h>
#include <sys/time.h>


#include "v8.h"
#include "node.h"

using namespace std;
using namespace std::tr1;
using namespace v8;


enum LogBlobType { LBMAP, LBINT, LBDOUBLE, LBSTRING };

class LogBlob {
public:
    LogBlob(const int &val);
    LogBlob(const double &val);
    LogBlob(const string &val);
    LogBlob(const LogBlob &val);
    LogBlob(const string &key, const int &val);
    LogBlob(const string &key, const double &val);
    LogBlob(const string &key, const string &val);
    LogBlob(const string &key, const LogBlob &val);

    virtual ~LogBlob();
    void clean();

    void insert(const string &key, const int &val);
    void insert(const string &key, const double &val);
    void insert(const string &key, const string &val);
    void insert(const string &key, const LogBlob &val);

    const LogBlob& operator=(const LogBlob &r);
    friend ostream& operator<<(ostream& ros, const LogBlob &rBlob);

    unordered_map<string, LogBlob* > m_Blob;
    int m_iVal;
    double m_dVal;
    string m_sVal;
    LogBlobType m_Type;
};

// make sure these are available to any modules that utilize Nile-Delta
extern string PARENT_REQUEST_HASH;
extern string REQUEST_HASH;

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

        LogStream& operator+(const LogBlob &rhs) {
            m_objects.push_back(&rhs);
            return *this; 
        }

        
        typedef LogStream& (*MyStreamManipulator)(LogStream&);

        LogStream& operator<<(MyStreamManipulator manip)
        {
            return manip(*this);
        }        
        
        
        static LogStream& endl(LogStream& stream) {

            LogBlob lb("action",stream.m_action);
            lb.insert("__parent", PARENT_REQUEST_HASH);
            lb.insert("__request", REQUEST_HASH);
            lb.insert("message", stream.m_oss.str());
            stringstream slog;
            slog << lb;
            for (unsigned long i=0;i < stream.m_objects.size();i++) {
                slog << "," << *(stream.m_objects[i]);
            }
            syslog(stream.m_logType,"%s",slog.str().c_str());  
            if (uv_guess_handle(1) == UV_TTY) {              
                std::cout << slog.str() << std::endl;
            }
            stream.m_oss.str("");
            stream.m_objects.resize(0);

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
       vector<const LogBlob* > m_objects;
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
