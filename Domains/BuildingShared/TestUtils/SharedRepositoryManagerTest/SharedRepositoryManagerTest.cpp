#include <BeXml/BeXml.h>
#include <Bentley/BeTest.h>
#include <DgnPlatform/UnitTests/DgnDbTestUtils.h>
#include <DgnPlatform/UnitTests/ScopedDgnHost.h>
#include <DgnClientFx/DgnClientUi.h>
#include <DgnView/DgnViewLib.h>
#include <DgnPlatform/FunctionalDomain.h>
#include <DgnPlatform/DgnPlatformAPI.h>
#include "SharedRepositoryManagerTest.h"


USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DGNCLIENTFX
USING_NAMESPACE_BUILDING_SHARED



#define PROJECT_SEED_NAME       L"BuildingTests/BuildingTestSeed.bim"
#define PROJECT_DEFAULT_NAME    L"BuildingTests/Default.bim"
#define EMPTY_DGNDB             L"DgnDb\\empty.bim"

//---------------------------------------------------------------------------------------
// @bsimethod                                   Martynas.Saulius                05/18
//---------------------------------------------------------------------------------------
void SharedRepositoryManagerTest::ClearRevisions(DgnDbR db)
    {
    // Ensure the seed file doesn't contain any changes pending for a revision
    if (!db.IsMasterCopy())
        {
        DgnRevisionPtr rev = db.Revisions().StartCreateRevision();
        if (rev.IsValid())
            {
            db.Revisions().FinishCreateRevision();
            db.SaveChanges();
            }
        }
    }