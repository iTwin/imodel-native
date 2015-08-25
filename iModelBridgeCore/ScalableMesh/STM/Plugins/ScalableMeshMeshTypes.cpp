/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/Plugins/ScalableMeshMeshTypes.cpp $
|    $RCSfile: ScalableMeshMeshTypes.cpp,v $
|   $Revision: 1.3 $
|       $Date: 2011/08/10 15:10:23 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

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

            AddOrg(DimOrg(DimDef(DimType::GetUnknown(), ROLE_UNKNOWN)));
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
Bentley::ScalableMesh::Import::DimensionType::Register s_RegisterMeshAsLinearHeaderType(MESH_AS_LINEAR_HEADER_TYPE_NAME, sizeof(IDTMFile::FeatureHeader));

const WChar MESH_AS_LINEAR_POINT_TYPE_NAME[] = L"MeshAsLinearPoint";
Bentley::ScalableMesh::Import::DimensionType::Register s_RegisterMeshAsLinearPointType(MESH_AS_LINEAR_POINT_TYPE_NAME, sizeof(IDTMFile::Point3d64f));

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
