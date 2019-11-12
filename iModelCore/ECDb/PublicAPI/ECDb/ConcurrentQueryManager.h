/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <chrono>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
using namespace std::chrono_literals;
typedef uint32_t TaskId;
struct ECDb;

//=======================================================================================
//! @bsiclass                                                        05/2019
//=======================================================================================
struct ConcurrentQueryManager final
    {
    enum class Priority
        {
        Low = 0,
        Normal = 1,
        High = 2
        };
    enum class PollStatus
        {
        NotInitalized = 0,
        Done = 1,
        Pending = 2,
        Partial = 3,
        Timeout = 4,
        Error = 5,
        NotFound = 6,
        };
    enum class PostStatus
        {
        NotInitalized = 0,
        Done = 1,
        QueueSizeExceded = 2,
        };

    struct Quota final
        {
        private:
            std::chrono::seconds m_timeLimit;
            uint32_t m_memoryLimit;
        public:
            Quota() : m_timeLimit(0), m_memoryLimit(0) {}
            explicit Quota(unsigned int timeLimit) : m_timeLimit(timeLimit), m_memoryLimit(0) {}
            Quota(std::chrono::seconds timeLimit, uint32_t sizeInBytes) : m_timeLimit(timeLimit), m_memoryLimit(sizeInBytes) {}
            ~Quota() {}
            std::chrono::seconds MaxTimeAllowed() const noexcept { return m_timeLimit; }
            uint32_t MaxMemoryAllowed() const noexcept { return m_memoryLimit; }
            bool IsEmpty() const { return m_memoryLimit == 0 && m_timeLimit == std::chrono::seconds(0); }
        };

    struct RequestContext final
        {
        private:
            std::function<void()> m_callbackOnTaskStart;
        public:
            RequestContext(std::function<void()> onTaskStart = nullptr)
                : m_callbackOnTaskStart(onTaskStart)
                {}
            void OnTaskStart() const {if (m_callbackOnTaskStart) {m_callbackOnTaskStart();}}
        };
    struct Config final
        {
        private:
            unsigned int m_concurrent;
            unsigned int m_cacheStatementsPerThread;
            unsigned int m_maxQueueSize;
            std::chrono::seconds m_idleCleanupTime;
            std::chrono::seconds m_minMonitorInterval;
            std::chrono::seconds m_completedTaskExpires;
            std::function<void(Db const&)> m_afterConnectionOpenned;
            std::function<void(Db const&)> m_beforeConnectionClosed;
            bool m_useSharedCache;
            bool m_useUncommitedRead;
            Quota m_quota;
        public:
            ECDB_EXPORT Config(const Config&& rhs);
            ECDB_EXPORT Config(const Config& rhs);
            ECDB_EXPORT Config& operator = (const Config&& rhs);
            ECDB_EXPORT Config& operator = (const Config& rhs);
            ECDB_EXPORT Config();
            ~Config() {}
            // get
            Quota const& GetQuota() const noexcept { return m_quota; }
            unsigned int GetConcurrent() const noexcept { return m_concurrent; }
            unsigned int GetMaxQueueSize() const noexcept { return m_maxQueueSize; }
            unsigned int GetCacheStatementPerThread() const noexcept { return m_cacheStatementsPerThread; }            
            unsigned int GetUseSharedCache() const noexcept { return m_useSharedCache; }
            unsigned int GetUseUncommitedRead() const noexcept { return m_useUncommitedRead; }
            std::chrono::seconds GetMinMonitorInterval() const noexcept { return m_minMonitorInterval; }
            std::chrono::seconds GetIdleCleanupTime() const noexcept { return m_idleCleanupTime; }
            std::chrono::seconds GetAutoExpireTimeForCompletedQuery() const { return m_completedTaskExpires; }
            std::function<void(Db const&)> const& GetAfterConnectionOpenedCallback() const noexcept { return  m_afterConnectionOpenned; }
            std::function<void(Db const&)> const& GetBeforeConnectionClosedCallback() const noexcept { return  m_beforeConnectionClosed; }
            // set
            Config& SetAutoExpireTimeForCompletedQuery(std::chrono::seconds seconds) noexcept { m_completedTaskExpires = seconds; return *this; }
            Config& SetConcurrent(unsigned int v) noexcept { m_concurrent = v; return *this; }
            Config& SetCacheStatementsPerThread(unsigned int v) noexcept { m_cacheStatementsPerThread = v; return *this; }
            Config& SetMaxQueueSize(unsigned int v) noexcept { m_maxQueueSize = v; return *this; }
            Config& SetMinMonitorInterval(std::chrono::seconds seconds) noexcept { m_minMonitorInterval = seconds;  return *this; }
            Config& SetIdleCleanupTime(std::chrono::seconds seconds) noexcept { m_idleCleanupTime = seconds; return *this; }
            Config& SetQuota(Quota quota) noexcept { m_quota = quota;  return *this; }
            Config& SetAfterConnectionOpenned(std::function<void(Db const&)> callback) noexcept { m_afterConnectionOpenned = callback;  return *this; }
            Config& SetBeforeConnectionClosed(std::function<void(Db const&)> callback) noexcept { m_beforeConnectionClosed = callback;  return *this; }
            Config& SetUseSharedCache(bool v) noexcept { m_useSharedCache = v;  return *this; }
            Config& SetUseUncommitedRead(bool v) noexcept { m_useUncommitedRead = v;  return *this; }
        };
    struct Limit final
        {
        private:
            int64_t m_count;
            int64_t m_offset;
        public:
            Limit(int64_t count = -1, int64_t offset = -1) : m_count(count), m_offset(offset) {}
            int64_t GetCount() const { return m_count; }
            int64_t GetOffset() const { return m_offset; }
        };
    public:
        struct Impl;
    private:
        Impl* m_pimpl = nullptr;
    public:

        explicit ConcurrentQueryManager(ECDb const& ecdb);
        ECDB_EXPORT ~ConcurrentQueryManager();
        ECDB_EXPORT bool IsInitalized() const;
        ECDB_EXPORT bool Initalize(Config config = Config());
        ECDB_EXPORT PostStatus PostQuery(TaskId& taskId, Utf8CP ecsql, Utf8CP bindings = nullptr, Limit limit = Limit(), Quota quota = Quota(), Priority priority = Priority::Normal, RequestContext requestContext = RequestContext());
        ECDB_EXPORT PollStatus PollQuery(Utf8StringR resultJson, int64_t& rows, TaskId taskId);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
