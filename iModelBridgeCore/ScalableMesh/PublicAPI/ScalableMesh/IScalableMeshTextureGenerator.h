/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh\IScalableMesh.h>
#include <ScalableMesh/IScalableMeshCreator.h>
#include <Bentley\Bentley.h>
#include <Bentley/RefCounted.h>

#undef static_assert

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IScalableMeshTextureGenerator;

typedef RefCountedPtr<IScalableMeshTextureGenerator> IScalableMeshTextureGeneratorPtr;

/*=================================================================================**//**
* Interface implemented by MRDTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IScalableMeshTextureGenerator : virtual public RefCountedBase
    {
    private:        

    protected:                         
             
        //Synchonization with data sources functions        
        virtual StatusInt _SetPixelSize(double pixelSize) = 0;

        virtual StatusInt _SetTextureTempDir(const BeFileName& textureDir) = 0;

        virtual StatusInt _SetTransform(const Transform& transToUOR) = 0;
              
        virtual StatusInt _GenerateTexture(const bvector<DPoint3d>& area, IScalableMeshProgress* progress = nullptr) = 0;
          
    public:

        BENTLEY_SM_EXPORT StatusInt SetPixelSize(double pixelSize);

        BENTLEY_SM_EXPORT StatusInt SetTextureTempDir(const BeFileName& textureDir);

        BENTLEY_SM_EXPORT StatusInt SetTransform(const Transform& transToUOR);
              
        BENTLEY_SM_EXPORT StatusInt GenerateTexture(const bvector<DPoint3d>& area, IScalableMeshProgress* progress=nullptr);                        
    };


END_BENTLEY_SCALABLEMESH_NAMESPACE
