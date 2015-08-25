/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/DataTypeFamily.h $
|    $RCSfile: DataTypeFamily.h,v $
|   $Revision: 1.8 $
|       $Date: 2011/08/02 14:58:24 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE(0)
struct DataTypeFamilyBase;
struct DataTypeFamilyCreatorBase;
END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

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

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
