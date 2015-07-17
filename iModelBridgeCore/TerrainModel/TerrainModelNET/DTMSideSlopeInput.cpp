/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTMSideSlopeInput.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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

BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
DTMSideSlopeInput::DTMSideSlopeInput(DTMSideSlopeDirection direction, DTMSideSlopeCornerOption cornerOption,DTMSideSlopeStrokeCornerOption strokeCornerOption, double cornerStrokeTolearance, double pointToPointTolerance,::DTMUserTag breakTag,::DTMUserTag sideSlopeTag)
    {
    m_slopeTable = nullptr ;
    m_direction = direction;
    m_cornerOption = cornerOption;
    m_strokeCornerOption = strokeCornerOption ;
    m_cornerStrokeTolearance = cornerStrokeTolearance;
    m_pointToPointTolerance = pointToPointTolerance;
    m_breakLineUserTag = breakTag  ;
    m_sideSlopeElementUserTag = sideSlopeTag ;
    }
//=======================================================================================
// @bsimethod                                            Rob.Cormack     07/2011
//+===============+===============+===============+===============+===============+======
DTMSideSlopeInput::DTMSideSlopeInput(DTMSlopeTable^ slopeTable,DTMSideSlopeDirection direction, DTMSideSlopeCornerOption cornerOption,DTMSideSlopeStrokeCornerOption strokeCornerOption, double cornerStrokeTolearance, double pointToPointTolerance,::DTMUserTag breakTag,::DTMUserTag sideSlopeTag)
    {
    m_slopeTable = slopeTable ;
    m_direction = direction;
    m_cornerOption = cornerOption;
    m_strokeCornerOption = strokeCornerOption ;
    m_cornerStrokeTolearance = cornerStrokeTolearance;
    m_pointToPointTolerance = pointToPointTolerance;
    m_breakLineUserTag = breakTag  ;
    m_sideSlopeElementUserTag = sideSlopeTag ;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
DTMSideSlopeInputPoint^ DTMSideSlopeInput::AddPoint(BGEO::DPoint3d startPoint, DTM^ slopeToDTM, double cutSlope, double fillSlope)
    {
    DTMSideSlopeInputPoint^ sideSlopeInputPoint = gcnew DTMSideSlopeInputPoint(startPoint, slopeToDTM, cutSlope, fillSlope);

    InnerList->Add(sideSlopeInputPoint);

    return sideSlopeInputPoint;
    }

//=======================================================================================
// @bsimethod                                            Rob.Cormack      01/2010
//+===============+===============+===============+===============+===============+======
DTMSideSlopeInputPoint^ DTMSideSlopeInput::AddRadialToSurface(BGEO::DPoint3d startPoint, DTM^ slopeToDTM, double cutSlope, double fillSlope)
    {
    DTMSideSlopeInputPoint^ sideSlopeInputPoint = gcnew DTMSideSlopeInputPoint(startPoint, slopeToDTM, cutSlope, fillSlope);

    sideSlopeInputPoint->SideSlopeOption = DTMSideSlopeOption::ToTin ; 

    InnerList->Add(sideSlopeInputPoint);

    return sideSlopeInputPoint;
    }

//=======================================================================================
// @bsimethod                                            Rob.Cormack      01/2010
//+===============+===============+===============+===============+===============+======
DTMSideSlopeInputPoint^ DTMSideSlopeInput::AddRadialToCutFillSurface(BGEO::DPoint3d startPoint, DTM^ cutFillDTM, DTMSideSlopeCutFillOption cutFillOption,double cutSlope, double fillSlope )
    {
    DTMSideSlopeInputPoint^ sideSlopeInputPoint = gcnew DTMSideSlopeInputPoint(startPoint,cutFillDTM,cutSlope, fillSlope);

    sideSlopeInputPoint->SideSlopeOption = DTMSideSlopeOption::ToTin ; 
    sideSlopeInputPoint->CutFillDTM      = cutFillDTM ; 
    sideSlopeInputPoint->CutFillSlopeOption = cutFillOption ; 

    InnerList->Add(sideSlopeInputPoint);

    return sideSlopeInputPoint;
    }

//=======================================================================================
// @bsimethod                                            Rob.Cormack      06/2011
//+===============+===============+===============+===============+===============+======
DTMSideSlopeInputPoint^ DTMSideSlopeInput::AddRadialToElevation(BGEO::DPoint3d startPoint,DTMSideSlopeRadialOption radialOption,double cutSlope,double fillSlope,double elevation )
    {
    DTMSideSlopeInputPoint^ sideSlopeInputPoint = gcnew DTMSideSlopeInputPoint(startPoint,radialOption,elevation,cutSlope,fillSlope);
    sideSlopeInputPoint->SideSlopeOption = DTMSideSlopeOption::ToElevation ; 
    InnerList->Add(sideSlopeInputPoint);
    return sideSlopeInputPoint;
    }


//=======================================================================================
// @bsimethod                                            Rob.Cormack      06/2011
//+===============+===============+===============+===============+===============+======
DTMSideSlopeInputPoint^ DTMSideSlopeInput::AddRadialToDeltaElevation(BGEO::DPoint3d startPoint,DTMSideSlopeRadialOption radialOption,DTM^ slopeToDTM ,double cutSlope, double fillSlope, double deltaElevation )
    {
    DTMSideSlopeInputPoint^ sideSlopeInputPoint = gcnew DTMSideSlopeInputPoint(startPoint,radialOption,slopeToDTM,cutSlope,fillSlope);
    sideSlopeInputPoint->SideSlopeOption = DTMSideSlopeOption::UpDownVerticalDistance ; 
    sideSlopeInputPoint->ToDeltaElevation = deltaElevation ;
    InnerList->Add(sideSlopeInputPoint);
    return sideSlopeInputPoint;
    }
    
//=======================================================================================
// @bsimethod                                            Rob.Cormack      07/2011
//+===============+===============+===============+===============+===============+======
DTMSideSlopeInputPoint^ DTMSideSlopeInput::AddRadialOutHorizontalDistance(BGEO::DPoint3d startPoint,DTMSideSlopeRadialOption radialOption,DTM^ slopeToDTM,double cutSlope, double fillSlope,double horizontalDistance )
    {
    DTMSideSlopeInputPoint^ sideSlopeInputPoint = gcnew DTMSideSlopeInputPoint(startPoint,radialOption,slopeToDTM,cutSlope,fillSlope);
    sideSlopeInputPoint->SideSlopeOption = DTMSideSlopeOption::OutHorizontalDistance ; 
    sideSlopeInputPoint->ToHorizontalDistance = horizontalDistance ;
    InnerList->Add(sideSlopeInputPoint);
    return sideSlopeInputPoint;
    }

//=======================================================================================
// @bsimethod                                            Rob.Cormack      02/2012
//+===============+===============+===============+===============+===============+======
DTMSideSlopeInputPoint^ DTMSideSlopeInput::AddRadialToSurfaceWithHorizontalLimit(BGEO::DPoint3d startPoint, DTM^ slopeToDTM, double cutSlope, double fillSlope,double horizontalDistance )
    {
    DTMSideSlopeInputPoint^ sideSlopeInputPoint = gcnew DTMSideSlopeInputPoint(startPoint, slopeToDTM, cutSlope, fillSlope);
    sideSlopeInputPoint->SideSlopeOption = DTMSideSlopeOption::ToTinOrHorizontalDistance ; 
    sideSlopeInputPoint->ToHorizontalDistance = horizontalDistance ;
    InnerList->Add(sideSlopeInputPoint);
    return sideSlopeInputPoint;
    }


//=======================================================================================
// @bsimethod                                            Rob.Cormack      02/2012
//+===============+===============+===============+===============+===============+======
DTMSideSlopeInputPoint^ DTMSideSlopeInput::AddRadialToSurfaceWithElevationLimit(BGEO::DPoint3d startPoint, DTM^ slopeToDTM, double cutSlope, double fillSlope,double elevation  )
    {
    DTMSideSlopeInputPoint^ sideSlopeInputPoint = gcnew DTMSideSlopeInputPoint(startPoint, slopeToDTM, cutSlope, fillSlope);

    sideSlopeInputPoint->SideSlopeOption = DTMSideSlopeOption::ToTinOrElevation ; 

    sideSlopeInputPoint->ToElevation = elevation ;

    InnerList->Add(sideSlopeInputPoint);

    return sideSlopeInputPoint;
    }


//=======================================================================================
// @bsimethod                                            Rob.Cormack      02/2012
//+===============+===============+===============+===============+===============+======
DTMSideSlopeInputPoint^ DTMSideSlopeInput::AddRadialToSurfaceWithVerticalDistanceLimit(BGEO::DPoint3d startPoint, DTM^ slopeToDTM, double cutSlope, double fillSlope,double verticalDistance  )
    {
    DTMSideSlopeInputPoint^ sideSlopeInputPoint = gcnew DTMSideSlopeInputPoint(startPoint, slopeToDTM, cutSlope, fillSlope);

    sideSlopeInputPoint->SideSlopeOption = DTMSideSlopeOption::ToTinOrVerticalDistance ; 

    sideSlopeInputPoint->ToDeltaElevation = verticalDistance ;

    InnerList->Add(sideSlopeInputPoint);

    return sideSlopeInputPoint;
    }


//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
DTMSideSlopeInputPoint^ DTMSideSlopeInput::InsertPoint(int index, BGEO::DPoint3d startPoint, DTM^ slopeToDTM, double cutSlope, double fillSlope)
    {
    DTMSideSlopeInputPoint^ sideSlopeInputPoint = gcnew DTMSideSlopeInputPoint(startPoint, slopeToDTM, cutSlope, fillSlope);

    InnerList->Insert(index, sideSlopeInputPoint);

    return sideSlopeInputPoint;
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
void DTMSideSlopeInput::RemovePoint(DTMSideSlopeInputPoint^ sideSlopePoint)
    {
    InnerList->Remove(sideSlopePoint);
    }

//=======================================================================================
// @bsimethod                                            Sylvain.Pucci      10/2005
//+===============+===============+===============+===============+===============+======
DTM_SIDE_SLOPE_TABLE* DTMSideSlopeInput::CreateSideSlopeInputTable()
    {
    int numPnts = InnerList->Count;

    DTM_SIDE_SLOPE_TABLE* sideSlopeInputTableP = (DTM_SIDE_SLOPE_TABLE*)bcMem_calloc (numPnts, sizeof(DTM_SIDE_SLOPE_TABLE));
    if (sideSlopeInputTableP == NULL) 
        return NULL;

    // Load all the values
    for (int i = 0; i < numPnts; i++)
        {
        DTMSideSlopeInputPoint^ pt = dynamic_cast<DTMSideSlopeInputPoint^>(InnerList[i]);
        sideSlopeInputTableP[i].useSlopeTable = 0;
        sideSlopeInputTableP[i].radialStartPoint.x = pt->StartPoint.X;
        sideSlopeInputTableP[i].radialStartPoint.y = pt->StartPoint.Y;
        sideSlopeInputTableP[i].radialStartPoint.z = pt->StartPoint.Z;

        sideSlopeInputTableP[i].radialOption = (int)pt->RadialOption;
        sideSlopeInputTableP[i].sideSlopeOption = (int)pt->SideSlopeOption;
        if (pt->SlopeToDTM) sideSlopeInputTableP[i].slopeToTin = pt->SlopeToDTM->Handle->GetTinHandle();

        sideSlopeInputTableP[i].toElev = pt->ToElevation;
        sideSlopeInputTableP[i].toDeltaElev = pt->ToDeltaElevation;
        sideSlopeInputTableP[i].toHorizOffset = pt->ToHorizontalDistance;

        sideSlopeInputTableP[i].cutSlope = pt->CutSlope;
        sideSlopeInputTableP[i].fillSlope = pt->FillSlope;

        sideSlopeInputTableP[i].cutFillOption = (int)pt->CutFillSlopeOption;
        if (pt->CutFillDTM) sideSlopeInputTableP[i].cutFillTin = pt->CutFillDTM->Handle->GetTinHandle();
        // 		sideSlopeInputTableP[i].useSlopeTable = 0; // Not used yet
        sideSlopeInputTableP[i].isForceSlope = pt->IsSlopeForced;
        sideSlopeInputTableP[i].forcedSlope = pt->ForcedSlope;

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
array<DTM^>^ DTMSideSlopeInput::CalculateSideSlopes() 
{

// Create Side Slope Table

bcdtmWrite_message(0,0,0,"Creating Side Slope Table") ;

 DTM_SIDE_SLOPE_TABLE* dtmSideSlopeInputTableP = CreateSideSlopeInputTable();
 long sideSlopeTableSize = this->InnerList->Count; 

// Get Slope Table For Assigning Side Slope Values 

 DTM_SLOPE_TABLE* slopeTableP=NULL ;
 long slopeTableSize=0 ;
 if( m_slopeTable != nullptr )
 {
      slopeTableSize = m_slopeTable->GetSlopeTableSize() ;
      if( slopeTableSize >  0 )
      {
           slopeTableP = m_slopeTable->CreateSlopeTable() ; 
      }
 } 

// Initialise Other Side Slope Parameters 

 DPoint3d *parallelEdgePtsP=NULL;           // ==> Parallel Edge Points For Truncating Side Slope Radials     
 long numParallelEdgePts=0;                 // ==> Number Of Parallel Edge Points 
 BC_DTM_OBJ* *dtmSideSlopesPP=NULL;         // <== Array Of Pointers To The Created Side Slope DTM Objects 
 long numberOfDtmSideSlopes=0;              // <== Size Of Array Of DTM Object Pointers  

//  Call Core DTM Code

DTMException::CheckForErrorStatus(dtmSideSlope_createSideSlopesForSideSlopeTableDtmObject
(
 &dtmSideSlopeInputTableP,
 &sideSlopeTableSize, 
 (long)m_direction, 
 slopeTableP, 
 slopeTableSize, 
 (long) m_cornerOption,  
 (long)(m_strokeCornerOption)-1,  
 m_cornerStrokeTolearance,   
 m_pointToPointTolerance, 
 parallelEdgePtsP,     
 numParallelEdgePts, 
 m_breakLineUserTag, 
 m_sideSlopeElementUserTag,  
 &dtmSideSlopesPP, 
 &numberOfDtmSideSlopes  
)
);

//  Free Memory

 bcMem_freeAndClear((void**)&dtmSideSlopeInputTableP) ;

 if( numberOfDtmSideSlopes > 0 )
 {
       array<DTM^>^ dtmArray = gcnew array<DTM ^>(numberOfDtmSideSlopes) ;
       for( int n = 0 ; n <  numberOfDtmSideSlopes ; ++n )
       {
        dtmArray[n] = DTM::FromNativeDtmHandle (System::IntPtr(dtmSideSlopesPP[n]));
       }
      if( dtmSideSlopesPP != NULL )  bcMem_freeAndClear((void**)&dtmSideSlopesPP) ;
      return(dtmArray) ;     
  }
 else 
   {
    if( dtmSideSlopesPP != NULL )  bcMem_freeAndClear((void**)&dtmSideSlopesPP) ;
    return(nullptr) ; 
   } 

}

//=======================================================================================
// @bsimethod                                            Rob.Cormack         07/2011
//+===============+===============+===============+===============+===============+======
 DTMSlopeTableRange^ DTMSlopeTable::AddSlopeRange(double lowRangeValue,double highRangeValue, double slopeValue)
 {
     DTMSlopeTableRange^ slopeTableRange = gcnew DTMSlopeTableRange(lowRangeValue,highRangeValue,slopeValue);
     InnerList->Add(slopeTableRange);
     return slopeTableRange ;
 }
//=======================================================================================
// @bsimethod                                            Rob.Cormack         07/2011
//+===============+===============+===============+===============+===============+======
DTM_SLOPE_TABLE* DTMSlopeTable::CreateSlopeTable()
{
    int numPnts = InnerList->Count;

    DTM_SLOPE_TABLE* slopeTableP = (DTM_SLOPE_TABLE*)bcMem_calloc (numPnts, sizeof(DTM_SLOPE_TABLE));
    if (slopeTableP == NULL) 
        return NULL;

    // Load all the values
    for (int i = 0; i < numPnts; i++)
        {
        DTMSlopeTableRange^ pt = dynamic_cast<DTMSlopeTableRange^>(InnerList[i]);
        slopeTableP[i].Low   = pt->LowElevation;
        slopeTableP[i].High  = pt->HighElevation;
        slopeTableP[i].Slope = pt->SlopeValue;
        }

    return slopeTableP ;
}
//=======================================================================================
// @bsimethod                                            Rob.Cormack         07/2011
//+===============+===============+===============+===============+===============+======
int DTMSlopeTable::GetSlopeTableSize()
{
    int size = InnerList->Count;
    return size ;
}


END_BENTLEY_TERRAINMODELNET_NAMESPACE
