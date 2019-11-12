/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "BridgeAsyncWorker.h"
#include "BridgeAddon.h"

using namespace BridgeNative;

#define DEFINE_CONSTRUCTOR static Napi::FunctionReference& Constructor() { static Napi::FunctionReference s_ctor; return s_ctor; }

// Per N-API docs: Call this on a reference that is declared as static data, to prevent its destructor
// from running at program shutdown time, which would attempt to reset the reference when the environment is no longer valid.
#define SET_CONSTRUCTOR(t) Constructor() = Napi::Persistent(t); Constructor().SuppressDestruct();

#define THROW_JS_TYPE_ERROR(str) Napi::TypeError::New(info.Env(), str).ThrowAsJavaScriptException();

#define THROW_TYPE_EXCEPTION_AND_RETURN(str,retval) {THROW_JS_TYPE_ERROR(str) return retval;}

#define REQUIRE_ARGUMENT_FUNCTION(i, var, retval)\
    if (info.Length() <= (i) || !info[i].IsFunction()) {\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be a function", retval)\
    }\
    Napi::Function var = info[i].As<Napi::Function>();

#define REQUIRE_ARGUMENT_STRING(i, var, retval)\
    if (info.Length() <= (i) || !info[i].IsString()) {\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be a string", retval)\
    }\
    Utf8String var = info[i].As<Napi::String>().Utf8Value().c_str();

static bool s_BeObjectWrap_inDestructor;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 
+---------------+---------------+---------------+---------------+---------------+------*/
// Every ObjectWrap subclass must derive from BeObjectWrap!
template<typename OBJ>
struct BeObjectWrap : Napi::ObjectWrap<OBJ>
    {
    protected:

    BeObjectWrap(Napi::CallbackInfo const& info) : Napi::ObjectWrap<OBJ>(info) {}

    // Every derived class must call this function on the first line of its destructor
    static void SetInDestructor()
        {
        BeAssert(!s_BeObjectWrap_inDestructor && "Call SetInDestructor once, as the first line of the subclass destructor.");
        s_BeObjectWrap_inDestructor = true;
        }

    ~BeObjectWrap()
        {
        BeAssert(s_BeObjectWrap_inDestructor && "Subclass should have called SetInDestructor at the start of its destructor");
        // We can re-enable JS callbacks, now that we know that the destructors for the subclass and its member variables have finished.
        s_BeObjectWrap_inDestructor = false;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct BridgeJob: BeObjectWrap< BridgeJob>
    {
    private:
    DEFINE_CONSTRUCTOR
    void RunAsync(Napi::CallbackInfo const &info);
    Napi::Value RunSync(Napi::CallbackInfo const &info);

    public:

    BridgeJob(Napi::CallbackInfo const& info) : BeObjectWrap<BridgeJob>(info) {}

    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "BridgeJob", {
            InstanceMethod("runAsync", &BridgeJob::RunAsync),
            InstanceMethod("runSync", &BridgeJob::RunSync),
        });
        exports.Set("BridgeJob", t);
        SET_CONSTRUCTOR(t);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void BridgeJob::RunAsync(Napi::CallbackInfo const &info)
    {
    REQUIRE_ARGUMENT_FUNCTION(1, doJobCallBack, );
    REQUIRE_ARGUMENT_STRING(0, jobOptions, );
    auto worker = new RunBridgeAsyncWorker(doJobCallBack, jobOptions);
    worker->Queue();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Napi::Value BridgeJob::RunSync(Napi::CallbackInfo const &info)
    {
    Napi::Env env = info.Env();
    REQUIRE_ARGUMENT_STRING(0, jobOptions, env.Undefined());
    auto status = BridgeWorker::RunBridge(env,jobOptions.c_str(), true);
    return Napi::Number::New (env, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
RunBridgeAsyncWorker::RunBridgeAsyncWorker(const Napi::Function& callback, Utf8StringCR jobInfo)
    :Napi::AsyncWorker(callback), m_jobInfo(jobInfo)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void            RunBridgeAsyncWorker::Execute()
    {
    Napi::Env env = this->Env();
    m_status = BridgeWorker::RunBridge(env,m_jobInfo.c_str(), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void BridgeWorker::InitAsyncWorker(Napi::Env& env, Napi::Object exports)
    {
    BridgeJob::Init(env, exports);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void RunBridgeAsyncWorker::OnOK()
    {
    auto retval = Napi::Number::New(this->Env(), (int)m_status);
    Callback().MakeCallback(Receiver().Value(), { this->Env().Null(), retval });
    }