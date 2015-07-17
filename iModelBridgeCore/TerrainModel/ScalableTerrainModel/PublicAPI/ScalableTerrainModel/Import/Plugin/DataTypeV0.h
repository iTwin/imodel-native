/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Plugin/DataTypeV0.h $
|    $RCSfile: DataTypeV0.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/08/30 19:04:16 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Definitions.h>
#include <ScalableTerrainModel/Import/DataTypeDescription.h>
#include <ScalableTerrainModel/Import/DataTypeFamily.h>
#include <ScalableTerrainModel/Import/Plugin/DataTypeRegistry.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE
struct DataType;
struct StaticDataTypeCreator;
END_BENTLEY_MRDTM_IMPORT_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE(0)

struct DataTypeBase;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DataTypeCreatorBase : private Uncopyable
    {
protected:
    typedef Plugin::V0::DataTypeBase        DataTypeBase;

    IMPORT_DLLE explicit                    DataTypeCreatorBase                ();
    IMPORT_DLLE                             ~DataTypeCreatorBase               ();

    IMPORT_DLLE DataType                    CreateFrom                         (DataTypeBase*               implP) const;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct StaticDataTypeCreatorBase : protected DataTypeCreatorBase
    {
private:
    friend struct                           StaticDataTypeCreator;

    virtual const DataType&                 _Create                            () const = 0;

protected:
    IMPORT_DLLE explicit                    StaticDataTypeCreatorBase          ();
public:
    typedef const StaticDataTypeCreatorBase*            
                                            ID;

    IMPORT_DLLE virtual                     ~StaticDataTypeCreatorBase         () = 0;

    IMPORT_DLLE const DataType&             Create                             () const;
    };


/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct DataTypeBase : public ShareableObjectTypeTrait<DataTypeBase>::type
    {
private:

    // Ensure that points types are only based on DataTypeImplBase template
    template <typename T>
    friend                  struct StaticDataTypeBase;
    friend                  struct DataType;

    typedef const void*     ClassID;

    ClassID                 m_classID;
    DataTypeFamily          m_family;
    DimensionOrgGroup       m_orgGroup;
    bool                    m_isComplete;
    bool                    m_isPOD;
    const void*             m_implP; // Reserve some space for further use

    IMPORT_DLLE explicit    DataTypeBase           (ClassID                         pi_id, 
                                                    const DataTypeFamily&           pi_family,
                                                    size_t                          pi_orgCapacity);

    IMPORT_DLLE explicit    DataTypeBase           (ClassID                         pi_id, 
                                                    const DataTypeFamily&           pi_family,
                                                    const DimensionOrgGroup&        pi_orgGroup);


protected:
    // Setup friendlier names for description classes
    typedef DimensionOrg    DimOrg;
    typedef DimensionDef    DimDef;
    typedef DimensionType   DimType;


    IMPORT_DLLE explicit    DataTypeBase           (const DataTypeFamily&           pi_family,
                                                    size_t                          pi_orgCapacity = 0);

    IMPORT_DLLE explicit    DataTypeBase           (const DataTypeFamily&           pi_family,
                                                    const DimensionOrgGroup&        pi_orgGroup);

    IMPORT_DLLE virtual     ~DataTypeBase          () = 0;

    IMPORT_DLLE void        AddOrg                 (const DimensionOrg&             pi_rOrg);
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
struct StaticDataTypeBase : public DataTypeBase
    {

protected:
    typedef StaticDataTypeBase<T> 
                            super_class;

    explicit                StaticDataTypeBase                     (const DataTypeFamily&       pi_family,
                                                                    size_t                      pi_orgCapacity = 0)
            :   DataTypeBase(0, pi_family, pi_orgCapacity) 
        {
        m_classID = GenerateClassID();
        }

    explicit                StaticDataTypeBase                     (const DataTypeFamily&       pi_family,
                                                                    const DimensionOrgGroup&    pi_orgGroup)
            :   DataTypeBase(0, pi_family, pi_orgGroup) 
        {
        m_classID = GenerateClassID();
        }

    virtual                 ~StaticDataTypeBase                    () = 0 {}

private:
    ClassID                  GenerateClassID                       () const 
        {
        static ClassID CLASS_ID = 0;
        if (0 != CLASS_ID)
            return CLASS_ID;

        CLASS_ID = &typeid(static_cast<const T&>(*this));
        return CLASS_ID;
        }
    };


END_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE