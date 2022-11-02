
/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "BeSQLite.h"
#include <unordered_map>

BEGIN_BENTLEY_SQLITE_NAMESPACE
//=======================================================================================
// @bsiclass
//=======================================================================================
struct Profiler final {
    struct Params final {
        private:
            bool m_computeExecutionPlan;
            bool m_overrideProfileDb;
        public:
            Params() : m_computeExecutionPlan(true), m_overrideProfileDb(false) {}
            Params(bool overrideProfileDb, bool computeExecutionPlan): m_computeExecutionPlan(computeExecutionPlan), m_overrideProfileDb(overrideProfileDb) {}
            bool ComputeExecutionPlan() const { return m_computeExecutionPlan; }
            bool OverrideProfileDb() const { return m_overrideProfileDb;}
    };
    struct Scope final: Db::AppData {
        private:
            struct CmpChar { bool operator()(Utf8CP lhs, Utf8CP rhs) const; };
            struct HashChar { size_t operator()(Utf8CP str) const;};
            struct QueryStats {
                int64_t m_count;
                int64_t m_elapsed;
                Utf8String m_plan;
                int m_scans;
            };
    
            DbCR m_db;
            mutable cancel_callback_type m_cancelCb;
            mutable Db m_profileDb;
            mutable int64_t m_scopeId;
            mutable BeFileName m_fileName;
            mutable Utf8String m_lastError;
            Utf8String m_scopeName;
            Utf8String m_scenarioName;
            mutable bool m_running;
            mutable bool m_paused;
            mutable std::chrono::steady_clock::time_point m_timepoint;
            mutable std::chrono::steady_clock::time_point m_pauseTimePoint;
            mutable std::unordered_map<Utf8CP, QueryStats, HashChar, CmpChar> m_profile;
            mutable std::list<Utf8String> m_sqlList;
            Profiler::Params m_param;

            BE_SQLITE_EXPORT DbResult BeginSession() const;
            BE_SQLITE_EXPORT DbResult EndSession() const;
            BE_SQLITE_EXPORT DbResult InitProfileDb() const;
            BE_SQLITE_EXPORT void ComputeExecutionPlan(Utf8CP query, Utf8StringR plan, int& scans) const;
            BE_SQLITE_EXPORT DbResult InstallListener() const;
            BE_SQLITE_EXPORT void RemoveListener() const;
            BE_SQLITE_EXPORT Scope(DbCR db, Utf8CP scopeName, Utf8CP sessionName, Profiler::Params param);
            BE_SQLITE_EXPORT ~Scope();
        public:

            BeFileNameCR GetProfileDbFileName() const { return m_fileName; }
            BE_SQLITE_EXPORT DbResult Stop() const;
            BE_SQLITE_EXPORT DbResult Start() const;
            BE_SQLITE_EXPORT DbResult Pause() const;
            BE_SQLITE_EXPORT DbResult Resume() const;
            bool IsRunning() const { return m_running; }
            bool IsPaused() const { return m_paused; }
            BE_SQLITE_EXPORT int64_t GetElapsedTime() const;
            int64_t GetScopeId() const { return m_scopeId; }
            Utf8StringCR GetLastError() const { return m_lastError; }
            BE_SQLITE_EXPORT static RefCountedPtr<Scope> Create(DbCR db, Utf8CP scopeName, Utf8CP sessionName, Profiler::Params param);
        };
         
    private:
        Profiler (){}
    
    public:
        
        BE_SQLITE_EXPORT static DbResult InitScope(DbCR db, Utf8CP scopeName, Utf8CP sessionName, Profiler::Params param);
        BE_SQLITE_EXPORT static Scope const* GetScope(DbCR db);
};

END_BENTLEY_SQLITE_NAMESPACE
