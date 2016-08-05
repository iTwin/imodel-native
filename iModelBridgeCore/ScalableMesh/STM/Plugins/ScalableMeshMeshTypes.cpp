/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/Plugins/ScalableMeshMeshTypes.cpp $
|    $RCSfile: ScalableMeshMeshTypes.cpp,v $
|   $Revision: 1.3 $
|       $Date: 2011/08/10 15:10:23 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "../ImagePPHeaders.h"
#include "../Stores/SMStoreUtils.h"
#include <ScalableMesh/Type/IScalableMeshMesh.h>
#include <ScalableMesh/Import/Plugin/DataTypeRegistry.h>


USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VERSION(0)


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& MeshTypeFamilyCreator::_Create () const
    {
    struct TypeFamily : public StaticDataTypeFamilyBase<TypeFamily>
        {
        public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

        explicit TypeFamily ()
            :   super_class(MeshDimensionDef::ROLE_QTY)
            {
            }
        };

    static const DataTypeFamily SINGLETON(CreateFrom(new TypeFamily()));
    return SINGLETON;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MeshTypeFamilyCreator::MeshTypeFamilyCreator ()
    {
    }


namespace {
const WChar MESH_PTS_NAME[] = L"MeshPoints";
BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType::Register s_RegisterMeshHeaderType(MESH_PTS_NAME, sizeof(DPoint3d));

const WChar MESH_INDEX_NAME[] = L"MeshIndex";
BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType::Register s_RegisterMeshPointType(MESH_INDEX_NAME, sizeof(int32_t));

const WChar MESH_METADATA_NAME[] = L"MeshMetadata";
BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType::Register s_RegisterMeshMetadataType(MESH_METADATA_NAME, sizeof(uint8_t));

const WChar MESH_TEX_NAME[] = L"MeshTex";
BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType::Register s_RegisterMeshTexType(MESH_TEX_NAME, sizeof(uint8_t));

const WChar MESH_UV_NAME[] = L"MeshUv";
BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType::Register s_RegisterMeshUvType(MESH_UV_NAME, sizeof(DPoint2d));
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& MeshType3d64fCreator::_Create () const
    {
    struct Type : public StaticDataTypeBase<Type>
        {
        public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

        explicit Type ()
            :   super_class(MeshTypeFamilyCreator().Create())
            {
            using namespace MeshDimensionDef;

            AddOrg(DimOrg(DimDef(DimType::GetFor(MESH_PTS_NAME), ROLE_CUSTOM)));
            AddOrg(DimOrg(DimDef(DimType::GetFor(MESH_INDEX_NAME), ROLE_CUSTOM)));
            AddOrg(DimOrg(DimDef(DimType::GetFor(MESH_METADATA_NAME), ROLE_CUSTOM)));
            AddOrg(DimOrg(DimDef(DimType::GetFor(MESH_TEX_NAME), ROLE_CUSTOM)));
            AddOrg(DimOrg(DimDef(DimType::GetFor(MESH_UV_NAME), ROLE_CUSTOM)));
            }
        };

    static DataType SINGLETON = CreateFrom(new Type());
    return SINGLETON;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MeshType3d64fCreator::MeshType3d64fCreator ()
    {
    }

namespace {
const WChar MESH_AS_LINEAR_HEADER_TYPE_NAME[] = L"MeshAsLinearHeader";
BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType::Register s_RegisterMesAsLinearPointType(MESH_AS_LINEAR_HEADER_TYPE_NAME, sizeof(ISMStore::FeatureHeader));

const WChar MESH_AS_LINEAR_POINT_TYPE_NAME[] = L"MeshAsLinearPoint";
BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::DimensionType::Register s_RegisterMeshAsLinearPointIdxType(MESH_AS_LINEAR_POINT_TYPE_NAME, sizeof(DPoint3d));

}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& MeshTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator::_Create () const
    {
    struct Type : public StaticDataTypeBase<Type>
        {
        public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

        explicit Type ()
            :   super_class(MeshTypeFamilyCreator().Create())
            {
            using namespace MeshDimensionDef;
            AddOrg(DimOrg(DimDef(DimType::GetFor(MESH_AS_LINEAR_HEADER_TYPE_NAME), ROLE_CUSTOM)));
            AddOrg(DimOrg(DimDef(DimType::GetFor(MESH_AS_LINEAR_POINT_TYPE_NAME), ROLE_CUSTOM)));
            }
        };

    static DataType SINGLETON = CreateFrom(new Type());
    return SINGLETON;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MeshTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator::MeshTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator ()
    {
    }


END_BENTLEY_SCALABLEMESH_NAMESPACE
