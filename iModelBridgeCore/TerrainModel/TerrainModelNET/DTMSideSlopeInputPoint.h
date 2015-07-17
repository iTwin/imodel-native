/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTMSideSlopeInputPoint.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "Bentley.Civil.DTM.h"
#include "DTMFeatureEnumerator.h"
#include "DTM.h"

BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

public enum class DTMSideSlopeRadialOption
{
    Radial = 1,
    Planar = 2,
    RadialCS = 3,
};

public enum class DTMSideSlopeOption
{
    // Side Slope To Tin Object
    ToTin = 1,
    // Side Slope To Set Elevation
    ToElevation = 2,
    // Side Slope Out A  Horizontal Distance
    OutHorizontalDistance = 3,
    // Side Slope Up/Down A Vertical Distance
    UpDownVerticalDistance = 4,
    // Side Slope To Tin Surface Or Set Elevation Whichever Comes First
    ToTinOrElevation = 5,
    // Side Slope To Tin Surface Or Horizontal Distance Whichever Comes First     
    ToTinOrHorizontalDistance = 6,
    // Side Slope To Tin Surface Or Up/Down A Vertical Distance Whichever Comes First     
    ToTinOrVerticalDistance = 7,
    // Side Slope To Set Elevation And Extend Obtuse Radials For Copy parallel applications     
    ToElevationAndExtendForCopyParallel = 8,
};

public enum class DTMSideSlopeCutFillOption
{
    CutAndFill = 0,
    CutOnly = 1,
    FillOnly = 2,
};

public enum class DTMSideSlopeOffsetDefinition
{
    AlongRadialDirection = 1,
    AlongNormalDirection = 2,
};

public ref class DTMSideSlopeInputPoint
{
private:
    // Radial origination point
    BGEO::DPoint3d m_startPoint;

    // Radial option - "radial" "planar" or "radial-cs"
    DTMSideSlopeRadialOption m_radialOption;

    // Side Slope option
    DTMSideSlopeOption m_option;

    // To tin object if slope option = to tin NULL otherwise
    DTM^ m_slopeToDTM;

    // To constant elevation if slope option is to elevation
    double m_toElevation;

    // To delta elevation if slope option is to delta elevation
    double m_toDeltaElevation;

    // To horizontal offset if slope option is to hor distance
    double m_toHorizontalDistance;

    // Cut Slope - unit per unit
    double m_cutSlope;

    // Fill Slope - unit per unit
    double m_fillSlope;

    // Option to indicate whether to process in CutAndFill, CutOnly, FillOnly
    DTMSideSlopeCutFillOption m_cutFillOption;

    // Cut fill DTM object if CutFillOption == CutOnly or FillOnly
    DTM^ m_cutFillDTM;

    // int     useSlopeTable;          // => Flag to indicate that the slope is to be determined from the Slope Table
    // NOT used.

    // Flag to indicate wether the slope is to be forced
    bool m_isSlopeForced;

    // Value to use as force slope
    double m_forcedSlope;

    /* NOT USED YET
    int     isRadialDir;            // => Flag to indicate whether slope direction is supplied
    double  radialDir;              // => Slope direction if isSlopeDir is TRUE
    int     offsetDef ;             // => Offset Definition To Indicate whether the offsets apply ALONG_RADILADIR or ALONG_NORMALDIR 
    int     isMinHorizLimit;        // => Flag to indicate whether we have a minimum horizontal limit
    double  limitMinHoriz;          // => Value of minimum horizontal limit IF isMinHorizLimit = true
    int     isMaxHorizLimit;        // => Flag to inidicate whether there is a maximum horizontal distance to go
    double  limitMaxHoriz;          // => Value of max horizontal limit if isMaxHorizLimit = TRUE

    int     isCutThreshold;         // => Value to indicate whether a vertical cut threshold ie.
    double  minCutThreshold;        // => Value of cut threshold - cut must exceed this before cut is considered
    double  maxCutThreshold;        // => Value of cut threshold - cut must exceed this before cut is considered
    int     isFillThreshold;        // => Value to indicate whether a vertical fill threshold ie.
    double  minFillThreshold;       // => Value of Fill threshold - fill must exceed this before cut is considered
    double  maxFillThreshold;       // => Value of Fill threshold - fill must exceed this before cut is considered
    */

internal: DTMSideSlopeInputPoint(BGEO::DPoint3d startPoint, DTM^ slopeToDTM,double cutSlope, double fillSlope);
internal: DTMSideSlopeInputPoint(BGEO::DPoint3d startPoint,DTMSideSlopeRadialOption radialOption, DTM^ slopeToDTM,double cutSlope, double fillSlope);
internal: DTMSideSlopeInputPoint(BGEO::DPoint3d startPoint,DTMSideSlopeRadialOption radialOption,double elevation,double cutSlope, double fillSlope);

public: property BGEO::DPoint3d StartPoint
        {
            BGEO::DPoint3d get();
        }

public: property DTM^ SlopeToDTM
        {
            DTM^ get();
        }

public: property DTM^ CutFillDTM
        {
            DTM^ get();
            void set(DTM^ val);
        }

public: property DTMSideSlopeRadialOption RadialOption
        {
            DTMSideSlopeRadialOption  get();
            void set(DTMSideSlopeRadialOption val);
        }

public: property DTMSideSlopeOption SideSlopeOption
        {
            DTMSideSlopeOption get();
            void set(DTMSideSlopeOption val);
        }
public: property double ToElevation
        {
            double get();
            void set(double);
        }

public: property double ToDeltaElevation
        {
            double get();
            void set(double);
        }

public: property double ToHorizontalDistance
        {
            double get();
            void set(double);
        }
public: property double CutSlope
        {
            double get();
        }
public: property double FillSlope
        {
            double get();
        }

public: property DTMSideSlopeCutFillOption CutFillSlopeOption
        {
            DTMSideSlopeCutFillOption get();
            void set(DTMSideSlopeCutFillOption);
        }

public: property bool IsSlopeForced
        {
            bool get();
            void set(bool);
        }

public: property double ForcedSlope
        {
            double get();
            void set(double);
        }
};

END_BENTLEY_TERRAINMODELNET_NAMESPACE
