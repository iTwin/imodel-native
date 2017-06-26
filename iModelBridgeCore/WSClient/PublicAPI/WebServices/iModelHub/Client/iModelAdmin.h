/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Client/iModelAdmin.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/iModelManager.h>
#include <DgnPlatform/RepositoryManager.h>
#include <DgnPlatform/DgnPlatformLib.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

typedef struct Client* ClientP;

//=======================================================================================
//! 
// @bsiclass                                      Karolis.Dziedzelis             09/2016
//=======================================================================================
struct iModelAdmin : public Dgn::DgnPlatformLib::Host::RepositoryAdmin
{
friend struct Client;
private:
    std::unique_ptr<bmap<Utf8String, iModelManagerPtr>> m_managers;
    ClientP m_client;
    iModelAdmin(ClientP client);
public:
    IMODELHUBCLIENT_EXPORT Dgn::IRepositoryManagerP _GetRepositoryManager(Dgn::DgnDbR db) const override;
};

END_BENTLEY_IMODELHUB_NAMESPACE
