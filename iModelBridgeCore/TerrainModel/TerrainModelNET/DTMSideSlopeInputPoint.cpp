/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTMSideSlopeInputPoint.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include <vcclr.h >
#using <mscorlib.dll>
#include ".\DTMSideSlopeInputPoint.h"

BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
DTMSideSlopeInputPoint::DTMSideSlopeInputPoint(BGEO::DPoint3d startPoint, DTM^ slopeToDTM, double cutSlope, double fillSlope)
    {
    m_startPoint = startPoint;
    m_slopeToDTM = slopeToDTM;
    m_cutSlope = cutSlope;
    m_fillSlope = fillSlope;
    }

//=======================================================================================
// @bsimethod                                            Rob.Cormack        02/2012
//+===============+===============+===============+===============+===============+======
DTMSideSlopeInputPoint::DTMSideSlopeInputPoint(BGEO::DPoint3d startPoint,DTMSideSlopeRadialOption radialOption, DTM^ slopeToDTM, double cutSlope, double fillSlope)
    {
    m_startPoint = startPoint;
    m_radialOption = radialOption ;
    m_slopeToDTM = slopeToDTM;
    m_cutSlope = cutSlope;
    m_fillSlope = fillSlope;
    }

//=======================================================================================
// @bsimethod                                            Rob.Cormack      06/2011
//+===============+===============+===============+===============+===============+======
DTMSideSlopeInputPoint::DTMSideSlopeInputPoint(BGEO::DPoint3d startPoint,DTMSideSlopeRadialOption radialOption,double elevation,double cutSlope, double fillSlope)
    {
    m_startPoint = startPoint;
    m_cutSlope = cutSlope;
    m_fillSlope = fillSlope;
    m_toElevation = elevation ;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
BGEO::DPoint3d DTMSideSlopeInputPoint::StartPoint::get()
    {
    return m_startPoint;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
DTM^ DTMSideSlopeInputPoint::SlopeToDTM::get()
    {
    return m_slopeToDTM;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
DTM^ DTMSideSlopeInputPoint::CutFillDTM::get()
    {
    return m_cutFillDTM;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
void DTMSideSlopeInputPoint::CutFillDTM::set(DTM^ val)
    {
    m_cutFillDTM = val;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
DTMSideSlopeRadialOption DTMSideSlopeInputPoint::RadialOption::get()
    {
    return m_radialOption;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
void DTMSideSlopeInputPoint::RadialOption::set(DTMSideSlopeRadialOption val)
    {
    m_radialOption = val;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
DTMSideSlopeOption DTMSideSlopeInputPoint::SideSlopeOption::get()
    {
    return m_option;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
void DTMSideSlopeInputPoint::SideSlopeOption::set(DTMSideSlopeOption val)
    {
    m_option = val;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
double DTMSideSlopeInputPoint::ToElevation::get()
    {
    return m_toElevation;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
void DTMSideSlopeInputPoint::ToElevation::set(double val)
    {
    m_toElevation = val;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
double DTMSideSlopeInputPoint::ToDeltaElevation::get()
    {
    return m_toDeltaElevation;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
void DTMSideSlopeInputPoint::ToDeltaElevation::set(double val)
    {
    m_toDeltaElevation = val;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
double DTMSideSlopeInputPoint::ToHorizontalDistance::get()
    {
    return m_toHorizontalDistance;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
void DTMSideSlopeInputPoint::ToHorizontalDistance::set(double val)
    {
    m_toHorizontalDistance = val;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
double DTMSideSlopeInputPoint::CutSlope::get()
    {
    return m_cutSlope;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
double DTMSideSlopeInputPoint::FillSlope::get()
    {
    return m_fillSlope;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
DTMSideSlopeCutFillOption DTMSideSlopeInputPoint::CutFillSlopeOption::get()
    {
    return m_cutFillOption;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
void DTMSideSlopeInputPoint::CutFillSlopeOption::set(DTMSideSlopeCutFillOption val)
    {
    m_cutFillOption = val;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
bool DTMSideSlopeInputPoint::IsSlopeForced::get()
    {
    return m_isSlopeForced;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
void DTMSideSlopeInputPoint::IsSlopeForced::set(bool val)
    {
    m_isSlopeForced = val;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
double DTMSideSlopeInputPoint::ForcedSlope::get()
    {
    return m_forcedSlope;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
void DTMSideSlopeInputPoint::ForcedSlope::set(double val)
    {
    m_forcedSlope = val;
    }


END_BENTLEY_TERRAINMODELNET_NAMESPACE
