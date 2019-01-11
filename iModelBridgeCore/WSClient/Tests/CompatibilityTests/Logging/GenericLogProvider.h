/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/Logging/GenericLogProvider.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

// TODO: better way to reference non-published API?
#include <Logging/bentleylogging.h>

#include <Bentley/bmap.h>
#include <Bentley/BeSharedMutex.h>

#include "GenericLogProviderActivator.h"

USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_LOGGING_PROVIDER

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    01/2017
* Implementation copied from:
* Bentley\LoggingSDK\src\native\interface\bsilogprivate.h
* Bentley\LoggingSDK\src\native\interface\consoleprovider.cpp
+---------------+---------------+---------------+---------------+---------------+------*/
class SeverityMap
    {
    public:
        SeverityMap();

        SeverityMap(SEVERITY defaultSeverity);

        virtual ~SeverityMap(void);

        bool IsSeverityEnabled(WCharCP nameSpace, SEVERITY sev);

        int SetSeverity(WCharCP nameSpace, SEVERITY severity);

        SEVERITY GetDefaultSeverity();

        void SetDefaultSeverity(SEVERITY severity);

    protected:
        typedef bmap<WString, SEVERITY>     NamespaceSeverityMap;

        NamespaceSeverityMap        m_severity;
        SEVERITY                    m_defaultSeverity;
        BeSharedMutex               m_lock;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    01/2017
* ILogProvider implementation is very involved and without code reuse just for redirecting output...
* Implementation copied from:
* Bentley\LoggingSDK\src\native\interface\consoleprovider.cpp
+---------------+---------------+---------------+---------------+---------------+------*/
struct GenericLogProvider : public ILogProvider
    {
    private:
        SeverityMap m_severity;

    private:
        GenericLogWriter m_writer;

    public:
        //! Pass writer for custom output
        GenericLogProvider(GenericLogWriter writer);
        virtual ~GenericLogProvider();

        virtual int Initialize() override;
        virtual int Uninitialize() override;
        virtual int CreateLogger(WCharCP nameSpace, ILogProviderContext** context) override;
        virtual int DestroyLogger(ILogProviderContext* context) override;
        virtual void LogMessage(ILogProviderContext* context, SEVERITY sev, WCharCP msg) override;
        virtual void LogMessage(ILogProviderContext* context, SEVERITY sev, Utf8CP msg) override;
        virtual int SetOption(WCharCP attribName, WCharCP attribValue) override;
        virtual int GetOption(WCharCP attribName, WCharP attribValue, uint32_t valueSize) override;
        virtual int SetSeverity(WCharCP nameSpace, SEVERITY severity) override;
        virtual bool IsSeverityEnabled(ILogProviderContext* context, SEVERITY sev) override;
    };
