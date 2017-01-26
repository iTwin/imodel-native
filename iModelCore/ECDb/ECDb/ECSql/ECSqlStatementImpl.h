/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStatementImpl.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECSqlStatementBase.h"
#include <Bentley/BeTimeUtilities.h>
#include <Logging/bentleylogging.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! ECSqlStatement::Impl is the private implementation of ECSqlStatement hidden from the public headers
//! (PIMPL idiom)
//! @bsiclass                                                Krischan.Eberle      10/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlStatement::Impl : ECSqlStatementBase, NonCopyableClass
    {
private:
    struct Diagnostics : NonCopyableClass
        {
        private:
            static const NativeLogging::SEVERITY LOG_SEVERITY = NativeLogging::LOG_DEBUG;
            NativeLogging::ILogger& m_logger;
            std::unique_ptr<StopWatch> m_timer;
            Utf8CP m_ecsql;

            void Log();
            bool CanLog() const;
        public:
            Diagnostics(Utf8CP ecsql, NativeLogging::ILogger& logger, bool startTimer);
            ~Diagnostics() { Log(); }
        };

    static NativeLogging::ILogger* s_prepareDiagnosticsLogger;

    ECSqlStatus _Prepare (ECSqlPrepareContext&, Utf8CP ecsql) override;
    ECSqlPrepareContext _InitializePrepare (ECDbCR ecdb, ECCrudWriteToken const* writeToken) override { return ECSqlPrepareContext(ecdb, *this, writeToken); }
        
    static NativeLogging::ILogger& GetPrepareDiagnosticsLogger ();

public:
    explicit Impl () : ECSqlStatementBase() {}
    ~Impl() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE