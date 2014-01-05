#ifndef LOG_STREAM_H
#define LOG_STREAM_H

#include <cstdlib>
#include <cstdio>
#include <string>
#include <iostream>
#include <sstream>
#include <syslog.h>
#include <typeinfo>
#include <vector>
#include <iterator>
#include <mutex>

#include <sys/types.h>
#include <sys/time.h>


#include "node.h"

using namespace std;

#ifdef __APPLE__
#include <unordered_map>
#else
#include <tr1/unordered_map>
using namespace std::tr1;
#endif

const bool VERBOSE_LOGGING = false;

enum LogBlobType { LBMAP, LBARRAY, LBINT64, LBDOUBLE, LBSTRING, LBUNDEFINED };

class LogBlob {
public:
    LogBlob();
    LogBlob(int64_t val);
    LogBlob(double   val);
    LogBlob(const string  &val);
    LogBlob(const LogBlob &val);
    LogBlob(const string &key, int64_t val);
    LogBlob(const string &key, double   val);
    LogBlob(const string &key, const string &val);
    LogBlob(const string &key, const LogBlob &val);

    virtual ~LogBlob();
    void clean();

    void insert(const string &key, int64_t val);
    void insert(const string &key, double   val);
    void insert(const string &key, const string &val);
    void insert(const string &key, const LogBlob &val);
    void push(const LogBlob &val);
    unsigned long length();

    const LogBlob& operator=(int64_t r);
    const LogBlob& operator=(double r);
    const LogBlob& operator=(const string &r);
    const LogBlob& operator=(const LogBlob &r);
    friend ostream& operator<<(ostream& ros, const LogBlob &rBlob);

    const LogBlob& operator[](const string &key) const;
    LogBlob& operator[](const string &key);
    const LogBlob& operator[](size_t index) const;
    LogBlob& operator[](size_t index);

    int64_t toInt64() const;
    double  toDouble() const;
    string  toString() const;

    bool exists() const { return (m_Type != LBUNDEFINED) && ( m_sVal != string("(null)") ); };

    unordered_map<string, LogBlob* >    m_Blob;
    vector<LogBlob *>                   m_BlobArray;
    int64_t m_iVal;
    double  m_dVal;
    string  m_sVal;
    LogBlobType m_Type;
};

// make sure these are available to any modules that utilize Nile-Delta
extern string PARENT_REQUEST_HASH;
extern string REQUEST_HASH;
extern string THREAD_HASH;
extern string BUILD_TYPE;
extern int64_t USER_ID;
extern int64_t MONTAGE_ID;
extern int64_t BUILD_ID;
extern mutex   LOG_IO;
extern mutex   DEBUG_IO;

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
        
        string setAction(const string &action) 
        {
            string oldAction = m_action;
            m_action = action;
            return oldAction;
        }

        template<typename T> 
        LogStream& operator<<(const T& rhs) {
            m_oss << rhs;
            return *this; 
        }

        LogStream& setObject(const LogBlob &rhs) {
            m_objects.resize(0);
            m_objects.push_back(&rhs);
            return *this; 
        }

        
        typedef LogStream& (*MyStreamManipulator)(LogStream&);

        LogStream& operator<<(MyStreamManipulator manip)
        {
            return manip(*this);
        }        
        
        
        static LogStream& endl(LogStream& stream) 
        {
            LogBlob lb;
            if (stream.m_objects.size()) lb = *(stream.m_objects[0]);
            lb.insert("action",stream.m_action);
            lb.insert("__p", PARENT_REQUEST_HASH);
            lb.insert("__r", REQUEST_HASH);
            lb.insert("__t", THREAD_HASH);
            lb.insert("buildType", BUILD_TYPE);
            lb.insert("__u", USER_ID);
            lb.insert("__m", MONTAGE_ID);
            lb.insert("__b", BUILD_ID);
            if (stream.m_oss.str().length()) lb.insert("message", stream.m_oss.str());

            stringstream slog;
            slog << lb;
            LOG_IO.lock();
            syslog(stream.m_logType,"%s",slog.str().c_str());  
            LOG_IO.unlock();

            if (uv_guess_handle(1) == UV_TTY) {              
                DEBUG_IO.lock();
                std::cout << slog.str() << std::endl << std::endl;
                DEBUG_IO.unlock();
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
