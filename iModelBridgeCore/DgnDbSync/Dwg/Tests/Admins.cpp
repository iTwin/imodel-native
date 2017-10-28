/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/Tests/Admins.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "Tests.h"
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ImporterViewManager : ViewManager
{
protected:
    virtual Display::SystemContext* _GetSystemContext() override {return nullptr;}
    virtual bool _DoesHostHaveFocus() override {return true;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
L10N::SqlangFiles ImporterTestsHost::_SupplySqlangFiles()
    {
    BentleyApi::BeFileName sqlangFile(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    sqlangFile.AppendToPath(L"sqlang/DwgImporterTests_en-US.sqlang.db3");
    BeAssert(sqlangFile.DoesPathExist());

    return L10N::SqlangFiles(sqlangFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewManager& ImporterTestsHost::_SupplyViewManager()
    {
    return *new ImporterViewManager();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewLib::Host::IKnownLocationsAdmin& ImporterTestsHost::_SupplyIKnownLocationsAdmin()
    {
    return *new KnownDesktopLocationsAdmin();
    }
