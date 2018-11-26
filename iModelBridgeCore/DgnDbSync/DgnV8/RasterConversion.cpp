/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/RasterConversion.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

#include <VersionedDgnV8Api/DgnPlatform/DgnFileIO/DgnV8FileIO.h>
#include <VersionedDgnV8Api/DgnPlatform/RasterHandlers.h>
#include <VersionedDgnV8Api/Rastercore/RasterCollection.h>
#include <VersionedDgnV8Api/Rastercore/RasterSource.h>
#include <VersionedDgnV8Api/Rastercore/RasterFile.h>
#include <VersionedDgnV8Api/Rastercore/RasterFileFormatDescriptor.h>
#include <VersionedDgnV8Api/Bentley/IStorage.h>

#include <ImagePP/h/ImageppAPI.h>
#include <Imagepp/all/h/HGF2DDCTransfoModel.h>

#include <Raster/RasterApi.h>
#include <Raster/RasterFileHandler.h>
#include <Raster/WmsHandler.h>
#include <DgnPlatform/image.h>

// Structured Storage open modes
enum
    {
    ROOTSTORE_OPENMODE_READ = (STGM_SHARE_DENY_WRITE | STGM_READ),
    ROOTSTORE_OPENMODE_WRITE = (STGM_SHARE_EXCLUSIVE | STGM_READWRITE),
    ROOTSTORE_OPENMODE_CREATE = (STGM_SHARE_EXCLUSIVE | STGM_READWRITE | STGM_CREATE),

    //  NB: when opening a sub-storage (in any mode), you *must* specify STGM_SHARE_EXCLUSIVE.
    SUBSTORE_OPENMODE_READ = (STGM_SHARE_EXCLUSIVE | STGM_READ),
    SUBSTORE_OPENMODE_WRITE = (STGM_SHARE_EXCLUSIVE | STGM_READWRITE),
    SUBSTORE_OPENMODE_CREATE = (STGM_SHARE_EXCLUSIVE | STGM_READWRITE | STGM_CREATE),

    STREAM_OPENMODE_READ = (STGM_SHARE_EXCLUSIVE | STGM_READ),
    STREAM_OPENMODE_WRITE = (STGM_SHARE_EXCLUSIVE | STGM_READWRITE),
    STREAM_OPENMODE_CREATE = (STGM_SHARE_EXCLUSIVE | STGM_READWRITE | STGM_CREATE)
    };

static WCharCP MODEL_STORAGE_FMT_STRING   = L"#%06x";
static WCharCP EMBEDDEDFILES_STORAGE_NAME = L".Embedded";
static WCharCP EMBEDDEDFILES_DATA_NAME    = L"~Data";

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

//=======================================================================================
//! xwms file reader. Can read 1.1 tru 1.3. 1.0 is not supported.
// @bsiclass                                                    Mathieu.Marchand  9/2016
//=======================================================================================
struct XWmsReader
    {
    Utf8String m_url;
    bvector<double> m_box;      // minx, miny, maxx, maxy
    Utf8String m_version;
    Utf8String m_layers;
    Utf8String m_styles;
    Utf8String m_csType;
    Utf8String m_csLabel;
    Utf8String m_format;
    bool       m_transparent = false;

    bool IsValid() const { return !m_url.empty(); }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  9/2016
    //----------------------------------------------------------------------------------------
    std::unique_ptr<Raster::WmsMap> CreateMap() const
        {
        if (!IsValid())
            return nullptr;

        // Format url to meet WmsMap spec.
        Utf8String serverUrl = m_url;
        if (!serverUrl.StartsWithI("http"))
            serverUrl.insert(0, "http://");

        if (serverUrl.at(serverUrl.size()-1) == '?')
            serverUrl.resize(serverUrl.size() - 1);
        
        std::unique_ptr<Raster::WmsMap> pMap(new Raster::WmsMap(serverUrl.c_str(), DRange2d::From(m_box[0], m_box[1], m_box[2], m_box[3]), m_version.c_str(), m_layers.c_str(), m_csLabel.c_str()));

        pMap->m_styles = m_styles;
        pMap->m_csType = m_csType;
        pMap->m_format = m_format;
        pMap->m_transparent = m_transparent;
        pMap->m_vendorSpecific = "SERVICE=WMS";     // Always added by HRFWMSFile

        return pMap;
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  9/2016
    //----------------------------------------------------------------------------------------
    XWmsReader(Utf8CP filename)
        {
        BeXmlStatus xmlStatus;
        BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, filename);
        if (pXmlDom.IsNull() || BEXML_Success != xmlStatus)
            return;

        if (SUCCESS != Read(*pXmlDom))
            m_url.clear();
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  9/2016
    //----------------------------------------------------------------------------------------
    StatusInt Read(BeXmlDom& xmlDoc)
        {
        BeXmlNodeP pRootNode = xmlDoc.GetRootElement();

        if (nullptr == pRootNode || !pRootNode->IsIName("BentleyWMSFile"))
            return ERROR;

        xmlXPathContextPtr pRootContext = xmlDoc.AcquireXPathContext(pRootNode);

        // Do not really care about the version. We try to extract things where they are supposed to be if we failed then the version is not supported
        //xmlDoc.SelectNodeContent(m_xwmsVersion, "VERSION", pRootContext, BeXmlDom::NODE_BIAS_First);

        // <URL>
        if (BeXmlStatus::BEXML_Success != xmlDoc.SelectNodeContent(m_url, "URL", pRootContext, BeXmlDom::NODE_BIAS_First))
            return ERROR;       

        // <REQUEST>
        BeXmlNodeP pRequestNode = pRootNode->SelectSingleNode("REQUEST");
        if (nullptr == pRequestNode)
            return ERROR;

        xmlXPathContextPtr pRequestContext = xmlDoc.AcquireXPathContext(pRequestNode);

        // <VERSION>
        if (BeXmlStatus::BEXML_Success != xmlDoc.SelectNodeContent(m_version, "VERSION", pRequestContext, BeXmlDom::NODE_BIAS_First))
            return ERROR;

        // <LAYERS>
        if (BeXmlStatus::BEXML_Success != xmlDoc.SelectNodeContent(m_layers, "LAYERS", pRequestContext, BeXmlDom::NODE_BIAS_First))
            return ERROR;

        // <STYLES> Optional, an empty "STYLES=" parameter will be added to the URL.
        xmlDoc.SelectNodeContent(m_styles, "STYLES", pRequestContext, BeXmlDom::NODE_BIAS_First);

        // <CRS> || <SRS>
        m_csType = "CRS";
        if (BeXmlStatus::BEXML_Success != xmlDoc.SelectNodeContent(m_csLabel, "CRS", pRequestContext, BeXmlDom::NODE_BIAS_First))
            {
            m_csType = "SRS";
            if (BeXmlStatus::BEXML_Success != xmlDoc.SelectNodeContent(m_csLabel, "SRS", pRequestContext, BeXmlDom::NODE_BIAS_First))
                return ERROR;
            }

        // <FORMAT> 
        if (BeXmlStatus::BEXML_Success != xmlDoc.SelectNodeContent(m_format, "FORMAT", pRequestContext, BeXmlDom::NODE_BIAS_First))
            return ERROR;

        // <TRANSPARENT> (Optional)
        xmlDoc.SelectNodeContentAsBool(m_transparent, "TRANSPARENT", pRequestContext, BeXmlDom::NODE_BIAS_First);

        // <BBOX> Could be part of <REQUEST> (xwms v1.1) or <MAPEXTENT> (xwms v1.2 and above)
        BeXmlNodeP pBoxNode = pRequestNode->SelectSingleNode("BBOX");
        if (nullptr == pBoxNode &&
            nullptr == (pBoxNode = pRootNode->SelectSingleNode("MAPEXTENT/BBOX")))
            return ERROR;

        if (BEXML_Success != pBoxNode->GetContentDoubleValues(m_box) || m_box.size() != 4)
            return ERROR;

        return SUCCESS;
        }
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static WString CleanupV8iModelFilename(WCharCP iModelFilename, DgnV8Api::ElementId elmID)
    {
    // V8 iModel filename should have the following form "filename.ext.{ElementId}.i.iTiff64"
    WPrintfString toRemove(L".%lld.i.", elmID); // do not use 'iTiff64' to avoid case problems

    WString filename = BeFileName::GetFileNameAndExtension(iModelFilename);

    WString::size_type toRemovePos = filename.find(toRemove.c_str());
    if (WString::npos != toRemovePos)
        filename.erase(toRemovePos, toRemove.length() - 1/*keep last '.'*/);

    return filename;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/00
+---------------+---------------+---------------+---------------+---------------+------*/
static void modelNameFromID (WCharP modelName, size_t maxname, Int32 modelID)
    {
#if defined (ANDROID)
    // Snwprintf does not work on Android!
    char        buffer[512];
    WString     modelNameWString;

    BeStringUtilities::Snprintf (buffer, _countof (buffer), "#%06x", modelID);
    BeStringUtilities::Utf8ToWChar (modelNameWString, buffer);
    BeStringUtilities::Wcsncpy (modelName, maxname, modelNameWString.c_str ());
#else
    BeStringUtilities::Snwprintf (modelName, maxname, MODEL_STORAGE_FMT_STRING, modelID);
#endif
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         9/2016
//-----------------------------------------------------------------------------------------
static BentleyStatus createFileUri(Utf8StringR fileUri, Utf8StringCR fileName)
    {
    BeFileName beFileName(fileName);

    WString fileNameAndExt (beFileName.GetFileNameAndExtension());
    if (WString::IsNullOrEmpty(fileNameAndExt.c_str()))
        {
        // Leave input file name unchanged
        fileUri = fileName;
        }
    else
        {
        Utf8String fileNameAndExtUtf8(fileNameAndExt);
        fileUri = fileNameAndExtUtf8;
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         9/2016
//-----------------------------------------------------------------------------------------
 BentleyStatus resolveFileUri(BeFileNameR fileName, Utf8StringCR fileUri, DgnDbCR db)
    {
    BeFileName dbFileName(db.GetDbFileName());
    BeFileName dbPath(dbFileName.GetDirectoryName());

    // Here, we expect that fileUri is a relative file name (relative to the DgnDb)
    BeFileName relativeName(fileUri.c_str()); 
    dbPath.AppendToPath(relativeName.c_str());

    fileName = dbPath;

    return SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         1/2016
//-----------------------------------------------------------------------------------------
BentleyStatus ExtractEmbeddedRaster(BeFileNameR outFilename, DgnV8EhCR v8eh, BeFileNameCR embedFileName, DgnDbR db, BeFileNameCR rootFileName, SpatialConverterBase& converter)
    {
    // Find embedded file id (the embedded file list can be found in the root model)
    DgnModelRefP rootModelP = converter.GetRootModelP();
    DgnFileP  dgnFile  = rootModelP->GetDgnFileP();
    DgnV8Api::EmbeddedFileList const* embeddedFileList;
    if (NULL == (embeddedFileList = dgnFile->GetEmbeddedFileList()))
        {
        converter.ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::MissingData(), Converter::Issue::EmbeddedRasterError(), nullptr, embedFileName.c_str());
        return ERROR;
        }

    Bentley::EmbeddedFileEntryCP entry;
    if (NULL == (entry = embeddedFileList->FindEntryByFileName(embedFileName.c_str())))
        {
        converter.ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::MissingData(), Converter::Issue::EmbeddedRasterError(), nullptr, embedFileName.c_str());
        return ERROR;
        }

    // Retrieve the embedded raster from the structured storage. The structure of the storage looks like:
    //      Root
    //      |
    //      ---- ".Embedded"
    //              |
    //              ---- "#000001"
    //              |    |
    //              |    ---- "~Data"   (this stream contains the raster's data)
    //              |
    //              ---- "#000002"
    //              |    |
    //              |    ---- "~Data"
    //              |
    //              ...

    // Open root storage
    IStorage *rootStorageP;
    HRESULT hr = BeStgOpenStorage(rootFileName.c_str(), NULL, ROOTSTORE_OPENMODE_READ, 0, 0, &rootStorageP);
    if (UNEXPECTED_CONDITION(FAILED(hr)))
        {
        converter.ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::MissingData(), Converter::Issue::EmbeddedRasterError(), nullptr, embedFileName.c_str());
        return ERROR;
        }

    // Open ".Embedded" storage
    IStorage* embeddedStorageP;
    hr = rootStorageP->OpenStorage (EMBEDDEDFILES_STORAGE_NAME, NULL, SUBSTORE_OPENMODE_READ, NULL, 0, &embeddedStorageP);
    if (UNEXPECTED_CONDITION(FAILED(hr)))
        {
        converter.ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::MissingData(), Converter::Issue::EmbeddedRasterError(), nullptr, embedFileName.c_str());
        return ERROR;
        }

    // Open raster storage (e.g. "#000001", "#000002", ...)
    IStorage* rasterStorageP;
    WChar embedFileStoreName[512];
    modelNameFromID (embedFileStoreName, _countof(embedFileStoreName), entry->GetID());
    hr = embeddedStorageP->OpenStorage (embedFileStoreName, NULL, SUBSTORE_OPENMODE_READ, NULL, 0, &rasterStorageP);
    if (UNEXPECTED_CONDITION(FAILED(hr)))
        {
        converter.ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::MissingData(), Converter::Issue::EmbeddedRasterError(), nullptr, embedFileName.c_str());
        return ERROR;
        }

    // Open raster stream
    IStream* rasterStreamP;
    hr = rasterStorageP->OpenStream (EMBEDDEDFILES_DATA_NAME, NULL, STREAM_OPENMODE_READ, 0, &rasterStreamP);
    if (UNEXPECTED_CONDITION(FAILED(hr)))
        {
        converter.ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::MissingData(), Converter::Issue::EmbeddedRasterError(), nullptr, embedFileName.c_str());
        return ERROR;
        }

    // Create the extracted file at the same place than the dgnDb
    BeFileName dbFileName(db.GetDbFileName());
    outFilename = dbFileName.GetDirectoryName();
    WString justFileName = CleanupV8iModelFilename(embedFileName.c_str(), v8eh.GetElementId());
    outFilename.AppendToPath(justFileName.c_str());

    BeFile file;
    if (file.Create(outFilename.c_str()) != BeFileStatus::Success)
        {
        converter.ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::Unknown(), Converter::Issue::RasterCreationError(), nullptr, outFilename.c_str());
        return ERROR;
        }

    // Read "embedded bytes" and rewrite directly into a new file
    UInt64 bytesRemaining;
    STATSTG stats;
    if (FAILED(rasterStreamP->Stat (&stats, STATFLAG_NONAME)))
        {
        converter.ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::Unknown(), Converter::Issue::EmbeddedRasterError(), nullptr, embedFileName.c_str());
        return ERROR;
        }
    bytesRemaining = stats.cbSize.QuadPart;

    ByteStream buffer(64 * 1024);
    ULONG  bytesRead;
    uint32_t bytesWritten;
    while (bytesRemaining > 0)
        {
        rasterStreamP->Read(buffer.GetDataP(), buffer.GetSize(), &bytesRead);
        if (0 < bytesRead)
            {
            file.Write(&bytesWritten, buffer.GetData(), bytesRead);
            if (bytesWritten != bytesRead)
                {
                converter.ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::Unknown(), Converter::Issue::EmbeddedRasterError(), nullptr, embedFileName.c_str());
                return ERROR;
                }
            }

        if (bytesRead != buffer.GetSize())
            break;
        }

    file.Flush();
    file.Close();

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
static StatusInt ConvertV8Clips(Raster::RasterClip& clip, DgnV8Api::IRasterAttachmentQuery const& rasterQuery, DgnV8EhCR v8eh, DMatrix4dCR sourceToWorld)
    {
    clip.Clear();

    if (!rasterQuery.GetClipState(v8eh))
        return SUCCESS;

    DgnV8Api::RasterClipPropertiesPtr clipPropP = rasterQuery.GetClipProperties(v8eh);
    if (clipPropP.IsNull())
        return SUCCESS;

    // Avoid clone when only an affine is required.
    bool useAffine = sourceToWorld.IsAffine();
    Transform sourceToWorld_Affine;
    if (useAffine)
        {
        sourceToWorld_Affine.InitFrom(sourceToWorld);
        }

    // Boundary
    if (clipPropP->HasBoundary())
        {
        Bentley::CurveVectorPtr pBoundaryV8 = DgnV8Api::ICurvePathQuery::ElementToCurveVector(clipPropP->GetBoundary().GetClipElement());

        CurveVectorPtr pBoundary = Converter::ConvertV8Curve(*pBoundaryV8, nullptr);
        if (pBoundary.IsValid() &&
            (pBoundary->GetBoundaryType() == CurveVector::BOUNDARY_TYPE_Inner) || (pBoundary->GetBoundaryType() == CurveVector::BOUNDARY_TYPE_Outer))
            {
            pBoundary->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Outer);

            if (useAffine)
                pBoundary->TransformInPlace(sourceToWorld_Affine);
            else
                pBoundary = pBoundary->Clone(sourceToWorld);
                
            if (SUCCESS != clip.SetBoundary(pBoundary.get()))
                return ERROR;
            }
        }

    // Masks
    for (auto& clipMaskP : clipPropP->GetMaskCollection())
        {
        Bentley::CurveVectorPtr pCurveV8 = DgnV8Api::ICurvePathQuery::ElementToCurveVector(clipMaskP->GetClipElement());

        CurveVectorPtr pMaskCurve = Converter::ConvertV8Curve(*pCurveV8, nullptr);
        if (pMaskCurve.IsValid() &&
            (pMaskCurve->GetBoundaryType() == CurveVector::BOUNDARY_TYPE_Inner) || (pMaskCurve->GetBoundaryType() == CurveVector::BOUNDARY_TYPE_Outer))
            {
            pMaskCurve->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Inner);
            
            if (useAffine)
                pMaskCurve->TransformInPlace(sourceToWorld_Affine);
            else
                pMaskCurve = pMaskCurve->Clone(sourceToWorld);
                
            if (SUCCESS != clip.AddMask(*pMaskCurve))
                return ERROR;
            }
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
//! Create a RasterFile model into the dgnDb.
// @bsimethod                                                   Eric.Paquet         1/2016
//-----------------------------------------------------------------------------------------
Raster::RasterFileModelPtr CreateRasterFileModel(DgnDbR       db, 
                                                 Utf8StringCR fileName, 
                                                 Utf8StringCR logicalName, 
                                                 Utf8StringCR description, 
                                                 DMatrix4dCP  rasterTransformP)
    {
    // Create the URI that will be used for the RasterFileModel creation
    Utf8String fileUri;
    if (createFileUri(fileUri, fileName) != SUCCESS)
        return nullptr;

    // Create model name
    Utf8String linkName(fileUri);
    if (!Utf8String::IsNullOrEmpty(logicalName.c_str()))
        {
        // Use raster's logical name if it exists, instead of model name
        linkName = logicalName;
        }

    // Create the repository link that the RasterFileModel will use as its modeled element
    DgnCode linkCode = RepositoryLink::CreateUniqueCode(*db.GetRealityDataSourcesModel(), linkName.c_str());
    RepositoryLinkPtr repositoryLink = RepositoryLink::Create(*db.GetRealityDataSourcesModel(), fileUri.c_str(), linkCode.GetValueUtf8CP());
    if (!repositoryLink.IsValid() || !repositoryLink->Insert().IsValid())
        return nullptr;

    return Raster::RasterFileModelHandler::CreateRasterFileModel(Raster::RasterFileModel::CreateParams(db, *repositoryLink, rasterTransformP));
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
Raster::WmsModelPtr CreateWMSModel(DgnDbR db, BeFileNameCR xmwsFile, Utf8StringCR logicalName, Utf8StringCR description)
    {
    XWmsReader reader(xmwsFile.GetNameUtf8().c_str());
    std::unique_ptr<Raster::WmsMap> pMap = reader.CreateMap();
    if (pMap == nullptr)
        return nullptr;
        
    Utf8String linkName;
    if (!Utf8String::IsNullOrEmpty(logicalName.c_str()))
        linkName = logicalName;   
    else
        linkName.Assign(xmwsFile.GetFileNameWithoutExtension().c_str());

    DgnCode linkCode = RepositoryLink::CreateUniqueCode(*db.GetRealityDataSourcesModel(), linkName.c_str());
    RepositoryLinkPtr repositoryLink = RepositoryLink::Create(*db.GetRealityDataSourcesModel(), pMap->m_url.c_str(), linkCode.GetValueUtf8CP(), description.c_str());
    if (!repositoryLink.IsValid() || !repositoryLink->Insert().IsValid())
        return nullptr;

    return Raster::WmsModelHandler::CreateWmsModel(db, *repositoryLink, *pMap);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/14
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyApi::Dgn::DgnGCSPtr fromV8GcsToDgnDbGcs(Bentley::GeoCoordinates::DgnGCS const& v8Gcs, Bentley::DgnModelR v8Model, DgnDbR db, bool primary = true)
    {
    //  Since DgnDb uses the same binary format as DgnV8, we can "convert" it by just removing the type66 element header.
    DgnV8Api::MSElement test66;
    if (v8Gcs.CreateGeoCoordType66(&test66.applicationElm, &v8Model, primary) != BSISUCCESS)
        return nullptr;
    return DgnGCS::FromGeoCoordType66AppData((short*) test66.applicationElm.appData, db);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  8/2016
//----------------------------------------------------------------------------------------
BentleyApi::Dgn::DgnGCSPtr GetEffectiveRasterGcs(DgnV8Api::Raster::DgnRaster const& raster, SpatialConverterBase& converter)
    {
    // Equivalent to Bentley::GeoCoordinates::DgnGCS* pRasterGcsV8 = raster.GetEffectiveDgnGCSP(). Export this method if we require modification to topaz.
    Bentley::GeoCoordinates::DgnGCS const* pRasterGcsV8;
    Bentley::GeoCoordinates::DgnGCSPtr pTempGcs;
    if (raster.GetGCSInheritedFromModelState())
        {
        pTempGcs = Bentley::GeoCoordinates::DgnGCS::FromModel(raster.GetModelRefP(), true);
        pRasterGcsV8 = pTempGcs.get();
        }
    else
        {
        pRasterGcsV8 = raster.GetDgnGCSCP();
        }

    if (nullptr == pRasterGcsV8)
        return nullptr;

    return fromV8GcsToDgnDbGcs(*pRasterGcsV8, *raster.GetModelRefP()->GetDgnModelP(), converter.GetDgnDb());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         9/2016
//-----------------------------------------------------------------------------------------
bool SpatialConverterBase::_RasterMustBeExported(Dgn::ImageFileFormat fileFormat)
    {
    // Following the parameter in config file, export the formats that are not portable on all platforms.
    // If "exportNonPortableFormats" is false, we will not export these formats. This may be a desired
    // behavior is the host application works exclusively on a Windows platform, for instance.
    if (!GetConfig().GetXPathBool("/ImportConfig/Raster/@exportNonPortableFormats", false))
        return false;

    // Some formats are not portable on all platforms. We'll export these files as itiff.
    if  (
        IMAGEFILE_MRSID          == fileFormat ||
        IMAGEFILE_ERMAPPER       == fileFormat ||
        IMAGEFILE_JPEG2000       == fileFormat ||
        IMAGEFILE_AAIG           == fileFormat ||
        IMAGEFILE_AIG            == fileFormat ||
        IMAGEFILE_BSB            == fileFormat ||
        IMAGEFILE_DTED           == fileFormat ||
        IMAGEFILE_ERDASIMG       == fileFormat ||
        IMAGEFILE_NITF           == fileFormat ||
        IMAGEFILE_USGSDEM        == fileFormat ||
        IMAGEFILE_USGSSDTSDEM    == fileFormat
        )
        {
        return true;
        }

    return false;
    }

//-----------------------------------------------------------------------------------------
// Return true if sourceFile is newer than outputFile
// @bsimethod                                                   Eric.Paquet         10/2016
//-----------------------------------------------------------------------------------------
bool SourceFileIsNewer(BeFileNameCR sourceFileName, BeFileNameCR outputFileName)
    {
    if (!outputFileName.DoesPathExist())
        return true;

    time_t outputFileTime = 0;
    BeFileName::GetFileTime (NULL, NULL, &outputFileTime, outputFileName.GetName());

    time_t sourceFileTime = 0;
    BeFileName::GetFileTime (NULL, NULL, &sourceFileTime, sourceFileName.GetName());

    if (sourceFileTime > outputFileTime)
        {
        // Source file is newer
        return true;
        }

    return false;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         9/2016
//-----------------------------------------------------------------------------------------
BentleyStatus ExportRaster(BeFileNameR outputFileName, DgnDbCR db, BeFileNameCR sourceFileName, DgnV8Api::Raster::DgnRaster const& raster)
    {
    // Export the raster at the same location than the Bim file. Export it as a "itiff"
    BeFileName dbFileName(db.GetDbFileName());
    WString fileName = sourceFileName.GetFileNameWithoutExtension();
    outputFileName = dbFileName.GetDirectoryName();
    outputFileName.AppendToPath(fileName.c_str());
    outputFileName.AppendExtension(L"itiff");

    if (outputFileName.DoesPathExist() && !SourceFileIsNewer(sourceFileName, outputFileName))
        {
        // The output file already exists and it is newer (or equal time). No need to copy.
        return SUCCESS;
        }

    FileToFileExportParam exportParam;
    memset(&exportParam, 0, sizeof (exportParam));

    exportParam.colorDscr.colorMode = raster.GetColorMode();
    exportParam.compressionLevel = DgnV8Api::CompressionRatio::COMPRESSIONRATIO_LOW;
    exportParam.compressMode = DgnV8Api::CompressionType::COMPRESSTYPE_JPEG;  
    exportParam.fileType = DgnV8Api::ImageFileFormat::IMAGEFILE_ITIFF;        
    exportParam.sisterFile = DgnV8Api::Raster::RasterSisterFileType::RASTER_SISTERFILE_NONE;    
    exportParam.resample = false;
    exportParam.resamplingMethod    = MDLRASTER_INTERPOLATION_AVERAGE;
    exportParam.tile = TILE_YES;

    // Set compression mode according to source raster's color mode
    if( raster.GetColorMode() == DgnV8Api::ImageColorMode::RGBA || 
        raster.GetColorMode() == DgnV8Api::ImageColorMode::RGB || 
        raster.GetDEMFilters() != NULL)
        {
        exportParam.compressMode = DgnV8Api::CompressionType::COMPRESSTYPE_DEFLATE;    
        }
    else if (raster.GetColorMode() == DgnV8Api::ImageColorMode::Monochrome)
        {
        exportParam.compressMode = DgnV8Api::CompressionType::COMPRESSTYPE_CCITTFAX4;    
        }

    // Note: We use mdlRaster_fileToFileExport instead of reprojecting the raster (possibly using mdlRaster_fileExport), otherwise some cases would be 
    // very difficult to support (a raster with a different gcs than the dgn, a dgn with a reference that contains a reprojected raster, 
    // a dgn with a transformed reference containing a raster, ...). So consider all cases before attempting to reproject the raster when converting it...
    StatusInt statusInt = mdlRaster_fileToFileExport (sourceFileName.c_str(), outputFileName.c_str(), &exportParam);
    return statusInt == SUCCESS ? SUCCESS : ERROR;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         9/2016
//-----------------------------------------------------------------------------------------
BentleyStatus CopyRasterToBimLocation(BeFileNameR outputFileName, DgnDbCR db, BeFileNameCR sourceFileName, DgnV8Api::Raster::DgnRaster const& raster, bool rasterMustBeExported)
    {
    if (rasterMustBeExported)
        {
        return ExportRaster(outputFileName, db, sourceFileName, raster);
        }
    else
        {
        // Copy the raster at the same location than the Bim file
        BeFileName dbFileName(db.GetDbFileName());
        WString fileName = sourceFileName.GetFileNameAndExtension();
        outputFileName = dbFileName.GetDirectoryName();
        outputFileName.AppendToPath(fileName.c_str());

        if (outputFileName.DoesPathExist() && !SourceFileIsNewer(sourceFileName, outputFileName))
            {
            // The output file already exists and it is newer (or equal time). No need to copy.
            return SUCCESS;
            }

        // Don't try to copy file if source == destination
        if ( (0 != BeStringUtilities::Wcsicmp(sourceFileName.c_str(), outputFileName.c_str())) )
            {
            BeFileNameStatus fileStatus = BeFileName::BeCopyFile(sourceFileName, outputFileName);
            if (fileStatus != BeFileNameStatus::Success && fileStatus != BeFileNameStatus::AlreadyExists)
                {
                return ERROR;
                }
            }
        }

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  8/2016
//----------------------------------------------------------------------------------------
static bool ComputeRasterTransformation(DMatrix4dR sourceToBim, DgnV8Api::Raster::DgnRaster const& raster, 
                                        SpatialConverterBase& converter, TransformCR modelV8ToBim)
    {
    // We assumed that we always import DgnModels and that modelV8ToBim contain the unit and any DgnAttachment(reference) transformation. 
    // If the model is a DgnAttachment, 'mdlRaster_getPixelShape' will also add the DgnAttachment transformation so it would be added twice.
    BeAssert(raster.GetModelRefP()->AsDgnAttachmentP() == nullptr);

    DgnGCS* pDgnGcs = converter.GetDgnGcs();
    BentleyApi::Dgn::DgnGCSPtr pRasterGcs = GetEffectiveRasterGcs(raster, converter);
    
    if (pRasterGcs.IsNull() || nullptr == pDgnGcs || pRasterGcs->IsEquivalent(*pDgnGcs))
        {
        sourceToBim = DMatrix4d::From(Transform::FromProduct(modelV8ToBim, (TransformCR)raster.GetTransform()));
        return true;
        }

    // Reprojection is required. Compute approximation at most a DMatrix4d
    DPoint3d sourceCorners[4];
    sourceCorners[0].x = sourceCorners[2].x = 0;
    sourceCorners[1].x = sourceCorners[3].x = raster.GetPhysicalSize().x;
    sourceCorners[0].y = sourceCorners[1].y = 0;
    sourceCorners[2].y = sourceCorners[3].y = raster.GetPhysicalSize().y;
    sourceCorners[0].z = sourceCorners[1].z = sourceCorners[2].z = sourceCorners[3].z = 0;

    double seed[] = {0.10, 0.5, 0.90};
    //double seed[] = {0.0, 1.0}; // 4 corners only.
    size_t seedCount = sizeof(seed) / sizeof(seed[0]);
    std::vector<DPoint3d> tiePoints;

    for (size_t y = 0; y < seedCount; ++y)
        {
        for (size_t x = 0; x < seedCount; ++x)
            {
            DPoint3d pointPixel = DPoint3d::FromInterpolateBilinear(sourceCorners[0], sourceCorners[1], sourceCorners[2], sourceCorners[3], seed[x], seed[y]);

            // mdlRaster expect the center of the pixel.
            Bentley::DPoint2d centerPointPixel = Bentley::DPoint2d::From(pointPixel.x + 0.5, pointPixel.y + 0.5);
            Bentley::DPoint3d pixelShape[4];
            if (SUCCESS == mdlRaster_getPixelShape(pixelShape, &centerPointPixel, converter.GetRootModelP(), &raster))
                {
                tiePoints.push_back(pointPixel);                    // uncorrected
                tiePoints.push_back((DPoint3dCR)pixelShape[0]);     // corrected
                }
            }
        }

    DMatrix4d sourceToWorld;
    if (0 != ImagePP::HGF2DDCTransfoModel::GetAffineTransfoMatrixFromScaleAndTiePts(sourceToWorld.coff, (uint16_t) tiePoints.size() * 3, (double const*) tiePoints.data()))
        return false;

    DMatrix4d worldToSource;
    worldToSource.QrInverseOf(sourceToWorld);

    for (size_t i = 0; i < tiePoints.size(); i += 2)
        {
        DPoint3d sourcePixel;
        worldToSource.MultiplyAndRenormalize(sourcePixel, tiePoints[i + 1]);
        if (fabs(sourcePixel.x - tiePoints[i].x) > 0.4999999 ||
            fabs(sourcePixel.y - tiePoints[i].y) > 0.4999999)
            {
            // Affine transform have an error greater than half a pixel use projective.
            if (0 != ImagePP::HGF2DDCTransfoModel::GetProjectiveTransfoMatrixFromScaleAndTiePts(sourceToWorld.coff, (uint16_t) tiePoints.size() * 3, (double const*) tiePoints.data()))
                return false;

            break;
            }
        }    

    // Add the unit change from Dgn to Bim. note that modelV8ToBim will also include DgnAttachment transformation if any.
    sourceToBim.InitProduct(DMatrix4d::From(modelV8ToBim), sourceToWorld);

    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
bool IsParallelToXYPlane(DgnV8Api::Raster::DgnRaster& raster)
    {
    //directionVectorGet includes reprojection.
    Bentley::DVec3d xDirection;
    Bentley::DVec3d yDirection;
    if (SUCCESS != mdlRaster_directionVectorGet(&xDirection, &yDirection, &raster))
        return ERROR;

    Bentley::DVec3d product = Bentley::DVec3d::FromCrossProduct(xDirection, yDirection);
    return product.IsParallelTo(Bentley::DVec3d::From(0, 0, 1));
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
bool ComputeDepthBias(double& depthBias, DgnV8Api::Raster::DgnRaster& raster)
    {
    depthBias = 0.0;

    if (raster.GetDisplayPriorityPlane() != DgnV8Api::RasterDisplayPriorityPlane::DisplayPriority_BackPlane ||
        !IsParallelToXYPlane(raster))
        return false;

    DgnV8Api::Raster::DgnRasterVector rasterOrderList;
    DgnV8Api::Raster::DgnRasterCollection::QueryRastersOrderedList(rasterOrderList, raster.GetModelRefP(), MRITERATE_Root, 0);

    uint32_t position = 0xFFFFFFFF;

    uint32_t backPlaneCount = 0;
    for (uint32_t i = 0; i < rasterOrderList.size(); ++i)
        {
        if (&raster == rasterOrderList[i])
            {
            position = backPlaneCount;
            ++backPlaneCount;
            continue;
            }

        if (rasterOrderList[i]->GetDisplayPriorityPlane() != DgnV8Api::RasterDisplayPriorityPlane::DisplayPriority_BackPlane)
            break; // everything else will be on design and front plane.  We do not care about them.

        if (IsParallelToXYPlane(*rasterOrderList[i]))
            ++backPlaneCount;
        }

    if (0xFFFFFFFF == position)
        {
        BeAssert(false);
        return false; // no bias
        }

    // Or top most background raster will be at -1M, every other one will be underneath that.
    static double biasOrigin = -DgnUnits::OneMeter();

    // Offset by 10 cm, less than that and we still see z-fighting.
    depthBias = biasOrigin + (backPlaneCount - (position + 1)) * (-DgnUnits::OneCentimeter()*10.0);
    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  2/2016
//----------------------------------------------------------------------------------------
BentleyStatus SpatialConverterBase::_ConvertRasterElement(DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm, bool copyRaster, bool isNewElement)
    {
    DgnModel& targetModel = v8mm.GetDgnModel();
    TransformCR dgnToBim = v8mm.GetTransform();

    DgnElementId existingId;
    IChangeDetector::SearchResults changeInfo;
    if (GetChangeDetector()._IsElementChanged(changeInfo, *this, v8eh, v8mm) && IChangeDetector::ChangeType::Update == changeInfo.m_changeType)
        {
        existingId = changeInfo.GetExistingElementId();
        }

    if (!m_config.GetXPathBool("/ImportConfig/Raster/@importAttachments", false))
        {
        Utf8String filename;
        DgnV8Api::IRasterAttachmentQuery* pRasterQuery = dynamic_cast<DgnV8Api::IRasterAttachmentQuery*>(&v8eh.GetHandler());
        if (nullptr != pRasterQuery)
            filename.Assign(pRasterQuery->GetFilename(v8eh).c_str());

        ReportIssueV(IssueSeverity::Info, IssueCategory::Filtering(), Issue::RasterFile(), nullptr, filename.c_str());
        return SUCCESS; // Raster import is disabled in the configuration file
        }

    // Retrieve the options for the attachment (if any) that references this model
    DgnV8Api::Fd_opts fdOpts;
    auto attachment = v8mm.GetV8Attachment();
    if (attachment != nullptr)
        {
        // If the "displayRasterRefs" option is off, don't import this raster, since it was not visible in the source file
        fdOpts = attachment->GetFDOptsCR();
        if (fdOpts.displayRasterRefs != true)
            return SUCCESS;
        }

    // In case that it wasn't called during model load.
    DgnV8Api::DgnPlatformLib::GetHost().GetRasterAttachmentAdmin()._LoadRasters(*v8eh.GetModelRef());

    DgnV8Api::Raster::DgnRaster* rasterP = nullptr;
    mdlRaster_handleFromElementRefGet(&rasterP, v8eh.GetElementRef(), v8eh.GetModelRef());

    auto const& openParam = rasterP->GetOpenParams();

    RasterFileInfo fileInfo;
    if (nullptr == rasterP || SUCCESS != mdlRaster_fileInfoGet(&fileInfo, rasterP))
        {
        Utf8String filename(fileInfo.szFilename);
        ReportError(Converter::IssueCategory::Unknown(), Converter::Issue::ConvertFailure(), filename.c_str());
        return ERROR;
        }

    // Make sure we actually open the source file
    if (fileInfo.fileStatus != SUCCESS/*RASTERFILE_STATUS_Opened*/
        //&& fileInfo.fileStatus != 0x20 /*RASTERFILE_STATUS_InvalidPassword*/
        //&& fileInfo.fileStatus != 0x40 /*RASTERFILE_STATUS_InvalidAccessMode*/ 
        )
        {
        // Secure WMS failed to open because we need a user/password but we still want to convert them.
        if (openParam.GetFilename().EndsWithI(L".xwms"))
            {
            auto const& moniker = openParam.GetAttachMoniker();
            auto resolvedName(moniker.ResolveFileName());
            if (!resolvedName.empty())
                {
                BeStringUtilities::Wcsncpy(fileInfo.szFilename, MAXFILELENGTH, resolvedName.c_str());
                BeStringUtilities::Wcsncpy(fileInfo.szFileSpec, MAXFILELENGTH, resolvedName.c_str());
                fileInfo.fileFormat = DgnV8Api::ImageFileFormat::IMAGEFILE_XWMS;
                fileInfo.fileStatus = SUCCESS;
                }
            }
        }

    if (fileInfo.fileStatus != SUCCESS)
        {
        Utf8String filename(openParam.GetFilename().c_str());
        ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::MissingData(), Converter::Issue::RasterFile(), nullptr, filename.c_str());
        return ERROR;
        }

    // Write progress information to output window
    SetTaskName(Converter::ProgressMessage::TASK_CONVERTING_RASTER(), Utf8String(fileInfo.szFilename).c_str());

    DgnV8Api::IRasterAttachmentQuery* pRasterQuery = dynamic_cast<DgnV8Api::IRasterAttachmentQuery*>(&v8eh.GetHandler());
    if (nullptr == pRasterQuery)
        return ERROR;

    Utf8String logicalName(rasterP->GetLogicalName().c_str());
    Utf8String description(rasterP->GetDescription().c_str());

    BeFileName v8LocalFilename(fileInfo.szFileSpec);
    BeFileName localFilename = v8LocalFilename;

    // Extract i-model embed files.
    if (BeStringUtilities::Wcsicmp(fileInfo.szSchemeType, L"embed") == 0)
        {
        if (SUCCESS != ExtractEmbeddedRaster(localFilename, v8eh, BeFileName(fileInfo.szFilename), GetDgnDb(), GetRootFileName(), *this))
            {
            ReportError(Converter::IssueCategory::Unknown(), Converter::Issue::ConvertFailure(), v8LocalFilename.GetNameUtf8().c_str());
            return ERROR;
            }
        copyRaster = false; //Don't do it if the raster is embedded because it was already extracted there.
        }
    else if (BeStringUtilities::Wcsicmp(fileInfo.szSchemeType, L"file") != 0)
        {
        // "memory:" Was for georaster(oracle). DC only feature that was deprecated in Connect Edition.

        // WIP_RASTER_CONVERTER todo http, ecwp, bing map and what else?
        ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::Unsupported(), Converter::Issue::RasterFile(), nullptr, v8LocalFilename.GetNameUtf8().c_str());
        return ERROR;
        }

    Raster::RasterModelPtr pModel;

    // -- WMS Model.
    if (DgnV8Api::ImageFileFormat::IMAGEFILE_XWMS == fileInfo.fileFormat)
        {
        Raster::WmsModelPtr pWmsModel = CreateWMSModel(GetDgnDb(), localFilename, logicalName, description);
        if (pWmsModel != nullptr)
            {
            // -- Set a depth bias for raster in the background plane
            double depthBias = 0.0;
            if (ComputeDepthBias(depthBias, *rasterP))
                pWmsModel->SetDepthBias(depthBias);
            pModel = pWmsModel.get();
            }
        }
    // -- Raster file model
    else
        {
        // -- Copy raster to the Bim's location. 
        if (copyRaster)
            {
            // Determine if raster must be exported (because it is a non-portable format)
            RasterFileInfo fileInfo;
            StatusInt statusInfo = mdlRaster_fileInfoGet(&fileInfo, rasterP);
            BeAssert(statusInfo == SUCCESS);
            bool mustBeExported = _RasterMustBeExported((Dgn::ImageFileFormat) fileInfo.fileFormat);

            if (SUCCESS != CopyRasterToBimLocation(localFilename, GetDgnDb(), v8LocalFilename, *rasterP, mustBeExported))
                {
                ReportError(Converter::IssueCategory::Unknown(), Converter::Issue::RasterCreationError(), localFilename.GetNameUtf8().c_str());
                return ERROR;
                }
            }

        // -- Compute raster transformation
        DMatrix4d sourceToBim;
        if (!ComputeRasterTransformation(sourceToBim, *rasterP, *this, dgnToBim))
            {
            ReportError(Converter::IssueCategory::Unknown(), Converter::Issue::ConvertFailure(), v8LocalFilename.GetNameUtf8().c_str());
            return ERROR;
            }

        // Compute depth bias and if required remove translation in Z.
        double depthBias = 0.0;
        bool needBias = false;
        if (needBias = ComputeDepthBias(depthBias, *rasterP))
            {
            // We remove Z translation for rasters in the background plane so that all rasters are on the same plane. Theirs Z value did not 
            // affect the depth at which they are displayed only the display order matters in v8. For each raster will compute a depth bias 
            // according to the display order.
            sourceToBim.coff[2][3] = 0.0;
            }

        Raster::RasterFileModelPtr pFileModel;
        if (!existingId.IsValid())
            {
            // -- Raster file model creation.
            pFileModel = CreateRasterFileModel(GetDgnDb(), localFilename.GetNameUtf8(), logicalName, description, &sourceToBim);
            if (pFileModel.IsNull())
                {
                ReportError(Converter::IssueCategory::Unknown(), Converter::Issue::ConvertFailure(), v8LocalFilename.GetNameUtf8().c_str());
                return ERROR;
                }
            }
        else
            {
            pFileModel = GetDgnDb().Models().Get<Raster::RasterFileModel>(DgnModelId(existingId.GetValue()));
            pFileModel->SetSourceToWorld(sourceToBim);
            }
        //-- Convert clipping
        Raster::RasterClip clip;
        if (SUCCESS != ConvertV8Clips(clip, *pRasterQuery, v8eh, sourceToBim))
            {
            // ignore clipping error and go on.
            }
        pFileModel->SetClip(clip);

        // -- Set a depth bias for raster in the background plane
        if (needBias)
            pFileModel->SetDepthBias(depthBias);

        pModel = pFileModel.get();
        }

    SyncInfo::ElementProvenance prov = SyncInfo::ElementProvenance(v8eh, GetSyncInfo(), GetCurrentIdPolicy());
    SyncInfo::V8ElementMapping mapping = SyncInfo::V8ElementMapping(pModel->GetModeledElementId(), v8eh, v8mm.GetV8ModelSyncInfoId(), prov);
    if (!existingId.IsValid())
        {
        // -- Insert model
        DgnDbStatus insertStatus = DgnDbStatus::Success;
        if (DgnDbStatus::Success != (insertStatus = pModel->Insert()))
            {
            BeAssert((DgnDbStatus::LockNotHeld != insertStatus) && "Failed to get or retain necessary locks");
            ReportError(Converter::IssueCategory::Unknown(), Converter::Issue::ConvertFailure(), v8LocalFilename.GetNameUtf8().c_str());
            return ERROR;
            }
        m_syncInfo.InsertElement(mapping);
        _GetChangeDetector()._OnElementSeen(*this, pModel->GetModeledElementId());
        }
    else
        {
        DgnDbStatus updateStatus;
        if (DgnDbStatus::Success != (updateStatus = pModel->Update()))
            {
            BeAssert((DgnDbStatus::LockNotHeld != updateStatus) && "Failed to get or retain necessary locks");
            ReportError(Converter::IssueCategory::Unknown(), Converter::Issue::ConvertFailure(), v8LocalFilename.GetNameUtf8().c_str());
            return ERROR;
            }
        m_syncInfo.UpdateElement(mapping);
        _OnElementConverted(mapping.GetElementId(), &v8eh, Converter::ChangeOperation::Update);

        }
    // -- Enable display (or not) in views.    
    DgnCategoryId category = GetSyncInfo().GetCategory(v8eh, v8mm);
    for (auto const& entry : ViewDefinition::MakeIterator(GetDgnDb()))
        {
        auto viewNumberItr = m_viewNumberMap.find(entry.GetId());
        if (viewNumberItr == m_viewNumberMap.end() || !rasterP->GetViewState(viewNumberItr->second))
            continue;

        auto viewController = ViewDefinition::LoadViewController(entry.GetId(), GetDgnDb());
        if (!viewController.IsValid() || !viewController->IsSpatialView() || !viewController->GetViewDefinitionR().GetCategorySelector().IsCategoryViewed(category))
            continue;

        auto& viewdef = viewController->ToSpatialViewP()->GetSpatialViewDefinition();
        auto& models = viewdef.GetModelSelector();
        models.AddModel(pModel->GetModelId());
        models.Update();
        }
    // Schedule reality model tileset creation.
    AddModelRequiringRealityTiles(pModel->GetModelId(), v8LocalFilename.GetNameUtf8(), Converter::GetV8FileSyncInfoIdFromAppData(*v8eh.GetDgnFileP()));

    return SUCCESS;
    }

END_DGNDBSYNC_DGNV8_NAMESPACE









