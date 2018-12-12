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

#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>

#undef static_assert

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct ITextureProvider;

typedef RefCountedPtr<ITextureProvider> ITextureProviderPtr;


struct ITextureProvider  : virtual public RefCountedBase
    {
    private:        

    protected:                         
             
        virtual DPoint2d _GetMinPixelSize() = 0;

        virtual DRange2d _GetTextureExtent() = 0;

        virtual StatusInt _GetTextureForArea(bvector<uint8_t>& texData, int width, int height, DRange2d& area) = 0;
          
    public:

        BENTLEY_SM_EXPORT DPoint2d GetMinPixelSize();

        BENTLEY_SM_EXPORT DRange2d GetTextureExtent();

        BENTLEY_SM_EXPORT StatusInt GetTextureForArea(bvector<uint8_t>& texData, int width, int height, DRange2d& area);
    };


END_BENTLEY_SCALABLEMESH_NAMESPACE