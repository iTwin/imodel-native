/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
static double s_defaultEqualPointTolerance      = 1.0e-7;
static double s_defaultMaxDirectAdjust          = 1.0e-4;
static double s_defaultMaxAdjustAlongCurve      = 1.0e-3;
static bool   s_defaultRemovePriorGapPrimitives = true;

/*=================================================================================**//**
*  options to gap closure
* @bsiclass
+===============+===============+===============+===============+===============+======*/
CurveGapOptions::CurveGapOptions ()
    {
    m_equalPointTolerance = s_defaultEqualPointTolerance;
    m_maxDirectAdjust = s_defaultMaxDirectAdjust;
    m_maxAdjustAlongCurve = s_defaultMaxAdjustAlongCurve;
    m_removePriorGapPrimitives = s_defaultRemovePriorGapPrimitives;

    memset (m_unusedDouble, 0, sizeof (m_unusedDouble));
    memset (m_unusedInt, 0, sizeof (m_unusedInt));
    memset (m_unusedBool, 0, sizeof (m_unusedBool));
    }

CurveGapOptions::CurveGapOptions (double equalPointTolerance, double maxDirectAdjust, double maxAdjustAlongCurve)
    {
    m_equalPointTolerance = equalPointTolerance;
    m_maxDirectAdjust = maxDirectAdjust;
    m_maxAdjustAlongCurve = maxAdjustAlongCurve;
    m_removePriorGapPrimitives = s_defaultRemovePriorGapPrimitives;

    memset (m_unusedDouble, 0, sizeof (m_unusedDouble));
    memset (m_unusedInt, 0, sizeof (m_unusedInt));
    memset (m_unusedBool, 0, sizeof (m_unusedBool));
    }

void CurveGapOptions::SetEqualPointTolerance (double value){m_equalPointTolerance = value;}
void CurveGapOptions::SetMaxDirectAdjustTolerance (double value) {m_maxDirectAdjust = value;}
void CurveGapOptions::SetMaxAdjustAlongCurve (double value) {m_maxAdjustAlongCurve = value;}
void CurveGapOptions::SetRemovePriorGapPrimitives (bool value) {m_removePriorGapPrimitives = value;}

double CurveGapOptions::GetEqualPointTolerance () const {return m_equalPointTolerance;}
double CurveGapOptions::GetMaxDirectAdjustTolerance () const {return m_maxDirectAdjust;}
double CurveGapOptions::GetMaxAdjustAlongCurve () const {return m_maxAdjustAlongCurve;}
bool CurveGapOptions::GetRemovePriorGapPrimitives () const {return m_removePriorGapPrimitives;}

END_BENTLEY_GEOMETRY_NAMESPACE
