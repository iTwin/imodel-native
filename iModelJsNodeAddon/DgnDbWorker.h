/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include <Bentley/BeThread.h>
#include <Napi/napi.h>
#include <DgnPlatform/DgnDb.h>
#include <memory>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN

namespace IModelJsNative {

DEFINE_POINTER_SUFFIX_TYPEDEFS(DgnDbWorker);
DEFINE_POINTER_SUFFIX_TYPEDEFS(DgnDbWorkers);

DEFINE_REF_COUNTED_PTR(DgnDbWorker);
DEFINE_REF_COUNTED_PTR(DgnDbWorkers);

//=======================================================================================
//! Napi::AsyncWorker that forwards to a DgnDbWorker.
// @bsistruct
//=======================================================================================
struct DgnDbAsyncWorker : Napi::AsyncWorker
{
    friend struct DgnDbWorker;
    DEFINE_T_SUPER(Napi::AsyncWorker);
private:
    // NB: Yes, the AsyncWorker owns the DgnDbWorker. It is deleted by Napi on main thread after completion.
    // The DgnDbWorker is removed from DgnDbWorkers on worker thread just before completion.
    // This is likely the last ref-counted pointer to it.
    DgnDbWorkerPtr m_worker;

    DgnDbAsyncWorker(DgnDbWorkerR worker, Napi::Env env) : T_Super(env), m_worker(&worker) { }

    void Execute() final;
    void OnOK() final;
    void OnError(Napi::Error const&) final;
public:
    // Redefine protected members as public.
    Napi::Env Env() { return T_Super::Env(); }
    Napi::ObjectReference& Receiver() { return T_Super::Receiver(); }
    Napi::FunctionReference& Callback() { return T_Super::Callback(); }
    void SetError(std::string const& error) { T_Super::SetError(error); }
};


//=======================================================================================
//! Manages the lifetimes of DgnDbWorkers associated with a DgnDb.
// @bsistruct
//=======================================================================================
struct DgnDbWorkers : BeSQLite::Db::AppData
{
    friend struct DgnDbWorker;
private:
    BeConditionVariable m_cv;
    DgnDbPtr m_db;
    bset<DgnDbWorkerPtr> m_workers;

    explicit DgnDbWorkers(DgnDbR db) : m_db(&db) { }

    void Add(DgnDbWorkerR);
    void Drop(DgnDbWorkerR);
public:
    // Return existing DgnDbWorkers from db's app data, or nullptr if not present.
    static DgnDbWorkersPtr Get(DgnDbR db)
        {
        return db.FindAppDataOfType<DgnDbWorkers>(GetKey());
        }

    // Get or create DgnDbWorkers from db's app data. Never returns nullptr.
    static DgnDbWorkersPtr Obtain(DgnDbR db)
        {
        return db.ObtainAppData(GetKey(), [&]() { return new DgnDbWorkers(db); });
        }

    static Key& GetKey();

    DgnDbR GetDb() const { return *m_db; }

    // Cancel all extant DgnDbWorkers, and optionally wait for them to be removed from the queue.
    void CancelAll(bool andWait);
};

//=======================================================================================
// A Napi::AsyncWorker that uses a DgnDb. The worker is automatically canceled
// when the DgnDb is closed.
// To schedule a worker, do:
//  DgnDbWorkerPtr worker = new MyWorker(dgndb, callback, <other args>);
//  // Or RefCountedPtr<MyWorker> - either way, must assign to a RefCountedPtr, not a raw pointer!
//  worker->Queue();
//
// The worker will be added to the queue to be processed on a worker thread. As with Napi::
// AsyncWorker, you must not interact with Javascript on the worker thread.
//
// The worker can be canceled at any time via its Cancel method. When the DgnDb is about to
// be closed on the main thread, all extant workers will be canceled, and the main thread will
// wait to close the DgnDb until the worker queue is empty. You can check for cancellation at
// using its IsCanceled method. Workers that do iterative, potentially long-running work
// should check for cancellation periodically during that work and abort if canceled.
//
// Once scheduled, the worker's lifetime proceeds as follows:
//  - It waits to be assigned a worker thread. If canceled before that, then
//    its OnSkipped method is called. By default this calls SetError("canceled"), which will
//    trigger OnError on the main thread.
//  - Execute begins executing on a worker thread. It either completes successfully or produces
//    an error (either by throwing an exception or calling SetError).
//  - The worker is removed from the queue
//  - On the main thread, if Execute completed successfully, OnOK is invoked; otherwise
//    OnError is invoked.
//
// OnOK and OnError should avoid accessing the DgnDb if IsCanceled returns true.
//
// @bsistruct
//=======================================================================================
struct DgnDbWorker : ICancellable
{
    friend struct DgnDbAsyncWorker;
    friend struct DgnDbWorkerScope;
private:
    DgnDbAsyncWorker& m_napiWorker;
    DgnDbWorkersPtr m_workers;
    BeAtomic<bool> m_canceled;
    Napi::ObjectReference m_promiseRef;

    void Run();
    void Finish();
protected:
    BeJsDocument m_output;
    Napi::Promise::Deferred m_promise;

    // If the request has not been cancelled, invoked to execute the work on a worker thread.
    virtual void Execute() = 0;

    // Invoked on the main thread after Execute completes successfully.
    virtual void OnOK() {
        BeJsNapiObject out(Env());
        out.From(m_output);
        m_promise.Resolve(out);
    }
    virtual void OnError(Napi::Error const& e) {m_promise.Reject(e.Value());}

    // Invoked by Napi::AsyncWorker::Execute if DgnDbWorker::Execute is not called because IsCanceled returns true.
    virtual void OnSkipped() { SetError("The operation was canceled"); }
public:
    // Construct only on the main thread.
    DgnDbWorker(DgnDbR db, Napi::Env env);
    ~DgnDbWorker() { m_promiseRef.Reset(); };

    // Schedule the worker to be executed on the worker thread pool. To be called at most once, and only from the main thread.
    Napi::Promise Queue();

    // Cancel the worker's scheduled execution if it has not already begun executing.
    void Cancel() { m_canceled.store(true); }
    bool IsCanceled() override { return m_canceled.load(); }

    DgnDbR GetDb() const;
    Napi::Env Env() { return m_napiWorker.Env(); }
    Napi::Promise::Deferred& Promise() { return m_promise; }
    void SetError(std::string const& error) { m_napiWorker.SetError(error); }

    static void Init(Napi::Env env, Napi::Object exports);

    // Returns a valid worker for obtaining a texture image, or throws an appropriate exception.
    static DgnDbWorkerPtr CreateTextureImageWorker(DgnDbR db, Napi::CallbackInfo const&);
};

//=======================================================================================
//! Ensures that a DgnDbWorker's Finish method is always invoked even if exceptions are thrown.
// @bsistruct
//=======================================================================================
struct DgnDbWorkerScope
{
private:
    DgnDbWorkerR m_worker;
public:
    explicit DgnDbWorkerScope(DgnDbWorkerR worker) : m_worker(worker) { }
    ~DgnDbWorkerScope() { m_worker.Finish(); }
};

} // namespace IModelJsNative

