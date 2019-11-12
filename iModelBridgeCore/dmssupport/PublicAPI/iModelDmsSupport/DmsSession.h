/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
        static BeFileName  GetDefaultWorkspacePath(bool isv8i);
        iModelDmsSupport::SessionType m_sessionType;
    public:
    
        DmsSession(Utf8StringCR userName, Utf8StringCR password, iModelDmsSupport::SessionType sessionType);

        static Utf8StringCR GetDataSourceFromMoniker(Utf8StringCR moniker);
        
        bool InitPwLibraries(BeFileNameCR pwBinaryPath);
        //!Return true if the session was succesfully established.
        bool Initialize();

        //!Return true if the session was successfully shutdown.
        bool UnInitialize();

        bool SetDataSource(Utf8StringCR dataSource);

        //!Search path to find dependant application libraries and symbols
        void SetApplicationResourcePath(BeFileNameCR applicationResourcePath);

        BeFileName GetApplicationResourcePath(bool isv8i) const;

        BeFileName GetDefaultConfigPath(bool isv8i) const;

        iModelDmsSupport::SessionType GetSessionType() const;
    };

END_BENTLEY_DGN_NAMESPACE