/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include <vcclr.h >
#using <mscorlib.dll>
#include "./DTMHelpers.h"
#include "./Bentley.Civil.DTM.h"
#include "./dtm.h"
#include "./dtmexception.h"
#include "TerrainModel/TerrainModel.h"
#include "TerrainModel/Drainage/drainage.h"
#include "DTMPond.h"

#if defined(Public)

#undef Public
#endif

#using <mscorlib.dll>

using namespace System;
using namespace System::Runtime::InteropServices;
namespace SRI = System::Runtime::InteropServices;


BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

///<summary>Pond Design Criteria - designs a pond given parameters</summary>
///<author>james.goode</author>
///<date>6/2011</date>

Bentley::TerrainModelNET::DTMPondDesignCriteria::DTMPondResult DTMPondDesignCriteria::CreatePond([Out] DTM^% pondDTM)
    {
    BcDTMP fillTargetP = nullptr;
    long numPoints = (long)m_points->Length;
    // Convert to Native
    pin_ptr<BGEO::DPoint3d const> tPoint = &m_points[0];
    if (m_targetDTM != nullptr)
        fillTargetP = m_targetDTM->Handle;

    // Create the Unmanaged pond Criteria
    DtmPondDesignCriteria pondDesignCrit((::DTMPondDesignMethod)m_designMethod, (::DPoint3d*)&tPoint[0], numPoints, SideSlope, FreeBoard, (::DTMPondTarget)m_pondTarget, m_targetElevation, m_targetVolume,
        IsBerm, BermSlope, BermWidth, IsCrown, CrownWidth, CornerStrokeTolerance, IsBermFillOnly, fillTargetP);

    ::DTMPondResult pondFlag;
    BcDTMPtr dtmP;
    double outPondElevation = 0.0, outPondVolume = 0.0;
    // Call the unmanaged function to get the result dtm. 
    pondFlag = pondDesignCrit.CreatePond(&outPondElevation, &outPondVolume, dtmP);
    if (dtmP != NULL)
        {
        pondDTM = gcnew DTM(dtmP.get());
        Bentley::TerrainModelNET::DTMPondDesignCriteria::AchievedPondElevation = outPondElevation;
        Bentley::TerrainModelNET::DTMPondDesignCriteria::AchievedPondVolume = outPondVolume;
        return (Bentley::TerrainModelNET::DTMPondDesignCriteria::DTMPondResult) (int (pondFlag));
        }
    else
        return Bentley::TerrainModelNET::DTMPondDesignCriteria::DTMPondResult::UnknownError;
    }


END_BENTLEY_TERRAINMODELNET_NAMESPACE
