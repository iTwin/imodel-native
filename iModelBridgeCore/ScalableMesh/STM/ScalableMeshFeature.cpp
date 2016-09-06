/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshFeature.cpp $
|    $RCSfile: ScalableMeshFeature.cpp,v $
|   $Revision: 1.3 $
|       $Date: 2013/10/04 17:16:07 $
|     $Author: Richard.Bois $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"
#include "ScalableMeshFeature.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*----------------------------------------------------------------------------+
|IScalableMeshFeature Method Definition Section - Begin
+----------------------------------------------------------------------------*/

IScalableMeshFeaturePtr IScalableMeshFeature::Create ()
    {
    return new ScalableMeshFeature();
    }

IScalableMeshFeaturePtr IScalableMeshFeature::CreateFor (const size_t& type, const DPoint2d* pointsPtr, const size_t& numPoints)
    {
    return new ScalableMeshFeature(type, pointsPtr, numPoints);
    }

size_t IScalableMeshFeature::GetType() const
    {
    return _GetType();
    }

size_t IScalableMeshFeature::GetSize() const
    {
    return _GetSize();
    }

DPoint2d IScalableMeshFeature::GetPoint(size_t idx) const
    {
    return _GetPoint(idx);
    }

void IScalableMeshFeature::SetType (const size_t type)
    {
    _SetType (type);
    }

void IScalableMeshFeature::AppendPoint (const DPoint2d& point)
    {
    _AppendPoint (point);
    }

/*----------------------------------------------------------------------------+
|ScalableMeshFeature Method Definition Section - End
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|ScalableMeshFeature Method Definition Section - Begin
+----------------------------------------------------------------------------*/
ScalableMeshFeature::ScalableMeshFeature()
    {
    }

ScalableMeshFeature::~ScalableMeshFeature()
    {
    }

ScalableMeshFeature::ScalableMeshFeature(const size_t& type, const DPoint2d* pointsPtr, const size_t& numPoints)
: m_type (type)
    {
    m_points.resize(numPoints);
    for (size_t i = 0; i<numPoints; i++)
        m_points[i] = pointsPtr[i];
    }

size_t ScalableMeshFeature::_GetType() const
    {
    return m_type;
    }

size_t ScalableMeshFeature::_GetSize() const
    {
    return m_points.size();
    }

DPoint2d ScalableMeshFeature::_GetPoint(size_t idx) const
    {
    return m_points[idx];
    }

void ScalableMeshFeature::_SetType (const size_t type)
    {
    m_type = type;
    }

void ScalableMeshFeature::_AppendPoint (const DPoint2d& point)
    {
    m_points.push_back(point);
    }

/*----------------------------------------------------------------------------+
|ScalableMeshFeature Method Definition Section - End
+----------------------------------------------------------------------------*/

END_BENTLEY_SCALABLEMESH_NAMESPACE
