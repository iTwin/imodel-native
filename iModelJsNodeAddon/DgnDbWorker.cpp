/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DgnDbWorker.h"
#include "IModelJsNative.h"

using namespace IModelJsNative;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbAsyncWorker::Execute()
    {
    m_worker->Run();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbAsyncWorker::OnOK()
    {
    m_worker->OnOK();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbAsyncWorker::OnError(Napi::Error const& e)
    {
    m_worker->OnError(e);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbWorker::DgnDbWorker(DgnDbR db, Napi::Env env)
    : m_napiWorker(*new DgnDbAsyncWorker(*this, env)), m_workers(DgnDbWorkers::Obtain(db)), m_promise(env)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbR DgnDbWorker::GetDb() const
    {
    return m_workers->GetDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Napi::Promise DgnDbWorker::Queue()
    {
    m_promise = Napi::Promise::Deferred::New(Env());
    m_promiseRef.Reset(m_promise.Promise(), 1); // so promise is valid until we are destroyed
    m_workers->Add(*this);
    m_napiWorker.Queue();
    return m_promise.Promise();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbWorker::Run()
    {
    DgnDbWorkerScope scope(*this);
    if (IsCanceled())
        {
        OnSkipped();
        return;
        }

    // Ensure any interaction with the solid kernel is protected.
    BeAssert(T_HOST.GetBRepGeometryAdmin()._IsSessionStarted());
    RefCountedPtr<IRefCounted> outerMark = T_HOST.GetBRepGeometryAdmin()._CreateWorkerThreadOuterMark();

    Execute();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbWorker::Finish()
    {
    m_workers->Drop(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbWorkers::Add(DgnDbWorkerR worker)
    {
    BeMutexHolder lock(m_cv.GetMutex());
    BeAssert(m_workers.end() == m_workers.find(&worker));
    m_workers.insert(&worker);
    m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::Db::AppData::Key& DgnDbWorkers::GetKey()
    {
    static BeSQLite::Db::AppData::Key s_key;
    return s_key;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbWorkers::Drop(DgnDbWorkerR worker)
    {
    BeMutexHolder lock(m_cv.GetMutex());
    BeAssert(m_workers.end() != m_workers.find(&worker));
    m_workers.erase(&worker);
    m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbWorkers::CancelAll(bool andWait)
    {
    BeMutexHolder lock(m_cv.GetMutex());
    for (auto& worker : m_workers)
        worker->Cancel();

    if (andWait)
        while (!m_workers.empty())
            m_cv.InfiniteWait(lock);
    }

BEGIN_UNNAMED_NAMESPACE

//=======================================================================================
// @bsistruct
//=======================================================================================
enum class TestWorkerState : uint8_t
{
    NotQueued,
    Queued,
    Skipped,
    Running,
    Ok,
    Error,
    Aborted,
};

//=======================================================================================
// @bsistruct
//=======================================================================================
enum class TestWorkerAction
{
    Wait,
    Ok,
    Error,
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct TestWorkerImpl : DgnDbWorker
{
    DEFINE_T_SUPER(DgnDbWorker);

private:
    BeAtomic<TestWorkerAction> m_action;
    BeAtomic<bool> m_wasExecuted;
    BeAtomic<TestWorkerState> m_state;

    void Execute() final
        {
        m_state.store(TestWorkerState::Running);
        m_wasExecuted.store(true);
        while (!IsCanceled() && TestWorkerAction::Wait == m_action.load())
            ;

        if (TestWorkerAction::Error == m_action.load())
            SetError("throw");
        else if (IsCanceled())
            m_state.store(TestWorkerState::Aborted);
        }

    void OnSkipped() final
        {
        m_state.store(TestWorkerState::Skipped);
        DgnDbWorker::OnSkipped();
        }

    void OnOK() final
        {
        if (TestWorkerState::Aborted != m_state.load())
            m_state.store(TestWorkerState::Ok);

        T_Super::OnOK();
        }

    void OnError(Napi::Error const& e) final
        {
        if (TestWorkerState::Skipped != m_state.load())
            m_state.store(TestWorkerState::Error);

        T_Super::OnError(e);
        }
public:
    TestWorkerImpl(DgnDbR db, Napi::Env env) : DgnDbWorker(db,  env), m_state(TestWorkerState::NotQueued) { }

    double GetState() { return static_cast<double>(m_state.load()); }
    bool WasExecuted() { return m_wasExecuted.load(); }
    void SetReady() { m_action.store(TestWorkerAction::Ok); }
    void SetThrow() { m_action.store(TestWorkerAction::Error); }
    Napi::Promise Queue() { m_state.store(TestWorkerState::Queued); return DgnDbWorker::Queue(); }
};

//=======================================================================================
// Implementation of TestWorker in NativeLibrary.ts, strictly for tests.
// @bsistruct
//=======================================================================================
struct TestWorker : BeObjectWrap<TestWorker>
{
private:
    DEFINE_CONSTRUCTOR;
    RefCountedPtr<TestWorkerImpl> m_worker;
public:
    explicit TestWorker(Napi::CallbackInfo const& info) : BeObjectWrap<TestWorker>(info)
        {
        REQUIRE_ARGUMENT_NATIVE_DGNDB(0, db);
        m_worker = new TestWorkerImpl(*db, info.Env());
        }

    ~TestWorker() { SetInDestructor(); }

    void Cancel(Napi::CallbackInfo const&) { m_worker->Cancel(); }
    Napi::Value Queue(Napi::CallbackInfo const&) { return m_worker->Queue(); }
    void SetReady(Napi::CallbackInfo const&) { m_worker->SetReady(); }
    void SetThrow(Napi::CallbackInfo const&) { m_worker->SetThrow(); }

    Napi::Value IsCanceled(Napi::CallbackInfo const& info) { return Napi::Boolean::New(info.Env(), m_worker->IsCanceled()); }
    Napi::Value WasExecuted(Napi::CallbackInfo const& info) { return Napi::Boolean::New(info.Env(), m_worker->WasExecuted()); }
    Napi::Value GetState(Napi::CallbackInfo const& info) { return Napi::Number::New(info.Env(), m_worker->GetState()); }

    static void Init(Napi::Env env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "TestWorker",
            {
            InstanceMethod("setReady", &TestWorker::SetReady),
            InstanceMethod("isCanceled", &TestWorker::IsCanceled),
            InstanceMethod("wasExecuted", &TestWorker::WasExecuted),
            InstanceMethod("getState", &TestWorker::GetState),
            InstanceMethod("queue", &TestWorker::Queue),
            InstanceMethod("cancel", &TestWorker::Cancel),
            InstanceMethod("setThrow", &TestWorker::SetThrow),
            });

        exports.Set("TestWorker", t);
        SET_CONSTRUCTOR(t);
        }
};

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDbWorker::Init(Napi::Env env, Napi::Object exports)
    {
    TestWorker::Init(env, exports);
    }

