/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Config/DefaultTargetLayer.h $
|    $RCSfile: DefaultTargetLayer.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/11/22 21:57:51 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh/Import/Definitions.h>

#include <ScalableMesh/Import/Config/Base.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DefaultTargetLayerConfig : public ImportConfigComponentMixinBase<DefaultTargetLayerConfig>
    {
private:
    uint32_t                                    m_layer;
    void*                                   m_implP; // Reserved some space for further use

public:
    IMPORT_DLLE static ClassID              s_GetClassID                   ();

    IMPORT_DLLE explicit                    DefaultTargetLayerConfig       (uint32_t                                    layer);
    IMPORT_DLLE virtual                     ~DefaultTargetLayerConfig      ();

    IMPORT_DLLE                             DefaultTargetLayerConfig       (const DefaultTargetLayerConfig&         rhs);

    uint32_t                                    Get                            () const;
    };


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
