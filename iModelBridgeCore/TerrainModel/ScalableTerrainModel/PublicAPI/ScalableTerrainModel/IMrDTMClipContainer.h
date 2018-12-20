/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/IMrDTMClipContainer.h $
|    $RCSfile: MrDTMClipContainer.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/08/02 14:59:53 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

typedef vector<DPoint3d> MrDTMClipPointContainer;

struct IMrDTMClipInfo;
typedef RefCountedPtr<IMrDTMClipInfo> IMrDTMClipInfoPtr;

struct IMrDTMClipInfo : public RefCountedBase
    {

    protected : 

        IMrDTMClipInfo ();

        virtual ~IMrDTMClipInfo();

        virtual DPoint3d* _GetClipPoints() = 0;

        virtual size_t _GetNbClipPoints() const = 0;    

        virtual bool _IsClipMask() const = 0;

    public : 

        BENTLEYSTM_EXPORT DPoint3d* GetClipPoints();

        BENTLEYSTM_EXPORT size_t GetNbClipPoints() const;

        BENTLEYSTM_EXPORT bool IsClipMask() const;
            
        BENTLEYSTM_EXPORT static IMrDTMClipInfoPtr Create(DPoint3d* clipPointsP, size_t numberOfPoints, bool isClipMask);
    };

struct IMrDTMClipContainer;
typedef RefCountedPtr<IMrDTMClipContainer> IMrDTMClipContainerPtr;

struct IMrDTMClipContainer : public RefCountedBase
    {

    protected : 

        IMrDTMClipContainer ();

        virtual ~IMrDTMClipContainer();

        virtual int _AddClip(IMrDTMClipInfoPtr& clipInfo) = 0;        
        
        virtual size_t _GetNbClips() const = 0;

        virtual int _GetClip(IMrDTMClipInfoPtr& clipInfo, size_t clipInd) const = 0;

        virtual int _RemoveClip(size_t toRemoveClipInd) = 0;

    public : 
            
        BENTLEYSTM_EXPORT int AddClip(IMrDTMClipInfoPtr& clipInfo);
        
        BENTLEYSTM_EXPORT size_t GetNbClips() const;

        BENTLEYSTM_EXPORT int GetClip(IMrDTMClipInfoPtr& clipInfo, size_t clipInd) const;

        BENTLEYSTM_EXPORT int RemoveClip(size_t toRemoveClipInd);

        BENTLEYSTM_EXPORT static IMrDTMClipContainerPtr Create();
    };
