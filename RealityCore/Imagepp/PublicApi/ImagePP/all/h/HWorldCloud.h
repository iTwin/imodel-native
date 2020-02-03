//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <ImagePP/all/h/HGF2DWorld.h>
#include <ImagePP/all/h/HGF2DStretch.h>

//
// Temp World .....
//
BEGIN_IMAGEPP_NAMESPACE
class HWorldCloud
    {

public:
    HWorldCloud();

    HFCPtr<HGF2DCoordSys>&  GetCoordSys(HGF2DWorldIdentificator pi_WorldID);
private:

    HFCPtr<HGF2DCoordSys>   m_pUnKnownCoordSys;
    HFCPtr<HGF2DCoordSys>   m_pHMRCoordSys;
    HFCPtr<HGF2DCoordSys>   m_pDGNCoordSys;

    };

inline HWorldCloud::HWorldCloud()
    {
    m_pUnKnownCoordSys = new HGF2DCoordSys();

    // HMR CoordSys, Origin Bottom-Left
    HGF2DStretch NewModel();
    NewModel.SetYScaling(-1.0);
    m_pHMRCoordSys = new HGF2DCoordSys(NewModel, m_pUnKnownCoordSys);

    // DGN CoordSys, Origin Bottom-Left
    HGF2DStretch NewModel2();
    m_pDGNCoordSys = new HGF2DCoordSys(NewModel2, m_pUnKnownCoordSys);
    }

inline HFCPtr<HGF2DCoordSys>& HWorldCloud::GetCoordSys(HGF2DWorldIdentificator pi_WorldID)
    {
    switch (pi_WorldID)
        {
        case HGF2DWorld_HMRWORLD:
            return m_pHMRCoordSys;
            break;

        case HGF2DWorld_DGNWORLD:
            return m_pDGNCoordSys;
            break;

        default:
            return m_pUnKnownCoordSys;
            break;
        }
    }
END_IMAGEPP_NAMESPACE
