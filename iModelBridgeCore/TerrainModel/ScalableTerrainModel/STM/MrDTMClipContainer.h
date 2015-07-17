/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMClipContainer.h $
|    $RCSfile: MrDTMClipContainer.h,v $
|   $Revision: 1.4 $
|       $Date: 2013/07/22 13:57:39 $
|     $Author: Richard.Bois $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableTerrainModel\IMrDTMClipContainer.h>

typedef vector<DPoint3d> MrDTMClipPointContainer;

struct MrDTMClipInfo : public IMrDTMClipInfo
    {
    
    private : 

        MrDTMClipPointContainer m_points;
        bool                    m_isClipMask;

    protected : 

        virtual DPoint3d* _GetClipPoints() override;

        virtual size_t _GetNbClipPoints() const override;

        virtual bool _IsClipMask() const override;

    public :         

        MrDTMClipInfo (DPoint3d* clipPointsP, size_t numberOfPoints, bool isClipMask);

        ~MrDTMClipInfo();
    };


typedef vector<IMrDTMClipInfoPtr> MrDTMClipInfoVector;


struct MrDTMClipContainer : public IMrDTMClipContainer
    {
    private :

        MrDTMClipInfoVector m_clipInfoVector;

    protected : 

        virtual int _AddClip(IMrDTMClipInfoPtr& clipInfo) override;
                
        virtual size_t _GetNbClips() const override;

        virtual  int _GetClip(IMrDTMClipInfoPtr& clipInfo, size_t clipInd) const override;

        virtual int _RemoveClip(size_t clipInd) override;

    public : 

        MrDTMClipContainer();

        virtual ~MrDTMClipContainer();
    };


