#include <RealityPlatform/RealityDataObjects.h>
#include <RealityPlatformTools/WSGServices.h>
#include <RealityPlatformTools/GeoCoordinationService.h>
#include <RealityPlatformTools/SimpleWSGBase.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

typedef std::function<bvector<Utf8String>(SpatialEntityDatasetPtr dataset)> SE_selectionFunction;

struct GCSRequestManager
    {
    REALITYDATAPLATFORM_EXPORT static void Setup();
    REALITYDATAPLATFORM_EXPORT void SimplePackageDownload(bvector<GeoPoint2d> footprint, bvector<RealityDataBase::Classification> classes, SE_selectionFunction pi_func, BeFileName path);
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE