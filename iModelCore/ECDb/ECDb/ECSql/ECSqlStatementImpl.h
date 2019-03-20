/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStatementImpl.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlPrepareContext.h"

#include <ECDb/ECDb.h>
#include <ECDb/IECSqlBinder.h>
#include <ECDb/IECSqlValue.h>
#include <Logging/bentleylogging.h>
#include <Bentley/BeTimeUtilities.h>
#include "ECSqlPreparedStatement.h"
#include "Exp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! ECSqlStatement::Impl is the private implementation of ECSqlStatement hidden from the public headers
//! (PIMPL idiom)
//! @bsiclass                                                Krischan.Eberle      10/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlStatement::Impl final
    {
    private:
#ifndef NDEBUG        
        struct Diagnostics final
            {
            private:
                static const NativeLogging::SEVERITY LOG_SEVERITY = NativeLogging::LOG_DEBUG;
                NativeLogging::ILogger& m_logger;
                std::unique_ptr<StopWatch> m_timer;
                Utf8CP m_ecsql;

                //not copyable
                Diagnostics(Diagnostics const&) = delete;
                Diagnostics& operator=(Diagnostics const&) = delete;

                void Log();
                bool CanLog() const;
            public:
                Diagnostics(Utf8CP ecsql, NativeLogging::ILogger& logger, bool startTimer);
                ~Diagnostics() { Log(); }
            };
        static NativeLogging::ILogger* s_prepareDiagnosticsLogger;
#endif
        std::unique_ptr<IECSqlPreparedStatement> m_preparedStatement;

        //not copyable
        Impl(Impl const&) = delete;
        Impl& operator=(Impl const&) = delete;

        IECSqlPreparedStatement& CreatePreparedStatement(ECDb const&, Exp const&);

        ECSqlStatus FailIfNotPrepared(Utf8CP errorMessage) const;
        ECSqlStatus FailIfWrongType(ECSqlType expectedType, Utf8CP errorMessage) const;
#ifndef NDEBUG
        static NativeLogging::ILogger& GetPrepareDiagnosticsLogger();
#endif
        mutable Nullable<uint64_t> m_hash64;
        
    public:
        Impl() {}
        ~Impl() {}

        ECSqlStatus Prepare(ECDbCR, Db const* dataSourceECDb, Utf8CP, ECCrudWriteToken const*, bool logOnError);

        bool IsPrepared() const { return m_preparedStatement != nullptr; }

        IECSqlBinder& GetBinder(int parameterIndex) const;
        int GetParameterIndex(Utf8CP parameterName) const;
        ECSqlStatus ClearBindings();

        DbResult Step();
        DbResult Step(ECInstanceKey&);
        ECSqlStatus Reset();

        int GetColumnCount() const;
        IECSqlValue const& GetValue(int columnIndex) const;

        Utf8CP GetECSql() const;
        Utf8CP GetNativeSql() const;
        ECDb const* GetECDb() const;

        void Finalize() { m_preparedStatement = nullptr; m_hash64 = nullptr; BeAssert(!IsPrepared()); }

        // Helpers
        IECSqlPreparedStatement* GetPreparedStatementP() const { return m_preparedStatement.get(); }

        template <class TECSqlPreparedStatement>
        TECSqlPreparedStatement* GetPreparedStatementP() const
            {
            BeAssert(dynamic_cast<TECSqlPreparedStatement*> (GetPreparedStatementP()) != nullptr);
            return static_cast<TECSqlPreparedStatement*> (GetPreparedStatementP());
            }

        uint64_t GetHashCode() const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE