/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshContentConfigPersistance.cpp $
|    $RCSfile: ScalableMeshContentConfigPersistance.cpp,v $
|   $Revision: 1.19 $
|       $Date: 2012/02/16 22:19:31 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

#include "ImagePPHeaders.h"
//#include "InternalUtilityFunctions.h"

#include "ScalableMeshContentConfigPersistance.h"

#include <ScalableMesh/GeoCoords/GCS.h>

#include <ScalableMesh/Import/Config/Content/All.h>
#include <ScalableMesh/Import/DataTypeDescription.h>

#include <ScalableMesh/Type/IScalableMeshLinear.h>
#include <ScalableMesh/Type/IScalableMeshPoint.h>
#include <ScalableMesh/Type/IScalableMeshTIN.h>
#include <ScalableMesh/Type/IScalableMeshMesh.h>

#include <ScalableMesh/IScalableMeshStream.h>

#include <ScalableMesh/IScalableMeshPolicy.h>

#include <STMInternal/GeoCoords/WKTUtils.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESH_GEOCOORDINATES
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT

    

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*
 * Driver current version
 */ 
const uint32_t ContentConfigSerializer::FORMAT_VERSION = 0;





bool                                OutputType(SourceDataSQLite&                              sourceData,
                                               const DataType&                             type)
    {
    static const DataType* typeIDs[12]
        =
        {
        &PointType3d64fCreator().Create(),
        &PointType3d64fM64fCreator().Create(),
        &PointType3d64fG32Creator().Create(),
        &PointType3d64fM64fG32Creator().Create(),
        &PointType3d64f_R16G16B16_I16Creator().Create(),
        &PointType3d64f_R16G16B16_I16_C8Creator().Create(),
        &LinearTypeTi32Pi32Pq32Gi32_3d64fCreator().Create(),
        &LinearTypeTi32Pi32Pq32Gi32_3d64fM64fCreator().Create(),
        &TINType3d64fCreator().Create(),
        &TINTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator().Create(),
        &MeshType3d64fCreator().Create(),
        &MeshTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator().Create()
        };
    for (size_t i = 0; i < 12; ++i)
        {
        if (type == *typeIDs[i])
            {
            sourceData.SetTypeID((uint32_t)i);
            return true;
            }
        }

    return false;
    }


const DataType&                                GetType(SourceDataSQLite&                              sourceData)
    {
    static const DataType* typeIDs[12]
        =
        {
        &PointType3d64fCreator().Create(),
        &PointType3d64fM64fCreator().Create(),
        &PointType3d64fG32Creator().Create(),
        &PointType3d64fM64fG32Creator().Create(),
        &PointType3d64f_R16G16B16_I16Creator().Create(),
        &PointType3d64f_R16G16B16_I16_C8Creator().Create(),
        &LinearTypeTi32Pi32Pq32Gi32_3d64fCreator().Create(),
        &LinearTypeTi32Pi32Pq32Gi32_3d64fM64fCreator().Create(),
        &TINType3d64fCreator().Create(),
        &TINTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator().Create(),
        &MeshType3d64fCreator().Create(),
        &MeshTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator().Create()
        };
    if (sourceData.GetTypeID() > 11) return *typeIDs[0];
    else return *typeIDs[sourceData.GetTypeID()];
    }
/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentConfigSerializer::Serialize(const ContentConfig&    config,
    SourceDataSQLite&          sourceData) const
{

    if (config.GetScalableMeshConfig().IsSet())
        sourceData.SetScalableMeshData(config.GetScalableMeshConfig().GetScalableMeshData());
    if (config.GetGCSConfig().IsSet())
        {
        const GCS& gcs = config.GetGCSConfig().GetGCS();
        GCS::Status status = GCS::S_SUCCESS;
        WKT gcsWKT(gcs.GetWKT(status));
        if (GCS::S_SUCCESS != status)
            return false;

        WString extendedWktStr(gcsWKT.GetCStr());
        wchar_t wktFlavor[2] = { (wchar_t)IDTMFile::WktFlavor_Autodesk, L'\0' };

        extendedWktStr += WString(wktFlavor);

        sourceData.SetGCS(extendedWktStr);
        uint32_t flagsField(0);

        SetBitsTo(flagsField, 0x1, config.GetGCSConfig().IsPrependedToExistingLocalTransform());
        SetBitsTo(flagsField, 0x2, config.GetGCSConfig().IsExistingPreservedIfGeoreferenced());
        SetBitsTo(flagsField, 0x4, config.GetGCSConfig().IsExistingPreservedIfLocalCS());

        sourceData.SetFlags(flagsField);
        }
    if (config.GetTypeConfig().IsSet())
        {
        return OutputType(sourceData, config.GetTypeConfig().GetType());
        }
    return true;
}




/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/

bool ContentConfigSerializer::Deserialize(SourceDataSQLite&      sourceData,
                                          ContentConfig&      config,
                                          uint32_t                formatVersion) const
    {


    config.SetScalableMeshConfig(ScalableMeshConfig(sourceData.GetScalableMeshData()));
    WString gcsWKT = sourceData.GetGCS();

    IDTMFile::WktFlavor fileWktFlavor = GetWKTFlavor(&gcsWKT, gcsWKT);

    BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::WktFlavor baseGcsWktFlavor;

    bool result = MapWktFlavorEnum(baseGcsWktFlavor, fileWktFlavor);
    assert(result == true);

    GCS gcs(GCS::GetNull());
    GCSFactory::Status gcsFromWKTStatus = GCSFactory::S_SUCCESS;
    gcs = GetGCSFactory().Create(gcsWKT.c_str(), baseGcsWktFlavor, gcsFromWKTStatus);

    if (GCSFactory::S_SUCCESS != gcsFromWKTStatus)
        return false;
    config.SetGCSConfig(GCSConfig(gcs, sourceData.GetFlags()));

    const DataType& type = GetType(sourceData);
    config.SetTypeConfig(TypeConfig(type));
    return true;

    // return GenericDeserialize(sourceData, config);
    }




END_BENTLEY_SCALABLEMESH_NAMESPACE
