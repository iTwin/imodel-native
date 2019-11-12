/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <folly/BeFolly.h>
#include <folly/futures/SharedPromise.h>
#include <numeric>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct TaskDependencies
{
private:
    Utf8String m_connectionId;
    Utf8String m_rulesetId;
    Utf8String m_displayType;
    SelectionInfoCPtr m_selectionInfo;
public:
    TaskDependencies(Utf8String connectionId = "", Utf8String rulesetId = "", Utf8String displayType = nullptr, SelectionInfo const* selectionInfo = nullptr)
        : m_connectionId(connectionId), m_rulesetId(rulesetId), m_displayType(displayType), m_selectionInfo(selectionInfo)
        {}
    TaskDependencies(TaskDependencies const& other)
        : m_connectionId(other.m_connectionId), m_rulesetId(other.m_rulesetId), m_displayType(other.m_displayType),
        m_selectionInfo(other.m_selectionInfo)
        {}
    TaskDependencies(TaskDependencies&& other)
        : m_connectionId(std::move(other.m_connectionId)), m_rulesetId(std::move(other.m_rulesetId)),
        m_displayType(std::move(other.m_displayType)), m_selectionInfo(std::move(other.m_selectionInfo))
        {}
    TaskDependencies& operator=(TaskDependencies const& other)
        {
        m_connectionId = other.m_connectionId;
        m_rulesetId = other.m_rulesetId;
        m_displayType = other.m_displayType;
        m_selectionInfo = other.m_selectionInfo;
        return *this;
        }
    TaskDependencies& operator=(TaskDependencies&& other)
        {
        m_connectionId = std::move(other.m_connectionId);
        m_rulesetId = std::move(other.m_rulesetId);
        m_displayType = std::move(other.m_displayType);
        m_selectionInfo = std::move(other.m_selectionInfo);
        return *this;
        }
    Utf8StringCR GetConnectionId() const {return m_connectionId;}
    Utf8StringCR GetRulesetId() const {return m_rulesetId;}
    Utf8StringCR GetDisplayType() const {return m_displayType;}
    SelectionInfo const* GetSelectionInfo() const {return m_selectionInfo.get();}
};

typedef bmap<int, unsigned> TThreadAllocationsMap;

typedef std::function<folly::Future<folly::Unit>(folly::Func)> TExecutorFunc;

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2019
+===============+===============+===============+===============+===============+======*/
struct ThreadsHelper
{
private:
    ThreadsHelper() {}
public:
    ECPRESENTATION_EXPORT static unsigned ComputeThreadsCount(TThreadAllocationsMap const&);
    ECPRESENTATION_EXPORT static TExecutorFunc WrapFollyExecutor(folly::Executor&);
    static TThreadAllocationsMap::const_iterator FindAllocationSlot(TThreadAllocationsMap const&, int priority);
    static TThreadAllocationsMap SubtractAllocations(TThreadAllocationsMap const&, bvector<int> const& priorities);
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2019
+===============+===============+===============+===============+===============+======*/
struct IECPresentationTask : RefCountedBase
{
    typedef std::function<bool(IECPresentationTask const&)> Predicate;
protected:
    virtual folly::Future<folly::Unit> _GetCompletion() const = 0;
    virtual void _Complete() = 0;
    virtual ICancelationTokenCP _GetCancelationToken() const = 0;
    virtual void _Cancel() = 0;
    virtual int _GetPriority() const = 0;
    virtual void _Execute() = 0;
    virtual TaskDependencies const& _GetDependencies() const = 0;
    virtual Predicate const& _GetBlockedTasksPredicate() const = 0;
public:
    folly::Future<folly::Unit> GetCompletion() const {return _GetCompletion();}
    void Complete() {_Complete();}
    ICancelationTokenCP GetCancelationToken() const {return _GetCancelationToken();}
    void Cancel() {_Cancel();}
    int GetPriority() const {return _GetPriority();}
    void Execute() {return _Execute();}
    TaskDependencies const& GetDependencies() const {return _GetDependencies();}
    Predicate const& GetBlockedTasksPredicate() const {return _GetBlockedTasksPredicate();}
};
DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(IECPresentationTask);
DEFINE_REF_COUNTED_PTR(IECPresentationTask);

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2019
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
* @bsiclass                                     Grigas.Petraitis                08/2019
+===============+===============+===============+===============+===============+======*/
template<typename TBase, typename TResult>
struct ECPresentationTaskBase : TBase
{
    typedef folly::SharedPromise<TResult> TPromise;
    typedef folly::Future<TResult> TFuture;
private:
    RefCountedPtr<SimpleCancelationToken> m_cancelationToken;
    TaskDependencies m_dependencies;
    int m_priority;
    IECPresentationTask::Predicate m_blockedTasksPredicate;
    folly::SharedPromise<folly::Unit> m_completionPromise;
protected:
    BeMutex& m_mutex;
    TPromise m_promise;
protected:
    virtual TFuture _GetFuture() const override {return const_cast<TPromise&>(m_promise).getFuture();}
    virtual folly::Future<folly::Unit> _GetCompletion() const override {return const_cast<folly::SharedPromise<folly::Unit>&>(m_completionPromise).getFuture();}
    virtual void _Complete() override {m_completionPromise.setValue();}
    virtual ICancelationTokenCP _GetCancelationToken() const override {return m_cancelationToken.get();}
    virtual void _Cancel() override {m_promise.getFuture().cancel();}
    virtual TaskDependencies const& _GetDependencies() const override {return m_dependencies;}
    virtual int _GetPriority() const override {return m_priority;}
    virtual IECPresentationTask::Predicate const& _GetBlockedTasksPredicate() const override {return m_blockedTasksPredicate;}
public:
    ECPresentationTaskBase(BeMutex& mutex)
        : m_mutex(mutex), m_priority(1000)
        {
        m_promise.setInterruptHandler([&](folly::exception_wrapper const& e)
            {
            BeMutexHolder lock(m_mutex);
            if (m_cancelationToken.IsValid())
                m_cancelationToken->SetCanceled(true);
            if (!m_promise.isFulfilled())
                m_promise.setException(e);
            });
        }
    void SetDependencies(TaskDependencies deps) {m_dependencies = deps;}
    void SetPriority(int priority) {m_priority = priority;}
    void SetIsCancelable(bool isCancelable) {m_cancelationToken = isCancelable ? SimpleCancelationToken::Create() : nullptr;}
    void SetBlockedTasksPredicate(IECPresentationTask::Predicate pred) {m_blockedTasksPredicate = pred;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2019
+===============+===============+===============+===============+===============+======*/
struct ECPresentationTask : ECPresentationTaskBase<IECPresentationTaskWithResult<folly::Unit>, folly::Unit>
{
private:
    std::function<void(IECPresentationTaskCR)> m_handler;
protected:
    virtual void _Execute() override
        {
        m_handler(*this);
        BeMutexHolder lock(m_mutex);
        if (!m_promise.isFulfilled())
            m_promise.setValue();
        }
public:
    ECPresentationTask(BeMutex& mutex, std::function<void(IECPresentationTaskCR)> handler)
        : ECPresentationTaskBase<IECPresentationTaskWithResult<folly::Unit>, folly::Unit>(mutex), m_handler(handler)
        {}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2019
+===============+===============+===============+===============+===============+======*/
template<typename TResult>
struct ECPresentationTaskWithResult : ECPresentationTaskBase<IECPresentationTaskWithResult<TResult>, TResult>
{
private:
    std::function<TResult(IECPresentationTaskWithResult<TResult> const&)> m_handler;
protected:
    virtual void _Execute() override
        {
        TResult result = m_handler(*this);
        BeMutexHolder lock(this->m_mutex);
        if (!this->m_promise.isFulfilled())
            this->m_promise.setValue(result);
        }
public:
    ECPresentationTaskWithResult(BeMutex& mutex, std::function<TResult(IECPresentationTaskWithResult<TResult> const&)> handler)
        : ECPresentationTaskBase<IECPresentationTaskWithResult<TResult>, TResult>(mutex), m_handler(handler)
        {}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2019
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
    bset<IECPresentationTaskCPtr> const& GetTasks() const {return m_tasks;}
    folly::Future<folly::Unit>& GetCompletion() {return m_completion;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2019
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
* @bsiclass                                     Grigas.Petraitis                08/2019
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
// @bsiclass                                    Mantas.Kontrimas                06/2018
//=======================================================================================
struct ECPresentationThreadPool : BeFolly::ThreadPool
    {
    ECPresentationThreadPool(int threadsCount) : ThreadPool(threadsCount, "ECPresentation") {}
    ~ECPresentationThreadPool() {WaitForIdle();}
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2019
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
* @bsiclass                                     Grigas.Petraitis                09/2019
+===============+===============+===============+===============+===============+======*/
struct IECPresentationTasksScheduler
{
protected:
    virtual BeMutex& _GetMutex() const = 0;
    virtual void _Schedule(IECPresentationTaskR) = 0;
    virtual TasksCancelationResult _Cancel(IECPresentationTask::Predicate const&) = 0;
    virtual void _Block(IECPresentationTasksBlocker const*) = 0;
    virtual void _Unblock(IECPresentationTasksBlocker const*) = 0;
    virtual folly::Future<folly::Unit> _GetAllTasksCompletion(IECPresentationTask::Predicate const&) const = 0;
public:
    virtual ~IECPresentationTasksScheduler() {}
    BeMutex& GetMutex() const { return _GetMutex(); }
    void Schedule(IECPresentationTaskR task) {_Schedule(task);}
    TasksCancelationResult Cancel(IECPresentationTask::Predicate const& pred = nullptr) {return _Cancel(pred);}
    void Block(IECPresentationTasksBlocker const* blocker) {_Block(blocker);}
    void Unblock(IECPresentationTasksBlocker const* blocker) {_Unblock(blocker);}
    folly::Future<folly::Unit> GetAllTasksCompletion(IECPresentationTask::Predicate const& pred = nullptr) const {return _GetAllTasksCompletion(pred);}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2019
+===============+===============+===============+===============+===============+======*/
struct ECPresentationTasksScheduler : IECPresentationTasksScheduler
{
private:
    IECPresentationTasksQueue* m_queue;
    TExecutorFunc m_executor;
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
    ECPRESENTATION_EXPORT void _Block(IECPresentationTasksBlocker const*) override;
    ECPRESENTATION_EXPORT void _Unblock(IECPresentationTasksBlocker const*) override;
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> _GetAllTasksCompletion(IECPresentationTask::Predicate const&) const override;
public:
    ECPresentationTasksScheduler(TThreadAllocationsMap threadAllocations, IECPresentationTasksQueue& queue, TExecutorFunc executor)
        : m_queue(&queue), m_executor(executor), m_threadAllocations(threadAllocations)
        {
        m_threadsCount = ThreadsHelper::ComputeThreadsCount(m_threadAllocations);
        }
    ~ECPresentationTasksScheduler() {DELETE_AND_CLEAR(m_queue);}
    unsigned GetThreadsCount() const {return m_threadsCount;}
    bset<IECPresentationTaskPtr> const& GetRunningTasks() const {return m_runningTasks;}
    ECPRESENTATION_EXPORT void SetThreadAllocationsMap(TThreadAllocationsMap);
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2019
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
* @bsiclass                                     Grigas.Petraitis                08/2019
+===============+===============+===============+===============+===============+======*/
struct ECPresentationTasksManager
{
private:
    folly::Executor* m_tasksExecutor;
    IECPresentationTasksScheduler* m_scheduler;
    unsigned m_threadsCount;
private:
    static folly::Executor* CreateExecutor(TThreadAllocationsMap const&);
    static ECPresentationTasksScheduler* CreateScheduler(TThreadAllocationsMap const&, folly::Executor&);
public:
    ECPresentationTasksManager(TThreadAllocationsMap threadAllocations);
    ~ECPresentationTasksManager();
    BeMutex& GetMutex() const {return m_scheduler->GetMutex();}
    folly::Future<folly::Unit> Execute(IECPresentationTaskR task)
        {
        m_scheduler->Schedule(task);
        return task.GetCompletion();
        }
    folly::Future<folly::Unit> CreateAndExecute(std::function<void(IECPresentationTaskCR)> func,
        TaskDependencies deps = TaskDependencies(), bool isCancelable = true, int priority = 1000)
        {
        RefCountedPtr<ECPresentationTask> task = new ECPresentationTask(GetMutex(), func);
        task->SetDependencies(deps);
        task->SetIsCancelable(isCancelable);
        task->SetPriority(priority);
        return Execute(*task);
        }
    template<typename TResult> folly::Future<TResult> Execute(IECPresentationTaskWithResult<TResult>& task)
        {
        m_scheduler->Schedule(task);
        return task.GetFuture();
        }
    template<typename TResult> folly::Future<TResult> CreateAndExecute(std::function<TResult(IECPresentationTaskWithResult<TResult> const&)> func, 
        TaskDependencies deps = TaskDependencies(), bool isCancelable = true, int priority = 1000)
        {
        RefCountedPtr<ECPresentationTaskWithResult<TResult>> task = new ECPresentationTaskWithResult<TResult>(GetMutex(), func);
        task->SetDependencies(deps);
        task->SetIsCancelable(isCancelable);
        task->SetPriority(priority);
        return Execute(*task);
        }
    TasksCancelationResult Cancel(IECPresentationTask::Predicate const& pred) {return m_scheduler->Cancel(pred);}
    RefCountedPtr<ECPresentationTasksBlocker> Block(IECPresentationTask::Predicate pred) {return ECPresentationTasksBlocker::Create(*m_scheduler, pred);}
    folly::Future<folly::Unit> GetAllTasksCompletion(IECPresentationTask::Predicate const& pred = nullptr) const {return m_scheduler->GetAllTasksCompletion(pred);}
    unsigned GetThreadsCount() const {return m_threadsCount;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
