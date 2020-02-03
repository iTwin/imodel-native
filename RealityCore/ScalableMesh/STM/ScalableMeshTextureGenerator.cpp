/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
  
#include <ScalableMeshPCH.h>

#include <ScalableMesh/IScalableMeshTextureGenerator.h>



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
