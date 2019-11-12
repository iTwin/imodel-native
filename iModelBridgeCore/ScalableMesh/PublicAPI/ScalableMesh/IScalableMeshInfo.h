/*--------------------------------------------------------------------------------------+
|
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
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
struct IScalableMeshTextureInfo: virtual public RefCountedBase
    {
    private:

    protected:

        virtual WString       _GetBingMapsType() const = 0;

        virtual SMTextureType _GetTextureType() const = 0;

        virtual bool          _IsTextureAvailable() const = 0;
        
        virtual bool          _IsUsingBingMap() const = 0;        
        
    public:

        BENTLEY_SM_EXPORT WString       GetBingMapsType() const;
      
        BENTLEY_SM_EXPORT SMTextureType GetTextureType() const;

        BENTLEY_SM_EXPORT bool          IsTextureAvailable() const;

        BENTLEY_SM_EXPORT bool          IsUsingBingMap() const;
                   
        BENTLEY_SM_EXPORT static const Byte* GetBingMapLogo(DPoint2d& bingMapLogoSize);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
