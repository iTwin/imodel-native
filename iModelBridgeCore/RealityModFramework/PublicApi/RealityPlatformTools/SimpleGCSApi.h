#include <RealityPlatform/RealityDataObjects.h>
#include <RealityPlatformTools/WSGServices.h>
#include <RealityPlatformTools/GeoCoordinationService.h>
#include <RealityPlatformTools/SimpleWSGBase.h>
#include <RealityPlatformTools/RealityDataDownload.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

typedef std::function<bvector<Utf8String>(bvector<GeoPoint2d> footprint, SpatialEntityDatasetPtr dataset)> SE_selectionFunction;

typedef std::function<RealityDataDownload::DownloadReport*(RealityDataDownload::Link_File_wMirrors_wSisters, BeFileName certificatePath, RealityDataDownload_ProxyCallBack proxyCallback)>RPT_DownloadFunction;

struct GCSRequestManager : public WSGRequestManager
    {
    //! Sets all necessary parameters to complete calls
    //! the serverUrl can be provider by the user, if it is known
    //! otherwise (if the ConnectionClient is available) the serverUrl will be determined automatically
    //! serverUrl format example "https://connect-realitydataservices.bentley.com/"
    REALITYDATAPLATFORM_EXPORT static void Setup(Utf8String serverUrl = "", RPT_DownloadFunction downloadCallback = nullptr);

    //! Sets the projectId passed to the server
    //! when querying data from the RDS server, some entries will only be visible to users on the same project
    REALITYDATAPLATFORM_EXPORT static void SetProjectId(Utf8StringCR projectId);

    //! Download files from GCS
    //! @param[in] footprint points representing the polygon of interest. Last point must be the same as the first
    //! @param[in] classes of interest
    //! @param[in] callback function that, when presented with a list files corresponding to the selection criteria
    //!            will select which ones to download
    //! @param[in] path to an existing folder, in which to save downloaded files
    //! @param[in] path to the certificate file, used to authenticate the GCS server
    //! @param[in] call function that will provide proxy log in information, when prompted
    REALITYDATAPLATFORM_EXPORT static void SimplePackageDownload(bvector<GeoPoint2d> footprint, 
        bvector<RealityDataBase::Classification> classes, SE_selectionFunction pi_func, BeFileName path,
        BeFileName certificatePath = BeFileName(), RealityDataDownload_ProxyCallBack proxyCallback = nullptr);

    //! Download files from GCS, if you already have an xrdp file
    //! you do not need to use this function if you have already used SimplePackageDownload
    //! @param[in] path to the xrdp file containing the download list
    //! @param[in] path to a folder that will contain the downloaded files
    //! @param[in] path to the certificate file, used to authenticate the GCS server
    //! @param[in] call function that will provide proxy log in information, when prompted
    REALITYDATAPLATFORM_EXPORT static void SimpleFileDownload(BeFileName xrdpPath, BeFileName downloadPath,
        BeFileName certificatePath = BeFileName(), RealityDataDownload_ProxyCallBack proxyCallback = nullptr);

    //! request a bingkey from the GCS server, for the product specified
    //! expirationDate will contain the time at which the key will need to be renewed
    REALITYDATAPLATFORM_EXPORT static ConnectedResponse SimpleBingKeyRequest(Utf8StringCR productId, Utf8StringR key, Utf8StringR expirationDate);

    //! used if you do not wish to (or cannot) use the default CURL download implementation
    //! the callback will be provided with a list of urls, the certificate path and proxy callback
    REALITYDATAPLATFORM_EXPORT static void SetDownloadFunction(RPT_DownloadFunction downloadCallback);


    //! used to override the default progress messaging system
    REALITYDATAPLATFORM_EXPORT static void SetProgressCallBack(RealityDataDownload_ProgressCallBack callback, float progressStep = 0.01f);

    //! used to override the default status messaging system
    REALITYDATAPLATFORM_EXPORT static void SetStatusCallback(RealityDataDownload_StatusCallBack callback);

private:
    REALITYDATAPLATFORM_EXPORT static int GCS_progress_func(int index, void *pClient, size_t ByteCurrent, size_t ByteTotal);
    REALITYDATAPLATFORM_EXPORT static void GCS_status_func(int index, void *pClient, int ErrorCode, const char* pMsg);
    REALITYDATAPLATFORM_EXPORT static bool AlternateDownload(RealityDataDownload::DownloadReport** report, 
        const RealityDataDownload::Link_File_wMirrors_wSisters& downloadOrder, 
        BeFileName certificatePath, RealityDataDownload_ProxyCallBack proxyCallback);

    REALITYDATAPLATFORM_EXPORT static float GetProgressStep();
    static RPT_DownloadFunction s_downloadFunction;
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE