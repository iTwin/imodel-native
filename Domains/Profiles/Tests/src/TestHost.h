/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley\BeFileName.h>
#include <DgnView\DgnViewLib.h>


/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct ProfilesTestHost : Dgn::DgnViewLib::Host
    {
private:
    ProfilesTestHost();

public:
    static ProfilesTestHost& Instance();
    virtual ~ProfilesTestHost();

    BeFileName GetBaseDbPath() const { return s_baseDbPath; }

protected:
    virtual void _SupplyProductName (BentleyApi::Utf8StringR name) override;
    virtual NotificationAdmin& _SupplyNotificationAdmin() override;
    virtual Dgn::ViewManager& _SupplyViewManager() override;
    virtual BentleyApi::BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override;
    virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override;

private:
    static BeFileName s_baseDbPath;
    };
