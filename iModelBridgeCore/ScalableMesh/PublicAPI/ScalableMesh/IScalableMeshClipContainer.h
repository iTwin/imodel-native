/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshClipContainer.h $
|    $RCSfile: ScalableMeshClipContainer.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/08/02 14:59:53 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

typedef vector<DPoint3d> ScalableMeshClipPointContainer;

struct IScalableMeshClipInfo;
typedef RefCountedPtr<IScalableMeshClipInfo> IScalableMeshClipInfoPtr;

struct IScalableMeshClipInfo : public RefCountedBase
    {

    protected : 

        IScalableMeshClipInfo ();

        virtual ~IScalableMeshClipInfo();

        virtual DPoint3d* _GetClipPoints() = 0;

        virtual size_t _GetNbClipPoints() const = 0;    

        virtual bool _IsClipMask() const = 0;

    public : 

        BENTLEYSTM_EXPORT DPoint3d* GetClipPoints();

        BENTLEYSTM_EXPORT size_t GetNbClipPoints() const;

        BENTLEYSTM_EXPORT bool IsClipMask() const;
            
        BENTLEYSTM_EXPORT static IScalableMeshClipInfoPtr Create(DPoint3d* clipPointsP, size_t numberOfPoints, bool isClipMask);
    };

struct IScalableMeshClipContainer;
typedef RefCountedPtr<IScalableMeshClipContainer> IScalableMeshClipContainerPtr;

struct IScalableMeshClipContainer : public RefCountedBase
    {

    protected : 

        IScalableMeshClipContainer ();

        virtual ~IScalableMeshClipContainer();

        virtual int _AddClip(IScalableMeshClipInfoPtr& clipInfo) = 0;        
        
        virtual size_t _GetNbClips() const = 0;

        virtual int _GetClip(IScalableMeshClipInfoPtr& clipInfo, size_t clipInd) const = 0;

        virtual int _RemoveClip(size_t toRemoveClipInd) = 0;

    public : 
            
        BENTLEYSTM_EXPORT int AddClip(IScalableMeshClipInfoPtr& clipInfo);
        
        BENTLEYSTM_EXPORT size_t GetNbClips() const;

        BENTLEYSTM_EXPORT int GetClip(IScalableMeshClipInfoPtr& clipInfo, size_t clipInd) const;

        BENTLEYSTM_EXPORT int RemoveClip(size_t toRemoveClipInd);

        BENTLEYSTM_EXPORT static IScalableMeshClipContainerPtr Create();
    };
