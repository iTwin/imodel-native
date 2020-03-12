/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <iModelDmsSupport/iModelDmsSupport.h>
#include <Bentley/BeFileName.h>
#include <Bentley/bmap.h>
#include <BeSQLite/BeSQLite.h>
BEGIN_BENTLEY_DGN_NAMESPACE

struct DmsSession
    {
    private:
        BeFileName  m_applicationResourcePath;
        BeFileName  m_pwBinaryPath;

        void*  m_activeDataSource;
        //!Search path to find dependant application libraries and symbols
        void SetPWBinaryPath(BeFileNameCR pwBinaryPath);
        static BeFileName  GetDefaultWorkspacePath(bool isv8i);
        iModelDmsSupport::SessionType m_sessionType;
    protected:
        Utf8String  m_dataSource;
        Utf8String  m_dnsServerUrl;
        Utf8String  m_dnsServerName;
    public:
    
        DmsSession(iModelDmsSupport::SessionType sessionType, Utf8StringCR dnsServerUrl);
        virtual ~DmsSession();

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

        virtual bool Login() = 0;

        void AddDNServer();

        void DeleteDNServer();

        bool RunCommandForDNSServer(bool addDNSServer);

        bool IsDnsServerPresentInRegistry();
    };

struct UserCredentialsSession : DmsSession
    {
    private:
        Utf8String  m_userName;
        Utf8String  m_password;
    public:
        UserCredentialsSession(Utf8String userName, Utf8String password, iModelDmsSupport::SessionType sessionType, Utf8StringCR dnsServerUrl);
        ~UserCredentialsSession() override;
        bool Login() override;
    };

struct SamlTokenSession : DmsSession
    {
    private:
        Utf8String m_accessToken;
        unsigned long m_productId;
    public:
        SamlTokenSession(Utf8String accessToken, unsigned long productId, iModelDmsSupport::SessionType sessionType, Utf8StringCR dnsServerUrl);
        ~SamlTokenSession() override;
        bool Login() override;
    };

END_BENTLEY_DGN_NAMESPACE