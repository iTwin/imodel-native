/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Plugin/DataTypeFamilyV0.h $
|    $RCSfile: DataTypeFamilyV0.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/08/30 19:04:18 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Definitions.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE
struct DataTypeFamily;
END_BENTLEY_MRDTM_IMPORT_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE(0)
struct DataTypeFamilyBase;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DataTypeFamilyCreatorBase : private Uncopyable
    {
protected:
    IMPORT_DLLE explicit                DataTypeFamilyCreatorBase              ();
    IMPORT_DLLE                         ~DataTypeFamilyCreatorBase             ();

    IMPORT_DLLE DataTypeFamily          CreateFrom                             (DataTypeFamilyBase*             implP) const;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct StaticDataTypeFamilyCreatorBase : public DataTypeFamilyCreatorBase
    {
private:
    virtual const DataTypeFamily&       _Create                                () const = 0;

protected:
    IMPORT_DLLE explicit                StaticDataTypeFamilyCreatorBase        ();

public:
    typedef const StaticDataTypeFamilyCreatorBase*            
                                        ID;


    IMPORT_DLLE virtual                 ~StaticDataTypeFamilyCreatorBase       () = 0;

    IMPORT_DLLE const DataTypeFamily&   Create                                 () const;

    };

/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct DataTypeFamilyBase : public ShareableObjectTypeTrait<DataTypeFamilyBase>::type
    {
private:
    // Ensure that points types are only based on DataTypeFamilyImplBase template
    template <typename T>
    friend                  struct StaticDataTypeFamilyBase;
    friend                  struct DataTypeFamily;

    typedef const void*     ClassID;

    ClassID                 m_id;
    UInt                    m_roleQty;
    const void*             m_implP; // Reserved for further use

    IMPORT_DLLE explicit    DataTypeFamilyBase                     (ClassID         id,
                                                                    UInt            roleQty);

public:
    IMPORT_DLLE virtual     ~DataTypeFamilyBase                    ();

    
    };


/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
struct StaticDataTypeFamilyBase : public DataTypeFamilyBase
    {

protected:
    typedef StaticDataTypeFamilyBase<T> 
                            super_class;

    explicit                StaticDataTypeFamilyBase               (UInt            roleQty)
            :   DataTypeFamilyBase(0, roleQty) 
        {
        m_id = GenerateClassID();
        }

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
