#include "SMStreamedSourceStore.h"
#include "..\RasterUtilities.h"
#include <ScalableMesh/GeoCoords/GCS.h>
#include <STMInternal/GeoCoords/WKTUtils.h>
#include <ScalableMesh/GeoCoords/Reprojection.h>
#include <ImagePP/all/h/HRAClearOptions.h>
#include <ImagePP/all/h/HRACopyFromOptions.h>
#include <ImagePP/all/h/HRPPixelTypeV24B8G8R8.h>
#include "..\ScalableMeshSources.h"
#include "ISMDataStore.h"
#include "SMSQLiteSisterFile.h"
#include "SMStoreUtils.h"

#include "..\ScalableMeshSourcesPersistance.h"
#include <ScalableMesh/IScalableMeshSourceImportConfig.h>
#include <ScalableMesh/IScalableMeshDocumentEnv.h>
#include <ScalableMesh/IScalableMeshSourceVisitor.h>

#include <ScalableMesh/Import/SourceReference.h>

#include <ScalableMesh/Import/Source.h>

#include <ScalableMesh\IScalableMeshPolicy.h>

template <class DATATYPE, class EXTENT>  SMStreamedSourceStore<DATATYPE,EXTENT>::SMStreamedSourceStore(SMStoreDataType dataType, SMIndexNodeHeader<EXTENT>* nodeHeader, SMSQLiteFilePtr& smSQLiteFile, DRange3d totalExt)
    {
    m_nodeHeader = nodeHeader;
    m_dataType = dataType;
    m_smSQLiteFile = smSQLiteFile;

    if (!smSQLiteFile->HasSources())
        {
        assert(false && "Trying to use a streamed source but no source found!");
        return;
        }

   
    SourcesDataSQLite* sourcesData = new SourcesDataSQLite();
    smSQLiteFile->LoadSources(*sourcesData);

    WString wktStr;
    smSQLiteFile->GetWkt(wktStr);

    GeoCoordinates::BaseGCSCPtr cs;
    if (!wktStr.empty())
        {
        ISMStore::WktFlavor fileWktFlavor = GetWKTFlavor(&wktStr, wktStr);

        BaseGCS::WktFlavor wktFlavor;

        bool result = MapWktFlavorEnum(wktFlavor, fileWktFlavor);

        assert(result);

        SMStatus gcsFromWKTStatus = SMStatus::S_SUCCESS;
        GCS fileGCS(GetGCSFactory().Create(wktStr.c_str(), wktFlavor, gcsFromWKTStatus));
        cs = fileGCS.GetGeoRef().GetBasePtr();
        }

    IDTMSourceCollection sources;
    DocumentEnv sourceEnv(L"");
    bool success = BENTLEY_NAMESPACE_NAME::ScalableMesh::LoadSources(sources, *sourcesData, sourceEnv);
    assert(success == true);

    const IDTMSource* rasterSource =  nullptr;
    for (IDTMSourceCollection::const_iterator sourceIt = sources.Begin(), sourcesEnd = sources.End(); sourceIt != sourcesEnd;
         ++sourceIt)
        {
        const IDTMSource& source = *sourceIt;
        if (source.GetSourceType() == DTM_SOURCE_DATA_IMAGE)
            {
            rasterSource = &source;
            break;
            }
        }

    if (rasterSource == nullptr)
        {
        assert(false && "Trying to use a streamed source but no raster source found!");
        return;
        }
    WString path;
    if (rasterSource->GetPath().StartsWith(L"http://"))
        {
        path = rasterSource->GetPath();
        }
    else
        {
        path = WString(L"file://") + rasterSource->GetPath();
        }

    DRange2d extent2d = DRange2d::From(totalExt);
    m_source = RasterUtilities::LoadRaster(path,cs, extent2d);
    }

template <class DATATYPE, class EXTENT>  SMStreamedSourceStore<DATATYPE, EXTENT>::~SMStreamedSourceStore()
    {

    }

template <class DATATYPE, class EXTENT> size_t SMStreamedSourceStore<DATATYPE, EXTENT>::GetBlockDataCount(HPMBlockID blockID) const 
    {
    return 512 * 512 * 3;
    }

template <class DATATYPE, class EXTENT>  size_t SMStreamedSourceStore<DATATYPE, EXTENT>::GetBlockDataCount(HPMBlockID blockID, SMStoreDataType dataType) const 
    {
    assert(dataType == m_dataType);
    return 512 * 512 * 3 + 3 * sizeof(int);
    }

template <class DATATYPE, class EXTENT>  size_t SMStreamedSourceStore<DATATYPE, EXTENT>::LoadBlock(DATATYPE* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
    {
    assert(maxCountData >= 512 * 512 * 3);

    DRange2d nodeExtent2d = DRange2d::From(m_nodeHeader->m_contentExtentDefined ? m_nodeHeader->m_contentExtent : m_nodeHeader->m_nodeExtent);
    double unitsPerPixelX = (nodeExtent2d.high.x - nodeExtent2d.low.x) / 512;
    double unitsPerPixelY = (nodeExtent2d.high.y - nodeExtent2d.low.y) / 512;
    nodeExtent2d.low.x -= 5 * unitsPerPixelX;
    nodeExtent2d.low.y -= 5 * unitsPerPixelY;
    nodeExtent2d.high.x += 5 * unitsPerPixelX;
    nodeExtent2d.high.y += 5 * unitsPerPixelY;
    bvector<uint8_t> tex;
    RasterUtilities::CopyFromArea(tex, 512, 512, nodeExtent2d, *m_source);
    assert(tex.size() <= maxCountData);

    memcpy(DataTypeArray, &tex[0], std::min(maxCountData, tex.size()));

    return std::min(maxCountData, tex.size());
    }

