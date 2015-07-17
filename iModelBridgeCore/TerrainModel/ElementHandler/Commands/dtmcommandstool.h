/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/Commands/dtmcommandstool.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if ! defined (__measureH__)
#define     __measureH__
    
#define     MEASVIEWMODE_TRUE                           0
#define     MEASVIEWMODE_VIEW                           1
#define     MEASVIEWMODE_ACS                            2
#define     MEASVIEWMODE_ACCUDRAW                       3

/*---------------------------------------------------------------------------------**//**
Newly added for measure area tool to be separated from region tool              
+---------------+---------------+---------------+---------------+---------------+------*/
#define MeasureArea_REGION_Elements             0
#define MeasureArea_REGION_Fence                1
#define MeasureArea_REGION_ElementIntersection  2
#define MeasureArea_REGION_ElementUnion         3
#define MeasureArea_REGION_ElementDifference    4
#define MeasureArea_REGION_Flood                5
#define MeasureArea_REGION_Points               6

/*----------------------------------------------------------------------+
|                                                                       |
|   Immediate Defines                                                   |
|                                                                       |
+----------------------------------------------------------------------*/
#define     UNKNOWN                                     0
#define     LINEAR                                      1
#define     AREA                                        2
#define     SURFACEAREA                                 3
#define     VOLUME                                      4
 
#define     MOMENT_SYSTEM_Centroid                      0
#define     MOMENT_SYSTEM_Global                        1
#define     MOMENT_SYSTEM_ACS                           2
#define     MOMENT_SYSTEM_ACSAtCentroid                 3

#define     MEASUREMENT_RESULT_Length                   (2 * (80+MAX_UNIT_LABEL_LENGTH))

#define     DEFAULT_DENSITY                             -9999

#define     MIN_CurvedSurfaceTolerance                  0.05
#define     TOLERENCE_DotProduct                        1.0e-4
#define     MAX_ITER                                    50

typedef struct dtmcommandsInfo
    {
/*
    int         dimension;
    double      perimeter;
    double      enclosed;
    double      slopeArea;
    double      weight;

    char        strLastDistance[MEASUREMENT_RESULT_Length];
    char        strAccuDistance[MEASUREMENT_RESULT_Length];
    char        strLastStartPtX[MEASUREMENT_RESULT_Length];
    char        strLastStartPtY[MEASUREMENT_RESULT_Length];
    char        strLastStartPtZ[MEASUREMENT_RESULT_Length];
    char        strLastEndPtX[MEASUREMENT_RESULT_Length];
    char        strLastEndPtY[MEASUREMENT_RESULT_Length];
    char        strLastEndPtZ[MEASUREMENT_RESULT_Length];
    char        strLastDeltaXDist[MEASUREMENT_RESULT_Length];
    char        strLastDeltaYDist[MEASUREMENT_RESULT_Length];
    char        strLastDeltaZDist[MEASUREMENT_RESULT_Length];
    char        strLastDistanceProjected[MEASUREMENT_RESULT_Length];
    char        strAccuDistanceProjected[MEASUREMENT_RESULT_Length];
    int         projectionMode;

    int         minimumPEMode;
    char        strOnPlane[MEASUREMENT_RESULT_Length];
    char        strOutofPlane[MEASUREMENT_RESULT_Length];
    
    char        strLastRadius[MEASUREMENT_RESULT_Length];
    char        strLastSecondaryRadius[MEASUREMENT_RESULT_Length];
    char        strLastDiameter[MEASUREMENT_RESULT_Length];
    char        strLastSecondaryDiameter[MEASUREMENT_RESULT_Length];
    char        strLastAngle[MEASUREMENT_RESULT_Length];
    char        strLastLength[MEASUREMENT_RESULT_Length];
    char        strLastAngleOfLine[MEASUREMENT_RESULT_Length];
    
    char        strLastPerimeter[MEASUREMENT_RESULT_Length];
    char        strLastArea[MEASUREMENT_RESULT_Length];
    char        strLastSlopeArea[MEASUREMENT_RESULT_Length];
    char        strLastVolume[MEASUREMENT_RESULT_Length];
    char        strLastVolPerimeter[MEASUREMENT_RESULT_Length];
    
    DPoint3d        centroid;
    DPoint3d        centroidACS;
    DPoint3d        centerOfMass;

    InertiaParams   unWeighted;
    InertiaParams   weighted;
    InertiaParams   display;                        // For the dialog only

    DVec3d          principalDirections[3];         // One vector for each moment
    DPoint3d        radiiOfGyration;
    
    bool         orientNormals;   // Legacy thing for orienting normals of faces at certain cases

    int             momentSystem;
    DPoint3d        origin;
    RotMatrix       rMatrix;

    //*** When announcemode, don't zero the following values.
    //If later maybe in the next version, will consider move all three indecies to tcb.
    int             areaUnitIndex;    
    int             perimeterUnitIndex;
    int             volumeUnitIndex;
*/
    int             annotateContoursMode;
    int             annotateContoursTxtAlignment;
#if !defined (resource)
    ElementRefP     dtmElemRef;
    DgnModelRefP    dtmModelRef;
#endif
    } dtmcommandsInfo;


#endif    
    
