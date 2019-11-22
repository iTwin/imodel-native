/*---------------------------------------------------------------------------------------------
 * Adapted from https://github.com/atom/node-keytar
 * See LICENSE.md in the libsrc/keytar folder for license information. 
 *--------------------------------------------------------------------------------------------*/
#include "KeyTarInterop.h"
#include <Napi/napi.h>
#include <BeSecurity/keytar/keytar.h>

#include <string>
#include <vector>

#define REQUIRE_ARGUMENT_STD_STRING(i, var) \
    if (info.Length() <= (i) || !info[i].IsString()) { \
        deferred.Reject(Napi::String::New(env, "Argument " #i " must be a string")); \
        return deferred.Promise(); \
    } \
    std::string var = info[i].As<Napi::String>().Utf8Value();

USING_NAMESPACE_KEYTAR

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
Napi::Value EmptyCallback(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    return env.Undefined();
}

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
SetPasswordWorker::SetPasswordWorker(Napi::Function &callback, Napi::Promise::Deferred& deferred, const std::string &service, const std::string &account, const std::string &password) 
    : Napi::AsyncWorker(callback), deferred(deferred), service(service), account(account), password(password) {}

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
SetPasswordWorker::~SetPasswordWorker() {}

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
void SetPasswordWorker::Execute()
{
    std::string error;
    KEYTAR_OP_RESULT result = SetPassword(service, account, password, &error);
    if (result == FAIL_ERROR)
    {
        SetError(error.c_str());
    }
}

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
void SetPasswordWorker::OnOK()
    {
    deferred.Resolve(Env().Undefined());
    Callback().Call({}); // Call empty function
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
void SetPasswordWorker::OnError(Napi::Error const &error)
    {
    deferred.Reject(error.Value());
    Callback().Call({}); // Call empty function
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
Napi::Value SetPasswordWorker::SetPasswordAsync(Napi::CallbackInfo const &info)
    {
    // setPassword(service: string, account: string, password: string): Promise<void>;    
    Napi::Env env = info.Env();
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
    Napi::Function callback = Napi::Function::New(env, EmptyCallback);

    REQUIRE_ARGUMENT_STD_STRING(0, service);
    REQUIRE_ARGUMENT_STD_STRING(1, account);
    REQUIRE_ARGUMENT_STD_STRING(2, password);

    SetPasswordWorker* worker = new SetPasswordWorker(callback, deferred, service, account, password);
    worker->Queue();
    return deferred.Promise();
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
GetPasswordWorker::GetPasswordWorker(Napi::Function &callback, Napi::Promise::Deferred& deferred, const std::string &service, const std::string &account) 
    : Napi::AsyncWorker(callback), deferred(deferred), service(service), account(account) {}

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
GetPasswordWorker::~GetPasswordWorker() {}

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
void GetPasswordWorker::Execute()
    {
    std::string error;
    KEYTAR_OP_RESULT result = GetPassword(service, account, &password, &error);
    if (result == FAIL_ERROR)
        SetError(error.c_str());
    else if (result == FAIL_NONFATAL)
        success = false;
    else
        success = true;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
void GetPasswordWorker::OnOK()
    {
    Napi::Value retval = success ? Napi::String::New(Env(), password.data(), password.length()) : Env().Null();
    deferred.Resolve(retval);
    Callback().Call({}); // Call empty function
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
void GetPasswordWorker::OnError(Napi::Error const &error)
    {
    deferred.Reject(error.Value());
    Callback().Call({}); // Call empty function
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
Napi::Value GetPasswordWorker::GetPasswordAsync(Napi::CallbackInfo const &info)
    {    
    // getPassword(service: string, account: string): Promise<string | null>;
    Napi::Env env = info.Env();
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
    Napi::Function callback = Napi::Function::New(env, EmptyCallback);

    REQUIRE_ARGUMENT_STD_STRING(0, service);
    REQUIRE_ARGUMENT_STD_STRING(1, account);

    GetPasswordWorker* worker = new GetPasswordWorker(callback, deferred, service, account);
    worker->Queue();
    return deferred.Promise();
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
DeletePasswordWorker::DeletePasswordWorker(Napi::Function &callback, Napi::Promise::Deferred& deferred, const std::string &service, const std::string &account)
    : Napi::AsyncWorker(callback), deferred(deferred), service(service), account(account) {}

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
DeletePasswordWorker::~DeletePasswordWorker() {}

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
void DeletePasswordWorker::Execute()
    {
    std::string error;
    KEYTAR_OP_RESULT result = DeletePassword(service, account, &error);
    if (result == FAIL_ERROR)
        SetError(error.c_str());
    else if (result == FAIL_NONFATAL)
        success = false;
    else
        success = true;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
void DeletePasswordWorker::OnOK()
    {
    Napi::HandleScope scope(Env());
    Napi::Value retval = Napi::Boolean::New(Env(), success);
    deferred.Resolve(retval);
    Callback().Call({}); // Call empty function
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
void DeletePasswordWorker::OnError(Napi::Error const &error)
    {
    Napi::HandleScope scope(Env());
    deferred.Reject(error.Value());
    Callback().Call({}); // Call empty function
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
Napi::Value DeletePasswordWorker::DeletePasswordAsync(Napi::CallbackInfo const &info)
    {
    // deletePassword(service: string, account: string): Promise<boolean>;
    Napi::Env env = info.Env();
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
    Napi::Function callback = Napi::Function::New(env, EmptyCallback);

    REQUIRE_ARGUMENT_STD_STRING(0, service);
    REQUIRE_ARGUMENT_STD_STRING(1, account);
    
    DeletePasswordWorker* worker = new DeletePasswordWorker(callback, deferred, service, account);
    worker->Queue();
    return deferred.Promise();
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
FindPasswordWorker::FindPasswordWorker(Napi::Function &callback, Napi::Promise::Deferred& deferred, const std::string &service) 
    : Napi::AsyncWorker(callback), deferred(deferred), service(service) {}

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
FindPasswordWorker::~FindPasswordWorker() {}

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
void FindPasswordWorker::Execute()
    {
    std::string error;
    KEYTAR_OP_RESULT result = FindPassword(service, &password, &error);
    if (result == FAIL_ERROR)
        SetError(error.c_str());
    else if (result == FAIL_NONFATAL)
        success = false;
    else
        success = true;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
void FindPasswordWorker::OnOK()
    {
    Napi::HandleScope scope(Env());
    Napi::Value retval = success ? Napi::String::New(Env(), password.data(), password.length()) : Env().Null();
    deferred.Resolve(retval);
    Callback().Call({}); // Call empty function
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
void FindPasswordWorker::OnError(Napi::Error const &error)
    {
    Napi::HandleScope scope(Env());
    deferred.Reject(error.Value());
    Callback().Call({}); // Call empty function
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
Napi::Value FindPasswordWorker::FindPasswordAsync(Napi::CallbackInfo const &info)
    {
    // findPassword(service: string): Promise<string | null>;
    Napi::Env env = info.Env();
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
    Napi::Function callback = Napi::Function::New(env, EmptyCallback);

    REQUIRE_ARGUMENT_STD_STRING(0, service);

    FindPasswordWorker* worker = new FindPasswordWorker(callback, deferred, service);
    worker->Queue();
    return deferred.Promise();
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
FindCredentialsWorker::FindCredentialsWorker(Napi::Function &callback, Napi::Promise::Deferred& deferred, const std::string &service) 
    : Napi::AsyncWorker(callback), deferred(deferred), service(service) {}

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
FindCredentialsWorker::~FindCredentialsWorker() {}

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
void FindCredentialsWorker::Execute()
    {
    std::string error;
    KEYTAR_OP_RESULT result = FindCredentials(service, &credentials, &error);
    if (result == FAIL_ERROR)
        SetError(error.c_str());
    else if (result == FAIL_NONFATAL)
        success = false;
    else
        success = true;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
void FindCredentialsWorker::OnOK()
    {
    Napi::HandleScope scope(Env());
    if (success)
        {
        Napi::Array retval = Napi::Array::New(Env(), credentials.size());
        unsigned int idx = 0;
        std::vector<Credentials>::iterator it;
        for (it = credentials.begin(); it != credentials.end(); it++)
            {
            Credentials cred = *it;
            Napi::Object obj = Napi::Object::New(Env());

            Napi::String account = Napi::String::New(Env(), cred.first.data(), cred.first.length());
            Napi::String password = Napi::String::New(Env(), cred.second.data(), cred.second.length());

            obj.Set("account", account);
            obj.Set("password", password);

            retval.Set(idx, obj);
            ++idx;
            }

        deferred.Resolve(retval);
        }
    else
        {
        Napi::Array retval = Napi::Array::New(Env(), 0);
        deferred.Resolve(retval);
        }

    Callback().Call({}); // Call empty function
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
void FindCredentialsWorker::OnError(Napi::Error const &error)
    {
    Napi::HandleScope scope(Env());
    deferred.Reject(error.Value());
    Callback().Call({}); // Call empty function
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                           11/2019
//---------------------------------------------------------------------------------
Napi::Value FindCredentialsWorker::FindCredentialsAsync(Napi::CallbackInfo const &info)
    {
    // findCredentials(service: string): Promise<Array<{ account: string, password: string }>>;
    Napi::Env env = info.Env();
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
    Napi::Function callback = Napi::Function::New(env, EmptyCallback);

    REQUIRE_ARGUMENT_STD_STRING(0, service);

    FindCredentialsWorker* worker = new FindCredentialsWorker(callback, deferred, service);
    worker->Queue();
    return deferred.Promise();
    }
