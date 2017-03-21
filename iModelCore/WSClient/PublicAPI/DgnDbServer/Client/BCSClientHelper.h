/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/BCSClientHelper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/RefCounted.h>
#include <Bentley/Tasks/AsyncError.h>
#include <DgnDbServer/Client/DgnDbClient.h>
#include <DgnDbServer/Client/DgnDbServerError.h>
#include <BeHttp/Credentials.h>
#include <BeHttp/HttpRequest.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE

/*--------------------------------------------------------------------------------------+
* The input information required for a user to sign into the BIM Collaboration Service via Connect.
* @bsiclass                                                     Sam.Wilson      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
struct BCSSignInInfo
    {
    Http::Credentials m_credentials; //!< The user's credentials
    WebServices::UrlProvider::Environment m_environment; //!< The BCS deployment to use

    BCSSignInInfo()
        {
#ifndef NDEBUG
        m_environment = WebServices::UrlProvider::Environment::Qa;
#else
        m_environment = WebServices::UrlProvider::Environment::Release;
#endif
        }
    //! Set m_environment by parsing a string
    BentleyStatus SetEnvironmentFromString(Utf8StringCR str)
        {
        if (str.EqualsI("qa"))
            m_environment = WebServices::UrlProvider::Environment::Qa;
        else if (str.EqualsI("dev"))
            m_environment = WebServices::UrlProvider::Environment::Dev;
        else if (str.EqualsI("release"))
            m_environment = WebServices::UrlProvider::Environment::Release;
        else
            return BSIERROR;
        return BSISUCCESS;
        }
    };

/*--------------------------------------------------------------------------------------+
* Helps the caller with tasks such as signing into the BIM Collaboration Service via Connect
* and looking up a project ID from a project name.
* @bsiclass                                                     Sam.Wilson      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
struct BCSClientHelper
    {
    private:
    IJsonLocalState* m_localState;
    WebServices::ClientInfoPtr m_clientInfo;
    WebServices::ConnectSignInManagerPtr m_signinMgr;

    static Utf8CP str_BentleyConnectGlobal() {return "BentleyCONNECT.Global--CONNECT.GLOBAL";}

    public:
    //! Construct the helper
    BCSClientHelper(WebServices::ClientInfoPtr clientInfo, IJsonLocalState* ls = nullptr) : m_clientInfo(clientInfo), m_localState(ls) {}

    //! Sign in
    //! @param errorOut     Optional. If not null, an explanation of signin failure is returned here.
    //! @param signinInfo       User credentials, etc.
    //! @return a connected client if signin succeeds
    DGNDBSERVERCLIENT_EXPORT DgnDbServer::DgnDbClientPtr SignIn(Tasks::AsyncError* errorOut, BCSSignInInfo const& signinInfo);

    //! Look up a BCS project ID from a BCS project name
    //! @param wserrorOut     Optional. If not null, then details about a query failure are returned here if the failure is due to a communications error or some 
    //! server failure. No details would be returned if the lookup failed simply because the project name was not found.
    //! @param bcsProjectName  The project name to look up
    //! @param wsgBentleyConnectRepository The WSG Bentley Connect repository to query.
    //! @return the project ID if the lookup succeeded or the empty string if not
    DGNDBSERVERCLIENT_EXPORT Utf8String QueryProjectId(WebServices::WSError* wserrorOut, Utf8StringCR bcsProjectName, Utf8CP wsgBentleyConnectRepository = str_BentleyConnectGlobal());
    };

END_BENTLEY_DGNDBSERVER_NAMESPACE
