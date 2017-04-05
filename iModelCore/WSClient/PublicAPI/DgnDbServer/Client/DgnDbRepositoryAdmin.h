/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbServer/Client/DgnDbRepositoryAdmin.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnDbServer/DgnDbServerCommon.h>
#include <DgnDbServer/Client/DgnDbRepositoryManager.h>
#include <DgnPlatform/RepositoryManager.h>
#include <DgnPlatform/DgnPlatformLib.h>

BEGIN_BENTLEY_DGNDBSERVER_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

typedef struct DgnDbClient* DgnDbClientP;

//=======================================================================================
//! 
// @bsiclass                                      Karolis.Dziedzelis             09/2016
//=======================================================================================
struct DgnDbRepositoryAdmin : public DgnPlatformLib::Host::RepositoryAdmin
{
private:
    std::unique_ptr<bmap<Utf8String, DgnDbRepositoryManagerPtr>> m_managers;
    DgnDbClientP m_client;
public:
    DgnDbRepositoryAdmin(DgnDbClientP client);
    DGNDBSERVERCLIENT_EXPORT IRepositoryManagerP _GetRepositoryManager(DgnDbR db) const override;
};

END_BENTLEY_DGNDBSERVER_NAMESPACE
