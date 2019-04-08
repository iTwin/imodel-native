/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Config/DefaultTargetType.h $
|    $RCSfile: DefaultTargetType.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/11/22 21:57:53 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableTerrainModel/Import/Definitions.h>
#include <ScalableTerrainModel/Import/Config/Base.h>
#include <ScalableTerrainModel/Import/DataTypeFamily.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DefaultTargetTypeConfig : public ImportConfigComponentMixinBase<DefaultTargetTypeConfig>
    {
private:
    DataTypeFamily                          m_type;
    void*                                   m_implP; // Reserved some space for further use

public:
    IMPORT_DLLE static ClassID              s_GetClassID                   ();

    IMPORT_DLLE explicit                    DefaultTargetTypeConfig        (const DataTypeFamily&                   type);
    IMPORT_DLLE virtual                     ~DefaultTargetTypeConfig       ();

    IMPORT_DLLE                             DefaultTargetTypeConfig        (const DefaultTargetTypeConfig&          rhs);

    const DataTypeFamily&                   Get                            () const;
    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE