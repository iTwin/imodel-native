/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Config/Content/Type.h $
|    $RCSfile: Type.h,v $
|   $Revision: 1.7 $
|       $Date: 2011/11/22 21:58:02 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/Import/Definitions.h>

#include <ScalableMesh/Import/DataType.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

struct TypeConfigImpl;
/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TypeConfig 
    {
public:  // OPERATOR_NEW_KLUDGE


private:
    RefCountedPtr<TypeConfigImpl>
        m_pImpl;

public:
//    IMPORT_DLLE static ClassID          s_GetClassID                       ();
    IMPORT_DLLE explicit                TypeConfig();
    IMPORT_DLLE explicit                TypeConfig                         (const DataType&             type);
    IMPORT_DLLE virtual                 ~TypeConfig();

    IMPORT_DLLE                         TypeConfig                         (const TypeConfig&           rhs);

    const DataType&                     GetType                            () const;
    bool                                IsSet()   const;
    };




END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
