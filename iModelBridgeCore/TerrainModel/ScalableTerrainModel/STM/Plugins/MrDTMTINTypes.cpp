/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/Plugins/MrDTMTINTypes.cpp $
|    $RCSfile: MrDTMTINTypes.cpp,v $
|   $Revision: 1.3 $
|       $Date: 2011/08/10 15:10:20 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>

#include <ScalableTerrainModel/Type/IMrDTMTIN.h>
#include <ScalableTerrainModel/Import/Plugin/DataTypeRegistry.h>

USING_NAMESPACE_BENTLEY_MRDTM_IMPORT_PLUGIN_VERSION(0)


BEGIN_BENTLEY_MRDTM_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& TINTypeFamilyCreator::_Create () const
    {
    struct TypeFamily : public StaticDataTypeFamilyBase<TypeFamily>
        {
        public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

        explicit TypeFamily ()
            :   super_class(TINDimensionDef::ROLE_QTY)
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
TINTypeFamilyCreator::TINTypeFamilyCreator ()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& TINType3d64fCreator::_Create () const
    {
    struct Type : public StaticDataTypeBase<Type>
        {
        public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

        explicit Type ()
            :   super_class(TINTypeFamilyCreator().Create())
            {
            using namespace TINDimensionDef;

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
TINType3d64fCreator::TINType3d64fCreator ()
    {
    }

namespace {
const WChar TIN_AS_LINEAR_HEADER_TYPE_NAME[] = L"TINAsLinearHeader";
DimensionType::Register s_RegisterTINAsLinearHeaderType(TIN_AS_LINEAR_HEADER_TYPE_NAME, sizeof(IDTMFile::FeatureHeader));

const WChar TIN_AS_LINEAR_POINT_TYPE_NAME[] = L"TINAsLinearPoint";
DimensionType::Register s_RegisterTINAsLinearPointType(TIN_AS_LINEAR_POINT_TYPE_NAME, sizeof(IDTMFile::Point3d64f));
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& TINTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator::_Create () const
    {
    struct Type : public StaticDataTypeBase<Type>
        {
        public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

        explicit Type ()
            :   super_class(TINTypeFamilyCreator().Create())
            {
            using namespace TINDimensionDef;
            AddOrg(DimOrg(DimDef(DimType::GetFor(TIN_AS_LINEAR_HEADER_TYPE_NAME), ROLE_CUSTOM)));
            AddOrg(DimOrg(DimDef(DimType::GetFor(TIN_AS_LINEAR_POINT_TYPE_NAME), ROLE_CUSTOM)));
            }
        };

    static DataType SINGLETON = CreateFrom(new Type());
    return SINGLETON;

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TINTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator::TINTypeAsLinearTi32Pi32Pq32Gi32_3d64fCreator ()
    {
    }


END_BENTLEY_MRDTM_NAMESPACE