#ifndef LOG_STREAM_H
#define LOG_STREAM_H


#include <string>
#include <iostream>
#include <sstream>
#include <syslog.h>

#include <sys/types.h>
#include <sys/time.h>


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
        
        void MakeNodePackageManagerHappy();    

   protected:       
       string m_action;
       int m_logType;
       ostringstream m_oss; 
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
    private:
        timeval m_first;
};


string RunTime::TimeFormat(const double NumSeconds) const
{
    stringstream line;
    const long BSIZE = 100;
    char buf[BSIZE];
    if (NumSeconds > 1.0) {
        double total_seconds = NumSeconds;
        long minutes = (long)total_seconds / 60;
        long hours = minutes / 60;
        minutes = minutes - hours * 60;
        double seconds = total_seconds - minutes * 60 - hours * 3600;
        if (hours) sprintf(buf,"%ld:%2.2ld:%5.3f",hours,minutes,seconds);
        else if (minutes) sprintf(buf,"%2.2ld:%5.3f",minutes,seconds);
        else sprintf(buf,"%5.3f seconds",seconds);
    }
    else {
        double milliSeconds = NumSeconds * 1000.0;
        if (milliSeconds > 1.0) {
            sprintf(buf,"%5.3f ms",milliSeconds);
        }
        else {
            double microSeconds = NumSeconds * 1e6;
            if (microSeconds > 1.0) {
                sprintf(buf,"%5.3f us",microSeconds);
            }
            else {
                double nanoSeconds = NumSeconds * 1e9;
                sprintf(buf,"%5.3f ns",nanoSeconds);
            }
        }
    }
    return buf;
}

double RunTime::TotalSeconds() const
{
    timeval val;
    gettimeofday(&val,NULL);
    double dMicroSec = (val.tv_sec * 1e6 + val.tv_usec) - (m_first.tv_sec * 1e6 + m_first.tv_usec);
    return (dMicroSec/1e6);
}

double RunTime::TotalMilliSeconds() const
{
    return (TotalSeconds() * 1000);
}

double RunTime::TotalMicroSeconds() const
{
    return (TotalSeconds() * 1e6);
}

ostream& operator<<( ostream& ros, const RunTime &rTime) 
{
    ros << rTime.TimeFormat(rTime.TotalSeconds());
    return ros;
}

#endif
