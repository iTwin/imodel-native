
|
|     $Source: TerrainModelNET/DTMSideSlopeInput.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include <vcclr.h >
#using <mscorlib.dll>
#include ".\DTMHelpers.h"
#include ".\DTMSideSlopeInput.h"
#include ".\Bentley.Civil.DTM.h"
#include ".\dtm.h"
#include ".\dtmexception.h"
#include "TerrainModel\TerrainModel.h"
#include "TerrainModel\Core\bcdtmSideSlope.h"
#include "TerrainModel\Core\TMTransformHelper.h"

BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE


static StatusInt bcCalculateSideSlopes
(
DTM_SIDE_SLOPE_TABLE **sideSlopeTablePP,
long *sideSlopeTableSizeP,
long sideSlopeDirection,
DTM_SLOPE_TABLE *slopeTableP,
long slopeTableSize,
long cornerOption,
long strokeCornerOption,
double cornerStrokeTolerance,
double p2pTol,
DPoint3d* parallelEdgePtsP,
long numParallelEdgePts,
DTMUserTag userRadialTag,
DTMUserTag userElementTag,
BC_DTM_OBJ* **dtmSideSlopesPP,
long *numberOfDtmSideSlopesP
)
    {
    int status = bcdtmSideSlope_createSideSlopesForSideSlopeTableDtmObject(sideSlopeTablePP, sideSlopeTableSizeP, sideSlopeDirection, slopeTableP, slopeTableSize, cornerOption, strokeCornerOption, cornerStrokeTolerance, p2pTol, (DPoint3d *)parallelEdgePtsP, numParallelEdgePts, userRadialTag, userElementTag, dtmSideSlopesPP, numberOfDtmSideSlopesP);
    if (status) return DTM_ERROR;
    else         return DTM_SUCCESS;
    return DTM_ERROR;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
DTMSideSlopeInput::DTMSideSlopeInput (DTMSideSlopeDirection direction, DTMSideSlopeCornerOption cornerOption, DTMSideSlopeStrokeCornerOption strokeCornerOption, double cornerStrokeTolearance, double pointToPointTolerance, DTMUserTag breakTag, DTMUserTag sideSlopeTag)
    {
    m_slopeTable = nullptr;
    m_direction = direction;
    m_cornerOption = cornerOption;
    m_strokeCornerOption = strokeCornerOption;
    m_cornerStrokeTolearance = cornerStrokeTolearance;
    m_pointToPointTolerance = pointToPointTolerance;
    m_breakLineUserTag = breakTag;
    m_sideSlopeElementUserTag = sideSlopeTag;
    }
//=======================================================================================
// @bsimethod                                            Rob.Cormack     07/2011
//+===============+===============+===============+===============+===============+======
DTMSideSlopeInput::DTMSideSlopeInput (DTMSlopeTable^ slopeTable, DTMSideSlopeDirection direction, DTMSideSlopeCornerOption cornerOption, DTMSideSlopeStrokeCornerOption strokeCornerOption, double cornerStrokeTolearance, double pointToPointTolerance, DTMUserTag breakTag, DTMUserTag sideSlopeTag)
    {
    m_slopeTable = slopeTable;
    m_direction = direction;
    m_cornerOption = cornerOption;
    m_strokeCornerOption = strokeCornerOption;
    m_cornerStrokeTolearance = cornerStrokeTolearance;
    m_pointToPointTolerance = pointToPointTolerance;
    m_breakLineUserTag = breakTag;
    m_sideSlopeElementUserTag = sideSlopeTag;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
DTMSideSlopeInputPoint^ DTMSideSlopeInput::AddPoint (BGEO::DPoint3d startPoint, DTM^ slopeToDTM, double cutSlope, double fillSlope)
    {
    DTMSideSlopeInputPoint^ sideSlopeInputPoint = gcnew DTMSideSlopeInputPoint (startPoint, slopeToDTM, cutSlope, fillSlope);

    InnerList->Add (sideSlopeInputPoint);

    return sideSlopeInputPoint;
    }

//=======================================================================================
// @bsimethod                                            Rob.Cormack      03/2012
//+===============+===============+===============+===============+===============+======

DTMSideSlopeInputPoint^ DTMSideSlopeInput::AddRadialToSurface (BGEO::DPoint3d startPoint, DTMSideSlopeRadialOption radialOption, DTMSideSlopeCutFillOption cutFillOption, DTM^ slopeToDTM, double cutSlope, double fillSlope)
    {
    bool dbg = false;
    if (dbg)bcdtmWrite_message (0, 0, 0, "Adding Radial To Surface");
    DTMSideSlopeInputPoint^ sideSlopeInputPoint = gcnew DTMSideSlopeInputPoint (startPoint, slopeToDTM, cutSlope, fillSlope);
    sideSlopeInputPoint->SideSlopeOption = DTMSideSlopeOption::ToTin;
    sideSlopeInputPoint->RadialOption = radialOption;
    sideSlopeInputPoint->CutFillDTM = nullptr;
    sideSlopeInputPoint->CutFillSlopeOption = cutFillOption;
    if (cutFillOption == DTMSideSlopeCutFillOption::CutOnly || cutFillOption == DTMSideSlopeCutFillOption::FillOnly)
        {
        sideSlopeInputPoint->CutFillDTM = slopeToDTM;
        }
    InnerList->Add (sideSlopeInputPoint);
    return sideSlopeInputPoint;
    }

//=======================================================================================
// @bsimethod                                            Rob.Cormack      03/2012
//+===============+===============+===============+===============+===============+======

DTMSideSlopeInputPoint^ DTMSideSlopeInput::AddRadialToSurfaceWithElevationLimit
(
BGEO::DPoint3d startPoint,
DTMSideSlopeRadialOption radialOption,
DTMSideSlopeCutFillOption cutFillOption,
DTM^ slopeToDTM,
double cutSlope,
double fillSlope,
double elevation
)
    {
    bool dbg = false;
    if (dbg)bcdtmWrite_message (0, 0, 0, "Adding Radial To Surface With Elevation");
    DTMSideSlopeInputPoint^ sideSlopeInputPoint = gcnew DTMSideSlopeInputPoint (startPoint, slopeToDTM, cutSlope, fillSlope);
    sideSlopeInputPoint->SideSlopeOption = DTMSideSlopeOption::ToTinOrElevation;
    sideSlopeInputPoint->RadialOption = radialOption;
    sideSlopeInputPoint->CutFillDTM = nullptr;
    sideSlopeInputPoint->CutFillSlopeOption = cutFillOption;
    if (cutFillOption == DTMSideSlopeCutFillOption::CutOnly || cutFillOption == DTMSideSlopeCutFillOption::FillOnly)
        {
        sideSlopeInputPoint->CutFillDTM = slopeToDTM;
        }
    sideSlopeInputPoint->ToElevation = elevation;
    InnerList->Add (sideSlopeInputPoint);
    return sideSlopeInputPoint;
    }


//=======================================================================================
// @bsimethod                                            Rob.Cormack      03/2012
//+===============+===============+===============+===============+===============+======

DTMSideSlopeInputPoint^ DTMSideSlopeInput::AddRadialToSurfaceWithVerticalDistanceLimit
(
BGEO::DPoint3d startPoint,
DTMSideSlopeRadialOption radialOption,
DTMSideSlopeCutFillOption cutFillOption,
DTM^ slopeToDTM,
double cutSlope,
double fillSlope,
double verticalDistance
)
    {
    bool dbg = false;
    if (dbg)bcdtmWrite_message (0, 0, 0, "Adding Radial To Surface With Vertical Distance Limit");
    DTMSideSlopeInputPoint^ sideSlopeInputPoint = gcnew DTMSideSlopeInputPoint (startPoint, slopeToDTM, cutSlope, fillSlope);
    sideSlopeInputPoint->SideSlopeOption = DTMSideSlopeOption::ToTinOrVerticalDistance;
    sideSlopeInputPoint->RadialOption = radialOption;
    sideSlopeInputPoint->CutFillDTM = nullptr;
    sideSlopeInputPoint->CutFillSlopeOption = cutFillOption;
    if (cutFillOption == DTMSideSlopeCutFillOption::CutOnly || cutFillOption == DTMSideSlopeCutFillOption::FillOnly)
        {
        sideSlopeInputPoint->CutFillDTM = slopeToDTM;
        }
    sideSlopeInputPoint->ToDeltaElevation = verticalDistance;
    InnerList->Add (sideSlopeInputPoint);
    return sideSlopeInputPoint;
    }

//=======================================================================================
// @bsimethod                                            Rob.Cormack      03/2012
//+===============+===============+===============+===============+===============+======

DTMSideSlopeInputPoint^ DTMSideSlopeInput::AddRadialToSurfaceWithHorizontalLimit
(
BGEO::DPoint3d startPoint,
DTMSideSlopeRadialOption radialOption,
DTMSideSlopeCutFillOption cutFillOption,
DTM^ slopeToDTM,
double cutSlope,
double fillSlope,
double horizontalDistance
)
    {
    bool dbg = false;
    if (dbg)bcdtmWrite_message (0, 0, 0, "Adding Radial To Surface With Horizontal Limit");
    DTMSideSlopeInputPoint^ sideSlopeInputPoint = gcnew DTMSideSlopeInputPoint (startPoint, slopeToDTM, cutSlope, fillSlope);
    sideSlopeInputPoint->SideSlopeOption = DTMSideSlopeOption::ToTinOrHorizontalDistance;
    sideSlopeInputPoint->RadialOption = radialOption;
    sideSlopeInputPoint->CutFillDTM = nullptr;
    sideSlopeInputPoint->CutFillSlopeOption = cutFillOption;
    if (cutFillOption == DTMSideSlopeCutFillOption::CutOnly || cutFillOption == DTMSideSlopeCutFillOption::FillOnly)
        {
        sideSlopeInputPoint->CutFillDTM = slopeToDTM;
        }
    sideSlopeInputPoint->ToHorizontalDistance = horizontalDistance;
    InnerList->Add (sideSlopeInputPoint);
    return sideSlopeInputPoint;
    }

//=======================================================================================
// @bsimethod                                            Rob.Cormack      03/2012
//+===============+===============+===============+===============+===============+======

DTMSideSlopeInputPoint^ DTMSideSlopeInput::AddRadialToElevation
(
BGEO::DPoint3d startPoint,
DTMSideSlopeRadialOption radialOption,
DTMSideSlopeCutFillOption cutFillOption,
DTM^ cutFillDTM,
double cutSlope,
double fillSlope,
double elevation
)
    {
    bool dbg = false;
    if (dbg)bcdtmWrite_message (0, 0, 0, "Adding Radial To Eleavtion");
    DTMSideSlopeInputPoint^ sideSlopeInputPoint = gcnew DTMSideSlopeInputPoint (startPoint, nullptr, cutSlope, fillSlope);
    sideSlopeInputPoint->SideSlopeOption = DTMSideSlopeOption::ToElevation;
    sideSlopeInputPoint->RadialOption = radialOption;
    sideSlopeInputPoint->CutFillDTM = nullptr;
    sideSlopeInputPoint->CutFillSlopeOption = cutFillOption;
    if (cutFillOption == DTMSideSlopeCutFillOption::CutOnly || cutFillOption == DTMSideSlopeCutFillOption::FillOnly)
        {
        sideSlopeInputPoint->CutFillDTM = cutFillDTM;
        }
    sideSlopeInputPoint->ToElevation = elevation;
    InnerList->Add (sideSlopeInputPoint);
    return sideSlopeInputPoint;
    }


//=======================================================================================
// @bsimethod                                            Rob.Cormack      03/2012
//+===============+===============+===============+===============+===============+======

DTMSideSlopeInputPoint^ DTMSideSlopeInput::AddRadialToDeltaElevation
(
BGEO::DPoint3d startPoint,
DTMSideSlopeRadialOption radialOption,
DTMSideSlopeCutFillOption cutFillOption,
DTM^ cutFillDTM,
double cutSlope,
double fillSlope,
double deltaElevation
)
    {
    bool dbg = false;
    if (dbg)bcdtmWrite_message (0, 0, 0, "Adding Radial To Delta Elevation ** CutSlope = %10.4lf ** FillSlope = %10.4lf ** deltaElevation = %10.4lf", cutSlope, fillSlope, deltaElevation);
    DTMSideSlopeInputPoint^ sideSlopeInputPoint = gcnew DTMSideSlopeInputPoint (startPoint, nullptr, cutSlope, fillSlope);
    sideSlopeInputPoint->SideSlopeOption = DTMSideSlopeOption::UpDownVerticalDistance;
    sideSlopeInputPoint->RadialOption = radialOption;
    sideSlopeInputPoint->CutFillDTM = nullptr;
    sideSlopeInputPoint->CutFillSlopeOption = cutFillOption;
    if (cutFillOption == DTMSideSlopeCutFillOption::CutOnly || cutFillOption == DTMSideSlopeCutFillOption::FillOnly)
        {
        sideSlopeInputPoint->CutFillDTM = cutFillDTM;
        }
    sideSlopeInputPoint->ToDeltaElevation = deltaElevation;
    InnerList->Add (sideSlopeInputPoint);
    return sideSlopeInputPoint;
    }


//=======================================================================================
// @bsimethod                                            Rob.Cormack      03/2012
//+===============+===============+===============+===============+===============+======

DTMSideSlopeInputPoint^ DTMSideSlopeInput::AddRadialOutHorizontalDistance
(
BGEO::DPoint3d startPoint,
DTMSideSlopeRadialOption radialOption,
DTMSideSlopeCutFillOption cutFillOption,
DTM^ cutFillDTM,
double slope,
double horizontalDistance
)
    {
    bool dbg = false;
    if (dbg)bcdtmWrite_message (0, 0, 0, "Adding Radial Out To A Horizonatl Distance ** Slope = %10.4lf ** horizontal Distance = %10.4lf", slope, horizontalDistance);
    DTMSideSlopeInputPoint^ sideSlopeInputPoint = gcnew DTMSideSlopeInputPoint (startPoint, nullptr, slope, slope);
    sideSlopeInputPoint->SideSlopeOption = DTMSideSlopeOption::OutHorizontalDistance;
    sideSlopeInputPoint->RadialOption = radialOption;
    sideSlopeInputPoint->CutFillDTM = nullptr;
    sideSlopeInputPoint->CutFillSlopeOption = cutFillOption;
    if (cutFillOption == DTMSideSlopeCutFillOption::CutOnly || cutFillOption == DTMSideSlopeCutFillOption::FillOnly)
        {
        sideSlopeInputPoint->CutFillDTM = cutFillDTM;
        }
    sideSlopeInputPoint->ForcedSlope = slope;
    sideSlopeInputPoint->ToHorizontalDistance = horizontalDistance;
    InnerList->Add (sideSlopeInputPoint);
    return sideSlopeInputPoint;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
DTMSideSlopeInputPoint^ DTMSideSlopeInput::InsertPoint (int index, BGEO::DPoint3d startPoint, DTM^ slopeToDTM, double cutSlope, double fillSlope)
    {
    DTMSideSlopeInputPoint^ sideSlopeInputPoint = gcnew DTMSideSlopeInputPoint (startPoint, slopeToDTM, cutSlope, fillSlope);

    InnerList->Insert (index, sideSlopeInputPoint);

    return sideSlopeInputPoint;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
void DTMSideSlopeInput::RemovePoint (DTMSideSlopeInputPoint^ sideSlopePoint)
    {
    InnerList->Remove (sideSlopePoint);
    }

//=======================================================================================
// @bsimethod                                            Vince.Unrau      08/2016
//+===============+===============+===============+===============+===============+======
TMTransformHelper* DTMSideSlopeInput::GetTMTransformHelper()
    {
    DTMSideSlopeInputPoint^ pt = dynamic_cast<DTMSideSlopeInputPoint^>(InnerList[0]);
    TMTransformHelper *transformHelperP = ((BcDTM*)pt->SlopeToDTM->ExternalHandle.ToPointer())->GetTransformHelper();
    return transformHelperP;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
DTM_SIDE_SLOPE_TABLE* DTMSideSlopeInput::CreateSideSlopeInputTable ()
    {
    int numPnts = InnerList->Count;

    DTM_SIDE_SLOPE_TABLE* sideSlopeInputTableP = (DTM_SIDE_SLOPE_TABLE*)calloc (numPnts, sizeof (DTM_SIDE_SLOPE_TABLE));
    if (sideSlopeInputTableP == NULL)
        return NULL;

    // Load all the values
    for (int i = 0; i < numPnts; i++)
        {
        DPoint3d nativePoint;
        DTMSideSlopeInputPoint^ pt = dynamic_cast<DTMSideSlopeInputPoint^>(InnerList[i]);
        TMTransformHelper *transformHelperP = ((BcDTM*)pt->SlopeToDTM->ExternalHandle.ToPointer())->GetTransformHelper();
        // Set the native point
        nativePoint.x = pt->StartPoint.X;
        nativePoint.y = pt->StartPoint.Y;
        nativePoint.z = pt->StartPoint.Z;
        if (transformHelperP != NULL)
            transformHelperP->convertPointToDTM(nativePoint);

        sideSlopeInputTableP[i].radialStartPoint = nativePoint;

        sideSlopeInputTableP[i].useSlopeTable = 0;
        sideSlopeInputTableP[i].radialOption = (int)pt->RadialOption;
        sideSlopeInputTableP[i].sideSlopeOption = (int)pt->SideSlopeOption;
        if (pt->SlopeToDTM) sideSlopeInputTableP[i].slopeToTin = ((BcDTM*)pt->SlopeToDTM->ExternalHandle.ToPointer ())->GetTinHandle ();

        if (transformHelperP != NULL)
            {
            sideSlopeInputTableP[i].toElev = transformHelperP->convertElevationToDTM(pt->ToElevation);
            sideSlopeInputTableP[i].toDeltaElev = transformHelperP->convertElevationToDTM(pt->ToDeltaElevation);
            sideSlopeInputTableP[i].toHorizOffset = transformHelperP->convertDistanceToDTM(pt->ToHorizontalDistance);
            sideSlopeInputTableP[i].cutSlope = transformHelperP->convertSlopeToDTM(pt->CutSlope);
            sideSlopeInputTableP[i].fillSlope = transformHelperP->convertSlopeToDTM(pt->FillSlope);
            sideSlopeInputTableP[i].forcedSlope = transformHelperP->convertSlopeToDTM(pt->ForcedSlope);
            }
        else
            {
            sideSlopeInputTableP[i].toElev = pt->ToElevation;
            sideSlopeInputTableP[i].toDeltaElev = pt->ToDeltaElevation;
            sideSlopeInputTableP[i].toHorizOffset = pt->ToHorizontalDistance;
            sideSlopeInputTableP[i].cutSlope = pt->CutSlope;
            sideSlopeInputTableP[i].fillSlope = pt->FillSlope;
            sideSlopeInputTableP[i].forcedSlope = pt->ForcedSlope;
            }

        sideSlopeInputTableP[i].cutFillOption = (int)pt->CutFillSlopeOption;
        if (pt->CutFillDTM) sideSlopeInputTableP[i].cutFillTin = ((BcDTM*)pt->CutFillDTM->ExternalHandle.ToPointer ())->GetTinHandle ();
        sideSlopeInputTableP[i].useSlopeTable = 0; // Not used yet
        sideSlopeInputTableP[i].isForceSlope = pt->IsSlopeForced;

        sideSlopeInputTableP[i].isRadialDir = 0; // Not used yet
        sideSlopeInputTableP[i].radialDir = 0.0; // Not used yet
        sideSlopeInputTableP[i].offsetDef = 0; // Not used yet

        sideSlopeInputTableP[i].isMinHorizLimit = 0; // Not used yet
        sideSlopeInputTableP[i].limitMinHoriz = 0.0; // Not used yet
        sideSlopeInputTableP[i].isMaxHorizLimit = 0; // Not used yet
        sideSlopeInputTableP[i].limitMaxHoriz = 0.0; // Not used yet

        sideSlopeInputTableP[i].isCutThreshold = 0; // Not used yet
        sideSlopeInputTableP[i].minCutThreshold = 0.0; // Not used yet
        sideSlopeInputTableP[i].maxCutThreshold = 0.0; // Not used yet
        sideSlopeInputTableP[i].isFillThreshold = 0; // Not used yet
        sideSlopeInputTableP[i].minFillThreshold = 0.0; // Not used yet
        sideSlopeInputTableP[i].maxFillThreshold = 0.0; // Not used yet
        }

    return sideSlopeInputTableP;
    }

//=======================================================================================
// @bsimethod                                            Rob.Cormack        01/2010
//+===============+===============+===============+===============+===============+======
array<DTM^>^ DTMSideSlopeInput::CalculateSideSlopes ()
    {
    // Create Side Slope Table
    DTM_SIDE_SLOPE_TABLE* dtmSideSlopeInputTableP = CreateSideSlopeInputTable ();
    long sideSlopeTableSize = this->InnerList->Count;
    // Get the target DTM
    TMTransformHelper *transformHelperP = GetTMTransformHelper();
    // Get Slope Table For Assigning Side Slope Values
    DTM_SLOPE_TABLE* slopeTableP = NULL;
    long slopeTableSize = 0;
    if (m_slopeTable != nullptr)
        {
        slopeTableSize = m_slopeTable->GetSlopeTableSize ();
        if (slopeTableSize >  0)
            {
            slopeTableP = m_slopeTable->CreateSlopeTable ();
            }
        }

    // Initialize Other Side Slope Parameters

    DPoint3d* parallelEdgePtsP = NULL;          // ==> Parallel Edge Points For Truncating Side Slope Radials
    long numParallelEdgePts = 0;                 // ==> Number Of Parallel Edge Points
    BC_DTM_OBJ* *dtmSideSlopesPP = NULL;         // <== Array Of Pointers To The Created Side Slope DTM Objects
    long numberOfDtmSideSlopes = 0;              // <== Size Of Array Of DTM Object Pointers

     //  Call Core DTM Code
    // Convert tolerances to DTM transform
    double cornerStrokeTolearance = 1.0, pointToPointTolerance = 0.001;

    if (transformHelperP != NULL)
        {
        double native_cornerStrokeTolearance = m_cornerStrokeTolearance;
        double native_pointToPointTolerance = m_pointToPointTolerance;
        cornerStrokeTolearance = transformHelperP->convertDistanceToDTM(native_cornerStrokeTolearance);
        pointToPointTolerance = transformHelperP->convertDistanceToDTM(native_pointToPointTolerance);
        }
    else
        {
        cornerStrokeTolearance = m_cornerStrokeTolearance;
        pointToPointTolerance = m_pointToPointTolerance;
        }

    DTMException::CheckForErrorStatus (bcCalculateSideSlopes
                                       (
                                              &dtmSideSlopeInputTableP,
                                              &sideSlopeTableSize,
                                              (long)m_direction,
                                              slopeTableP,
                                              slopeTableSize,
                                              (long)m_cornerOption,
                                              (long)(m_strokeCornerOption)-1,
                                              cornerStrokeTolearance,
                                              pointToPointTolerance,
                                              parallelEdgePtsP,
                                              numParallelEdgePts,
                                              m_breakLineUserTag,
                                              m_sideSlopeElementUserTag,
                                              &dtmSideSlopesPP,
                                              &numberOfDtmSideSlopes
                                              )
                                              );

    //  Free Memory
    free (dtmSideSlopeInputTableP);
    dtmSideSlopeInputTableP = NULL;

    if (numberOfDtmSideSlopes > 0)
        {
        array<DTM^>^ dtmArray = gcnew array<DTM ^> (numberOfDtmSideSlopes);
        for (int n = 0; n < numberOfDtmSideSlopes; ++n)
            {
            BcDTMPtr bcDtmP = BcDTM::CreateFromDtmHandle(*dtmSideSlopesPP[n]);
            if (transformHelperP != NULL)
                {
                Bentley::Transform transFormNative;
                transformHelperP->GetTransformationFromDTM(transFormNative);
                DTMPtr dtm = nullptr;
                bcDtmP->GetTransformDTM(dtm, transFormNative);
                dtmArray[n] = gcnew DTM(dtm->GetBcDTM());
                }
            else
                dtmArray[n] = gcnew DTM(bcDtmP.get());
            }
        if (dtmSideSlopesPP != NULL)  free (dtmSideSlopesPP);
        return(dtmArray);
        }
    else
        {
        if (dtmSideSlopesPP != NULL)  free (dtmSideSlopesPP);
        return(nullptr);
        }
    }

//==================================================th=====================================
// @bsimethod                                            Rob.Cormack         07/2011
//+===============+===============+===============+===============+===============+======
DTMSlopeTableRange^ DTMSlopeTable::AddSlopeRange (double lowRangeValue, double highRangeValue, double slopeValue)
    {
    DTMSlopeTableRange^ slopeTableRange = gcnew DTMSlopeTableRange (lowRangeValue, highRangeValue, slopeValue);
    InnerList->Add (slopeTableRange);
    return slopeTableRange;
    }
//=======================================================================================
// @bsimethod                                            Rob.Cormack         07/2011
//+===============+===============+===============+===============+===============+======
DTM_SLOPE_TABLE* DTMSlopeTable::CreateSlopeTable ()
    {
    int numPnts = InnerList->Count;

    DTM_SLOPE_TABLE* slopeTableP = (DTM_SLOPE_TABLE*)calloc (numPnts, sizeof (DTM_SLOPE_TABLE));
    if (slopeTableP == NULL)
        return NULL;

    // Load all the values
    for (int i = 0; i < numPnts; i++)
        {
        DTMSlopeTableRange^ pt = dynamic_cast<DTMSlopeTableRange^>(InnerList[i]);
        slopeTableP[i].Low = pt->LowElevation;
        slopeTableP[i].High = pt->HighElevation;
        slopeTableP[i].Slope = pt->SlopeValue;
        }

    return slopeTableP;
    }
//=======================================================================================
// @bsimethod                                            Rob.Cormack         07/2011
//+===============+===============+===============+===============+===============+======
int DTMSlopeTable::GetSlopeTableSize ()
    {
    int size = InnerList->Count;
    return size;
    }


END_BENTLEY_TERRAINMODELNET_NAMESPACE
