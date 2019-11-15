/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "SMStreamedSourceStore.h"
#include "../RasterUtilities.h"
#include <ScalableMesh/GeoCoords/GCS.h>
#include <STMInternal/GeoCoords/WKTUtils.h>
#include <ScalableMesh/GeoCoords/Reprojection.h>
#include <ImagePP/all/h/HRAClearOptions.h>
#include <ImagePP/all/h/HRACopyFromOptions.h>
#include <ImagePP/all/h/HRPPixelTypeV24B8G8R8.h>
#include "../ScalableMeshSources.h"
#include "ISMDataStore.h"
#include "SMSQLiteSisterFile.h"
#include "SMStoreUtils.h"

#include "../ScalableMeshSourcesPersistance.h"
#include <ScalableMesh/IScalableMeshSourceImportConfig.h>
#include <ScalableMesh/IScalableMeshDocumentEnv.h>
#include <ScalableMesh/IScalableMeshSourceVisitor.h>

#include <ScalableMesh/Import/SourceReference.h>

#include <ScalableMesh/Import/Source.h>

#include <ScalableMesh/IScalableMeshPolicy.h>

template <class DATATYPE, class EXTENT>  SMStreamedSourceStore<DATATYPE, EXTENT>::SMStreamedSourceStore(SMStoreDataType dataType, SMIndexNodeHeader<EXTENT>* nodeHeader, SMSQLiteFilePtr& smSQLiteFile, DRange3d totalExt, HFCPtr<HRARASTER> source)
    {
    assert(source != nullptr);
    m_nodeHeader = nodeHeader;
    m_dataType = dataType;
    m_smSQLiteFile = smSQLiteFile;    
    m_source = source;
    }

template <class DATATYPE, class EXTENT>  SMStreamedSourceStore<DATATYPE, EXTENT>::~SMStreamedSourceStore()
    {

    }

template <class DATATYPE, class EXTENT> size_t SMStreamedSourceStore<DATATYPE, EXTENT>::GetBlockDataCount(HPMBlockID blockID) const 
    {
    return 256 * 256 * 3;
    }

template <class DATATYPE, class EXTENT>  size_t SMStreamedSourceStore<DATATYPE, EXTENT>::GetBlockDataCount(HPMBlockID blockID, SMStoreDataType dataType) const 
    {
    assert(dataType == m_dataType);
    return 256 * 256 * 3 + 3 * sizeof(int);
    }

template <class DATATYPE, class EXTENT>  size_t SMStreamedSourceStore<DATATYPE, EXTENT>::LoadBlock(DATATYPE* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
    {
    assert(maxCountData >= 256 * 256 * 3);
    assert(m_source != nullptr);
    

    //DRange2d nodeExtent2d = DRange2d::From(m_nodeHeader->m_contentExtentDefined ? m_nodeHeader->m_contentExtent : m_nodeHeader->m_nodeExtent);
    DRange2d nodeExtent2d = DRange2d::From(m_nodeHeader->m_nodeExtent);

    double unitsPerPixelX = (nodeExtent2d.high.x - nodeExtent2d.low.x) / 256;
    double unitsPerPixelY = (nodeExtent2d.high.y - nodeExtent2d.low.y) / 256;
    nodeExtent2d.low.x -= 5 * unitsPerPixelX;
    nodeExtent2d.low.y -= 5 * unitsPerPixelY;
    nodeExtent2d.high.x += 5 * unitsPerPixelX;
    nodeExtent2d.high.y += 5 * unitsPerPixelY;

    bvector<uint8_t> tex;
    RasterUtilities::CopyFromArea(tex, 256, 256, nodeExtent2d, &(m_nodeHeader)->m_textureResolution, *m_source);
    //assert(tex.size() <= maxCountData);

    memcpy(DataTypeArray, &tex[0], std::min(maxCountData, tex.size()));

    return std::min(maxCountData, tex.size());
    }

