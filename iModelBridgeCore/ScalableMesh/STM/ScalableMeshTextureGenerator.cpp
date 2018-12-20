/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshTextureGenerator.cpp $
|    $RCSfile: ScalableMesh.cpp,v $
|   $Revision: 1.106 $
|       $Date: 2012/01/06 16:30:15 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
  
#include <ScalableMeshPCH.h>

#include <ScalableMesh\IScalableMeshTextureGenerator.h>



BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*----------------------------------------------------------------------------+
|IScalableMeshTextureGenerator - Begin
+----------------------------------------------------------------------------*/
StatusInt IScalableMeshTextureGenerator::SetPixelSize(double pixelSize)
    {
    return _SetPixelSize(pixelSize);
    }

StatusInt IScalableMeshTextureGenerator::SetTextureTempDir(const BeFileName& textureDir)
    {
    return _SetTextureTempDir(textureDir);
    }
              
StatusInt IScalableMeshTextureGenerator::GenerateTexture(const bvector<DPoint3d>& area,IScalableMeshProgress* progress)
    {
    return _GenerateTexture(area, progress);
    }

StatusInt IScalableMeshTextureGenerator::SetTransform(const Transform& transToUOR)
    {
    return _SetTransform(transToUOR);
    }
 
/*----------------------------------------------------------------------------+
|IScalableMeshTextureGenerator Method Definition Section - End
+----------------------------------------------------------------------------*/

END_BENTLEY_SCALABLEMESH_NAMESPACE
