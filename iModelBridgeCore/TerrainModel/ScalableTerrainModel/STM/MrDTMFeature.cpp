/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMFeature.cpp $
|    $RCSfile: MrDTMFeature.cpp,v $
|   $Revision: 1.3 $
|       $Date: 2013/10/04 17:16:07 $
|     $Author: Richard.Bois $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableTerrainModelPCH.h>

#include "MrDTMFeature.h"

BEGIN_BENTLEY_MRDTM_NAMESPACE

/*----------------------------------------------------------------------------+
|IMrDTMFeature Method Definition Section - Begin
+----------------------------------------------------------------------------*/

IMrDTMFeaturePtr IMrDTMFeature::Create ()
    {
    return new MrDTMFeature();
    }

IMrDTMFeaturePtr IMrDTMFeature::CreateFor (const size_t& type, const DPoint2d* pointsPtr, const size_t& numPoints)
    {
    return new MrDTMFeature(type, pointsPtr, numPoints);
    }

size_t IMrDTMFeature::GetType() const
    {
    return _GetType();
    }

size_t IMrDTMFeature::GetSize() const
    {
    return _GetSize();
    }

DPoint2d IMrDTMFeature::GetPoint(size_t idx) const
    {
    return _GetPoint(idx);
    }

void IMrDTMFeature::SetType (const size_t type)
    {
    _SetType (type);
    }

void IMrDTMFeature::AppendPoint (const DPoint2d& point)
    {
    _AppendPoint (point);
    }

/*----------------------------------------------------------------------------+
|MrDTMFeature Method Definition Section - End
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|MrDTMFeature Method Definition Section - Begin
+----------------------------------------------------------------------------*/
MrDTMFeature::MrDTMFeature()
    {
    }

MrDTMFeature::~MrDTMFeature()
    {
    }

MrDTMFeature::MrDTMFeature(const size_t& type, const DPoint2d* pointsPtr, const size_t& numPoints)
: m_type (type)
    {
    m_points.resize(numPoints);
    for (size_t i = 0; i<numPoints; i++)
        m_points[i] = pointsPtr[i];
    }

size_t MrDTMFeature::_GetType() const
    {
    return m_type;
    }

size_t MrDTMFeature::_GetSize() const
    {
    return m_points.size();
    }

DPoint2d MrDTMFeature::_GetPoint(size_t idx) const
    {
    return m_points[idx];
    }

void MrDTMFeature::_SetType (const size_t type)
    {
    m_type = type;
    }

void MrDTMFeature::_AppendPoint (const DPoint2d& point)
    {
    m_points.push_back(point);
    }

/*----------------------------------------------------------------------------+
|MrDTMFeature Method Definition Section - End
+----------------------------------------------------------------------------*/

END_BENTLEY_MRDTM_NAMESPACE