

#include <v8.h>
#include <node.h>
#include <iostream>
#include <string>

#include "logStream.h"


using namespace node;
using namespace v8;
using namespace std;

#define PROCESS_OK 0
#define PROCESS_FAIL 1

using namespace std;

int GetArgumentIntValue ( const Arguments& args, int argNum, int &value )
{
    if (args[argNum]->IsNumber())
    {
        value = args[argNum]->Int32Value();
    } 
    else {
        return PROCESS_FAIL;
    }
    return PROCESS_OK;
}


int GetArgumentDoubleValue ( const Arguments& args, int argNum, double &value )
{
    if (args[argNum]->IsNumber())
    {
        value = args[argNum]->NumberValue();
    } 
    else {
        v8::String::Utf8Value param1(args[argNum]->ToString());
        cout << "GetArgumentDoubleValue arg " << std::string(*param1) << " not a number\n";
        return PROCESS_FAIL;
    }
    return PROCESS_OK;
}


int GetArgumentStringValue ( const Arguments& args, int argNum, string &value)
{
    if (args[argNum]->IsString())
    {
        // http://stackoverflow.com/a/10255816/51700
        v8::String::Utf8Value param1(args[argNum]->ToString());

        value = std::string(*param1);    
    } 
    else {
        return PROCESS_FAIL;
    }
    return PROCESS_OK;

}

static Handle<Value> hello(const Arguments& args)
{
    if (args.Length() < 1) {
        return Number::New(PROCESS_FAIL);
    }
    int targ=0;
    string val;

    int status = GetArgumentStringValue ( args, targ++, val);
    if (status == PROCESS_FAIL) {
        return String::New("PROCESS FAIL");
    }

    string out = "Hello " + val;

    return String::New(out.c_str());
}



extern "C" {
  static void init(Handle<Object> target)
  {
      NODE_SET_METHOD(target, "hello", hello);
  }

  NODE_MODULE(moduleName, init)
}


