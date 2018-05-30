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
    private:
        Utf8String  m_userName;
        Utf8String  m_password;
        Utf8String  m_dataSource;
        BeFileName  m_applicationResourcePath;
        BeFileName  m_pwBinaryPath;
        void*  m_activeDataSource;
        //!Search path to find dependant application libraries and symbols
        void SetPWBinaryPath(BeFileNameCR pwBinaryPath);
        iModelDmsSupport::SessionType m_sessionType;
    public:
    
        DmsSession(Utf8StringCR userName, Utf8StringCR password, Utf8StringCR dataSource, iModelDmsSupport::SessionType sessionType);

        static Utf8StringCR GetDataSourceFromMoniker(Utf8StringCR moniker);
        //!Return true if the session was succesfully established.
        bool Initialize(BeFileNameCR pwBinaryPath);

        //!Return true if the session was successfully shutdown.
        bool UnInitialize();

        //!Search path to find dependant application libraries and symbols
        void SetApplicationResourcePath(BeFileNameCR applicationResourcePath);

        BeFileNameCR GetApplicationResourcePath() const;

        iModelDmsSupport::SessionType GetSessionType() const;
    };

END_BENTLEY_DGN_NAMESPACE