/*--------------------------------------------------------------------------------------+
|    $RCSfile: DefaultTargetLayer.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/11/22 21:57:51 $
|     $Author: Raymond.Gauthier $
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableTerrainModel/Import/Definitions.h>

#include <ScalableTerrainModel/Import/Config/Base.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DefaultTargetLayerConfig : public ImportConfigComponentMixinBase<DefaultTargetLayerConfig>
    {
private:
    UInt                                    m_layer;
    void*                                   m_implP; // Reserved some space for further use

public:
    IMPORT_DLLE static ClassID              s_GetClassID                   ();

    IMPORT_DLLE explicit                    DefaultTargetLayerConfig       (UInt                                    layer);
    IMPORT_DLLE virtual                     ~DefaultTargetLayerConfig      ();

    IMPORT_DLLE                             DefaultTargetLayerConfig       (const DefaultTargetLayerConfig&         rhs);

    UInt                                    Get                            () const;
    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE