#include <RealityPlatform/RealityDataObjects.h>
#include <RealityPlatformTools/WSGServices.h>
#include <RealityPlatformTools/GeoCoordinationService.h>
#include <RealityPlatformTools/SimpleWSGBase.h>
#include <RealityPlatformTools/RealityDataDownload.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

typedef std::function<bvector<Utf8String>(bvector<GeoPoint2d> footprint, SpatialEntityDatasetPtr dataset)> SE_selectionFunction;

struct GCSRequestManager : public WSGRequestManager
    {
    REALITYDATAPLATFORM_EXPORT static void Setup();
    REALITYDATAPLATFORM_EXPORT static void SimplePackageDownload(bvector<GeoPoint2d> footprint, 
        bvector<RealityDataBase::Classification> classes, SE_selectionFunction pi_func, BeFileName path,
        BeFileName certificatePath = BeFileName(), RealityDataDownload_ProxyCallBack proxyCallback = nullptr);
private:
    REALITYDATAPLATFORM_EXPORT static int GCS_progress_func(int index, void *pClient, size_t ByteCurrent, size_t ByteTotal);
    REALITYDATAPLATFORM_EXPORT static void GCS_status_func(int index, void *pClient, int ErrorCode, const char* pMsg);
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE