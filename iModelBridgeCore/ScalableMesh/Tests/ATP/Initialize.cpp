#include "Initialize.h"

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <wtypes.h>

using namespace std;
#include <DgnPlatform\IAuxCoordSys.h>
#include <TerrainModel\TerrainModel.h>
#include <ScalableMesh\ScalableMeshDefs.h>
#include <ScalableMesh\IScalableMeshMoniker.h>
#include <ScalableMesh\IScalableMeshURL.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnView/DgnViewLib.h>
#include <DgnPlatform/DgnGeoCoord.h>

#include <DgnPlatform/DesktopTools/WindowsKnownLocationsAdmin.h>
#include <ScalableMesh\ScalableMeshAdmin.h>
#include <ScalableMesh\ScalableMeshLib.h>



USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN
USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace BENTLEY_NAMESPACE_NAME::DgnPlatform;
using namespace BENTLEY_NAMESPACE_NAME::GeoCoordinates;

namespace ScalableMeshATPexe
{
struct AppViewManager : ViewManager
    {
    protected:
        virtual DgnDisplay::QvSystemContextP _GetQvSystemContext() override { return nullptr; }
        virtual bool                _DoesHostHaveFocus()        override { return true; }
        virtual IndexedViewSetR     _GetActiveViewSet()         override { return *(IndexedViewSetP)nullptr; }
        virtual int                 _GetDynamicsStopInterval()  override { return 200; }

    public:
        AppViewManager() {}
        ~AppViewManager() {}
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/10
//=======================================================================================
class AppHost : public DgnViewLib::Host
    {

    protected:

        virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new BentleyApi::Dgn::WindowsKnownLocationsAdmin(); }
        virtual DgnPlatformLib::Host::NotificationAdmin&  _SupplyNotificationAdmin() override;
        virtual void                                      _SupplyProductName(Utf8StringR name) override;
        virtual DgnPlatformLib::Host::GeoCoordinationAdmin& _SupplyGeoCoordinationAdmin() override;
        virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() { return BeSQLite::L10N::SqlangFiles(BeFileName()); }
        virtual ViewManager& _SupplyViewManager() override { return *new AppViewManager(); }

    public:
        void Startup();

        void Terminate();
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/10
//=======================================================================================
struct AppNotificationAdmin : DgnPlatformLib::Host::NotificationAdmin
    {
    virtual StatusInt _OutputMessage(BentleyApi::Dgn::NotifyMessageDetails const& msg) override;
    virtual void      _OutputPrompt(Utf8CP) override;
    virtual NotificationManager::MessageBoxValue _OpenMessageBox(NotificationManager::MessageBoxType, Utf8CP, NotificationManager::MessageBoxIconType) override;
    };

void AppHost::Startup()
    {
    //Ensure basegeocoord is initialized.
    _SupplyGeoCoordinationAdmin()._GetServices();
    }

void AppHost::Terminate()
    {
    }

void                                                                   AppHost::_SupplyProductName(Utf8StringR name) { name.assign("DgnView Demo"); }
DgnPlatformLib::Host::NotificationAdmin&         AppHost::_SupplyNotificationAdmin() { return *new AppNotificationAdmin(); }
DgnPlatformLib::Host::GeoCoordinationAdmin&      AppHost::_SupplyGeoCoordinationAdmin()
    {
    BeFileName geocoordinateDataPath(L".\\GeoCoordinateData\\");

    return *DgnGeoCoordinationAdmin::Create(geocoordinateDataPath);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/09
+---------------+---------------+---------------+---------------+---------------+-****/
StatusInt AppNotificationAdmin::_OutputMessage(NotifyMessageDetails const& msg)
    {
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void      AppNotificationAdmin::_OutputPrompt(Utf8CP prompt)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
NotificationManager::MessageBoxValue AppNotificationAdmin::_OpenMessageBox(NotificationManager::MessageBoxType, Utf8CP message, NotificationManager::MessageBoxIconType iconType)
    {
    return NotificationManager::MESSAGEBOX_VALUE_Ok;
    }



struct ExeAdmin : BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshAdmin
    {
    DgnModel* s_activeDgnModelRefP;
    ExeAdmin() :s_activeDgnModelRefP(0) {};

    ~ExeAdmin() {};

    bool _CanImportPODfile() const
        {
        return true;
        }

    void SetActiveModelRef(DgnModel* activeDgnModelRefP)
        {
        s_activeDgnModelRefP = activeDgnModelRefP;
        }

    DgnModel* _GetActiveModelRef() const
        {
        return s_activeDgnModelRefP;
        }

    };

struct ExeHost : BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshLib::Host
    {

    ExeHost()
        {
        BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshLib::Initialize(*this);        
        }

    BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshAdmin& _SupplyScalableMeshAdmin()
        {
        return *new ExeAdmin();
        };
    };

void InitializeATP(DgnPlatformLib::Host& host)
    {
    static AppHost appHost;
    appHost.Startup();

    BeFileName name;
    BeFileNameStatus beStatus = BeFileName::BeGetTempPath(name);
    assert(BeFileNameStatus::Success == beStatus);
    name.AppendToPath(L"temp.dgn");

    static ExeHost smHost;
    }

void CloseATP()
    {
    }
}

