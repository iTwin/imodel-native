/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSourcesImport.cpp $
|    $RCSfile: ScalableMeshSourcesImport.cpp,v $
|   $Revision: 1.19 $
|       $Date: 2011/10/20 18:47:55 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"
#include "ScalableMeshSourcesImport.h"

#include <ScalableMesh/Import/Source.h>
#include <ScalableMesh/Import/Importer.h>

#include <ScalableMesh/Import/ContentDescriptor.h>
#include <ScalableMesh/Import/Attachment.h>

#include <ScalableMesh/Import/DataTypeFamily.h>
#include <ScalableMesh/Import/SourceReferenceVisitor.h>
#include <STMInternal/GeoCoords/WKTUtils.h>

#include "Plugins\ScalableMeshIDTMFileTraits.h"
#include <ScalableMesh/Memory/PacketAccess.h>

#include "../Import/Sink.h"

#include <ScalableMesh/IScalableMeshPolicy.h>
#undef static_assert
#include <DgnPlatform/DgnPlatformLib.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT
     
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct TransmittedDataHeader
    {
    TransmittedDataHeader(bool arePoints, bool is3dData)
        {
        m_arePoints = arePoints;
        m_is3dData = is3dData;
        }

    TransmittedDataHeader()
        {
        m_arePoints = true;
        m_is3dData = false;
        }

    bool m_arePoints;
    bool m_is3dData;
    };

struct  TransmittedPointsHeader : public TransmittedDataHeader
    {
    TransmittedPointsHeader(unsigned int nbOfPoints, bool is3dData)
        : TransmittedDataHeader(true, is3dData)
        {
        m_nbOfPoints = nbOfPoints;
        }

    TransmittedPointsHeader()
        {
        m_nbOfPoints = 0;
        }

    unsigned int m_nbOfPoints;
    };

struct  TransmittedFeatureHeader : public TransmittedDataHeader
    {
    TransmittedFeatureHeader(unsigned int nbOfFeaturePoints, short featureType, bool is3dData)
        : TransmittedDataHeader(false, is3dData)
        {
        m_nbOfFeaturePoints = nbOfFeaturePoints;
        m_featureType = featureType;
        }

    TransmittedFeatureHeader()
        {
        m_nbOfFeaturePoints = 0;
        m_featureType = 0;
        }

    short        m_featureType;
    unsigned int m_nbOfFeaturePoints;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourcesImporter::Impl
    {
    struct SourceItem
        {
        SourceRef                   m_sourceRef;
        ContentConfig               m_contentConfig;
        RefCountedPtr<const ImportConfig>                m_importConfig;
        ImportSequence              m_importSequence;
        SourceImportConfig*         m_sourceImportConf;

        explicit                    SourceItem                         (const SourceRef&                        sourceRef,
                                                                        const ContentConfig&                    contentConfig,
                                                                        const ImportConfig*                     importConfig,
                                                                        const ImportSequence&                   importSequence,
                                                                        SourceImportConfig&                     sourceImportConf)
            :   m_sourceRef(sourceRef),
                m_contentConfig(contentConfig),
                m_importConfig(importConfig),
                m_importSequence(importSequence),
                m_sourceImportConf(&sourceImportConf)
            {
            }

        };

    const LocalFileSourceRef        m_sinkSourceRef;
    SinkPtr                         m_sinkPtr;

    typedef vector<SourceItem>      SourceList;
    SourceList                      m_sources;
    SourceList                      m_sdkSources;

    bool                     m_waitingHeader;
    bool                     m_isReadingPoints;
    TransmittedPointsHeader  m_pointsHeader;
    TransmittedFeatureHeader m_featureHeader;
    bvector<DPoint3d>         m_points;
    size_t                   m_pointBufferPos;

    explicit                        Impl                               (const LocalFileSourceRef&               sinkSourceRef,
                                                                        const SinkPtr&                          sinkPtr)
        :   m_sinkSourceRef(sinkSourceRef),
            m_sinkPtr(sinkPtr)
        {
        }


    Status                          Import                             ();

    Status                          ImportSDKSources                    ();

    void                            ImportFromSDK                       (Utf8CP inputFileName);

    void                            ParseFeatureOrPointBuffer           (unsigned char* buffer, size_t bufferSize);

    Status                          ImportSource                       (SourceItem&                       sourceItem,
                                                                        SourcesImporter&                        attachmentsImporter);

    void                            AddAttachments                     (const Source&                           source,
                                                                        SourceItem&                       sourceItem,
                                                                        SourcesImporter&                        attachmentsImporter);

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcesImporter::SourcesImporter   (const LocalFileSourceRef&   sinkSourceRef,
                                    const SinkPtr&              storagePtr)
    :   m_implP(new Impl(sinkSourceRef, storagePtr))
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcesImporter::~SourcesImporter ()
    {

    }

void SourcesImporter::AddSDKSource(const SourceRef&        sourceRef,
                                   const ContentConfig&    contentConfig,
                                   const ImportConfig*     config,
                                   const ImportSequence&   sequence,
                                   SourceImportConfig&     sourceImportConf)
    {
    ScalableMeshData data = sourceImportConf.GetReplacementSMData();

    if (data.IsRepresenting3dData() == SMis3D::is3D)
        {
        m_implP->m_sdkSources.insert(m_implP->m_sdkSources.begin(), Impl::SourceItem(sourceRef, contentConfig, config, sequence, sourceImportConf));
        }
    else
        {
        m_implP->m_sdkSources.push_back(Impl::SourceItem(sourceRef, contentConfig, config, sequence, sourceImportConf));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourcesImporter::AddSource    (const SourceRef&        sourceRef,
                                    const ContentConfig&    contentConfig,
                                    const ImportConfig*     config,
                                    const ImportSequence&   sequence,
                                    SourceImportConfig&     sourceImportConf)
    {

    ScalableMeshData data = sourceImportConf.GetReplacementSMData();

    if (data.IsRepresenting3dData() == SMis3D::is3D)
        {
        m_implP->m_sources.insert(m_implP->m_sources.begin(), Impl::SourceItem(sourceRef, contentConfig, config, sequence, sourceImportConf));
        }
    else
        {
        m_implP->m_sources.push_back(Impl::SourceItem(sourceRef, contentConfig, config, sequence, sourceImportConf));
        }            
    }

bool SourcesImporter::IsEmpty () const
    {
    return m_implP->m_sources.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcesImporter::Status SourcesImporter::Import () const
    {
    return m_implP->Import();
    }

void SourcesImporter::Impl::ParseFeatureOrPointBuffer(unsigned char* buffer, size_t bufferSize)
    {
    if (m_waitingHeader)
        {
        size_t nbPoints;

        //The first byte should determine if the data are points or features
        if (*((bool*)buffer) == true)
            {
            assert(sizeof(TransmittedPointsHeader) == bufferSize);
            m_isReadingPoints = true;
            memcpy(&m_pointsHeader, buffer, sizeof(TransmittedPointsHeader));
            nbPoints = m_pointsHeader.m_nbOfPoints;
            }
        else
            {
            assert(sizeof(TransmittedFeatureHeader) == bufferSize);
            m_isReadingPoints = false;
            memcpy(&m_featureHeader, buffer, sizeof(TransmittedFeatureHeader));
            nbPoints = m_featureHeader.m_nbOfFeaturePoints;
            }

        m_points.resize(nbPoints);

        m_pointBufferPos = 0;
        m_waitingHeader = false;
        }
    else
        {
        size_t remainingSize = (m_points.size() * sizeof(DPoint3d)) - m_pointBufferPos;
        size_t copySize = min(remainingSize, bufferSize);

        memcpy(((byte*)&m_points[0]) + m_pointBufferPos, buffer, copySize);

        if (remainingSize > bufferSize)
            {
            m_pointBufferPos += copySize;
            }
        else
            {
            assert(remainingSize == bufferSize);

            if (m_isReadingPoints)
                {
                PacketGroup dstPackets(PointTypeCreatorTrait<DPoint3d>::type().Create().GetDimensionOrgCount(), GetMemoryAllocator());
                PODPacketProxy<DPoint3d> packet;
                packet.AssignTo(dstPackets[0]);
                packet.Reserve(m_points.size());
                memcpy(packet.Edit(), &m_points[0], m_points.size()*sizeof(DPoint3d));
                packet.SetSize(m_points.size());
                const BackInserterPtr sinkInserterPtr = m_sinkPtr->CreateBackInserterFor(dstPackets,
                                                                                         0, PointTypeCreatorTrait<DPoint3d>::type().Create(),
                                                                                         GetLog());
                sinkInserterPtr->Write();
                }
            else
                {

                PacketGroup dstPackets(LinearTypeCreatorTrait<DPoint3d>::type().Create().GetDimensionOrgCount(), GetMemoryAllocator());

                PODPacketProxy<IDTMFile::FeatureHeader> headerPacket;
                headerPacket.AssignTo(dstPackets[0]);
                headerPacket.Reserve(1);
                headerPacket.SetSize(1);
                headerPacket.Edit()->type = m_featureHeader.m_featureType;
                headerPacket.Edit()->offset = 0;
                headerPacket.Edit()->size = (uint32_t)m_points.size();

                PODPacketProxy<DPoint3d> packet;
                packet.AssignTo(dstPackets[1]);
                packet.Reserve(m_points.size());
                memcpy(packet.Edit(), &m_points[0], m_points.size()*sizeof(DPoint3d));
                packet.SetSize(m_points.size());
                const BackInserterPtr sinkInserterPtr = m_sinkPtr->CreateBackInserterFor(dstPackets,
                                                                                         0, LinearTypeCreatorTrait<DPoint3d>::type().Create(),
                                                                                         GetLog());
                sinkInserterPtr->Write();
                }
            m_pointBufferPos = 0;
            m_waitingHeader = true;
            }
        }
    }

#define MAX_EXE_WAIT_TIME 60
/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Elenie.Godzaridis   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SourcesImporter::Impl::ImportFromSDK(Utf8CP inputFileName)
    {
#ifndef VANCOUVER_API
    STARTUPINFOA info = { sizeof(info) };

    BeFileName sdkExePath(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    sdkExePath.AppendUtf8("ScalableMeshV8SDK\\");
    BeFileName sdkExe(sdkExePath);
    sdkExe.AppendUtf8("ScalableMeshSDKexe.exe");

    Utf8PrintfString cmdStr("%s import -i=\"%s\" -o=test", sdkExe.GetNameUtf8(), inputFileName);

    PROCESS_INFORMATION processInfo;


    if (!CreateProcessA(NULL, (LPSTR)cmdStr.c_str(),
        NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, (LPSTR)sdkExePath.GetNameUtf8().c_str(), &info, &processInfo))
        {
        DWORD lastError = GetLastError();
        lastError = lastError;

        return;
        }


    double totalWait = 0;
    HANDLE pipe = INVALID_HANDLE_VALUE;

    do
        {
        Sleep(500);
        totalWait += 0.5;
        pipe = CreateFile(
            "\\\\.\\pipe\\test",
            GENERIC_READ, // only need read access
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );
        }
    while (pipe == INVALID_HANDLE_VALUE && totalWait < MAX_EXE_WAIT_TIME);


        size_t bytes = 0;
        unsigned char* buffer = new unsigned char[1024 * 1024];
        bool dataLeftToRead = (pipe != INVALID_HANDLE_VALUE);

        while (dataLeftToRead)
            {
            DWORD numBytesRead = 0;
            dataLeftToRead = (ReadFile(
                pipe,
                buffer, // the data from the pipe will be put here
                1024 * 1024, // number of bytes allocated            
                &numBytesRead, // this will store number of bytes actually read
                (bool)0 // not using overlapped IO
                ) != 0);

            if (numBytesRead == 0)
                {
                dataLeftToRead = false;
                }
            else
                {
                ParseFeatureOrPointBuffer(buffer, numBytesRead);
                }

            bytes += numBytesRead;
            }

        delete[] buffer;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Elenie.Godzaridis   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SourcesImporter::Status SourcesImporter::Impl::ImportSDKSources()
    {
#ifndef VANCOUVER_API
    BeFileName tempDir(T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName());
    BeFileName tempSourcesToImportFile = tempDir;
    tempSourcesToImportFile.AppendUtf8("tempTerrainSourceImport.xml");
    FILE* pOutputFileStream = fopen(tempSourcesToImportFile.GetNameUtf8().c_str(), "w+");
    DRange3d extent = DRange3d::NullRange();
    char tempBuffer[100000];
    int  nbChars;
    char SourceBuffer[100000];
    int  nbCharsSource = 0;
    Utf8String gcsName;
    for (SourceList::iterator sourceIt = m_sdkSources.begin(), sourcesEnd = m_sdkSources.end(); sourceIt != sourcesEnd; ++sourceIt)
        {
        auto& sourceRef = *sourceIt->m_sourceRef.m_basePtr.get();
        const HVEClipShape* shape = sourceIt->m_importConfig->GetClipShape();
        if (shape != nullptr && !shape->IsEmpty())
            {
            for (auto& clip : shape->m_clips)
                {
                if (clip.m_isClipMask) continue;
                DRange3d ext = DRange3d::From(clip.m_pClipShape->GetExtent().GetXMin(), clip.m_pClipShape->GetExtent().GetYMin(), 0, clip.m_pClipShape->GetExtent().GetXMax(), clip.m_pClipShape->GetExtent().GetYMax(), 0);
                extent.Extend(ext);
                }
            }
        if (sourceIt->m_importConfig->HasDefaultTargetGCS())
            {
            GCS::Status wktCreateStatus;
            WString extendedWktStr(sourceIt->m_importConfig->GetDefaultTargetGCS().GetWKT(wktCreateStatus).GetCStr());
            auto bGCS = BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::CreateGCS();
            IDTMFile::WktFlavor fileWktFlavor = GetWKTFlavor(&extendedWktStr, extendedWktStr);

            BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::WktFlavor baseGcsWktFlavor;

            MapWktFlavorEnum(baseGcsWktFlavor, fileWktFlavor);
            bGCS->InitFromWellKnownText(nullptr, nullptr, baseGcsWktFlavor, extendedWktStr.c_str());
            gcsName = Utf8String(bGCS->GetName());
            }
        auto* dgnLevelSourceRef = dynamic_cast<DGNLevelByNameSourceRef*>(&sourceRef);
        if (dgnLevelSourceRef != nullptr)
            {
            string dataType("DTM");
            nbCharsSource += sprintf(SourceBuffer + nbCharsSource, "<source dataType=\"%s\" path=\"%s\" model=\"%s\" level=\"%s\"/>\n", dataType.c_str(), Utf8String(dgnLevelSourceRef->GetDGNPathCStr()).c_str(), Utf8String(dgnLevelSourceRef->GetModelName().c_str()).c_str(), Utf8String(dgnLevelSourceRef->GetLevelName().c_str()).c_str());
            }
        }

    nbChars = sprintf(tempBuffer, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
    size_t NbWrittenChars = fwrite(tempBuffer, 1, nbChars, pOutputFileStream);
    assert(NbWrittenChars == nbChars);

    nbChars = sprintf(tempBuffer, "<list gcsKeyName=\"%s\" ", gcsName.c_str());
    NbWrittenChars = fwrite(tempBuffer, 1, nbChars, pOutputFileStream);
    assert(NbWrittenChars == nbChars);

    nbChars = sprintf(tempBuffer, "maxNbPointsToImport=\"%i\" ", 0);
    NbWrittenChars = fwrite(tempBuffer, 1, nbChars, pOutputFileStream);
    assert(NbWrittenChars == nbChars);

    if (!extent.IsNull())
        {
        nbChars = sprintf(tempBuffer,
                          "xmin=\"%.8f\" xmax=\"%.8f\" ymin=\"%.8f\" ymax=\"%.8f\" ",
                          extent.low.x, extent.high.x, extent.low.y, extent.high.y);

        NbWrittenChars = fwrite(tempBuffer, 1, nbChars, pOutputFileStream);

        assert(NbWrittenChars == nbChars);
        }

    WString geoCoordDir = T_HOST.GetGeoCoordinationAdmin()._GetDataDirectory();
    BeFileName userGeoCoordDir = BeFileName(geoCoordDir.c_str());
    userGeoCoordDir.AppendSeparator();
    userGeoCoordDir.AppendToPath(L"UserLibraries");
    userGeoCoordDir.AppendSeparator();

    nbChars = sprintf(tempBuffer,
                      "systemDtyPath=\"%s\" ",
                      BeFileName(geoCoordDir.c_str()).GetNameUtf8().c_str());

    NbWrittenChars = fwrite(tempBuffer, 1, nbChars, pOutputFileStream);

    assert(NbWrittenChars == nbChars);

    nbChars = sprintf(tempBuffer,
                      "customDtyPath=\"%s\" ",
                      userGeoCoordDir.GetNameUtf8().c_str());

    NbWrittenChars = fwrite(tempBuffer, 1, nbChars, pOutputFileStream);

    assert(NbWrittenChars == nbChars);



    nbChars = sprintf(tempBuffer,
                      "tempPath=\"%s\">\n",
                      tempDir.GetNameUtf8().c_str());

    NbWrittenChars = fwrite(tempBuffer, 1, nbChars, pOutputFileStream);

    assert(NbWrittenChars == nbChars);
    nbChars = sprintf(tempBuffer, "<noDecimation/>");
    NbWrittenChars = fwrite(tempBuffer, 1, nbChars, pOutputFileStream);
    assert(NbWrittenChars == nbChars);

    NbWrittenChars = fwrite(SourceBuffer, 1, nbCharsSource, pOutputFileStream);

    nbChars = sprintf(tempBuffer, "</list>");
    NbWrittenChars = fwrite(tempBuffer, 1, nbChars, pOutputFileStream);
    assert(NbWrittenChars == nbChars);
    fclose(pOutputFileStream);

    ImportFromSDK(tempSourcesToImportFile.GetNameUtf8().c_str());
#else
assert(!"Not available on this platform");
#endif
    return S_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcesImporter::Status SourcesImporter::Impl::Import ()
    {
    SourcesImporter attachmentsImporter(m_sinkSourceRef, m_sinkPtr);

    for (SourceList::iterator sourceIt = m_sources.begin(), sourcesEnd = m_sources.end(); sourceIt != sourcesEnd; ++sourceIt)
        {
        SourcesImporter::Status status = ImportSource(*sourceIt, attachmentsImporter);
        if (S_SUCCESS != status)
            return S_ERROR;
        }
    if (S_SUCCESS != ImportSDKSources())
        return S_ERROR;
    if (attachmentsImporter.IsEmpty())
        return S_SUCCESS;

    Status attachmentImportStatus = attachmentsImporter.Import();
    if (S_SUCCESS != attachmentImportStatus)
        return S_ERROR;

    return S_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourcesImporter::Status SourcesImporter::Impl::ImportSource   (SourceItem&    sourceItem,
                                                               SourcesImporter&     attachmentsImporter)
    {
    static const SourceFactory SOURCE_FACTORY(GetSourceFactory());
    static const ImporterFactory IMPORTER_FACTORY(GetImporterFactory());

    const SourcePtr originalSourcePtr(SOURCE_FACTORY.Create(sourceItem.m_sourceRef));
    if (0 == originalSourcePtr.get())
        return S_ERROR;

    const SourcePtr sourcePtr(Configure(originalSourcePtr, 
                                        sourceItem.m_contentConfig,
                                        GetLog()));
    if (0 == sourcePtr.get())
        return S_ERROR;

    sourcePtr->SetImportConfig(sourceItem.m_sourceImportConf);

    const ImporterPtr importerPtr = IMPORTER_FACTORY.Create(sourcePtr, 
                                                            m_sinkPtr);
    if (0 == importerPtr.get())
        return S_ERROR;

    const Importer::Status importStatus = importerPtr->Import(sourceItem.m_importSequence, 
                                                              *sourceItem.m_importConfig);

    sourceItem.m_sourceImportConf = sourcePtr->GetSourceImportConfig();

    if (importStatus != Importer::S_SUCCESS)
        return S_ERROR;


    AddAttachments(*sourcePtr, sourceItem, attachmentsImporter);

    return S_SUCCESS;
    }

namespace {


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportSequence              CreateAttachmentImportSequence                 (const ImportSequence&           sequence,
                                                                            uint32_t                            parentLayer)
    {
    ImportSequence seq;
    for (auto& command : sequence.GetCommands())
        {
        if (command.IsSourceLayerSet())
            {
            if (parentLayer == command.GetSourceLayer()) seq.push_back(command);
            }
        else seq.push_back(command);
        }
    return seq;
    }
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourcesImporter::Impl::AddAttachments (const Source&       source,
                                            SourceItem&   sourceItem,
                                            SourcesImporter&    attachmentsImporter)
    {

    // NTERAY: This is a bad way to do it. We should either let the importer import attachments or
    // visit the source's importSequence in order to generate the attachment's import sequence.

    const ContentDescriptor& contentDesc = source.GetDescriptor();


    for (ContentDescriptor::const_iterator layerIt = contentDesc.LayersBegin(), layersEnd = contentDesc.LayersEnd(); 
         layerIt != layersEnd; 
         ++layerIt)
        {
        ImportSequence attachmentImportSequence(CreateAttachmentImportSequence(sourceItem.m_importSequence, contentDesc.GetLayerIDFor(layerIt)));

        if (attachmentImportSequence.IsEmpty())
            continue; // Nothing to import

        const AttachmentRecord& attachments = (*layerIt)->GetAttachmentRecord();

        for (AttachmentRecord::const_iterator attachmentIt = attachments.begin(), attachmentsEnd = attachments.end();
             attachmentIt != attachmentsEnd;
             ++attachmentIt)
            {
            using namespace rel_ops;

            // Make sure STM and source are not the same.
            //LocalFileRefVisitor visitor;
            //attachmentIt->GetSourceRef().Accept(visitor);
            LocalFileSourceRef* refP = dynamic_cast<LocalFileSourceRef*>(attachmentIt->GetSourceRef().m_basePtr.get());

            if (0 != refP &&
                *refP == m_sinkSourceRef)
                continue;

            attachmentsImporter.AddSource(attachmentIt->GetSourceRef(), sourceItem.m_contentConfig, sourceItem.m_importConfig.get(), attachmentImportSequence, *sourceItem.m_sourceImportConf);
            }
        }
    }


END_BENTLEY_SCALABLEMESH_NAMESPACE
