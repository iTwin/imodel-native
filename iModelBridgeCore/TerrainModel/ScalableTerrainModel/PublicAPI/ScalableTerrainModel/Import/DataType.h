/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/DataType.h $
|    $RCSfile: DataType.h,v $
|   $Revision: 1.16 $
|       $Date: 2011/10/20 18:48:21 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Definitions.h>

#include <ScalableTerrainModel/Import/DataTypeFamily.h>


BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE
struct DataTypeRegistry;
END_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE(0)
struct DataTypeBase;
struct DataTypeCreatorBase;
struct StaticDataTypeCreatorBase;
END_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

struct DataType;
struct DimensionOrgGroup;
struct DimensionOrg;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct DataTypeFactory : private Unassignable
    {
private:
    struct                                  Impl;
    SharedPtrTypeTrait<const Impl>::type    m_implP;
public:
    IMPORT_DLLE explicit                    DataTypeFactory                        ();
    IMPORT_DLLE explicit                    DataTypeFactory                        (const Plugin::DataTypeRegistry&     registry);
    
    IMPORT_DLLE                             DataTypeFactory                        (const DataTypeFactory&              rhs);
    IMPORT_DLLE                             ~DataTypeFactory                       ();

    IMPORT_DLLE DataType                    Create                                 (const DataTypeFamily&               typeFamily,
                                                                                     const DimensionOrg&                dimensionsSpec) const;
    IMPORT_DLLE DataType                    Create                                 (const DataTypeFamily&               typeFamily,
                                                                                    const DimensionOrgGroup&            dimensionsSpec) const;

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DataType
    {
private:
    friend struct                           Plugin::V0::DataTypeCreatorBase;
    friend struct                           DataTypeFactory;

    typedef Plugin::V0::DataTypeBase        DataTypeBase;

    typedef const void*                     ClassID;
    typedef SharedPtrTypeTrait<const DataTypeBase>::type
                                            BaseCPtr; 

    BaseCPtr                                m_pImpl;  
    ClassID                                 m_classID;
  
    explicit                                DataType                           (DataTypeBase*           implP);

public:
    IMPORT_DLLE                             ~DataType                          ();

    IMPORT_DLLE                             DataType                           (const DataType&         rhs);
    IMPORT_DLLE DataType&                   operator=                          (const DataType&         rhs);

    bool                                    operator!=                         (const DataType&         rhs) const 
                                            { return m_classID != rhs.m_classID; }
    bool                                    operator==                         (const DataType&         rhs) const 
                                            { return m_classID == rhs.m_classID; }
    bool                                    operator<                          (const DataType&         rhs) const 
                                            { return m_classID < rhs.m_classID; }


    IMPORT_DLLE const DataTypeFamily&       GetFamily                          () const;


    IMPORT_DLLE const DimensionOrgGroup&    GetOrgGroup                        () const;

    IMPORT_DLLE size_t                      GetDimensionOrgCount               () const;
    IMPORT_DLLE size_t                      GetDimensionCount                  () const;
    IMPORT_DLLE size_t                      GetSize                            () const;


    IMPORT_DLLE bool                        IsComplete                         () const;
    IMPORT_DLLE bool                        IsPOD                              () const;
    IMPORT_DLLE bool                        IsStatic                           () const;

    };


inline bool                                 operator==                         (const DataType&         lhs,
                                                                                const DataTypeFamily&   rhs)
                                            { return lhs.GetFamily() == rhs; }

inline bool                                 operator==                         (const DataTypeFamily&   lhs,
                                                                                const DataType&         rhs)
                                            { return lhs == rhs.GetFamily(); }


/*---------------------------------------------------------------------------------**//**
* @description   
* NTERAY: Consider COW
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DataTypeSet
    {
private:
    struct                                  Impl;
    std::auto_ptr<Impl>                     m_implP;

public:
    typedef DataType                        value_type;
    typedef value_type&                     reference;
    typedef const value_type&               const_reference;
    typedef const DataType*                 const_iterator;


    IMPORT_DLLE explicit                    DataTypeSet ();

    // A DataType is implicitly converted to a DataTypeSet
    IMPORT_DLLE                             DataTypeSet                        (const DataType&         t1);

    // TDORAY: Use initializer list when available
    IMPORT_DLLE explicit                    DataTypeSet                        (const DataType&         t1,
                                                                                const DataType&         t2);

    IMPORT_DLLE explicit                    DataTypeSet                        (const DataType&         t1,
                                                                                const DataType&         t2,
                                                                                const DataType&         t3);

    IMPORT_DLLE explicit                    DataTypeSet                        (const DataType&         t1,
                                                                                const DataType&         t2,
                                                                                const DataType&         t3,
                                                                                const DataType&         t4);

    IMPORT_DLLE                             ~DataTypeSet                       ();

    IMPORT_DLLE                             DataTypeSet                        (const DataTypeSet&      rhs);
    IMPORT_DLLE DataTypeSet&                operator=                          (const DataTypeSet&      rhs);

    IMPORT_DLLE const_iterator              begin                              () const;
    IMPORT_DLLE const_iterator              end                                () const;

    IMPORT_DLLE size_t                      GetCount                           () const;

    IMPORT_DLLE void                        Add                                (const DataType&         type);
    IMPORT_DLLE void                        push_back                          (const DataType&         type);
    };


typedef DataTypeSet                         DataTypeCapabilities;



END_BENTLEY_MRDTM_IMPORT_NAMESPACE