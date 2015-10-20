/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTMTinEditor.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "DTMTinEditor.h"
#include "DTMException.h"
BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     4/2008
//=======================================================================================
DTMTinEditor::~DTMTinEditor(void)
    {
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     4/2008
//=======================================================================================
BcDTMP DTMTinEditor::BcDTM::get()
    {
    if (m_nativeDtm == NULL)
        {
        throw ThrowingPolicy::Apply (gcnew System::ArgumentNullException("m_nativeDtm"));
        }
    return m_nativeDtm;
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     4/2008
//=======================================================================================
bool DTMTinEditor::Select(DTMDynamicFeatureType featureType, BGEO::DPoint3d point)
    {
    DPoint3d pt;
    bool featureFound;
    bvector<DPoint3d> points;
    DTMHelpers::Copy (pt, point);

    DTMException::CheckForErrorStatus (BcDTM->EditorSelectDtmTinFeature ((::DTMFeatureType)featureType, pt, featureFound, points));

    if(featureFound)
        {
        m_featurePoints = gcnew array<BGEO::DPoint3d> ((int)points.size());
        interior_ptr<BGEO::DPoint3d> mPoints = &m_featurePoints[0];
        DPoint3d* endPt = &points[points.size() - 1];
        for (DPoint3d* pt = &points[0]; pt <= endPt; ++pt, ++mPoints)
            DTMHelpers::Copy (*mPoints, *pt);

        }
    else
        {
        m_featurePoints = nullptr;
        }
    return featureFound != 0;
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     4/2008
//=======================================================================================
bool DTMTinEditor::Delete()
    {
    long success;
    DTMException::CheckForErrorStatus (BcDTM->EditorDeleteDtmTinFeature(&success));
    return success != 0;
    }

END_BENTLEY_TERRAINMODELNET_NAMESPACE
