#pragma once
#include <Bentley\BeFileName.h>
#include <DgnView\DgnViewLib.h>


/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Arturas.Mizaras                 11/17
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
