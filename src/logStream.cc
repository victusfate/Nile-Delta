#include "logStream.h"


string PARENT_REQUEST_HASH;
string REQUEST_HASH;


LogBlob::LogBlob(const int &val) : m_Type(LBINT) {
    m_iVal = val;
}

LogBlob::LogBlob(const double &val) : m_Type(LBDOUBLE) {
    m_dVal = val;
}

LogBlob::LogBlob(const string &val) : m_Type(LBSTRING) {
    m_sVal = val;
}

LogBlob::LogBlob(const LogBlob &val) {
    *this = val;
}

LogBlob::LogBlob(const string &key, const int &val) {
    insert(key,val);
}

LogBlob::LogBlob(const string &key, const double &val) {
    insert(key,val);
}

LogBlob::LogBlob(const string &key, const string &val) {
    insert(key,val);
}

LogBlob::LogBlob(const string &key, const LogBlob &val) {
    insert(key,val);
}


void LogBlob::insert(const string &key, const int &val)
{
    m_Type = LBMAP;
    m_Blob[key] = new LogBlob(val);   
}

void LogBlob::insert(const string &key, const double &val)
{
    m_Type = LBMAP;
    m_Blob[key] = new LogBlob(val);   
}

void LogBlob::insert(const string &key, const string &val)
{
    m_Type = LBMAP;
    m_Blob[key] = new LogBlob(val);   
}

void LogBlob::insert(const string &key, const LogBlob &val)
{
    m_Type = LBMAP;
    m_Blob[key] = new LogBlob(val);   

}


LogBlob::~LogBlob()
{
    clean();
}

void LogBlob::clean()
{
    map<string, LogBlob* >::iterator i = m_Blob.begin();
    for (;i != m_Blob.end();i++) {
        if (i->second) delete i->second; // should cascade down complex LogBlob maps
    }
}

const LogBlob& LogBlob::operator=(const LogBlob &r)
{
    clean();

    map<string, LogBlob* >::const_iterator i = r.m_Blob.begin();
    for (;i != r.m_Blob.end();i++) {
        m_Blob[i->first] = new LogBlob(*(i->second));
    }
    m_iVal = r.m_iVal;
    m_dVal = r.m_dVal;
    m_sVal = r.m_sVal;
    m_Type = r.m_Type;

    return *this;
}



ostream& operator<<(ostream& ros, const LogBlob &rBlob)
{
    if (rBlob.m_Blob.size()) {
        ros << "{";
            map<string, LogBlob* > ::const_iterator i = rBlob.m_Blob.begin();
            for (;i != rBlob.m_Blob.end();i++) {
                if (i != rBlob.m_Blob.begin()) {
                    ros << ",";
                }
                
                ros << "\"" << i->first << "\":";
                if (i->second->m_Blob.size()) {
                    ros << *(i->second);
                }
                else {
                    ros << "\"" << *(i->second) << "\"";
                }
            }
        ros << "}";
    }
    else {
        if (rBlob.m_Type == LBINT) {
            ros << rBlob.m_iVal;    
        }
        else if (rBlob.m_Type == LBDOUBLE) {
            ros << rBlob.m_dVal;
        }
        else if (rBlob.m_Type == LBSTRING) {
            ros << rBlob.m_sVal;
        }
        else {
            stringstream emsg;
            emsg << "ERROR LogBlob unhandled Blob value type " << rBlob.m_Type;
            syslog(LOG_DEBUG,"%s",emsg.str().c_str());                
            std::cout << emsg.str() << std::endl;
            ThrowException(Exception::TypeError(String::New(emsg.str().c_str())));    
        }
    }

    return ros;
}


string RunTime::TimeFormat(const double NumSeconds) const
{
    stringstream line;
    const long BSIZE = 100;
    char buf[BSIZE];
    // if (NumSeconds > 1.0) {
    //     double total_seconds = NumSeconds;
    //     long minutes = (long)total_seconds / 60;
    //     long hours = minutes / 60;
    //     minutes = minutes - hours * 60;
    //     double seconds = total_seconds - minutes * 60 - hours * 3600;
    //     if (hours) sprintf(buf,"%ld:%2.2ld:%5.3f",hours,minutes,seconds);
    //     else if (minutes) sprintf(buf,"%2.2ld:%5.3f",minutes,seconds);
    //     else sprintf(buf,"%5.3f seconds",seconds);
    // }
    // else {
    double milliSeconds = NumSeconds * 1000.0;
    sprintf(buf,"\"duration\": %5.3f ",milliSeconds);

    return buf;
}

double RunTime::TotalSeconds() const
{
    timeval val;
    gettimeofday(&val,NULL);
    double dSec = val.tv_sec - m_first.tv_sec;
    double dMicroSec = val.tv_usec - m_first.tv_usec;
    return (dSec + (dMicroSec/1e6) );
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
