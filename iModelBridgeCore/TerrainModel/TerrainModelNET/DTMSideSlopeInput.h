/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTMSideSlopeInput.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "Bentley.Civil.DTM.h"
#include ".\DTM.h"
#include ".\DTMSideSlopeInputPoint.h"

BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

public enum class DTMSideSlopeDirection
{
    Right = 1,
    Left = 2,
    LeftAndRight=3,
};

public enum class DTMSideSlopeCornerOption
{
    Rounded = 1,
    Square = 2,
};


public enum class DTMSideSlopeStrokeCornerOption
{
    DoNotStroke = 1,
    Stroke = 2,
};

public ref class DTMSlopeTableRange 
{
    private: double m_lowElevation ;
             double m_highElevation ;
             double m_slopeValue ; 
    public : 
    DTMSlopeTableRange(double low, double high, double slope ) 
    {
         m_lowElevation  = low ;
         m_highElevation = high ;
         m_slopeValue    = slope ; 
    } 

    property double LowElevation
    {
          double get()
          { 
              return m_lowElevation ;
          }
    }

    property double HighElevation
    {
          double get()
          { 
              return m_highElevation ;
          }
    }

    property double SlopeValue
    {
          double get()
          { 
              return m_slopeValue ;
          }
    }

} ;

public ref class DTMSlopeTable : public System::Collections::CollectionBase
{

 public: DTMSlopeTable(void){} ;
 public: DTMSlopeTableRange^ AddSlopeRange(double lowRangeValue,double highRangeValue, double slopeValue) ;
 internal: DTM_SLOPE_TABLE* CreateSlopeTable();
 public: int GetSlopeTableSize();

} ;


public ref class DTMSideSlopeInput: public System::Collections::CollectionBase
{
private:    DTMSideSlopeDirection m_direction;
            DTMSideSlopeCornerOption m_cornerOption;
            DTMSideSlopeStrokeCornerOption m_strokeCornerOption;
            double m_cornerStrokeTolearance;
            double m_pointToPointTolerance;
            ::DTMUserTag  m_breakLineUserTag ;
            ::DTMUserTag  m_sideSlopeElementUserTag ;
            DTMSlopeTable^ m_slopeTable ;
            

public: DTMSideSlopeInput(DTMSlopeTable^ slopeTable,DTMSideSlopeDirection direction, DTMSideSlopeCornerOption cornerOption,DTMSideSlopeStrokeCornerOption strokeCornerOption, double cornerStrokeTolearance, double pointToPointTolerance, ::DTMUserTag breakTag,::DTMUserTag sideSlopeTag);

public: DTMSideSlopeInput(DTMSideSlopeDirection direction, DTMSideSlopeCornerOption cornerOption,DTMSideSlopeStrokeCornerOption strokeCornerOption, double cornerStrokeTolearance, double pointToPointTolerance,::DTMUserTag breakTag,::DTMUserTag sideSlopeTag) ;

public: DTMSideSlopeInputPoint^ AddPoint(BGEO::DPoint3d startPoint, DTM^ slopeToDTM, double cutSlope, double fillSlope);

public: DTMSideSlopeInputPoint^ InsertPoint(int index, BGEO::DPoint3d startPoint, DTM^ slopeToDTM, double cutSlope, double fillSlope);

public: void RemovePoint(DTMSideSlopeInputPoint^ sideSlopePoint);

public: DTMSideSlopeInputPoint^ AddRadialToSurface(BGEO::DPoint3d startPoint, DTM^ slopeToDTM, double cutSlope, double fillSlope) ;

public: DTMSideSlopeInputPoint^ AddRadialToCutFillSurface(BGEO::DPoint3d startPoint, DTM^ cutFillDTM,DTMSideSlopeCutFillOption cutFillOption, double cutSlope, double fillSlope ) ;

public: DTMSideSlopeInputPoint^ AddRadialToElevation(BGEO::DPoint3d startPoint,DTMSideSlopeRadialOption radialOption,double cutSlope, double fillSlope,double elevation ) ;

public: DTMSideSlopeInputPoint^ AddRadialToDeltaElevation(BGEO::DPoint3d startPoint,DTMSideSlopeRadialOption radialOption, DTM^ slopeToDTM ,double cutSlope, double fillSlope, double deltaElevation ) ;

public: DTMSideSlopeInputPoint^ AddRadialOutHorizontalDistance(BGEO::DPoint3d startPoint,DTMSideSlopeRadialOption radialOption,DTM^ slopeToDTM,double cutSlope, double fillSlope,double horizontalDistance ) ;

public: DTMSideSlopeInputPoint^ AddRadialToSurfaceWithHorizontalLimit(BGEO::DPoint3d startPoint, DTM^ slopeToDTM, double cutSlope, double fillSlope,double horizontalDistance ) ;

public: DTMSideSlopeInputPoint^ AddRadialToSurfaceWithElevationLimit(BGEO::DPoint3d startPoint, DTM^ slopeToDTM, double cutSlope, double fillSlope,double elevation) ;

public: DTMSideSlopeInputPoint^ AddRadialToSurfaceWithVerticalDistanceLimit(BGEO::DPoint3d startPoint, DTM^ slopeToDTM, double cutSlope, double fillSlope,double verticalDistance  ) ;


private: DTM_SIDE_SLOPE_TABLE* CreateSideSlopeInputTable();

public: array<DTM^>^ CalculateSideSlopes() ;
// public:  void CalculateSideSlopes() ;
};

END_BENTLEY_TERRAINMODELNET_NAMESPACE
