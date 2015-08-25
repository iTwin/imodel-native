/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Config/DefaultTargetScalableMeshConfig.h $
|    $RCSfile: DefaultTargetExtent.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/11/22 21:57:53 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh/Import/Definitions.h>
#include <ScalableMesh/Import/Config/Base.h>
#include <ScalableMesh/Import/ScalableMeshData.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DefaultTargetScalableMeshConfig : public ImportConfigComponentMixinBase < DefaultTargetScalableMeshConfig >
{
private:
    ScalableMeshData                    m_data;
    void*                                   m_implP; // Reserved some space for further use

public:
    IMPORT_DLLE static ClassID              s_GetClassID();

    IMPORT_DLLE explicit                    DefaultTargetScalableMeshConfig(const ScalableMeshData&                data);
    IMPORT_DLLE virtual                     ~DefaultTargetScalableMeshConfig();

    IMPORT_DLLE                             DefaultTargetScalableMeshConfig(const DefaultTargetScalableMeshConfig&          rhs);

    const ScalableMeshData&                 Get() const;
};


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
