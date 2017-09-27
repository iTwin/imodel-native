/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <ScalableMesh/ScalableMeshDefs.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IScalableMeshTextureInfo;

typedef RefCountedPtr<IScalableMeshTextureInfo> IScalableMeshTextureInfoPtr;

/*=================================================================================**//**
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IScalableMeshTextureInfo abstract : virtual public RefCountedBase
    {
    private:

    protected:

        virtual SMTextureType _GetTextureType() const = 0;
        
        virtual bool          _IsUsingBingMap() const = 0;

    public:

        BENTLEY_SM_EXPORT SMTextureType GetTextureType() const;

        BENTLEY_SM_EXPORT bool          IsUsingBingMap() const;        

        BENTLEY_SM_EXPORT static const Byte* GetBingMapLogo(DPoint2d& bingMapLogoSize);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
