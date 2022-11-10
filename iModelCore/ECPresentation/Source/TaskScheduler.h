/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentationManager.h>
#include <folly/BeFolly.h>
#include <folly/futures/SharedPromise.h>
#include <numeric>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ITaskDependency
{
protected:
    virtual Utf8CP _GetDependencyType() const = 0;
    virtual bool _Matches(ITaskDependency const& other) const {return _GetDependencyType() == other._GetDependencyType();}
public:
    virtual ~ITaskDependency() {}
    Utf8CP GetDependencyType() const {return _GetDependencyType();}
    bool Matches(ITaskDependency const& other) const {return _Matches(other);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct StringBasedTaskDependency : ITaskDependency
{
private:
    Utf8String m_value;
protected:
    virtual bool _Matches(ITaskDependency const& other) const override
        {
        return ITaskDependency::_Matches(other)
            && m_value.Equals(static_cast<StringBasedTaskDependency const&>(other).m_value);
        }
    StringBasedTaskDependency(Utf8String value) : m_value(value) {}
    static std::function<bool(ITaskDependency const&)> CreatePredicate(Utf8CP dependencyType, std::function<bool(Utf8StringCR)>);
public:
    Utf8StringCR GetValue() const {return m_value;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TaskDependencyOnConnection : StringBasedTaskDependency
{
private:
    static Utf8CP s_dependencyType;
protected:
    Utf8CP _GetDependencyType() const override {return s_dependencyType;}
    bool _Matches(ITaskDependency const& other) const override
        {
        return ITaskDependency::_Matches(other)
            && (GetValue().Equals(static_cast<TaskDependencyOnConnection const&>(other).GetValue()) || GetValue().Equals("*"));
        }
public:
    TaskDependencyOnConnection(Utf8String connectionId) : StringBasedTaskDependency(connectionId) { }
    static std::function<bool(ITaskDependency const&)> CreatePredicate(std::function<bool(Utf8StringCR)> pred) {return StringBasedTaskDependency::CreatePredicate(s_dependencyType, pred);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TaskDependencyOnRuleset : StringBasedTaskDependency
    {
    Utf8CP _GetDependencyType() const override {return "Ruleset";}
    TaskDependencyOnRuleset(Utf8String rulesetId) : StringBasedTaskDependency(rulesetId) {}
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TaskDependencyOnDisplayType : StringBasedTaskDependency
    {
    Utf8CP _GetDependencyType() const override {return "DisplayType";}
    TaskDependencyOnDisplayType(Utf8String displayType) : StringBasedTaskDependency(displayType) {}
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TaskDependencyOnSelection : ITaskDependency
{
private:
    static Utf8CP s_dependencyType;
    SelectionInfoCPtr m_selectionInfo;
protected:
    Utf8CP _GetDependencyType() const override {return s_dependencyType;}
    bool _Matches(ITaskDependency const& other) const override
        {
        return ITaskDependency::_Matches(other)
            && (*m_selectionInfo) == (*static_cast<TaskDependencyOnSelection const&>(other).m_selectionInfo);
        }
public:
    TaskDependencyOnSelection(SelectionInfoCR selectionInfo) : m_selectionInfo(&selectionInfo) {}
    SelectionInfoCR GetSelectionInfo() const {return *m_selectionInfo;}
    static std::function<bool(ITaskDependency const&)> CreatePredicate(std::function<bool(SelectionInfoCR)>);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TaskDependencyOnParentNode : ITaskDependency
{
private:
    BeGuid m_parentId;
protected:
    Utf8CP _GetDependencyType() const override { return "ParentNode"; }
    bool _Matches(ITaskDependency const& other) const override
        {
        if (!ITaskDependency::_Matches(other))
            return false;
        BeGuid otherParentId = static_cast<TaskDependencyOnParentNode const&>(other).m_parentId;
        return m_parentId == otherParentId;
        }
public:
    TaskDependencyOnParentNode(): m_parentId() {}
    TaskDependencyOnParentNode(BeGuid parentId) : m_parentId(parentId) {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TaskDependencies : bvector<std::shared_ptr<ITaskDependency>>
{
    DEFINE_T_SUPER(bvector<std::shared_ptr<ITaskDependency>>)
    TaskDependencies() {}
    TaskDependencies(TaskDependencies const& other) : T_Super(other) {}
    TaskDependencies(TaskDependencies&& other) : T_Super(std::move(other)) {}
    TaskDependencies(std::initializer_list<std::shared_ptr<ITaskDependency>> deps) : T_Super(deps) {}
    TaskDependencies& operator=(TaskDependencies const& other) {assign(other.begin(), other.end()); return *this;}
    TaskDependencies& operator=(TaskDependencies&& other) {swap(other); return *this;}
    bool Has(ITaskDependency const& dep) const;
    bool Has(std::function<bool(ITaskDependency const&)> pred) const;
};

typedef bmap<int, unsigned> TThreadAllocationsMap;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ThreadsHelper
{
private:
    ThreadsHelper() {}
public:
    ECPRESENTATION_EXPORT static unsigned ComputeThreadsCount(TThreadAllocationsMap const&);
    static TThreadAllocationsMap::const_iterator FindAllocationSlot(TThreadAllocationsMap const&, int priority);
    static TThreadAllocationsMap SubtractAllocations(TThreadAllocationsMap const&, bvector<int> const& priorities);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IECPresentationTask : RefCountedBase
{
    typedef std::function<bool(IECPresentationTask const&)> Predicate;
protected:
    virtual BeGuidCR _GetId() const = 0;
    virtual uint64_t _GetCreateTimestamp() const = 0;
    virtual folly::Future<folly::Unit> _GetCompletion() const = 0;
    virtual void _Complete() = 0;
    virtual ICancelationTokenCP _GetCancelationToken() const = 0;
    virtual void _Cancel() = 0;
    virtual void _Restart() = 0;
    virtual int _GetPriority() const = 0;
    virtual void _OnBeforeExecute() {}
    virtual void _OnAfterExecute() {}
    virtual std::function<void()> _Execute() = 0;
    virtual TaskDependencies const& _GetDependencies() const = 0;
    virtual Predicate const& _GetOtherTasksBlockingPredicate() const = 0;
    virtual bool _IsBlocked() const = 0;
    virtual void _SetTaskConnection(IConnectionCR) const = 0;
public:
    BeGuidCR GetId() const {return _GetId();}
    uint64_t GetCreateTimestamp() const {return _GetCreateTimestamp();}
    folly::Future<folly::Unit> GetCompletion() const {return _GetCompletion();}
    void Complete() {_Complete();}
    ICancelationTokenCP GetCancelationToken() const {return _GetCancelationToken();}
    void Cancel() {_Cancel();}
    void Restart() {_Restart();}
    int GetPriority() const {return _GetPriority();}
    void OnBeforeExecute() {_OnBeforeExecute();}
    void OnAfterExecute() {_OnAfterExecute();}
    std::function<void()> Execute() {return _Execute();}
    TaskDependencies const& GetDependencies() const {return _GetDependencies();}
    Predicate const& GetOtherTasksBlockingPredicate() const {return _GetOtherTasksBlockingPredicate();}
    bool IsBlocked() const {return _IsBlocked();}
    void SetTaskConnection(IConnectionCR connection) const {_SetTaskConnection(connection);}
};
DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(IECPresentationTask);
DEFINE_REF_COUNTED_PTR(IECPresentationTask);

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename TResult>
struct IECPresentationTaskWithResult : IECPresentationTask
{
protected:
    virtual folly::Future<TResult> _GetFuture() const = 0;
public:
    folly::Future<TResult> GetFuture() const {return _GetFuture();}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename TBase, typename TResult>
struct ECPresentationTaskBase : TBase
{
    typedef folly::SharedPromise<TResult> TPromise;
    typedef folly::Future<TResult> TFuture;
private:
    BeGuid m_id;
    uint64_t m_createTimestamp;
    RefCountedPtr<SimpleCancelationToken> m_cancelationToken;
    TaskDependencies m_dependencies;
    int m_priority;
    IECPresentationTask::Predicate m_otherTasksBlockingPredicate;
    std::function<bool()> m_blockPredicate;
    folly::SharedPromise<folly::Unit> m_completionPromise;
    folly::Executor* m_futureExecutor;
    std::shared_ptr<bool> m_promiseResolved;
    mutable IConnectionCPtr m_connection;
protected:
    BeMutex& m_mutex;
    TPromise m_promise;
    std::shared_ptr<Diagnostics::Scope> m_diagnosticsScope;
protected:
    void Resolve(folly::Try<TResult>&& res)
        {
        BeMutexHolder lock(m_mutex);
        if (m_promise.isFulfilled())
            return;

        // store the flag in a `shared_ptr<bool>` to allow accessing it after the task is destroyed
        (*m_promiseResolved) = true;
        // set the result
        m_promise.setTry(std::move(res));
        }
    virtual BeGuidCR _GetId() const override {return m_id;}
    virtual uint64_t _GetCreateTimestamp() const override {return m_createTimestamp;}
    virtual TResult _CreateResult() = 0;
    virtual TFuture _GetFuture() const override
        {
        TFuture f = const_cast<TPromise&>(m_promise).getFuture();
        // Note: returning the future directly will make all chained `then` callbacks resolved directly
        // in the same task execution context as soon as the promise resolves. On the other hand, returning it
        // via an executor allows the current task to complete and executes the callbacks as separate executor
        // tasks (possibly on other threads), not tracked by our tasks scheduler.
        return m_futureExecutor ? std::move(f).via(m_futureExecutor) : std::move(f);
        }
    virtual folly::Future<folly::Unit> _GetCompletion() const override {return const_cast<folly::SharedPromise<folly::Unit>&>(m_completionPromise).getFuture();}
    virtual void _Complete() override {m_completionPromise.setValue();}
    virtual ICancelationTokenCP _GetCancelationToken() const override {return m_cancelationToken.get();}
    virtual void _Cancel() override 
        {
        m_promise.getFuture().cancel();
        }
    virtual void _Restart() override
        {
        BeMutexHolder lock(m_mutex);
        if (m_cancelationToken.IsValid())
            m_cancelationToken->SetCanceled(true);

        if (m_connection.IsValid() && m_connection->IsOpen())
            {
            DisableProxyConnectionThreadVerification noThreadVerification(*m_connection);
            m_connection->InterruptRequests();
            }

        m_completionPromise.getFuture().ensure([this]()
            {
            m_cancelationToken = m_cancelationToken.IsValid() ? SimpleCancelationToken::Create() : nullptr;
            m_completionPromise = folly::SharedPromise<folly::Unit>();
            });
        }
    virtual std::function<void()> _Execute() override
        {
        try
            {
            TResult res = _CreateResult();
            return [this, res = std::move(res)](){Resolve(folly::Try<TResult>(res));};
            }
        catch (DbConnectionInterruptException const&)
            {
            // ignore db connection interrupt exception if task was canceled. It might happen during restart when cancelation token is canceled
            // but connection was interrupted before task could check cancelation token.
            if (!m_cancelationToken.IsValid() || !m_cancelationToken->IsCanceled())
                return [this](){Resolve(folly::Try<TResult>(InternalError("Unexpected Db connection interrupt occurred.")));};
            }
        catch (CancellationException const&)
            {
            // ignore cancellation exception. Task can only be canceled in two ways:
            // 1. 'm_promise.getFuture().cancel()' was called in which case m_promise will be fulfilled with
            //    correct exception through interrupt handler
            // 2. task was restarted in which case we do not need to fullfil m_promise yet
            }
        catch (std::exception const& e)
            {
            return [this, e = folly::exception_wrapper{ std::current_exception(), e }](){Resolve(folly::Try<TResult>(e));};
            } 
        catch (...) 
            {
            return [this](){Resolve(folly::Try<TResult>(InternalError("Unexpected error occurred.")));};
            }
        return nullptr;
        }
    virtual void _OnBeforeExecute() override
        {
        if (m_diagnosticsScope)
            m_diagnosticsScope->Attach();
        }
    virtual void _OnAfterExecute() override
        {
        if (m_diagnosticsScope)
            m_diagnosticsScope->Detach();
        }
    virtual TaskDependencies const& _GetDependencies() const override {return m_dependencies;}
    virtual int _GetPriority() const override {return m_priority;}
    virtual IECPresentationTask::Predicate const& _GetOtherTasksBlockingPredicate() const override {return m_otherTasksBlockingPredicate;}
    virtual bool _IsBlocked() const override {return m_blockPredicate && m_blockPredicate();}
    void _SetTaskConnection(IConnectionCR connection) const override {m_connection = &connection;}
public:
    ECPresentationTaskBase(BeMutex& mutex)
        : m_id(true), m_createTimestamp(BeTimeUtilities::GetCurrentTimeAsUnixMillis()), m_mutex(mutex), m_priority(1000), m_futureExecutor(nullptr)
        {
        m_diagnosticsScope = Diagnostics::GetCurrentScope().lock();
        m_promiseResolved = std::make_shared<bool>(false);
        m_promise.setInterruptHandler([&, promiseResolved = m_promiseResolved](folly::exception_wrapper const& e)
            {
            BeMutexHolder lock(m_mutex);
            if (*promiseResolved)
                {
                // the interrupt handler may outlive the task - the `promiseResolved` shared_ptr is used
                // to track when the promise is resolved, at which point there's no need to do any of the
                // following, which may also be unsafe due to task being destroyed
                return;
                }

            if (m_cancelationToken.IsValid())
                m_cancelationToken->SetCanceled(true);

            if (m_connection.IsValid() && m_connection->IsOpen())
                {
                DisableProxyConnectionThreadVerification noThreadVerification(*m_connection);
                m_connection->InterruptRequests();
                }

            if (e.is_compatible_with<folly::FutureCancellation>())
                {
                // this is only needed to handle the case of API consumer calling `folly::Future::cancel`, in
                // which case the interrupt handler is called with `folly::FutureCancellation`.
                Resolve(folly::Try<TResult>(CancellationException()));
                return;
                }
            Resolve(folly::Try<TResult>(e));
            });
        }
    void SetDependencies(TaskDependencies deps) {m_dependencies = deps;}
    void SetPriority(int priority) {m_priority = priority;}
    void SetIsCancelable(bool isCancelable) {m_cancelationToken = isCancelable ? SimpleCancelationToken::Create() : nullptr;}
    void SetOtherTasksBlockingPredicate(IECPresentationTask::Predicate pred) {m_otherTasksBlockingPredicate = pred;}
    void SetThisTaskBlockingPredicate(std::function<bool()> pred) {m_blockPredicate = pred;}
    void SetExecutor(folly::Executor* e) {m_futureExecutor = e;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECPresentationTask : ECPresentationTaskBase<IECPresentationTaskWithResult<folly::Unit>, folly::Unit>
{
private:
    std::function<void(IECPresentationTaskCR)> m_handler;
protected:
    folly::Unit _CreateResult() override {m_handler(*this); return folly::unit;}
public:
    ECPresentationTask(BeMutex& mutex, std::function<void(IECPresentationTaskCR)> handler)
        : ECPresentationTaskBase<IECPresentationTaskWithResult<folly::Unit>, folly::Unit>(mutex), m_handler(handler)
        {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename TResult>
struct ECPresentationTaskWithResult : ECPresentationTaskBase<IECPresentationTaskWithResult<TResult>, TResult>
{
private:
    std::function<TResult(IECPresentationTaskWithResult<TResult> const&)> m_handler;
protected:
    TResult _CreateResult() override {return m_handler(*this);}
public:
    ECPresentationTaskWithResult(BeMutex& mutex, std::function<TResult(IECPresentationTaskWithResult<TResult> const&)> handler)
        : ECPresentationTaskBase<IECPresentationTaskWithResult<TResult>, TResult>(mutex), m_handler(handler)
        {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TasksCancelationResult
{
private:
    bset<IECPresentationTaskCPtr> m_tasks;
    folly::Future<folly::Unit> m_completion;
private:
    static folly::Future<folly::Unit> CreateTasksCompletion(bset<IECPresentationTaskCPtr> const& tasks)
        {
        std::vector<folly::Future<folly::Unit>> completions;
        for (IECPresentationTaskCPtr const& task : tasks)
            completions.push_back(task->GetCompletion());
        return folly::collectAll(completions).ensure([]() {}).then();
        }
public:
    TasksCancelationResult(bset<IECPresentationTaskCPtr> tasks)
        : m_tasks(tasks), m_completion(CreateTasksCompletion(m_tasks))
        {}
    TasksCancelationResult(TasksCancelationResult&& other)
        : m_tasks(std::move(other.m_tasks)), m_completion(std::move(other.m_completion))
        {}
    bset<IECPresentationTaskCPtr> const& GetTasks() const {return m_tasks;}
    folly::Future<folly::Unit>& GetCompletion() {return m_completion;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IECPresentationTasksQueue
{
protected:
    virtual BeMutex& _GetMutex() const = 0;
    virtual bool _HasTasks() const = 0;
    virtual void _Add(IECPresentationTask&) = 0;
    virtual IECPresentationTaskPtr _Pop(IECPresentationTask::Predicate const&) = 0;
    virtual bvector<IECPresentationTaskPtr> _Get(IECPresentationTask::Predicate const&) const = 0;
    virtual TasksCancelationResult _Cancel(IECPresentationTask::Predicate const&) = 0;
public:
    virtual ~IECPresentationTasksQueue() {}
    BeMutex& GetMutex() const {return _GetMutex();}
    bool HasTasks() const {return _HasTasks();}
    void Add(IECPresentationTask& task) {_Add(task);}
    IECPresentationTaskPtr Pop(IECPresentationTask::Predicate const& filter = nullptr) {return _Pop(filter);}
    bvector<IECPresentationTaskPtr> Get(IECPresentationTask::Predicate const& filter = nullptr) const {return _Get(filter);}
    TasksCancelationResult Cancel(IECPresentationTask::Predicate const& pred = [](IECPresentationTaskCR){return true;}) {return _Cancel(pred);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECPresentationTasksQueue : IECPresentationTasksQueue
{
private:
    mutable BeMutex m_mutex;
    bmap<int, std::deque<IECPresentationTaskPtr>, std::greater<int>> m_prioritizedQueue;
protected:
    virtual BeMutex& _GetMutex() const override {return m_mutex;}
    ECPRESENTATION_EXPORT virtual bool _HasTasks() const override;
    ECPRESENTATION_EXPORT virtual void _Add(IECPresentationTask& task) override;
    ECPRESENTATION_EXPORT virtual IECPresentationTaskPtr _Pop(IECPresentationTask::Predicate const&) override;
    ECPRESENTATION_EXPORT virtual bvector<IECPresentationTaskPtr> _Get(IECPresentationTask::Predicate const&) const override;
    ECPRESENTATION_EXPORT virtual TasksCancelationResult _Cancel(IECPresentationTask::Predicate const&) override;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ECPresentationThreadPool : BeFolly::ThreadPool
    {
    ECPresentationThreadPool(int threadsCount) : ThreadPool(threadsCount, "ECPresentation") {}
    ~ECPresentationThreadPool() {WaitForIdle();}
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IECPresentationTasksBlocker
{
protected:
    virtual bool _IsBlocked(IECPresentationTaskCR) const = 0;
public:
    virtual ~IECPresentationTasksBlocker() {}
    bool IsBlocked(IECPresentationTaskCR task) const {return _IsBlocked(task);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IECPresentationTasksScheduler
{
protected:
    virtual BeMutex& _GetMutex() const = 0;
    virtual void _Schedule(IECPresentationTaskR) = 0;
    virtual TasksCancelationResult _Cancel(IECPresentationTask::Predicate const&) = 0;
    virtual TasksCancelationResult _Restart(IECPresentationTask::Predicate const&) = 0;
    virtual void _Block(IECPresentationTasksBlocker const*) = 0;
    virtual void _Unblock(IECPresentationTasksBlocker const*) = 0;
    virtual folly::Future<folly::Unit> _GetAllTasksCompletion(IECPresentationTask::Predicate const&) const = 0;
public:
    virtual ~IECPresentationTasksScheduler() {}
    BeMutex& GetMutex() const { return _GetMutex(); }
    void Schedule(IECPresentationTaskR task) {_Schedule(task);}
    TasksCancelationResult Cancel(IECPresentationTask::Predicate const& pred = nullptr) {return _Cancel(pred);}
    TasksCancelationResult Restart(IECPresentationTask::Predicate const& pred = nullptr) {return _Restart(pred);}
    void Block(IECPresentationTasksBlocker const* blocker) {_Block(blocker);}
    void Unblock(IECPresentationTasksBlocker const* blocker) {_Unblock(blocker);}
    folly::Future<folly::Unit> GetAllTasksCompletion(IECPresentationTask::Predicate const& pred = nullptr) const {return _GetAllTasksCompletion(pred);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECPresentationTasksScheduler : IECPresentationTasksScheduler
{
private:
    IECPresentationTasksQueue* m_queue;
    folly::Executor& m_executor;
    bvector<IECPresentationTaskPtr> m_pendingTasks;
    bset<IECPresentationTaskPtr> m_runningTasks;
    bvector<IECPresentationTasksBlocker const*> m_blockers;
    TThreadAllocationsMap m_threadAllocations;
    unsigned m_threadsCount;
private:
    void CheckTasks();
    IECPresentationTaskPtr PopTask(IECPresentationTask::Predicate const& filter, TThreadAllocationsMap&);
    bool CanExecute(IECPresentationTaskCR) const;
    void ExecuteTask(IECPresentationTaskR);
    IECPresentationTask::Predicate CreateTasksFilter(TThreadAllocationsMap const&) const;
    TThreadAllocationsMap CreateAvailableAllocationsMap() const;
protected:
    BeMutex& _GetMutex() const override {return m_queue->GetMutex();}
    ECPRESENTATION_EXPORT void _Schedule(IECPresentationTaskR) override;
    ECPRESENTATION_EXPORT TasksCancelationResult _Cancel(IECPresentationTask::Predicate const&) override;
    ECPRESENTATION_EXPORT TasksCancelationResult _Restart(IECPresentationTask::Predicate const&) override;
    ECPRESENTATION_EXPORT void _Block(IECPresentationTasksBlocker const*) override;
    ECPRESENTATION_EXPORT void _Unblock(IECPresentationTasksBlocker const*) override;
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> _GetAllTasksCompletion(IECPresentationTask::Predicate const&) const override;
public:
    ECPresentationTasksScheduler(TThreadAllocationsMap threadAllocations, IECPresentationTasksQueue& queue, folly::Executor& executor)
        : m_queue(&queue), m_executor(executor), m_threadAllocations(threadAllocations)
        {
        m_threadsCount = ThreadsHelper::ComputeThreadsCount(m_threadAllocations);
        }
    ~ECPresentationTasksScheduler() {DELETE_AND_CLEAR(m_queue);}
    unsigned GetThreadsCount() const {return m_threadsCount;}
    bset<IECPresentationTaskPtr> const& GetRunningTasks() const {return m_runningTasks;}
    bvector<IECPresentationTaskPtr> const& GetPendingTasks() const {return m_pendingTasks;}
    ECPRESENTATION_EXPORT void SetThreadAllocationsMap(TThreadAllocationsMap);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECPresentationTasksBlocker : RefCountedBase, IECPresentationTasksBlocker
{
private:
    IECPresentationTasksScheduler& m_scheduler;
    IECPresentationTask::Predicate m_predicate;
private:
    ECPresentationTasksBlocker(IECPresentationTasksScheduler& scheduler, IECPresentationTask::Predicate pred)
        : m_scheduler(scheduler), m_predicate(pred)
        {
        m_scheduler.Block(this);
        }
protected:
    bool _IsBlocked(IECPresentationTaskCR task) const override {return m_predicate(task);}
public:
    static RefCountedPtr<ECPresentationTasksBlocker> Create(IECPresentationTasksScheduler& scheduler, IECPresentationTask::Predicate pred)
        {
        return new ECPresentationTasksBlocker(scheduler, pred);
        }
    ~ECPresentationTasksBlocker() {m_scheduler.Unblock(this);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECPresentationTaskParams
{
private:
    int m_priority;
    bool m_isCancellable;
    TaskDependencies m_dependencies;
    IECPresentationTask::Predicate m_otherTasksBlockingPredicate;
    std::function<bool()> m_blockPredicate;
public:
    ECPresentationTaskParams() : m_priority(1000), m_isCancellable(true) {}

    TaskDependencies const& GetDependencies() const {return m_dependencies;}
    void SetDependencies(TaskDependencies deps) {m_dependencies = deps;}

    int GetPriority() const {return m_priority;}
    void SetPriority(int priority) {m_priority = priority;}

    bool IsCancellable() const {return m_isCancellable;}
    void SetIsCancelable(bool value) {m_isCancellable = value;}

    IECPresentationTask::Predicate const& GetOtherTasksBlockingPredicate() const {return m_otherTasksBlockingPredicate;}
    void SetOtherTasksBlockingPredicate(IECPresentationTask::Predicate pred) {m_otherTasksBlockingPredicate = pred;}

    std::function<bool()> const& GetThisTaskBlockingPredicate() const {return m_blockPredicate;}
    void SetThisTaskBlockingPredicate(std::function<bool()> pred) {m_blockPredicate = pred;}

    template<typename TResult>
    void Apply(ECPresentationTaskBase<IECPresentationTaskWithResult<TResult>, TResult>& task) const
        {
        task.SetDependencies(GetDependencies());
        task.SetIsCancelable(IsCancellable());
        task.SetPriority(GetPriority());
        task.SetThisTaskBlockingPredicate(GetThisTaskBlockingPredicate());
        task.SetOtherTasksBlockingPredicate(GetOtherTasksBlockingPredicate());
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECPresentationTasksManager
{
private:
    std::unique_ptr<folly::Executor> m_tasksExecutor;
    IECPresentationTasksScheduler* m_scheduler;
    unsigned m_threadsCount;
private:
    static std::unique_ptr<folly::Executor> CreateExecutor(TThreadAllocationsMap const&);
    static ECPresentationTasksScheduler* CreateScheduler(TThreadAllocationsMap const&, folly::Executor&);
    template<typename TResult> folly::Future<TResult> Execute(IECPresentationTaskWithResult<TResult>& task)
        {
        m_scheduler->Schedule(task);
        return task.GetFuture();
        }
public:
    ECPRESENTATION_EXPORT ECPresentationTasksManager(TThreadAllocationsMap threadAllocations);
    ECPRESENTATION_EXPORT ~ECPresentationTasksManager();
    BeMutex& GetMutex() const {return m_scheduler->GetMutex();}
    folly::Future<folly::Unit> CreateAndExecute(std::function<void(IECPresentationTaskCR)> func, ECPresentationTaskParams const& params = {})
        {
        RefCountedPtr<ECPresentationTask> task = new ECPresentationTask(m_scheduler->GetMutex(), func);
        task->SetExecutor(m_tasksExecutor.get());
        params.Apply(*task);
        return Execute<folly::Unit>(*task);
        }
    template<typename TResult> folly::Future<TResult> CreateAndExecute(std::function<TResult(IECPresentationTaskWithResult<TResult> const&)> func, ECPresentationTaskParams const& params = {})
        {
        RefCountedPtr<ECPresentationTaskWithResult<TResult>> task = new ECPresentationTaskWithResult<TResult>(m_scheduler->GetMutex(), func);
        task->SetExecutor(m_tasksExecutor.get());
        params.Apply(*task);
        return Execute<TResult>(*task);
        }
    TasksCancelationResult Cancel(IECPresentationTask::Predicate const& pred) {return m_scheduler->Cancel(pred);}
    TasksCancelationResult Restart(IECPresentationTask::Predicate const& pred) {return m_scheduler->Restart(pred);}
    RefCountedPtr<ECPresentationTasksBlocker> Block(IECPresentationTask::Predicate pred) {return ECPresentationTasksBlocker::Create(*m_scheduler, pred);}
    folly::Future<folly::Unit> GetAllTasksCompletion(IECPresentationTask::Predicate const& pred = nullptr) const {return m_scheduler->GetAllTasksCompletion(pred);}
    unsigned GetThreadsCount() const {return m_threadsCount;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
