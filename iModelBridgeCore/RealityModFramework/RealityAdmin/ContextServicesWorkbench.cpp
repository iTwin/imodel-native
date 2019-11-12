/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <RealityAdmin/ContextServicesWorkbench.h>
#include <RealityPlatformTools/RealityConversionTools.h>
#include <RealityPlatform/RealityPlatformAPI.h>
#include <RealityPlatform/SpatioTemporalData.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Francis Boily                   09/2015
//+---------------+---------------+---------------+---------------+---------------+------*/
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((Utf8String*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Francis Boily                   09/2015
//+---------------+---------------+---------------+---------------+---------------+------*/
static size_t WriteData(void *contents, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(contents, size, nmemb, stream);
    return written;
}

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Raphael.Lemieux                   10/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
ContextServicesWorkbench* ContextServicesWorkbench::Create(Utf8StringCR authorizationToken, GeoCoordinationParamsCR params)
    {
    return new ContextServicesWorkbench(authorizationToken, params);
    }

void ContextServicesWorkbench::SetGeoParam(GeoCoordinationParamsCR params)
    {
    m_params = params;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Raphael.Lemieux                   10/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
ContextServicesWorkbench::ContextServicesWorkbench(Utf8StringCR authorizationToken, GeoCoordinationParamsCR params)
    : m_authorizationToken(authorizationToken), m_params(params)
    {
    Init();
    BeFileName caBundlePath = getBaseFolder();
    
    m_certificatePath = caBundlePath.AppendToPath(L"Assets").AppendToPath(L"http").AppendToPath(L"ContextServices.pem");
    if(!m_certificatePath.DoesPathExist())
        m_certificatePath.clear();
    }

void ContextServicesWorkbench::Init()
    {
    m_errorObj = Json::objectValue; 
    m_spatialEntityWithDetailsJson = "";
    m_packageIdBuffer = "";
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Raphael.Lemieux                   10/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
CURLcode ContextServicesWorkbench::performCurl(Utf8StringCR url, Utf8StringCP writeString, FILE* fp, Utf8StringCR postFields)
    {
    BeAssert(nullptr != writeString || nullptr != fp);
    auto curl = curl_easy_init();
    if (nullptr == curl)
        {
        return CURLcode::CURLE_FAILED_INIT;
        }

    //Adjusting headers for the POST method
    struct curl_slist *headers = NULL;
    if (!postFields.empty())
        {
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "charsets: utf-8");
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postFields.length());
        }
    headers = curl_slist_append(headers, m_authorizationToken.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
    curl_easy_setopt(curl, CURLOPT_CAINFO, m_certificatePath.GetNameUtf8().c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_HEADEROPT, CURLHEADER_SEPARATE);
    if (nullptr != fp)
        {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        }
    else
        {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, writeString);
        }
    CURLcode result = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return result;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Raphael.Lemieux                   10/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ContextServicesWorkbench::DownloadSpatialEntityWithDetails(Utf8String filter)
    {
    //First query to get the whole list of elements
    CURLcode result = performCurl(CreateSpatialEntityWithDetailsViewUrl(filter), &m_spatialEntityWithDetailsJson);
    if (CURLE_OK != result)
        {
        if (result == CURLE_RECV_ERROR)
            m_errorObj["Error"] = "InvalidProxyCredentials";
        else
            m_errorObj["Error"] = "ContextServerUnreachable";

        return BentleyStatus::ERROR;
        }

    Json::Value regionItems(Json::objectValue);
    if (!Json::Reader::Parse(m_spatialEntityWithDetailsJson, regionItems) || (!regionItems.isMember("errorMessage") && !regionItems.isMember("instances")))
        {
        m_errorObj["Error"] = "ContextServerInvalidResponse";
        return BentleyStatus::ERROR;
        }

    if (regionItems.isMember("errorMessage"))
        {
        m_errorObj["Error"] = regionItems["errorMessage"].asCString();
        return BentleyStatus::ERROR;
        }

    m_downloadedSEWD = true;
    return BentleyStatus::SUCCESS;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Spencer.Mason                    11/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ContextServicesWorkbench::CreateSpatialEntityWithDetailsViewUrl(Utf8String filter)
    {
    Utf8String tempRealityServerUrl = getBaseUrl();
    
    Utf8String listUrl = tempRealityServerUrl.append("/RealityModeling/SpatialEntityWithDetailsView?polygon={points:[");
    listUrl.append(m_params.GetPolygonAsString(false));
    listUrl.append("],coordinate_system:'4326'}");
    listUrl.append(m_params.GetFilterString());
    listUrl.append(filter);

    return listUrl;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Spencer.Mason                    11/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ContextServicesWorkbench::getBaseUrl()
{
    Utf8String tempRealityServerUrl;
    switch (m_params.GetServerType())
    {
    case CONNECTServerType::DEV:
        tempRealityServerUrl = "https://dev-contextservices-eus.cloudapp.net/v2.4";//"https://naou10726:8443/ws/v2.4";
        break;
    case CONNECTServerType::PROD:
        tempRealityServerUrl = "https://connect-contextservices.bentley.com/v2.4";
        break;
    case CONNECTServerType::PERF:
        tempRealityServerUrl = "https://perf-contextservices-eus.cloudapp.net/v2.4";
        break;
    default:
        tempRealityServerUrl = "https://qa-connect-contextservices.bentley.com/v2.4";
        break;
    }

    Utf8String listUrl = tempRealityServerUrl.append("/Repositories/IndexECPlugin--Server");

    return listUrl;
}

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Raphael.Lemieux                   10/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
void ContextServicesWorkbench::FilterSpatialEntity(ContextServicesWorkbench_FilterFunction pi_func)
    {
    BeAssert(m_downloadedSEWD);

    SpatialEntityDatasetPtr dataset = SpatialEntityDataset::CreateFromJson(m_spatialEntityWithDetailsJson.c_str());
    if (dataset.IsNull())
        return;

    auto imageryIt(dataset->GetImageryGroupR().begin());
    while (imageryIt != dataset->GetImageryGroupR().end())
        {
        double occlusion = (*imageryIt)->GetOcclusion();
        if((occlusion  > 50.0) || pi_func(*imageryIt))
            {
            imageryIt = dataset->GetImageryGroupR().erase(imageryIt);
            }
        else
            {
            imageryIt++;
            }
        }

    m_selectedIds = SpatioTemporalSelector::GetIDsByRes(*dataset, m_params.GetPolygonVector());
    }


Utf8String ContextServicesWorkbench::GetPackageParameters()
    {
    return GetPackageParameters(m_selectedIds[m_selectedResolution]);
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Raphael.Lemieux                   01/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ContextServicesWorkbench::GetPackageParameters(bvector<Utf8String> selectedIds)
    {
    // Create the package url.
    Utf8String listAsPostFields = "{'instance':{'instanceId':null,'className':'PackageRequest','schemaName':'RealityModeling','properties':{'RequestedEntities':[";

    // Append all IDs previously selected.
    for (uint16_t i = 0; i < selectedIds.size(); ++i)
        {
        listAsPostFields.append("{ 'Id':'");
        listAsPostFields.append(selectedIds[i]);
        listAsPostFields.append("'},");
        }

    //if (containOsmClass())
        /*listAsPostFields.append("],'CoordinateSystem':null,'OSM': true,'Polygon':'[");
    else*/
        listAsPostFields.append("],'CoordinateSystem':null,'OSM': false,'Polygon':'[");

    listAsPostFields.append(m_params.GetPolygonAsString(false));
    listAsPostFields.append("]'}}, 'requestOptions':{'CustomOptions':{'Version':'2', 'Requestor':'ContextServicesWorkbench', 'RequestorVersion':'1.0' }}}");

    return listAsPostFields;
    }

Utf8String ContextServicesWorkbench::GetPackageIdUrl()
    {
    Utf8String tempRealityServerUrl = getBaseUrl();
    return tempRealityServerUrl.append("/RealityModeling/PackageRequest");
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Raphael.Lemieux                   01/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ContextServicesWorkbench::DownloadPackageId()
    {
    CURLcode result = performCurl(GetPackageIdUrl(), &m_packageIdBuffer, nullptr, GetPackageParameters());

    if (CURLE_OK != result)
        return BentleyStatus::ERROR;

    Json::Value packageInfos(Json::objectValue);
    Json::Reader::Parse(m_packageIdBuffer, packageInfos);

    if (!packageInfos.isMember("changedInstance"))
        {
        return BentleyStatus::ERROR;
        }

    m_instanceId = packageInfos["changedInstance"]["instanceAfterChange"]["instanceId"].asCString();
    return  BentleyStatus::SUCCESS;
    }

char* ContextServicesWorkbench::GetPackageFileUrl()
    {
    char* fileUrl = new char[1024];
    Utf8String tempRealityServerUrl = getBaseUrl();
    tempRealityServerUrl.append("/RealityModeling/PreparedPackage/");

    strcpy(fileUrl, tempRealityServerUrl.c_str());
    strcat(fileUrl, m_instanceId.c_str());
    strcat(fileUrl, "/$file");

    return fileUrl;
    }

FILE* ContextServicesWorkbench::OpenPackageFile()
    {
    m_downloadedPackage = false;
    //Third query to download the file from the server eg: GUID.xrdp
    BeFileName realityDataTempPath = getBaseFolder();
    if (realityDataTempPath.empty())
        return nullptr;

    m_packageFileName = BeFileName(realityDataTempPath);
    m_packageFileName.AppendToPath(BeFileName(m_instanceId));

    char outfile[1024] = "";
    strcpy(outfile, m_packageFileName.GetNameUtf8().c_str());
    return fopen(outfile, "wb");
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Raphael.Lemieux                   01/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ContextServicesWorkbench::DownloadPackageFile()
    {
    FILE* fp = OpenPackageFile();
    if (!fp)
        return BentleyStatus::ERROR;
    
    CURLcode result = performCurl(GetPackageFileUrl(), nullptr, fp);
    fclose(fp);
    if (CURLE_OK != result)
        return BentleyStatus::ERROR;

    m_downloadedPackage = true;
    return BentleyStatus::SUCCESS;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Raphael.Lemieux                   10/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName ContextServicesWorkbench::getBaseFolder()
{
    WChar exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    WString exeDir = exePath;
    size_t pos = exeDir.find_last_of(L"/\\");
    exeDir = exeDir.substr(0, pos + 1);

    return BeFileName(exeDir);
}

void ContextServicesWorkbench::SelectRandomResolution()
    {
    int randResolution = rand() % 3;
    if(randResolution == 0)
        m_selectedResolution = RealityPlatform::ResolutionCriteria::Low;
    else if(randResolution == 1)
        m_selectedResolution = RealityPlatform::ResolutionCriteria::Medium;
    else if(randResolution == 2)
        m_selectedResolution = RealityPlatform::ResolutionCriteria::High;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Spencer.Mason                   11/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
GeoCoordinationParams::GeoCoordinationParams(bvector<GeoPoint2d> params, CONNECTServerType serverType, Utf8String filterString)
    :m_filterPolygon(params), m_serverType(serverType), m_filterString()
    {}

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Francis Boily                   09/2015
//+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String GeoCoordinationParams::GetPolygonAsString(bool urlEncode) const
    {
    Utf8String polygon;
    bool first = true;
    for (auto& point : GetPolygonVector())
        {
        if (first)
            first = false;
        else
            polygon.append(urlEncode ? "%2C" : ",");
        polygon.append(urlEncode ? "%5B" : "[");
        polygon.append(Utf8PrintfString("%f%s%f", point.longitude, (urlEncode ? "%2C" : ","), point.latitude));
        polygon.append(urlEncode ? "%5D" : "]");
        }
    return polygon;
    }

END_BENTLEY_REALITYPLATFORM_NAMESPACE
