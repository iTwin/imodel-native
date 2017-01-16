/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/Example/RealityDataServiceGlobalExample.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "GeoCoordinationService.h"


USING_NAMESPACE_BENTLEY_REALITYPLATFORM
using namespace std;

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                                 12/2016
//* Simple example function that can be used as a callback from the upload module.
//* Note that using a plain function implies no other information provided can be transported
//* to the callback context. 
//* If access to additional data is needed consider using lambdas or object from a class
//* that satisfy the std::function definition of the callback
//*
//* The upload progess callback is called by the RealityDataServiceUpload module for every file
//* periodically during the upload process. The period of the call depends on the
//* configuration set on the upload process but could be, for example, a call at every 
//* percent of the file uploaded.
//* Take note that a group of files to be uploaded may be groupped together into an archive
//* by the upload process so the feedback should be adapted accordingly. The present callback will
//* be called for every file part of the archive.
//+---------------+---------------+---------------+---------------+---------------+------*/
int UploadProgressCallback(int index, void *pClient, size_t ByteCurrent, size_t ByteTotal)
    {
    // The index designates the file being uploaded

    // ByteCurrent is the number of bytes already uploaded for designated document

    // ByTotal is the total number of bytes in document

    // The ratio of uploaded of document is ByteCurrent/BytesTotal

    // Returning another value than 0 will cancel all uploads
    return 0;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                                 12/2016
//* Simple example function that can be used as a callback from the upload module.
//* Note that using a plain function implies no other information provided can be transported
//* to the callback context. 
//* If access to additional data is needed consider using lambdas or object from a class
//* that satisfy the std::function definition of the callback.
//*
//* The upload status callback is called for every file once the upload process
//* has completed whatever the reason. Note that since the upload process
//* may group files to upload in an archive the UploadStatus callback will be called for
//* every file part of the archive once upload of this archive is completed.
//* The error code indicated the reason for the interruption a value of 0 indicating the
//* upload successfully completed.
//+---------------+---------------+---------------+---------------+---------------+------*/
void UploadStatusCallback(int index, void *pClient, int ErrorCode, const char* pMsg)
    {
    if (0 ==  errorCode)
        {
        // Upload successful. At this time it is possible to perform some process upon the file
        // such as unzipping an archive file or copy the file at a more useful location.
        // It is also possible to simply wait for the whole process to complete for post-operation
        }
    else if (-2 == errorCode)
        {
        // Upload failed but the upload process will retry. Normally nothing other than
        // some feedback to user will be required.
        }
    else if (CURLE_ABORTED_BY_CALLBACK == errorCode)
        {
        // Some callback aborted the upload (cancel)
        }
    else
        {
        // Any other value indicates an error
        }
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                                 12/2016
//* Simple example function that can be used as a callback from the upload module.
//* Note that using a plain function implies no other information provided can be transported
//* to the callback context. 
//* If access to additional data is needed consider using lambdas or object from a class
//* that satisfy the std::function definition of the callback.
//*
//* The heartbeat callback is called periodically more often thant the progress callback
//* to indicate the upload process is not stalled and some uploading has been performed.
//* No information is provided or required.
//* This callback also provides a chance to cancel all upload operations faster than
//* what is offered by the progress callback.
//+---------------+---------------+---------------+---------------+---------------+------*/
int HeartbeatCallBack()
    {
    // The upload process operates
    // Continue

    // Returning any other alue than zero will cancel all uploads
    return 0;
    }


///*---------------------------------------------------------------------------------**//**
//* @bsiclass                                                                 12/2016
//* This class contains examples of access to RealityData Service
//+---------------+---------------+---------------+---------------+---------------+------*/
class RealityDataServiceClientExample
    {
    public:
    RealityDataServiceClientExample()
        {
        }

    virtual ~RealityDataServiceClientExample()
        {
        }


    void RunExampleAccessRealityDataList()
        {

        // Only required to change the target server to development, otherwise default values connect to the official service
        RealityDataService.SetServerComponents("https://dev-realitydataservices.bentley.com/", "2.4", "IndexECPlugin-Server", "S3MX");
     
        // Create a request for reality data list by enterprise
        RealityDataPaged myRequest = RealityDataPagedRequest();
        myRequest.SortBy(RealityDataPagedRequest::Dataset, true); // Sort results by dataset
        myRequest.FilterBySize(0, 2000000); // Only return reality that has a total size between 0 and 2 gygabytes

        bvector<RealityDataPtr> resultPage = RealityDataService.Request(myRequest);

        // Make sure that the page size was followed.
        BeAssert(resultPage.size() <= 25);

        myRequest.SetStartIndex(0);
        myRequest.SetPageSize(123);

        bvector<RealityDataPtr> resultPage2 = RealityDataService.Request(myRequest);

        BeAssert(resultPage2.size() <= 123);


        // create a new request
        myRequest = RealityDataRequest();


        bvector<GeoPoint2D> myProjectAreaInLatLong;
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.240, 48.240));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.241, 48.240));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.241, 48.241));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.240, 48.241));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.240, 48.240));

        myRequest.FilterSpatial(myProjectAreaInLatLong);

        myRequest.SortBy(RealityData::Owner, false); // Sort by owner descending

        bvector<RealityDataPtr> resultPage3 = RealityDataService.Request(myRequest);

        // It is possible to change a request but make sure to reset the paging to start from the start
        myRequest.FilterByOwner("alain.robert@bentley.com");
        myRequest.GotoPage(0);

        // The spatial filtering and sort order previously set still apply.
        bvector<RealityDataPtr> resultPage4 = RealityDataService.Request(myRequest);
        }



    void RunExampleUploadDirectory()
        {

        // Only required to change the target server to development, otherwise default values connect to the official service
        RealityDataService.SetServerComponents("https://dev-realitydataservices.bentley.com/", "2.4", "IndexECPlugin-Server", "S3MX");
     

        RealityDataUploadPtr myUploadEngine = RealityDataService.CreateRealityDataUpload();

        myUploadEngine->SetSourcePath("C:\Rivendell3MX\", "Rivendell.3mx", "RivendellThumb.png");

         // Set call backs ... notice to value provided fot the progress callback indicating
         // the frequency (a a percentage of downloaded file) to call the callback
         myUploadEngine->SetProgressCallBack(DownloadProgressCallback, 0.1);
         myUploadEngine->SetStatusCallBack(DownloadStatusCallback);
         myUploadEngine->SetHeartbeatCallBack(HeartbeatCallBack);
         RealityDataServiceUpload::UploadReportCR uploadReport = myUploadEngine->Perform();



        }

    void RunExampleDownloadRootFileAndThumbnailForEveryInProject()
        {

        // Only required to change the target server to development, otherwise default values connect to the official service
        RealityDataService.SetServerComponents("https://dev-realitydataservices.bentley.com/", "2.4", "IndexECPlugin-Server", "S3MX");
     

        // Create a request for reality data list by enterprise
        RealityDataPaged myRequest = RealityDataPagedRequest();

        myRequest.SetStartIndex(0);
        myRequest.SetPageSize(100);

        bvector<RealityDataPtr> resultPage = RealityDataService.Request(myRequest);

        // Make sure that the page size was followed.
        BeAssert(resultPage.size() <= 25);


        bvector<RealityDataPtr> resultList;


        bvector<GeoPoint2D> myProjectAreaInLatLong;
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.240, 48.240));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.241, 48.240));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.241, 48.241));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.240, 48.241));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.240, 48.240));

        myRequest.FilterSpatial(myProjectAreaInLatLong);

        myRequest.SortBy(RealityData::Owner, false); // Sort by owner descending



        bool ended = false;
        while (!ended)
            {
            bvector<RealityDataPtr> resultPage = RealityDataService.Request(myRequest);
            if (resultPage.size() > 0)
                {
                resultList.push_back(resultPage);
                }
            else
                ended = true;

            BeAssert(resultList.size() < 1000); // We should not get more than 1000 results
            }

        // We access the root document and thumbnail for all files
        for (int index = 0 ; index < resultList.size() ; index++)
            {
            RealityDataServiceDocumentContentRequest myThumbRequest(resultList[index]->GetId(), resultList[index]->GetThumbnailDocument());

            bvector<bytes> thumbnailContent = RealityDataService::Request(myThumbRequest);

            RealityDataServiceDocumentContentRequest myRootDocRequest(resultList[index]->GetId(), resultList[index]->GetRootDocument());

            bvector<bytes> rootDocumentContent = RealityDataService::Request(myRootDocRequest);
            }
        }

    };

    void RunExampleDownloadRootFileAndThumbnailForEveryInProject()
        {

        // Only required to change the target server to development, otherwise default values connect to the official service
        RealityDataService.SetServerComponents("https://dev-realitydataservices.bentley.com/", "2.4", "IndexECPlugin-Server", "S3MX");
     

        // Create a request for reality data list by enterprise
        RealityDataPaged myRequest = RealityDataByIdRequest("0586-358df-445-de34a-dd286");

        RealityDataPtr pRealityData = RealityDataService.Request(myRequest);

        if (pRealityData != NULL)
            {
            // Obtain link to root document
            RealityDataDocumentContentByIdRequest contentRequest(pRealityData->GetId(), pRealityData->GetRootDocument());

            bvector<byte> rootDocumentContent = RealityDataService::Request(contentRequest);

            // To change to another document ... this enables to preserve azure redirection URL and settings
            rootDocumentContent.AddPath("/Data/Scene/RootNode.3mxb");

            bvector<byte> rootNodeContent = RealityDataService::Request(contentRequest);

            // Copy construction also preserves the azure redirection settings
            RealityDataDocumentContentByIdRequest myNewContentRequest(myContentRequest);

            myNewContentRequest.SetId("/Data/Scene/Tile002_023/Tile0x34.3mxb");
            bvector<byte> otherNodeContent = RealityDataService::Request(myNewContentRequest);

            }



        bvector<RealityDataPtr> resultList;


        bvector<GeoPoint2D> myProjectAreaInLatLong;
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.240, 48.240));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.241, 48.240));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.241, 48.241));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.240, 48.241));
        myProjectAreaInLatLong.push_back(GeoPoint2D(71.240, 48.240));

        myRequest.FilterSpatial(myProjectAreaInLatLong);

        myRequest.SortBy(RealityData::Owner, false); // Sort by owner descending



        bool ended = false;
        while (!ended)
            {
            bvector<RealityDataPtr> resultPage = RealityDataService.Request(myRequest);
            if (resultPage.size() > 0)
                {
                resultList.push_back(resultPage);
                }
            else
                ended = true;

            BeAssert(resultList.size() < 1000); // We should not get more than 1000 results
            }

        // We access the root document and thumbnail for all files
        for (int index = 0 ; index < resultList.size() ; index++)
            {
            RealityDataServiceDocumentContentRequest myThumbRequest(resultList[index]->GetId(), resultList[index]->GetThumbnailDocument());

            bvector<bytes> thumbnailContent = RealityDataService::Request(myThumbRequest);

            RealityDataServiceDocumentContentRequest myRootDocRequest(resultList[index]->GetId(), resultList[index]->GetRootDocument());

            bvector<bytes> thumbnailContent = RealityDataService::Request(myRootDocRequest);
            }
        }

    void RunExampleDownloadRootFileAndThumbnailForEveryInProject()
        {

        // Only required to change the target server to development, otherwise default values connect to the official service
        RealityDataService.SetServerComponents("https://dev-realitydataservices.bentley.com/", "2.4", "IndexECPlugin-Server", "S3MX");
     

        // Create a request for reality data list by enterprise
        RealityDataPaged myRequest = RealityDataByIdRequest("0586-358df-445-de34a-dd286");

        RealityDataPtr pRealityData = RealityDataService.Request(myRequest);

        if (pRealityData != NULL)
            {
            // Obtain link to root document
            Utf8String rootDocumentIdentifier = pRealityData->GetRootDocument();

            if (rootDocumentIdentifier.size() != 0)
                {
                // Create a document content request
                RealityDataDocumentContentRequest pDocAccess(rootDocumentIdentifier);
                }
            }
        }
    };

int wmain(int pi_Argc, wchar_t *pi_ppArgv[])
{
    RealityPlatformClient c;
    c.Run();
}
