/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshClipContainer.h $
|    $RCSfile: ScalableMeshClipContainer.h,v $
|   $Revision: 1.4 $
|       $Date: 2013/07/22 13:57:39 $
|     $Author: Richard.Bois $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh\IScalableMeshClipContainer.h>

typedef vector<DPoint3d> ScalableMeshClipPointContainer;

struct ScalableMeshClipInfo : public IScalableMeshClipInfo
    {
    
    private : 

        ScalableMeshClipPointContainer m_points;
        bool                    m_isClipMask;

    protected : 

        virtual DPoint3d* _GetClipPoints() override;

        virtual size_t _GetNbClipPoints() const override;

        virtual bool _IsClipMask() const override;

    public :         

        ScalableMeshClipInfo (DPoint3d* clipPointsP, size_t numberOfPoints, bool isClipMask);

        ~ScalableMeshClipInfo();
    };


typedef vector<IScalableMeshClipInfoPtr> ScalableMeshClipInfoVector;


struct ScalableMeshClipContainer : public IScalableMeshClipContainer
    {
    private :

        ScalableMeshClipInfoVector m_clipInfoVector;

    protected : 

        virtual int _AddClip(IScalableMeshClipInfoPtr& clipInfo) override;
                
        virtual size_t _GetNbClips() const override;

        virtual  int _GetClip(IScalableMeshClipInfoPtr& clipInfo, size_t clipInd) const override;

        virtual int _RemoveClip(size_t clipInd) override;

    public : 

        ScalableMeshClipContainer();

        virtual ~ScalableMeshClipContainer();
    };


