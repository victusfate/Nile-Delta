#include "logStream.h"


string PARENT_REQUEST_HASH;
string REQUEST_HASH;
string THREAD_HASH;
string BUILD_TYPE;
int64_t USER_ID;
int64_t MONTAGE_ID;
int64_t BUILD_ID;



LogBlob::LogBlob() : m_Type(LBUNDEFINED) {}

LogBlob::LogBlob(int64_t val) : m_Type(LBINT64) {
    m_iVal = val;
}

LogBlob::LogBlob(double val) : m_Type(LBDOUBLE) {
    m_dVal = val;
}

LogBlob::LogBlob(const string &val) : m_Type(LBSTRING) {
    m_sVal = val;
}

LogBlob::LogBlob(const LogBlob &val) {
    *this = val;
}

LogBlob::LogBlob(const string &key, int64_t val) {
    insert(key,val);
}

LogBlob::LogBlob(const string &key, double val) {
    insert(key,val);
}

LogBlob::LogBlob(const string &key, const string &val) {
    insert(key,val);
}

LogBlob::LogBlob(const string &key, const LogBlob &val) {
    insert(key,val);
}


void LogBlob::insert(const string &key, int64_t val)
{
    m_Type = LBMAP;
    m_Blob[key] = new LogBlob(val);   
}

void LogBlob::insert(const string &key, double val)
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

void LogBlob::push(const LogBlob &val)
{
    if (m_Type != LBARRAY) {
        clean();
        m_Type = LBARRAY;
        m_BlobArray.resize(0);
        m_BlobArray.push_back(new LogBlob(val));
    }
    else {
        m_BlobArray.push_back(new LogBlob(val));
    }
}

unsigned long LogBlob::length()
{
    if (m_Type == LBMAP) return m_Blob.size();
    else if (m_Type == LBARRAY) return m_BlobArray.size();

    return 1;
}


LogBlob::~LogBlob()
{
    clean();
}

void LogBlob::clean()
{
    unordered_map<string, LogBlob* >::iterator i = m_Blob.begin();
    for (;i != m_Blob.end();i++) {
        if (i->second) delete i->second; // cascades down complex LogBlob maps
    }
    for (unsigned long j=0;j < m_BlobArray.size();j++) {
        if (m_BlobArray[j]) delete m_BlobArray[j]; // cascades down complex LogBlob arrays
    }
}

const LogBlob& LogBlob::operator=(int64_t r)
{
    m_Type = LBINT64;
    m_iVal = r;
    return *this; 
}

const LogBlob& LogBlob::operator=(double r)
{
    m_Type = LBDOUBLE;
    m_dVal = r;
    return *this; 
}

const LogBlob& LogBlob::operator=(const string &r)
{
    m_Type = LBSTRING;
    m_sVal = r;
    return *this;
}

const LogBlob& LogBlob::operator=(const LogBlob &r)
{
    clean();

    if (r.m_Type == LBMAP) {
        unordered_map<string, LogBlob* >::const_iterator i = r.m_Blob.begin();
        for (;i != r.m_Blob.end();i++) {
            m_Blob[i->first] = new LogBlob(*(i->second));
        }
    }
    else if (r.m_Type == LBARRAY) {
        m_BlobArray.resize(r.m_BlobArray.size());
        for (unsigned long i=0;i < r.m_BlobArray.size();i++) {
            m_BlobArray[i] = new LogBlob(*(r.m_BlobArray[i]));
        }
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
            unordered_map<string, LogBlob* > ::const_iterator i = rBlob.m_Blob.begin();
            for (;i != rBlob.m_Blob.end();i++) {
                if (i != rBlob.m_Blob.begin()) {
                    ros << ",";
                }
                
                ros << "\"" << i->first << "\":";
                if (i->second->m_Blob.size()) {
                    ros << *(i->second);
                }
                else {
                    ros << *(i->second);
                }
            }
        ros << "}";
    }
    else {
        if (rBlob.m_Type == LBINT64) {
            ros << rBlob.m_iVal;    
        }
        else if (rBlob.m_Type == LBDOUBLE) {
            ros << rBlob.m_dVal;
        }
        else if (rBlob.m_Type == LBSTRING) {
            ros << "\"" <<rBlob.m_sVal << "\"";
        }
        else if (rBlob.m_Type == LBUNDEFINED) {
            ros << "LBUNDEFINED";
        }
        else {
            stringstream emsg;
            emsg << "ERROR LogBlob unhandled Blob value type " << rBlob.m_Type;
            syslog(LOG_DEBUG,"%s",emsg.str().c_str());                
            std::cout << emsg.str() << std::endl;
            exit(1);
            // ThrowException(Exception::TypeError(String::New(emsg.str().c_str())));
            return ros;
        }
    }

    return ros;
}

const LogBlob& LogBlob::operator[](const string &key) const
{
    unordered_map<string, LogBlob* >::const_iterator i = m_Blob.find(key);
    if (i == m_Blob.end()) {
        stringstream err;
        err << "LogBlob::operator[] const, ERROR: key not found, key(" << key << ") LogBlob: " << *this; 
        exit(1);
        // throw err.str(); // getting fucked up exceptions disabled while building, temporary
    }
    return *(i->second);
}

LogBlob& LogBlob::operator[](const string &key)
{
    unordered_map<string, LogBlob* >::iterator i = m_Blob.find(key);
    if (i == m_Blob.end()) {
        insert(key, LogBlob());
        return *(m_Blob[key]);
    }
    return *(i->second);
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
    sprintf(buf,"\"__ms\": %5.3f ",milliSeconds);

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
