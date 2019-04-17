/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "GeoCoordinationService.h"


USING_NAMESPACE_BENTLEY_REALITYPLATFORM
using namespace std;




///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                                 12/2016
//* Simple example function that can be used as a callback from the download module.
//* Note that using a plain function implies no other information provided can be transported
//* to the callback context. 
//* If access to additional data is needed consider using lambdas or object from a class
//* that satisfy the std::function definition of the callback
//*
//* The download progess callback is called by the RealityModelDownload module for every file
//* periodically during the download process. The period of the call depends on the
//* configuration set on the download process but could be, for example, a call at every 
//* percent of the file downloaded.
//+---------------+---------------+---------------+---------------+---------------+------*/
int DownloadProgressCallback(int index, void *pClient, size_t ByteCurrent, size_t ByteTotal)
    {
    // The index designates the file being downloaded

    // ByteCurrent is the number of bytes already downloaded for designated document

    // ByTotal is the total number of bytes in document

    // The ratio of downloaded of document is ByteCurrent/BytesTotal

    // Returning another value than 0 will cancel all downloads
    return 0;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                                 12/2016
//* Simple example function that can be used as a callback from the download module.
//* Note that using a plain function implies no other information provided can be transported
//* to the callback context. 
//* If access to additional data is needed consider using lambdas or object from a class
//* that satisfy the std::function definition of the callback.
//*
//* The download status callback is called for every file once the download process
//* has completed whatever the reason.
//* The error code indicated the reason for the interruption a value of 0 indicating the
//* download successfully completed.
//+---------------+---------------+---------------+---------------+---------------+------*/
void DownloadStatusCallback(int index, void *pClient, int ErrorCode, const char* pMsg)
    {
    if (0 ==  errorCode)
        {
        // Download successful. At this time it is possible to perform some process upon the file
        // such as unzipping an archive file or copy the file at a more useful location.
        // It is also possible to simply wait for the whole process to complete for post-operation
        }
    else if (-2 == errorCode)
        {
        // Download failed but the download process will retry. Normally nothing other than
        // some feedback to user will be required.
        }
    else if (CURLE_ABORTED_BY_CALLBACK == errorCode)
        {
        // Some callback aborted the download (cancel)
        }
    else
        {
        // Any other value indicates an error
        }
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                                 12/2016
//* Simple example function that can be used as a callback from the download module.
//* Note that using a plain function implies no other information provided can be transported
//* to the callback context. 
//* If access to additional data is needed consider using lambdas or object from a class
//* that satisfy the std::function definition of the callback.
//*
//* The heartbeat callback is called periodically more often thant the progress callback
//* to indicate the download process is not stalled and some downloading has been performed.
//* No information is provided or required.
//* This callback also provides a chance to cancel all download operations faster than
//* what is offered by the progress callback.
//+---------------+---------------+---------------+---------------+---------------+------*/
int HeartbeatCallBack()
    {
    // The download process operates
    // Continue

    // Returning any other alue than zero will cancel all downloads
    return 0;
    }






///*---------------------------------------------------------------------------------**//**
//* @bsiclass                                                                 12/2016
//* This class contains examples of access to RealityData Service
//+---------------+---------------+---------------+---------------+---------------+------*/
class RealityPlatformClient
    {
    public:
    RealityPlatformClient()
        {
        }

    virtual ~RealityPlatformClient()
        {
        }


    void Run1()
        {

        // Only required to change the target server to development, otherwise default values connect to the official service
        GeoCoordinationService.SetServerComponents("https://dev-contextservices.bentley.com/", "2.4", "IndexECPlugin-Server", "RealityModeling");

        // Define the project extent
        bvector<GeoPoint2D> myProjectAreaInLatLong;
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.240, 48.240));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.241, 48.240));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.241, 48.241));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.240, 48.241));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.240, 48.240));


        // Create a request for spatial entity with details view
        SpatialEntityWithDetailsSpatialRequestPtr myRequest = SpatialEntityWithDetailsSpatialRequest(myProjectAreaInLatLong, RealityDataBase::Classification::IMAGERY | RealityDataBase::Classification::TERRAIN);

        // Run the request ... this request is paged but we are willing to fetch all pages (up to 500 total entries)
        StatusInt status = SUCCESS;
        bvector<SpatialEntityPtr> listOfSpatialEntities;

        while (SUCCESS == status && listOfSpatialEntities.size() < 500)
            listOfSpatialEntities.push_back(GeoCoordinationService::Request(myRequest, status));

        // normally the status must be either SUCCESS or NO_MORE_PAGE
        BeAssert (SUCCESS == status || NO_MORE_PAGE == status);
        status = SUCCESS;

        // Check if any result was obtained
        if (listOfSpatialEntities.size() > 0)
            {
            // We want to filter out automatically the result
            SpatioTemporalSelector::ResolutionMap myDatasetsPerResolution = SpatioTemporalSelector::GetIdsByRes(listOfSpatialEntities, myProjectAreaInLatLong);

            // What we need is the highest resolution available for both imagery and terrain
            if ((myDatasetsPerResolution.find(ResolutionCriteria::High) != myDatasetsPerResolution.end()) && 
                (myDatasetsPerResolution[ResolutionHigh].size() > 0))
                {
                // Request package for high resolution data
                PackagePreparationRequestPtr prepRequest = PackagePreparationRequest::Create(myDatasetsPerResolution[ResolutionCriteria::High]);

                Utf8String packageId = GeoCoordinationService::Request(prepRequest);

                // A package was prepared and is ready for download.
                if (packageId.size() != 0)
                    {
                    PreparedPackageRequestPtr packageRequest = PreparedPackageRequest::Create(packageId);

                    // Obtain the package
                    Utf8String packageText = GeoCoordinationService::Request(packageRequest);

                    if (packageText.size() != 0)
                        {
                        // We have a package
                        RealityDataPackagePtr package = RealityDataPackage::CreateFromString(packageText);

                        if (package != null)
                            {
                            // Now we want to download required data source as listed in the package
                            RealityDataDownloadPtr pDownload = RealityDataDownload::Create(urlList);
                            if (pDownload != NULL)
                                {
                                // Set call backs ... notice to value provided for the progress callback indicating
                                // the frequency (a a percentage of downloaded file) to call the callback
                                pDownload->SetProgressCallBack(DownloadProgressCallback, 0.1f);
                                pDownload->SetStatusCallBack(DownloadStatusCallback);
                                pDownload->SetHeartbeatCallBack(HeartbeatCallBack);
                                RealityDataDownload::DownloadReportCR downloadReport = pDownload->Perform();

                                // The donwload report variable accesses the log of the download process.
                                // This download report underlines what spatial entities data source were downloaded

                                // The final step is optional but helps the GeoCoordination Service be advised of 
                                // data sources that are unavailable. If such source remains unavailable for download
                                // it may be the data has moved to another server or the server was decommissioned.
                                DownloadReportUploadRequestPtr downloadReportUploadRequest = DownloadReportUploadRequest::Create(donwloadReport, packageId);

                                GeoCoordinationService::Request(downloadReportUploadRequest);

                                // Now all files have been downloaded and are available for consumption ...
                                }
                            }
                        }
                    }
                }
            }



    };

    void RunServerSideFiltering()
        {

        // Only required to change the target server to development, otherwise default values connect to the official service
        GeoCoordinationService.SetServerComponents("https://dev-contextservices.bentley.com/", "2.4", "IndexECPlugin-Server", "RealityModeling");

        // Obtaint he project extent
        bvector<GeoPoint2D> myProjectAreaInLatLong;
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.240, 48.240));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.241, 48.240));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.241, 48.241));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.240, 48.241));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.240, 48.240));


        // Create a request for spatial entity with details view
        // Here we only want Imagery
        SpatialEntityWithDetailsSpatialRequestPtr myRequest = SpatialEntityWithDetailsSpatialRequest(myProjectAreaInLatLong, RealityDataBase::Classification::IMAGERY);

        // As additional constraint we only want data originating from USGS
        myRequest.FilterDataProvider("USGS");

        // As additional constraint we only want data from USGS NAIP dataset
        myRequest.FilterDataset("NAIP");

        // As additional constraint we want data that has a resolution of at least 25 centimeter
        myRequest.FilterResolutionBetterThan("0.25");

        // As additional constraint we are more tolerant on the accuracy of the data allowing 1.5 meter accuracy
        myRequest.FilterAccuracyBetterThan("1.5");

        // We do not support all file formats
        myRequest.FilterFileType("tif;jp2");

        // And finaly we do not want huge files (no more than 2 gigabytes/2000000 kilobytes per file)
        myRequest.FilterFileSizeMax(2000000);
       

        // Run the request
        bvector<SpatialEntityPtr> listOfSpatialEntities = GeoCoordinationService::Request(myRequest);

        // Check if any result was obtained
        if (listOfSpatialEntities.size() > 0)
            {
            // We made no server-side filtering but now we are stuck with too many candidates
            // The SpatioTemporal selector is of no use for us.

            // We first want to remove all entities that do not satisfy our acquisition/production date requirements.
            DateTime minDate = DateTime::FromString("2013-09-15");
            DateTime maxDate = DateTime::FromString("2015-11-25");
            }
        }
    };

    void RunClientFiltering1()
        {

        // Only required to change the target server to development, otherwise default values connect to the official service
        GeoCoordinationService.SetServerComponents("https://dev-contextservices.bentley.com/", "2.4", "IndexECPlugin-Server", "RealityModeling");

        // Obtaint he project extent
        bvector<GeoPoint2D> myProjectAreaInLatLong;
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.240, 48.240));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.241, 48.240));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.241, 48.241));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.240, 48.241));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.240, 48.240));


        // Create a request for spatial entity with details view
        // Here we only want Imagery
        SpatialEntityWithDetailsSpatialRequestPtr myRequest = SpatialEntityWithDetailsSpatialRequest(myProjectAreaInLatLong, RealitydataBase::Classification::IMAGERY);

        // Run the request
        bvector<SpatialEntityPtr> listOfSpatialEntities = GeoCoordinationService::Request(myRequest);

        // Check if any result was obtained
        if (listOfSpatialEntities.size() > 0)
            {
            // ... remainder of the process
            }
        }
    };

int wmain(int pi_Argc, wchar_t *pi_ppArgv[])
{
    RealityPlatformClient c;
    c.Run();
}
