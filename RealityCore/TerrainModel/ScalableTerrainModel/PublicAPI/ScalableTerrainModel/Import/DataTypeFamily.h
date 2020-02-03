/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Definitions.h>


BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE(0)
struct DataTypeFamilyBase;
struct DataTypeFamilyCreatorBase;
END_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DataTypeFamily
    {
private:
    friend struct                       Plugin::V0::DataTypeFamilyCreatorBase;

    typedef Plugin::V0::DataTypeFamilyBase
                                        DataTypeFamilyBase;

    typedef const void*                 ClassID;
    typedef SharedPtrTypeTrait<const DataTypeFamilyBase>::type
                                        ImplCPtr; 

    ImplCPtr                            m_pImpl;  
    ClassID                             m_classID;
  
    explicit                            DataTypeFamily                         (DataTypeFamilyBase*             implP);

public:
    IMPORT_DLLE                         ~DataTypeFamily                        ();

    IMPORT_DLLE                         DataTypeFamily                         (const DataTypeFamily&           rhs);
    IMPORT_DLLE DataTypeFamily&         operator=                              (const DataTypeFamily&           rhs);

    bool                                operator==                             (const DataTypeFamily&           rhs) const 
                                        {return m_classID == rhs.m_classID;}
    bool                                operator<                              (const DataTypeFamily&           rhs) const 
                                        {return m_classID < rhs.m_classID;}

    IMPORT_DLLE UInt                    GetRoleQty                             () const;
    };

END_BENTLEY_MRDTM_IMPORT_NAMESPACE