/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/Plugins/MrDTMPointTypes.cpp $
|    $RCSfile: MrDTMPointTypes.cpp,v $
|   $Revision: 1.4 $
|       $Date: 2011/08/30 19:03:59 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>

#include <ScalableTerrainModel/Type/IMrDTMPoint.h>
#include <ScalableTerrainModel/Import/Plugin/DataTypeRegistry.h>


USING_NAMESPACE_BENTLEY_MRDTM_IMPORT_PLUGIN_VERSION(0)


BEGIN_BENTLEY_MRDTM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PointTypeFamilyCreator::PointTypeFamilyCreator ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& PointTypeFamilyCreator::_Create () const
    {
    struct TypeFamily : public StaticDataTypeFamilyBase<TypeFamily>
        {
        public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

        explicit TypeFamily ()
            :   super_class(PointDimensionDef::ROLE_QTY)
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
PointType3d64fCreator::PointType3d64fCreator ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& PointType3d64fCreator::_Create () const
    {
    struct Type : public StaticDataTypeBase<Type>
        {
        public:  // OPERATOR_NEW_KLUDGE
            void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
            void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
            void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
            void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

        explicit Type ()
            :   super_class(PointTypeFamilyCreator().Create())
            {
            using namespace PointDimensionDef;

            AddOrg  
                (
                DimOrg
                    (
                    DimDef(DimType::GetFloat64(),    ROLE_XCOORDINATE),
                    DimDef(DimType::GetFloat64(),    ROLE_YCOORDINATE),
                    DimDef(DimType::GetFloat64(),    ROLE_ZCOORDINATE)
                    )
                );
            }
        };

    static DataType SINGLETON = CreateFrom(new Type());
    return SINGLETON;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PointType3d64fM64fCreator::PointType3d64fM64fCreator ()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& PointType3d64fM64fCreator::_Create () const
    {
    struct Type : public StaticDataTypeBase<Type>
        {
    public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

        explicit Type ()
            :   super_class(PointTypeFamilyCreator().Create())
            {
            using namespace PointDimensionDef;

            AddOrg  
                (
                DimOrg
                    (
                    DimDef(DimType::GetFloat64(),    ROLE_XCOORDINATE),
                    DimDef(DimType::GetFloat64(),    ROLE_YCOORDINATE),
                    DimDef(DimType::GetFloat64(),    ROLE_ZCOORDINATE),
                    DimDef(DimType::GetFloat64(),    ROLE_SIGNIFICANCE_LEVEL)
                    )
                );

            }
        };

    static DataType SINGLETON(CreateFrom(new Type()));
    return SINGLETON;
    }
    

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PointType3d64fG32Creator::PointType3d64fG32Creator ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& PointType3d64fG32Creator::_Create () const
    {
    struct Type : public StaticDataTypeBase<Type>
        {
        public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

        explicit Type ()
            :   super_class(PointTypeFamilyCreator().Create())
            {
            using namespace PointDimensionDef;

            AddOrg  
                (
                DimOrg
                    (
                    DimDef(DimType::GetFloat64(),    ROLE_XCOORDINATE),
                    DimDef(DimType::GetFloat64(),    ROLE_YCOORDINATE),
                    DimDef(DimType::GetFloat64(),    ROLE_ZCOORDINATE),
                    DimDef(DimType::GetUInt32(),     ROLE_GROUP_ID)
                    )
                );
            }
        };

    static DataType SINGLETON(CreateFrom(new Type()));
    return SINGLETON;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PointType3d64fM64fG32Creator::PointType3d64fM64fG32Creator ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& PointType3d64fM64fG32Creator::_Create () const
    {
    struct Type : public StaticDataTypeBase<Type>
        {
        public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }
        explicit Type ()
            :   super_class(PointTypeFamilyCreator().Create())
            {
            using namespace PointDimensionDef;

            AddOrg  
                (
                DimOrg
                    (
                    DimDef(DimType::GetFloat64(),    ROLE_XCOORDINATE),
                    DimDef(DimType::GetFloat64(),    ROLE_YCOORDINATE),
                    DimDef(DimType::GetFloat64(),    ROLE_ZCOORDINATE),
                    DimDef(DimType::GetFloat64(),    ROLE_SIGNIFICANCE_LEVEL),
                    DimDef(DimType::GetUInt32(),     ROLE_GROUP_ID)
                    )
                );
            }
        };

    static DataType SINGLETON(CreateFrom(new Type()));
    return SINGLETON;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PointType3d64f_R16G16B16_I16Creator::PointType3d64f_R16G16B16_I16Creator ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& PointType3d64f_R16G16B16_I16Creator::_Create () const
    {
    struct Type : public StaticDataTypeBase<Type>
        {
        public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }
        explicit Type ()
            :   super_class(PointTypeFamilyCreator().Create())
            {
            using namespace PointDimensionDef;
            AddOrg  
                (
                DimOrg
                    (
                    DimDef(DimType::GetFloat64(),    ROLE_XCOORDINATE),
                    DimDef(DimType::GetFloat64(),    ROLE_YCOORDINATE),
                    DimDef(DimType::GetFloat64(),    ROLE_ZCOORDINATE)
                    )
                );
            AddOrg  
                (
                DimOrg
                    (
                    DimDef(DimType::GetUInt16(),     ROLE_RED_COLOR_COMPONENT),
                    DimDef(DimType::GetUInt16(),     ROLE_GREEN_COLOR_COMPONENT),
                    DimDef(DimType::GetUInt16(),     ROLE_BLUE_COLOR_COMPONENT)
                    )
                );
            AddOrg  
                (
                DimOrg
                    (
                    DimDef(DimType::GetUInt16(),     ROLE_COLOR_INTENSITY)
                    )
                );
            }
        };

    static DataType SINGLETON(CreateFrom(new Type()));
    return SINGLETON;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PointType3d64f_R16G16B16_I16_C8Creator::PointType3d64f_R16G16B16_I16_C8Creator ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& PointType3d64f_R16G16B16_I16_C8Creator::_Create () const
    {
    struct Type : public StaticDataTypeBase<Type>
        {
        public:  // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }
        explicit Type ()
            :   super_class(PointTypeFamilyCreator().Create())
            {
            using namespace PointDimensionDef;
            AddOrg  
                (
                DimOrg
                    (
                    DimDef(DimType::GetFloat64(),   ROLE_XCOORDINATE),
                    DimDef(DimType::GetFloat64(),   ROLE_YCOORDINATE),
                    DimDef(DimType::GetFloat64(),   ROLE_ZCOORDINATE)
                    )
                );
            AddOrg  
                (
                DimOrg
                    (
                    DimDef(DimType::GetUInt16(),    ROLE_RED_COLOR_COMPONENT),
                    DimDef(DimType::GetUInt16(),    ROLE_GREEN_COLOR_COMPONENT),
                    DimDef(DimType::GetUInt16(),    ROLE_BLUE_COLOR_COMPONENT)
                    )
                );
            AddOrg  
                (
                DimOrg
                    (
                    DimDef(DimType::GetUInt16(),    ROLE_COLOR_INTENSITY)
                    )
                );
            AddOrg  
                (
                DimOrg
                    (
                    DimDef(DimType::GetUInt8(),     ROLE_GROUP_ID) // TDORAY: Change for ROLE_CLASSIFICATION???
                    )
                );
            }
        };

    static Import::DataType SINGLETON(CreateFrom(new Type()));
    return SINGLETON;
    }


namespace {

DataTypeRegistry::AutoRegister<PointType3d64fCreator>                   s_registerPointType3d64f;
DataTypeRegistry::AutoRegister<PointType3d64fM64fCreator>               s_registerPointType3d64fM64f;
DataTypeRegistry::AutoRegister<PointType3d64fG32Creator>                s_registerPointType3d64fG32;
DataTypeRegistry::AutoRegister<PointType3d64fM64fG32Creator>            s_registerPointType3d64fM64fG32;
DataTypeRegistry::AutoRegister<PointType3d64f_R16G16B16_I16Creator>     s_registerPointType3d64f_R16G16B16_I16;
DataTypeRegistry::AutoRegister<PointType3d64f_R16G16B16_I16_C8Creator>  s_registerPointType3d64f_R16G16B16_I16_C8;
}


END_BENTLEY_MRDTM_NAMESPACE