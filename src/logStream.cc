#include "logStream.h"

Handle<Value> parseJson(Handle<Value> jsonString) {
    HandleScope scope;

    Handle<v8::Context> context = v8::Context::GetCurrent();
    Handle<Object> global = context->Global();

    Handle<Object> JSON = global->Get(String::New("JSON"))->ToObject();
    Handle<Function> JSON_parse = Handle<Function>::Cast(JSON->Get(String::New("parse")));

    // return JSON.parse.apply(JSON, jsonString);
    return scope.Close(JSON_parse->Call(JSON, 1, &jsonString));
}

Handle<Value> toJson(Handle<Value> object)
{
    HandleScope scope;

    Handle<v8::Context> context = v8::Context::GetCurrent();
    Handle<Object> global = context->Global();

    Handle<Object> JSON = global->Get(String::New("JSON"))->ToObject();
    Handle<Function> JSON_stringify = Handle<Function>::Cast(JSON->Get(String::New("stringify")));
    return scope.Close(JSON_stringify->Call(JSON, 1, &object));
}


string ObjectToString(Local<Value> value) {
  String::Utf8Value utf8_value(value);
  return string(*utf8_value);
}


Local<Object> kvPair(const string &key, double val) {
    HandleScope scope;
    Local<Object> pObj = Object::New();
    pObj->Set(String::New(key.c_str()), Number::New(val) );
    return scope.Close(pObj);
}

Local<Object> kvPair(const string &key, int val) {
    HandleScope scope;
    Local<Object> pObj = Object::New();
    pObj->Set(String::New(key.c_str()), Number::New(val) );
    return scope.Close(pObj);
}

Local<Object> kvPair(const string &key, const string &val) {
    HandleScope scope;
    Local<Object> pObj = Object::New();
    pObj->Set(String::New(key.c_str()), String::New(val.c_str()) );
    return scope.Close(pObj);
}

Local<Object> kvPair(const string &key, Local<Object> rObj) {
    HandleScope scope;
    Local<Object> pObj = Object::New();
    pObj->Set(String::New(key.c_str()), rObj );
    return scope.Close(pObj);
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
