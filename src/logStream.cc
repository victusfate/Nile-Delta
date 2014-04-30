#include "logStream.h"
#include <regex>

string PARENT_REQUEST_HASH;
string REQUEST_HASH;
string THREAD_HASH;
string BUILD_TYPE;
int64_t USER_ID;
int64_t MONTAGE_ID;
int64_t BUILD_ID;
int64_t NTHREADS;
mutex LOG_IO;
mutex DEBUG_IO;

void logStreamError(const string &action,const string &emsg)
{
    LogStream mout(LOG_ERR,action.c_str());
    mout << emsg << LogStream::endl;
    exit(1); 
}


bool isDoubleQuote(char c) {
    return c == '"';
}


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
    m_Blob[key] = shared_ptr<LogBlob>(new LogBlob(val));   
}

void LogBlob::insert(const string &key, double val)
{
    m_Type = LBMAP;
    m_Blob[key] = shared_ptr<LogBlob>(new LogBlob(val));   
}

void LogBlob::insert(const string &key, const string &val)
{
    m_Type = LBMAP;
    m_Blob[key] = shared_ptr<LogBlob>(new LogBlob(val));   
}

void LogBlob::insert(const string &key, const LogBlob &val)
{
    m_Type = LBMAP;
    m_Blob[key] = shared_ptr<LogBlob>(new LogBlob(val));   

}

void LogBlob::push(const LogBlob &val)
{
    if (m_Type != LBARRAY) {
        clean();
        m_Type = LBARRAY;
        m_ITERATOR_MUTEX.lock();
        m_BlobArray.resize(0);
        m_BlobArray.push_back(shared_ptr<LogBlob>(new LogBlob(val)));
        m_ITERATOR_MUTEX.unlock();
    }
    else {
        m_ITERATOR_MUTEX.lock();
        m_BlobArray.push_back(shared_ptr<LogBlob>(new LogBlob(val)));
        m_ITERATOR_MUTEX.unlock();
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
    m_Blob.clear();
    m_BlobArray.clear();
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
        // lock mutex over iterated blob
        r.m_ITERATOR_MUTEX.lock();
        auto i = r.m_Blob.begin();
        for (;i != r.m_Blob.end();i++) {
            m_Blob[i->first] = shared_ptr<LogBlob>(new LogBlob(*(i->second)));
        }
        r.m_ITERATOR_MUTEX.unlock();
    }
    else if (r.m_Type == LBARRAY) {
        m_BlobArray.resize(r.m_BlobArray.size());
        // can't lock around a lock in the right blob
        for (unsigned long i=0;i < r.m_BlobArray.size();i++) {
            m_BlobArray[i] = shared_ptr<LogBlob>(new LogBlob(*(r.m_BlobArray[i])));
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
    if (rBlob.m_Type == LBMAP) {
        ros << "{";
            rBlob.m_ITERATOR_MUTEX.lock();
            auto i = rBlob.m_Blob.begin();
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
            rBlob.m_ITERATOR_MUTEX.unlock();

        ros << "}";
    }
    else if (rBlob.m_Type == LBARRAY) {
        ros << "[";
            for (size_t i=0;i < rBlob.m_BlobArray.size();i++) {
                if (i) {
                    ros << ",";
                }                
                ros << *(rBlob.m_BlobArray[i]);
            }
        ros << "]";        
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
        }
    }

    return ros;
}

const LogBlob& LogBlob::operator[](const string &key) const
{
    shared_ptr<LogBlob> pBlob;
    m_ITERATOR_MUTEX.lock();
    auto i = m_Blob.find(key);
    if (i != m_Blob.end()) {
        pBlob = i->second;
    }
    m_ITERATOR_MUTEX.unlock();
    if (pBlob.use_count() == 0) {
        stringstream err;
        err << "LogBlob::operator[] const, ERROR: key not found, key(" << key << ") LogBlob: " << *this; 
        logStreamError("LogBlob.operator[]",err.str());
    }
    return *(pBlob);
}

LogBlob& LogBlob::operator[](const string &key)
{
    shared_ptr<LogBlob> pBlob;
    m_ITERATOR_MUTEX.lock();
    auto i = m_Blob.find(key);
    if (i != m_Blob.end()) {
        pBlob = i->second;
    }
    m_ITERATOR_MUTEX.unlock();
    if (pBlob.use_count() == 0) {

        string rString = key;
        rString.erase( remove_if( rString.begin(), rString.end(), isDoubleQuote), rString.end() );

        if (key != rString) {
            stringstream err;
            err << "LogBlob::operator[], ERROR: unable to insert invalid, key(" << key << ") LogBlob: " << *this; 
            logStreamError("LogBlob.operator[]",err.str());
        }   
        insert(key, LogBlob());
        // could have another version of insert that returns a reference to the inserted logblob if needed
        shared_ptr<LogBlob> pBlob2;
        m_ITERATOR_MUTEX.lock();
        auto j = m_Blob.find(key);
        if (j != m_Blob.end()) {
            pBlob2 = j->second;
        }
        m_ITERATOR_MUTEX.unlock();
        if (pBlob2.use_count() == 0) {
            stringstream err;
            err << "LogBlob::operator[], ERROR: INCONCEIVABLE! unable to find key(" << key << ") we just inserted into LogBlob: " << *this; 
            logStreamError("LogBlob.operator[]",err.str());
        }
        return *(pBlob2);
    }
    return *(pBlob);
}

const LogBlob& LogBlob::operator[](size_t index) const
{
    if (index >= m_BlobArray.size()) {
        stringstream err;
        err << "const LogBlob::operator[] const, ERROR: index outside bounds, index(" << index << ") LogBlob: " << *this; 
        logStreamError("LogBlob.operator[]",err.str());
    }
    // no need to make a critical region, operator[] is considered thread safe for vectors
    // LogBlob *pBlob = NULL;
    // m_ITERATOR_MUTEX.lock();
    // pBlob = m_BlobArray[index];
    // m_ITERATOR_MUTEX.unlock();
    // return *(pBlob);
    return *(m_BlobArray[index]);
}

LogBlob& LogBlob::operator[](size_t index)
{
    if (index >= m_BlobArray.size()) {
        stringstream err;
        err << "LogBlob::operator[] const, ERROR: index outside bounds, index(" << index << ") LogBlob: " << *this; 
        logStreamError("LogBlob.operator[]",err.str());
    }

    // no need to make a critical region, operator[] is considered thread safe for vectors
    // LogBlob *pBlob = NULL;
    // m_ITERATOR_MUTEX.lock();
    // pBlob = m_BlobArray[index];
    // m_ITERATOR_MUTEX.unlock();
    // return *(pBlob);

    return *(m_BlobArray[index]);
}

int64_t LogBlob::toInt64() const
{
    if (m_Type != LBDOUBLE && m_Type != LBINT64) {
        stringstream err;
        err << "LogBlob::toInt64 const, ERROR: LogBlob not type LBINT64, type(" << m_Type << ") LogBlob: " << *this; 
        logStreamError("LogBlob.toInt64",err.str());
    }
    if (m_Type == LBDOUBLE) {
        return m_dVal;
    }
    return m_iVal;
}

double  LogBlob::toDouble() const
{
    if (m_Type == LBSTRING && m_sVal == "") return (double)0;
    else if (m_Type != LBDOUBLE && m_Type != LBINT64) {
        stringstream err;
        err << "LogBlob::toDouble const, ERROR: LogBlob not type LBDOUBLE, type(" << m_Type << ") LogBlob: " << *this; 
        logStreamError("LogBlob.toInt64",err.str());
    }
    if (m_Type == LBINT64) {
        return m_iVal;
    }
    return m_dVal;    
}

string  LogBlob::toString() const
{
    if (m_Type == LBSTRING) {
        string rString = m_sVal;
        rString.erase( remove( rString.begin(), rString.end(), '"'), rString.end() );        
        return rString;
    }

    stringstream ostr;
    ostr << *this;

    // if (m_Type != LBSTRING) {
    //     stringstream err;
    //     err << "LogBlob::toString const, ERROR: LogBlob not type LBSTRING, type(" << m_Type << ") LogBlob: " << *this; 
    //     cout << err.str() << endl;
    //     exit(1);
    // }
    return ostr.str();
}

bool LogBlob::keyExists(const string &key) const
{
    shared_ptr<LogBlob> pBlob;
    m_ITERATOR_MUTEX.lock();
    auto i = m_Blob.find(key);
    if (i != m_Blob.end()) {
        pBlob = i->second;
    }
    m_ITERATOR_MUTEX.unlock();
    if (pBlob.use_count() != 0) {
        return (pBlob->m_Type != LBUNDEFINED) && ( pBlob->m_sVal != string("(null)") );
    }
    return false;

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
