/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/Plugins/MrDTMLinearTypes.cpp $
|    $RCSfile: MrDTMLinearTypes.cpp,v $
|   $Revision: 1.3 $
|       $Date: 2011/08/10 15:10:25 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>

#include <ScalableTerrainModel/Type/IMrDTMPoint.h>
#include <ScalableTerrainModel/Type/IMrDTMLinear.h>

#include <ScalableTerrainModel/Import/Plugin/DataTypeRegistry.h>

USING_NAMESPACE_BENTLEY_MRDTM_IMPORT_PLUGIN_VERSION(0)

BEGIN_BENTLEY_MRDTM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LinearTypeFamilyCreator::LinearTypeFamilyCreator ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& LinearTypeFamilyCreator::_Create () const
    {
    struct TypeFamily : public StaticDataTypeFamilyBase<TypeFamily>
        {
        public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

        explicit TypeFamily ()
            :   super_class(LinearDimensionDef::ROLE_QTY)
            {

            }

        };

    static const DataTypeFamily SINGLETON(CreateFrom(new TypeFamily()));
    return SINGLETON;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LinearTypeTi32Pi32Pq32Gi32_3d64fCreator::LinearTypeTi32Pi32Pq32Gi32_3d64fCreator ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& LinearTypeTi32Pi32Pq32Gi32_3d64fCreator::_Create () const
    {

    struct Type : public StaticDataTypeBase<Type>
        {
        public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

        explicit Type ()
            :   super_class(LinearTypeFamilyCreator().Create())
            {
            using namespace LinearDimensionDef;

            AddOrg  
                (
                DimOrg
                    (
                    DimDef(DimType::GetUInt32(),     ROLE_FEATURE_TYPE),
                    DimDef(DimType::GetUInt32(),     ROLE_POINT_INDEX),
                    DimDef(DimType::GetUInt32(),     ROLE_POINT_QTY),
                    DimDef(DimType::GetUInt32(),     ROLE_GROUP_ID)
                    )
                );

            AddOrg (PointType3d64fCreator().Create().GetOrgGroup()[0]);
            }
        };

    static DataType SINGLETON(CreateFrom(new Type()));
    return SINGLETON;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LinearTypeTi32Pi32Pq32Gi32_3d64fM64fCreator::LinearTypeTi32Pi32Pq32Gi32_3d64fM64fCreator ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& LinearTypeTi32Pi32Pq32Gi32_3d64fM64fCreator::_Create () const
    {
    struct Type : public StaticDataTypeBase<Type>
        {
        public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

        explicit Type ()
            :   super_class(LinearTypeFamilyCreator().Create())
            {
            AddOrg (LinearTypeTi32Pi32Pq32Gi32_3d64fCreator().Create().GetOrgGroup()[0]);
            AddOrg (PointType3d64fM64fCreator().Create().GetOrgGroup()[0]);
            }
        };

    static DataType SINGLETON(CreateFrom(new Type()));
    return SINGLETON;
    }


namespace {

DataTypeRegistry::AutoRegister<LinearTypeTi32Pi32Pq32Gi32_3d64fCreator>         s_registerLinearTypeTi32Pi32Pq32Gi32_3d64f;
DataTypeRegistry::AutoRegister<LinearTypeTi32Pi32Pq32Gi32_3d64fM64fCreator>     s_registerLinearTypeTi32Pi32Pq32Gi32_3d64fM64f;

}


END_BENTLEY_MRDTM_NAMESPACE