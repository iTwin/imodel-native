/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/iModelDmsSupport/DmsSession.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <iModelDmsSupport/iModelDmsSupport.h>
#include <Bentley/BeFileName.h>
BEGIN_BENTLEY_DGN_NAMESPACE

struct DmsSession
    {
    enum SessionType
        {
            PWDI = 0,
            PWShare = 1
        } ;

    private:
        Utf8String  m_userName;
        Utf8String  m_password;
        Utf8String  m_dataSource;
        BeFileName  m_applicationResourcePath;
        BeFileName  m_pwBinaryPath;
        void*  m_activeDataSource;
        //!Search path to find dependant application libraries and symbols
        void SetPWBinaryPath(BeFileNameCR pwBinaryPath);
        SessionType m_sessionType;
    public:
    
        IMODEL_DMSSUPPORT_EXPORT DmsSession(Utf8StringCR userName, Utf8StringCR password, Utf8StringCR dataSource, SessionType sessionType);

        //!Return true if the session was succesfully established.
        IMODEL_DMSSUPPORT_EXPORT bool Initialize(BeFileNameCR pwBinaryPath);

        //!Return true if the session was successfully shutdown.
        IMODEL_DMSSUPPORT_EXPORT bool UnInitialize();

        //!Search path to find dependant application libraries and symbols
        IMODEL_DMSSUPPORT_EXPORT void SetApplicationResourcePath(BeFileNameCR applicationResourcePath);

        IMODEL_DMSSUPPORT_EXPORT BeFileNameCR GetApplicationResourcePath() const;

        IMODEL_DMSSUPPORT_EXPORT SessionType GetSessionType() const;
    };

END_BENTLEY_DGN_NAMESPACE