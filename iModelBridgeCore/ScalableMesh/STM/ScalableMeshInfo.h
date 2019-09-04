/*--------------------------------------------------------------------------------------+
|    $RCSfile: ScalableMesh.h,v $
|   $Revision: 1.54 $
|       $Date: 2012/01/06 16:30:13 $
|     $Author: Raymond.Gauthier $
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                        |
|    ScalableMeshNewFileCreator.h                              (C) Copyright 2001.        |
|                                                BCIVIL Corporation.        |
|                                                All Rights Reserved.    |
|                                                                       |
+----------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/IScalableMeshInfo.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*----------------------------------------------------------------------------+
|Class ScalableMeshBase
+----------------------------------------------------------------------------*/
struct ScalableMeshTextureInfo : public RefCounted<IScalableMeshTextureInfo>
    {
    private :

        SMTextureType m_textureType;
        bool          m_isUsingBingMap;
        bool          m_isTextureAvailable;
        WString       m_bingMapType;
      
        
    protected : 

        virtual WString       _GetBingMapsType() const override;

        virtual SMTextureType _GetTextureType() const override;

        virtual bool          _IsTextureAvailable() const override;

        virtual bool          _IsUsingBingMap() const override;

    public:

        void*   operator new(size_t size) { return bentleyAllocator_allocateRefCounted(size); }
        void    operator delete(void* rawMemory, size_t size) { bentleyAllocator_deleteRefCounted(rawMemory, size); }
        void*   operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted(size); }
        void    operator delete [](void* rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted(rawMemory, size); }

        ScalableMeshTextureInfo(SMTextureType textureType, bool isUsingBingMap, bool isTextureAvailable, const WString& bingMapType);
        
        static Byte* GetBingMapLogo(DPoint2d& bingMapLogoSize);
        
        //static ScalableMeshTextureInfo* Create(SMTextureType textureType, bool isUsingBingMap);  ScalableMeshTextureInfo(SMTextureType textureType, bool isUsingBingMap);   
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
