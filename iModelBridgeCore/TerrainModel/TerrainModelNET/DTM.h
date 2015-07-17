/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTM.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "Bentley.Civil.DTM.h"
#include "DTMFeature.h"
#include "DTMHelpers.h"
#include "DTMTinEditor.h"
#include "DTMDrapedLinearElement.h"
#include "dtmmesh.h"
#include "dtmedges.h"

//using namespace Bentley::Civil::BCSystem::Base;

#include "DTMDelegates.h"

BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

using namespace System::Runtime::InteropServices;

ref class DTM;

public ref class DTMFeatureStatisticsInfo
    {
    public:
    long VerticesCount;
    long TrianglesCount;
    long TrianglesLinesCount;
    long FeaturesCount;
    long BreakLinesCount;
    long ContourLinesCount;
    long VoidsCount;
    long IslandsCount;
    long HolesCount;
    long PointFeaturesCount;
    bool HasHull;
    };

//=======================================================================================
/// <summary>
/// Holds information for a DTM feature
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>05/2008</date>
//=======================================================================================
public ref class DTMFeatureInfo abstract
    {
    public:
        //=======================================================================================
        /// <summary>
        /// Duplicate Point Feature Type
        /// </summary>                
        //=======================================================================================
        property DTMFeatureType DtmFeatureType
            {
            virtual DTMFeatureType get () abstract;
            }

        //=======================================================================================
        /// <summary>
        /// Duplicate Point Feature Id
        /// </summary>                
        //=======================================================================================
        property DTMFeatureId DtmFeatureId
            {
            virtual DTMFeatureId get () abstract;
            }

        //=======================================================================================
        /// <summary>
        /// Duplicate Point User Tag
        /// </summary>                
        //=======================================================================================
        property ::DTMUserTag DtmUserTag 
            {
            virtual ::DTMUserTag get () abstract;
            }

        //=======================================================================================
        /// <summary>
        /// Get the points.
        /// </summary>                
        //=======================================================================================
        property array<BGEO::DPoint3d>^ Points
            {
            virtual array<BGEO::DPoint3d>^ get () abstract;
            }
    };

private ref struct DTMStandAloneFeatureInfo : DTMFeatureInfo
    {
    private:
        DTMFeatureId    m_dtmFeatureId;
        DTMFeatureType  m_dtmFeatureType;
        ::DTMUserTag    m_dtmUserTag;
        array<BGEO::DPoint3d>^ m_points;

    internal:

        DTMStandAloneFeatureInfo (DTMFeatureId dtmFeatureId, DTMFeatureType dtmFeatureType, ::DTMUserTag dtmUserTag, array<BGEO::DPoint3d>^ points)
            {
            m_dtmFeatureId = dtmFeatureId;
            m_dtmFeatureType = dtmFeatureType;
            m_dtmUserTag = dtmUserTag;
            m_points = points;
            }

    public:

        //=======================================================================================
        /// <summary>
        /// Duplicate Point Feature Type
        /// </summary>                
        //=======================================================================================
        property DTMFeatureType DtmFeatureType
            {
            virtual DTMFeatureType get () override
                {
                return m_dtmFeatureType;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Duplicate Point Feature Id
        /// </summary>                
        //=======================================================================================
        property DTMFeatureId DtmFeatureId
            {
            virtual DTMFeatureId get () override
                {
                return m_dtmFeatureId;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Duplicate Point User Tag
        /// </summary>                
        //=======================================================================================
        property ::DTMUserTag DtmUserTag
            {
            virtual ::DTMUserTag get () override
                {
                return m_dtmUserTag;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Get the points
        /// </summary>                
        //=======================================================================================
        property array<BGEO::DPoint3d>^ Points
            {
            virtual array<BGEO::DPoint3d>^get () override
                {
                return m_points;
                }
            }
    };
//=======================================================================================
/// <summary>
/// Holds information for a DTM Dynamic feature
/// </summary>                
/// <author>Rob.Cormack</author>                              <date>03/2010</date>
//=======================================================================================
public ref class DTMDynamicFeatureInfo
    {
    private:

        DTMDynamicFeatureType  dtmFeatureType;
        array<BGEO::DPoint3d>^ dtmFeaturePts;

    internal:

        DTMDynamicFeatureInfo (DTMDynamicFeatureType dtmFeatureType,DPoint3d *dtmFeaturePtsP,long numDtmFeaturePts )
            {
             this->dtmFeatureType = dtmFeatureType;
             dtmFeaturePts = gcnew array<BGEO::DPoint3d>(numDtmFeaturePts);
             pin_ptr<BGEO::DPoint3d const> pPoints = &dtmFeaturePts[0];
             memcpy((void*)pPoints,dtmFeaturePtsP,numDtmFeaturePts*sizeof(DPoint3d)) ;
            }

    public:

        //=======================================================================================
        /// <summary>
        /// Get Dynamic DTM Feature Type
        /// </summary>                
        //=======================================================================================
        property DTMDynamicFeatureType FeatureType
            {
            DTMDynamicFeatureType get()
                {
                return dtmFeatureType;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Get Dynamic DTM Feature Points
        /// </summary>                
        //=======================================================================================
        property array<BGEO::DPoint3d>^ FeaturePoints
            {
            array<BGEO::DPoint3d>^ get()
                {
                return dtmFeaturePts ;
                }
            }

     };

//=======================================================================================
/// <summary>
/// Holds imformation for a duplicate point error.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>03/2008</date>
//=======================================================================================
public ref class DTMDuplicatePointError
    {
    public:

       //=======================================================================================
       /// <summary>
       /// X Coordinate Of Duplicate Point
       /// </summary>                
       //=======================================================================================
        property double X
            {
            double get()
                {
                return m_X;
                }
            }

       //=======================================================================================
       /// <summary>
       /// Y Coordinate Of Duplicate Point
       /// </summary>                
       //=======================================================================================
        property double Y
            {
            double get()
                {
                return m_Y;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Elevation Value Of Duplicate Point
        /// </summary>                
        //=======================================================================================
        property double Z
            {
            double get()
                {
                return m_Z;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Duplicate Point Feature Type
        /// </summary>                
        //=======================================================================================
        property DTMFeatureType DtmFeatureType
            {
            DTMFeatureType get()
                {
                return m_dtmFeatureType;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Duplicate Point Feature Id
        /// </summary>                
        //=======================================================================================
        property DTMFeatureId DtmFeatureId
            {
            DTMFeatureId get()
                {
                return m_dtmFeatureId;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Duplicate Point User Tag
        /// </summary>                
        //=======================================================================================
        property ::DTMUserTag DtmUserTag 
            {
            ::DTMUserTag get()
                {
                return m_dtmUserTag ;
                }
            }

    internal:

        DTMDuplicatePointError(DTM_DUPLICATE_POINT_ERROR& error)
            {
            m_X = error.x;
            m_Y = error.y;
            m_Z = error.z;
            DTMFeatureType type;
            DTMHelpers::Copy (type, error.dtmFeatureType);
            m_dtmFeatureType = type;
            m_dtmFeatureId = DTMFeatureId(error.dtmFeatureId);
            m_dtmUserTag = error.dtmUserTag;
            }

    private:

        double m_X;                         /* X Coordinate Of Duplicate Point    */
        double m_Y;                         /* Y Coordinate Of Duplicate Point    */
        double m_Z;                         /* X Coordinate Of Duplicate Point    */
        DTMFeatureType m_dtmFeatureType;    /* Duplicate Point Feature Type       */
        DTMFeatureId m_dtmFeatureId;        /* Duplicate Point Feature Id         */  
        ::DTMUserTag m_dtmUserTag;          /* Duplicate Point User Tag           */   
    };

//=======================================================================================
/// <summary>
/// Holds information for a crossing feature error.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>03/2008</date>
//=======================================================================================
public ref class DTMCrossingFeatureError
    {
    public:

        //=======================================================================================
        /// <summary>
        /// Gets XY coordinates of intersection point.
        /// </summary>                
        //=======================================================================================
        property BGEO::DPoint2d  IntersectionPointXY
            {
            BGEO::DPoint2d get()
                {
                BGEO::DPoint2d pt;
                pt.X = m_intersectionX;
                pt.Y = m_intersectionY;
                return pt;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets the type of the feature 1.
        /// </summary>                
        //=======================================================================================
        property DTMFeatureType TypeOfFeature1 
            {
            DTMFeatureType get()
                {
                return m_dtmFeatureType1 ;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets id of feature 1.
        /// </summary>                
        //=======================================================================================
        property DTMFeatureId IdOfFeature1
            {
            DTMFeatureId get()
                {
                return m_dtmFeatureId1;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets segment offset of feature 1
        /// </summary>                
        //=======================================================================================
        property long SegmentOfsetOnFeature1
            {
            long get()
                {
                return m_segmentOfset1;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets elevation of feature 1 at intersection.
        /// </summary>                
        //=======================================================================================
        property double ElevationOnFeature1 
            {
            double get()
                {
                return m_elevation1 ;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets distance from segment start on feature 1.
        /// </summary>                
        //=======================================================================================
        property double DistanceOnFeature1  
            {
            double get()
                {
                return m_distance1  ;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets the type of the feature 2.
        /// </summary>                
        //=======================================================================================
        property DTMFeatureType TypeOfFeature2
            {
            DTMFeatureType get()
                {
                return m_dtmFeatureType2;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Segment Offset Of Feature 2
        /// </summary>                
        //=======================================================================================
        property DTMFeatureId IdOfFeature2
            {
            DTMFeatureId get()
                {
                return m_dtmFeatureId2;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Dtm Feature Type 2
        /// </summary>                
        //=======================================================================================
        property long SegmentOfsetOnFeature2
            {
            long get()
                {
                return m_segmentOfset2;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Z Coordinate Of Intersection Point For Feature 2
        /// </summary>                
        //=======================================================================================
        property double ElevationOfFeature2 
            {
            double get()
                {
                return m_elevation2 ;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Distance From Feature 1 Segment Start
        /// </summary>                
        //=======================================================================================
        property double DistanceOnFeature2  
            {
            double get()
                {
                return m_distance2  ;
                }
            }

    internal:

        DTMCrossingFeatureError(DTM_CROSSING_FEATURE_ERROR& error)
            {
            m_intersectionX = error.intersectionX;
            m_intersectionY = error.intersectionY;
            DTMFeatureType type;
            DTMHelpers::Copy (type, error.dtmFeatureType1);
            m_dtmFeatureType1 = type;
            m_dtmFeatureId1 = DTMFeatureId(error.dtmFeatureId1);
            m_segmentOfset1 = error.segmentOfset1;
            m_elevation1 = error.elevation1;
            m_distance1 = error.distance1;
            DTMHelpers::Copy (type, error.dtmFeatureType2);
            m_dtmFeatureType2 = type;
            m_dtmFeatureId2 = DTMFeatureId(error.dtmFeatureId2);
            m_segmentOfset2 = error.segmentOfset2;
            m_elevation2 = error.elevation2;
            m_distance2 = error.distance2;
            }

    private:

        double m_intersectionX;             /* X Coordinate Of Intersection Point               */  
        double m_intersectionY;             /* Y Coordinate Of Intersection Point               */
        DTMFeatureType m_dtmFeatureType1;   /* Dtm Feature Type 1                               */ 
        DTMFeatureId m_dtmFeatureId1;       /* Dtm Feature Id 1                                 */ 
        long m_segmentOfset1;               /* Segment Offset Of Feature 1                      */
        double m_elevation1;                /* Z Coordinate Of Intersection Point For Feature 1 */
        double m_distance1;                 /* Distance From Feature 1 Segment Start            */    
        DTMFeatureType m_dtmFeatureType2;   /* Dtm Feature Type 2                               */
        DTMFeatureId m_dtmFeatureId2;       /* Segment Offset Of Feature 2                      */
        long m_segmentOfset2;               /* Dtm Feature Type 2                               */
        double m_elevation2;                /* Z Coordinate Of Intersection Point For Feature 2 */
        double m_distance2;                 /* Distance From Feature 1 Segment Start            */
    };


//=======================================================================================
/// <summary>
/// Defines values for the Purge flags
/// </summary>                
//=======================================================================================
[Flags]
public enum class DTMPurgeFlags : unsigned long
    {
    //=======================================================================================
    /// <summary>
    /// Remove deleted features.
    /// </summary>                
    //=======================================================================================
    RemoveDeletedFeatures = 1,

    //=======================================================================================
    /// <summary>
    /// Delete features with errors in.
    /// </summary>                
    //=======================================================================================
    RemoveFeatureWithErrors = 2,

    //=======================================================================================
    /// <summary>
    /// Delete rollback features.
    /// </summary>
    //=======================================================================================
    RemoveRollBackFeature = 4,
    };

//=======================================================================================
/// <summary>
/// Defines values for the API Dtm State
/// </summary>                
//=======================================================================================
public enum class DtmState : unsigned long
    {
    //=======================================================================================
    /// <summary>
    /// The DTM Is Not Triangulated
    /// </summary>                
    //=======================================================================================
    Data = (int)DTMState::Data,

//    PointsSorted = (int)DTMState::PointsSorted,
//    DuplicatesRemoved = (int)DTMState::DuplicatesRemoved,

    //=======================================================================================
    /// <summary>
    /// The DTM is Triangulated
    /// </summary>                
    //=======================================================================================
    Tin = (int)DTMState::Tin,

    //=======================================================================================
    /// <summary>
    /// The DTM had errors
    /// </summary>                
    //=======================================================================================
    TinError = (int)DTMState::TinError,

    };


//=======================================================================================
/// <summary>
/// Defines Point Selection Alternatives For Browsing Points
/// </summary>                
//=======================================================================================
public enum class PointSelection: long
{
      //=======================================================================================
      /// <summary>
      /// Point That Are Not Part Of Any Feature 
      /// </summary>                
      //=======================================================================================
      NoneFeaturePoints = 1 ,

      //=======================================================================================
      /// <summary>
      /// Points that belong to features.
      /// </summary>                
      //=======================================================================================
      FeaturePoints = 2 ,

     //=======================================================================================
      /// <summary>
      /// Orphan and Feature Points
      /// </summary>                
      //=======================================================================================
      AllPoints = 3 ,
} ;
//=======================================================================================
/// <summary>
/// Visibility Enumeration Types
/// </summary>                
/// <author>Rob.Cormack</author>                              <date>03/2010</date>
//=======================================================================================
public enum class VisibilityType : long
{
    /// <summary>
    /// Visibile
    /// The Feature Is Visible To The Eye
    /// </summary>
    Visible = 0,
    /// <summary>
    /// Invisibile
    /// The Feature Is Invisible To The Eye
    /// </summary>
    Invisible = 1,
    /// <summary>
    /// Partial
    /// The Feature Is Partially Visible To The Eye
    /// </summary>
    Partial = 2, 
} ;


//=======================================================================================
/// <summary>
/// Defines edge option. This is used in SetTriangulationParameters to determine
/// criteria used to remove edge triangles.
/// </summary>                
//=======================================================================================
public enum class DTMEdgeOption: long
    {
    //=======================================================================================
    /// <summary>
    /// Do not remove any triangle.
    /// Todo SPU: Ask whether we should use a DTM constant here ?
    /// </summary>                
    //=======================================================================================
    NoRemove = 1,

    //=======================================================================================
    /// <summary>
    /// Remove sliver triangles on hull.
    /// TODO SPu: ask what is a sliver triangle.
    /// Todo SPU: Ask whether we should use a DTM constant here ?
    /// </summary>                
    //=======================================================================================
    RemoveSliver = 2,

    //=======================================================================================
    /// <summary>
    /// Remove triangles on hull whose length > maxSide.
    /// Todo SPU: Ask whether we should use a DTM constant here ?
    /// </summary>                
    //=======================================================================================
    RemoveMaxSide = 3,
    };

//=======================================================================================
/// <summary>
/// Defines the clipping method.
/// </summary>                
//=======================================================================================
public enum class DTMClippingMethod:long
    {
    /// <summary>
    /// Keep triangles inside the clipping polygon.
    /// Todo SPU: Ask whether the triangles are clipped ?
    /// Todo SPU: Ask whether we should use a DTM constant here ?
    /// </summary>
    Internal = (long)::DTMClipOption::Internal,

    /// <summary>
    /// Keep triangles outside the clipping polygon.
    /// Todo SPU: Ask whether the triangles are clipped ?
    /// Todo SPU: Ask whether we should use a DTM constant here ?
    /// </summary>
    External = (long)::DTMClipOption::External
    };

//=======================================================================================
/// <summary>
/// Defines value for Fence Option
/// </summary>                
//=======================================================================================
public enum class DTMFenceOption:long
    {
    /// <summary>
    /// Browse and clip feature internal to fence.
    /// </summary>
    Internal = (long)::DTMFenceOption::Inside,

    /// <summary>
    /// Browse feature internal to fence.
    /// </summary>
    Overlap = (long)::DTMFenceOption::Overlap,

    /// <summary>
    /// Browse and clip feature external to fence.
    /// </summary>
    External = (long)::DTMFenceOption::Outside
    };            

//=======================================================================================
/// <summary>
/// Defines fence type.
/// </summary>                
//=======================================================================================
public enum class DTMFenceType:long
    {
    /// <summary>
    /// Block fence defined by 2 points.
    /// </summary>
    Block = (long)::DTMFenceType::Block,

    /// <summary>
    /// Shape fence defined by 3 or more points.
    /// </summary>
    Shape = (long)::DTMFenceType::Shape
    };            


//=======================================================================================
/// <summary>
/// Defines value for Smoothing method
/// </summary>                
//=======================================================================================
public enum class DTMContourSmoothingMethod:long
    {
    /// <summary>
    /// No smoothing.
    /// </summary>
    None = (long)DTMContourSmoothing::None,

    /// <summary>
    /// TODO Spu: Ask what to put here. 
    /// </summary>
    Vertex = (long)DTMContourSmoothing::Vertex,

    /// <summary>
    /// TODO Spu: Ask what to put here. 
    /// </summary>
    Spline = (long)DTMContourSmoothing::Spline,

    /// <summary>
    /// TODO Spu: Ask what to put here. 
    /// </summary>
    SplineWithoutOverLapDetection = (long)DTMContourSmoothing::SplineWithoutOverLapDetection
    };            

//=======================================================================================
/// <summary>
/// Defines code of draped point
/// </summary>                
/// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
//=======================================================================================
public enum class DTMDrapedPointCode: long
    {
    /// <summary>
    /// Drape Point External To Triangulation.
    /// </summary>
    External = (long)::DTMDrapePointCode::External,

    /// <summary>
    /// Drape Point inside a triangle.
    /// </summary>
    Triangle = (long)::DTMDrapePointCode::Tin,

    /// <summary>
    /// Point in a void area.
    /// </summary>
    Void = (long)::DTMDrapePointCode::Void,

    /// <summary>
    /// Point Coincident with a triangle vertex or triangle side.
    /// </summary>
    PointOrSide = (long)::DTMDrapePointCode::PointOrLine,
    };


 //=======================================================================================
 /// <summary>
 /// Holds The Results Of The Pond Calculation
 /// </summary>                
 /// <author>Rob.Cormack</author>                              <date>03/2010</date>
 //=======================================================================================
public ref struct PondCalculation
    {
    public:

    //=======================================================================================
    /// <summary>
    /// Gets Wether the Pond Was Calculated
    /// </summary>                
    //=======================================================================================
    property bool Calculated 
        {
        bool get()
            {
            return pondCalculated ;
            }
        }

    //=======================================================================================
    /// <summary>
    /// Gets the Pond Elevation
    /// </summary>                
    //=======================================================================================
    property double Elevation
        {
        double get()
            {
            return pondElevation ;
            }
        }

    //=======================================================================================
    /// <summary>
    /// Gets the pond Depth
    /// </summary>                
    //=======================================================================================
    property double Depth
        {
        double get()
            {
            return pondDepth ;
            }
        }

    //=======================================================================================
    /// <summary>
    /// Gets the Pond Area 
    /// </summary>                
    //=======================================================================================
    property double Area
        {
        double get()
            {
            return pondArea;
            }
        }

    //=======================================================================================
    /// <summary>
    /// Gets the Pond Volume 
    /// </summary>                
    //=======================================================================================
    property double Volume
        {
        double get()
            {
            return pondVolume;
            }
        }

    //=======================================================================================
    /// <summary>
    /// Gets the Pond Features 
    /// </summary>                
    //=======================================================================================
    property array<DTMDynamicFeatureInfo^>^ PondFeatures
        {
        array<DTMDynamicFeatureInfo^>^ get()
            {
            return pondFeatures;
            }
        }

    private:
    
    bool     pondCalculated ;
    double   pondElevation ;
    double   pondDepth ;
    double   pondArea ;
    double   pondVolume ;
    array<DTMDynamicFeatureInfo^>^ pondFeatures ;
    
    internal:

    PondCalculation (bool calculated , double elevation, double depth, double area, double volume ,array<DTMDynamicFeatureInfo^>^ features )
        {
            pondCalculated = calculated ;
            pondElevation  = elevation ;
            pondDepth      = depth ;
            pondArea       = area ;
            pondVolume     = volume ;
            pondFeatures   = features ;
        }
    } ;

//=======================================================================================
/// <summary>
/// Holds The Results Of A TraceCatchmentForPoint
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>08/2014</date>
//=======================================================================================
public ref struct PointCatchmentResult
    {
private:
    array<DTMDynamicFeatureInfo^>^ m_catchment;
    BGEO::DPoint3d m_sumpPoint;

    internal: PointCatchmentResult (array<DTMDynamicFeatureInfo^>^ catchment, BGEO::DPoint3d sumpPoint) : m_catchment (catchment), m_sumpPoint (sumpPoint)
        {
        }
    public:
        property array<DTMDynamicFeatureInfo^>^ Catchment
        { 
        array<DTMDynamicFeatureInfo^>^ get ()
            {
            return m_catchment;
            }
        }
    property BGEO::DPoint3d SumpPoint
        {
        BGEO::DPoint3d get ()
            {
            return m_sumpPoint;
            }
        }
    };

//=======================================================================================
/// <summary>
/// Holds The Results Of A Visibility Calculation
/// </summary>                
/// <author>Rob.Cormack</author>                              <date>03/2010</date>
//=======================================================================================
 public ref struct VisibilityResult
     {

        private:
 
        VisibilityType                     visibilityType ;   
        array<DTMDynamicFeatureInfo^>^  visibilityFeatures ;
    
        internal:

        VisibilityResult ( VisibilityType visibility, array<DTMDynamicFeatureInfo^>^ features )
            {
             visibilityType = visibility ;
             visibilityFeatures = features ;
            }

        public :

        //=======================================================================================
        /// <summary>
        /// Gets The Visibility Type
        /// </summary>                
        //=======================================================================================
         property VisibilityType Visibility 
           {
             VisibilityType get()
                {
                return visibilityType ;
                }
            } 
    
        //=======================================================================================
        /// <summary>
        /// Gets the Visibility Features 
        /// </summary>                
        //=======================================================================================
        property array<DTMDynamicFeatureInfo^>^ Features
            {
            array<DTMDynamicFeatureInfo^>^ get()
                {
                return visibilityFeatures;
                }
            }
     } ;

//=======================================================================================
/// <summary>
/// Holds cut and fill operation result.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>02/2008</date>
//=======================================================================================
public ref struct CutFillResult
    {
    public:

        //=======================================================================================
        /// <summary>
        /// Gets the cut volume.
        /// </summary>                
        //=======================================================================================
        property double CutVolume
            {
            double get()
                {
                return _cutVolume;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets the fill volume.
        /// </summary>                
        //=======================================================================================
        property double FillVolume
            {
            double get()
                {
                return _fillVolume;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets the cut area.
        /// </summary>                
        //=======================================================================================
        property double CutArea
            {
            double get()
                {
                return _cutArea;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets the fill area.
        /// </summary>                
        //=======================================================================================
        property double FillArea
            {
            double get ()
                {
                return _fillArea;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets the area.
        /// </summary>                
        //=======================================================================================
        property double Area
            {
            double get ()
                {
                return _totalArea;
                }
            }
        //=======================================================================================
        /// <summary>
        /// Gets the balance area.
        /// </summary>                
        //=======================================================================================
        property double BalanceVolume
            {
            double get()
                {
                return _balanceVolume;
                }
            }

    private:

        double _cutVolume;
        double _fillVolume;
        double _balanceVolume;
        double _totalArea;
        double _cutArea;
        double _fillArea;

    internal:

        CutFillResult (const BcDTMVolumeAreaResult& result)
            {
            _cutVolume = result.cutVolume;
            _fillVolume = result.fillVolume;
            _balanceVolume = result.balanceVolume;
            _totalArea = result.totalArea;
            _cutArea = result.cutArea;
            _fillArea = result.fillArea;
            }
        CutFillResult (double cutVolume, double fillVolume, double balanceVolume, double totalArea, double cutArea, double fillArea)
            {
            _cutVolume = cutVolume;
            _fillVolume = fillVolume;
            _balanceVolume = balanceVolume;
            _totalArea = totalArea;
            _cutArea = cutArea;
            _fillArea = fillArea;
            }
    };
//=======================================================================================
/// <summary>
/// Holds The Volume Polygons returned From The Core Volume Functions
/// </summary>                
/// <author>Rob.Cormack</author>                              <date>03/2009</date>
//=======================================================================================
 public ref class VolumePolygon 
    {
    internal:

        VolumePolygon(DPoint3d* pointsP, int numPoints )
           {
            _points = gcnew array<BGEO::DPoint3d>(numPoints);
            pin_ptr<BGEO::DPoint3d const> pPoints = &_points[0];
            memcpy((void*)pPoints,pointsP,numPoints*sizeof(DPoint3d)) ;
           }
    
     public:
 

         property array<BGEO::DPoint3d>^ Points 
            {
             array<BGEO::DPoint3d>^ get()
               {
                return _points ;
               }
            }
     
         property int NumPoints 
            {
             int get()
               {
                return _points->Length ;
               } 
            }

    private:
     array<BGEO::DPoint3d>^ _points ;

    };
//=======================================================================================
/// <summary>
/// Holds Results Of The Volume Calculation
/// </summary>                
/// <author>Rob.Cormack</author>                              <date>03/2009</date>
//=======================================================================================
 public ref struct VolumeResult : CutFillResult
    {
    public:

        //=======================================================================================
        /// <summary>
        /// Gets the From Area
        /// </summary>                
        //=======================================================================================
        property double FromArea
            {
            double get()
                {
                return _fromArea ;
                }
            }
       //=======================================================================================
        /// <summary>
        /// Gets the To Area
        /// </summary>                
        //=======================================================================================
        property double ToArea
            {
            double get()
                {
                return _toArea ;
                }
            }
    
        //=======================================================================================
        /// <summary>
        /// Gets the Number Of Cells Used
        /// </summary>                
        //=======================================================================================
        property int NumCellsUsed
            {
             int get()
                {
                return _numCellsUsed ;
                }
            }
 
        //=======================================================================================
        /// <summary>
        /// Gets Cell Area
        /// </summary>                
        //=======================================================================================
        property double CellArea
            {
            double get()
                {
                return _cellArea ;
                }
            }
   
        //=======================================================================================
        /// <summary>
        /// Gets the Number Of Volume Polygons
        /// </summary>                
        //=======================================================================================
        property int NumVolumePolygons
            {
            int get()
                {
                return _volumePolygons->Length ;
                }
            }

       //=======================================================================================
        /// <summary>
        /// Gets the Volume Polygon Array
        /// </summary>                
        //=======================================================================================
        property array<VolumePolygon^>^ VolumePolygons
            {
             array<VolumePolygon^>^ get()
                {
                return _volumePolygons;
                }
            }

    private:

        double _fromArea ;
        double _toArea ;
        int    _numCellsUsed ;   
        double _cellArea ;
        array <VolumePolygon^>^ _volumePolygons ;

    internal:

//      Constructor For Prismoidal Volume Calculations

        VolumeResult (const BcDTMVolumeAreaResult& result, DtmVectorString& volumePolygons) : CutFillResult (result)
            {
            _fromArea      = 0.0 ;
            _toArea        = 0.0 ;
            _numCellsUsed  = 0 ;
            _cellArea      = 0.0 ;
            _volumePolygons = nullptr ;
 
//           Count Number Of Volume Polygons

             int numVolumePolygons = 0 ; 
             for( DtmVectorString::iterator iter = volumePolygons.begin() ; iter < volumePolygons.end() ; iter++ )
               {
                if( iter->size() ) ++numVolumePolygons ;
               }

//           Store Volume Polygons In Array

             if( numVolumePolygons )
               {
                int numPolygons = 0 ;
                _volumePolygons = gcnew array <VolumePolygon^>(numVolumePolygons) ;  
                for( DtmVectorString::iterator iter = volumePolygons.begin() ; iter < volumePolygons.end() ; ++iter )
                  {
                   if( iter->size() )
                     {
                      _volumePolygons[numPolygons] = gcnew VolumePolygon(iter->data(), (int)iter->size()) ;
                      ++numPolygons ;
                     }
                  } 
               } 
            }
   
//      Constructor For Grid Volume Calculations

        VolumeResult (const BcDTMVolumeAreaResult& result, int numCellsUsed, double cellArea, DtmVectorString& volumePolygons) : CutFillResult (result)
            {
            _numCellsUsed  = numCellsUsed ;
            _cellArea      = cellArea ;
            _fromArea      = 0.0 ;
            _toArea        = 0.0 ;
            _volumePolygons = nullptr ;

//           Count Number Of Volume Polygons

             int numVolumePolygons = 0 ; 
             for( DtmVectorString::iterator iter = volumePolygons.begin() ; iter < volumePolygons.end() ; iter++ )
               {
                if( iter->size() ) ++numVolumePolygons ;
               }

//           Store Volume Polygons In Array

             if( numVolumePolygons )
               {
                int numPolygons = 0 ;
                _volumePolygons = gcnew array <VolumePolygon^>(numVolumePolygons) ;  
                for( DtmVectorString::iterator iter = volumePolygons.begin() ; iter < volumePolygons.end() ; ++iter )
                  {
                   if( iter->size() )
                     {
                      _volumePolygons[numPolygons] = gcnew VolumePolygon(iter->data(), (int)iter->size()) ;
                      ++numPolygons ;
                     }
                  } 
               } 
          }


//      Constructor For Prismoidal Volume Balance Calculations

        VolumeResult (double fromArea, double toArea, double balanceVolume, DtmVectorString volumePolygons) : CutFillResult (0, 0, balanceVolume, 0, 0, 0)
            {
            _fromArea      = fromArea;
            _toArea        = toArea ;
            _numCellsUsed  = 0 ;
            _cellArea      = 0.0 ;
            _volumePolygons = nullptr ;
//           Count Number Of Volume Polygons

             int numVolumePolygons = 0 ; 
             for( DtmVectorString::iterator iter = volumePolygons.begin() ; iter < volumePolygons.end() ; iter++ )
               {
                if( iter->size()) ++numVolumePolygons ;
               }

//           Store Volume Polygons In Array

             if( numVolumePolygons )
               {
                int numPolygons = 0 ;
                _volumePolygons = gcnew array <VolumePolygon^>(numVolumePolygons) ;  
                for( DtmVectorString::iterator iter = volumePolygons.begin() ; iter < volumePolygons.end() ; ++iter )
                  {
                   if( iter->size() )
                     {
                      _volumePolygons[numPolygons] = gcnew VolumePolygon(iter->data(), (int)iter->size()) ;
                      ++numPolygons ;
                     }
                  } 
              } 
          }
 
    };
//=======================================================================================
/// <summary>
/// Holds Interpolation Results
/// </summary>                
/// <author>Rob.Cormack</author>                              <date>01/2010</date>
//=======================================================================================
public ref struct InterpolationResult
    {
    public:

        //=======================================================================================
        /// <summary>
        /// Gets the Number Of Dtm Features
        /// </summary>                
        //=======================================================================================
        property double NumDtmFeatures
            {
            double get()
                {
                return _numDtmFeatures;
                }
            }

       //=======================================================================================
        /// <summary>
        /// Gets the Number Of Dtm Features Interpolated
        /// </summary>                
        //=======================================================================================
        property double NumDtmFeaturesInterpolated
            {
            double get()
                {
                return _numDtmFeaturesInterpolated ;
                }
            }

 
    private:

        int _numDtmFeatures;
        int _numDtmFeaturesInterpolated;

    internal:

//      Constructor For Interpolation Result

        InterpolationResult (int numDtmFeatures , int numDtmFeaturesInterpolated)
            {
             _numDtmFeatures             = numDtmFeatures ;
             _numDtmFeaturesInterpolated = numDtmFeaturesInterpolated ;
            }
   
 
    };

//=======================================================================================
/// <summary>
/// Holds slope area operation result.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>02/2008</date>
//=======================================================================================
public ref struct SlopeAreaResult
    {
    public:

        //=======================================================================================
        /// <summary>
        /// Gets the Area
        /// </summary>                
        //=======================================================================================
        property double Area
            {
            double get()
                {
                return _area;
                }
            }

    private:

        double _area;

    internal:

        SlopeAreaResult(double area)
            {
            _area = area;
            }
    };


//=======================================================================================
/// <summary>
/// Holds volume range.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>03/2008</date>
//=======================================================================================
public ref struct VolumeRange
    {
    public:

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the VolumeRange class.
        /// </summary>                
        //=======================================================================================
        VolumeRange (double low, double high)
            {
            m_low = low;
            m_high = high;
            }

  
        //=======================================================================================
        /// <summary>
        /// Gets/sets the low elevation of the volume.
        /// </summary>                
        //=======================================================================================
        property double Low
            {
            double get()
                {
                return m_low;
                }
            void set(double value)
                {
                m_low = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/sets the high elevation of the volume.
        /// </summary>                
        //=======================================================================================
        property double High
            {
            double get()
                {
                return m_high;
                }
            void set(double value)
                {
                m_high = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets the cut volume for the elevation range.
        /// </summary>                
        //=======================================================================================
        property double Cut
            {
            double get()
                {
                return m_cut;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets the fill volume for the elevatio range.
        /// </summary>                
        //=======================================================================================
        property double Fill
            {
            double get()
                {
                return m_fill;
                }
            }

    internal:

        void SetCutFillValues(double cut, double fill)
            {
            m_cut = cut;
            m_fill = fill;
            }

    private:

        double m_low;
        double m_high;
        double m_cut;
        double m_fill;
    };

//=======================================================================================
/// <summary>
/// Defines filter options.
/// </summary>                
//=======================================================================================
public enum class FilterOption : long
    {
    /// <summary>
    /// Fine filtering.
    /// </summary>
    Fine = 1,

    /// <summary>
    /// Coarse filtering.
    /// </summary>
    Coarse = 2
    };
//=======================================================================================
/// <summary>
/// Defines Radial View Shed Option.
/// </summary>                
//=======================================================================================
public enum class RadialViewShedOption : long
    {

    /// <summary>
    /// Coarse filtering.
    /// </summary>
    UseRadialIncrement = 0,

    /// <summary>
    /// Use Number Of Radials
    /// </summary>
    UseNumberOfRadials = 1,

    };
    
//=======================================================================================
/// <summary>
/// Defines Stock Pile Type
/// </summary>                
/// <author>Rob.Cormack</author>                              <date>10/2012</date>
//=======================================================================================
public enum class StockPileFeature 
    {

    /// <summary>
    /// Point Stock Pile
    /// </summary>
    PointStockPile = 0,

    /// <summary>
    /// Alignment Stock Pile
    /// </summary>
    AlignmentStockpile = 1,

    };

//=======================================================================================
/// <summary>
/// Stock Pile Criteria
/// </summary>                
/// <author>Rob.Cormack</author>                              <date>10/2012</date>
//=======================================================================================

public ref class StockPileCriteria

   { 

    public:

        //=======================================================================================
        /// <summary>
        /// Gets or Sets the Stock Pile Coordinates
        /// </summary>                
        //=======================================================================================
        
        property array<BGEO::DPoint3d>^ StockPilePoints
            {
            array<BGEO::DPoint3d>^ get()
                {
                return m_stockPilePoints  ;
                }
            void set(array<BGEO::DPoint3d>^ points)
                {
                 m_stockPilePoints = points;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets or Sets the Stock Pile Type
        /// </summary>                
        //=======================================================================================
        
        property StockPileFeature StockPileType
            {
             StockPileFeature get()
                {
                return m_stockPileType ;
                }
            void set( StockPileFeature value)
                {
                 m_stockPileType = value;
                }
            }

       //=======================================================================================
        /// <summary>
        /// Gets or Sets the Stock Pile Slope
        /// </summary>                
        //=======================================================================================
        
        property double StockPileSlope
            {
             double get() 
               {
                return m_stockPileSlope ;
               }
             void set ( double value )
               {
                m_stockPileSlope = value ;
               }  
            }

        //=======================================================================================
        /// <summary>
        /// Gets or Sets the Merge Oprtion
        /// </summary>                
        //=======================================================================================
        
        property bool MergeOption
            {
             bool get()
                {
                return m_mergeOption ;
                }
            void set( bool value)
                {
                 m_mergeOption = value ;
                }
            }

    private:

        array<BGEO::DPoint3d>^ m_stockPilePoints ;           //  Coordinates For Calculating Stockpile
        StockPileFeature       m_stockPileType   ;           //  Stock Pile Type
        bool                   m_mergeOption     ;           //  Merge Option. If True Create Merged DTM Og Ground And Stock Pile
        double                 m_stockPileSlope  ;           //  Slope Of Stock Pile Expressed As Ratio of run/rise 
                    
    };

//=======================================================================================
/// <summary>
/// Stock Pile Result
/// </summary>                
/// <author>Rob.Cormack</author>                              <date>10/2012</date>
//=======================================================================================

public ref class StockPileResult
   {
   
    internal : 
    
      StockPileResult(double volume , DTM^ stockPileTM , DTM ^ mergedTM )
         {
          m_stockPileVolume = volume ;
          m_stockPileDTM    = stockPileTM ;
          m_mergedDTM       = mergedTM ;
         }

    public:

        //=======================================================================================
        /// <summary>
        /// Gets the Stock Pile Volume
        /// </summary>                
        //=======================================================================================
        
        property double StockPileVolume
            {
             double get() 
               {
                return m_stockPileVolume ;
               }
             void set ( double volume )
               {
                m_stockPileVolume = volume ;
               }  
            }

        //=======================================================================================
        /// <summary>
        /// Gets/Sets The Stock Pile DTM
        /// </summary>                
        //=======================================================================================
        
         property DTM^ StockPileDTM 
            {
             DTM^ get()
                {
                return m_stockPileDTM ;
                }
               void set ( DTM^ value )
                 {
                  m_stockPileDTM =  value ;
                 }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/Sets The Merged DTM
        /// </summary>                
        //=======================================================================================
        
        property DTM^ MergedDTM 
            {
             DTM^ get()
                {
                 return m_mergedDTM ;
                }
              void set ( DTM^ value )
                {
                 m_mergedDTM =  value ;
                }
            }

    private:

        double                 m_stockPileVolume ;           //  Volume Of Stock Pile
        DTM^                   m_stockPileDTM    ;           //  Stock Pile DTM 
        DTM^                   m_mergedDTM     ;             //  Merged DTM Of Ground And Stock Pile
                    
    };


//=======================================================================================
/// <summary>
/// Holds filtering results.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>03/2008</date>
//=======================================================================================
public ref class FilterResult
    {
    public: 

        //=======================================================================================
        /// <summary>
        /// Gets the number of points after filtering.
        /// </summary>                
        //=======================================================================================
        property int NumberOfPointsAfterFiltering
            {
            int get()
                {
                return m_numPointsAfter;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets the number of points before filtering.
        /// </summary>                
        //=======================================================================================
        property int NumberOfPointsBeforeFiltering
            {
            int get()
                {
                return m_numPointsBefore;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets the filter reduction.
        /// </summary>                
        //=======================================================================================
        property double FilterReduction
            {
            double get()
                {
                return m_filterReduction;
                }
            }

    internal:

        FilterResult (int numPointsBefore, int numPointsAfter, double filterReduction)
            {
            m_numPointsBefore = numPointsBefore;
            m_numPointsAfter = numPointsAfter;
            m_filterReduction = filterReduction;
            }

    private:

        int m_numPointsBefore;
        int m_numPointsAfter;
        double m_filterReduction;
    };

//=======================================================================================
/// <summary>
/// Holds filtering criteria.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>03/2008</date>
//=======================================================================================
public ref class TinFilterCriteria
    {
    public:

        //=======================================================================================
        /// <summary>
        /// Gets/sets the Z tolerance.
        /// </summary>                
        //=======================================================================================
        property double ZTolerance
            {
            double get()
                {
                return m_zTolerance;
                }
            void set(double value)
                {
                m_zTolerance = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/sets the filter option.
        /// </summary>                
        //=======================================================================================
        property FilterOption Option
            {
            FilterOption get()
                {
                return m_filterOption;
                }
            void set(FilterOption value)
                {
                m_filterOption = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/sets the reinsert option.
        /// TODO SPu: Ask whether we can find a better name here.
        /// </summary>                
        //=======================================================================================
        property bool ReinsertOption
            {
            bool get()
                {
                return m_reinsertOption;
                }
            void set(bool value)
                {
                m_reinsertOption = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the FilterCriteria class.
        /// </summary>                
        //=======================================================================================
        TinFilterCriteria()
            {
            m_filterOption = FilterOption::Fine;
            m_reinsertOption = true;
            m_zTolerance = 0.25;
            }

    private:

        FilterOption m_filterOption;
        bool m_reinsertOption;
        double m_zTolerance;
    };

//=======================================================================================
/// <summary>
/// Holds filtering criteria for a tile.
/// TODO SPu: ask what is this ?
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>03/2008</date>
//=======================================================================================
public ref class TileFilterCriteria
    {
    public:

        //=======================================================================================
        /// <summary>
        /// Gets/sets the Z tolerance.
        /// </summary>                
        //=======================================================================================
        property double ZTolerance
            {
            double get()
                {
                return m_zTolerance;
                }
            void set(double value)
                {
                m_zTolerance = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/sets max tile points.
        /// TODO SPu: ask what is this ?
        /// </summary>                
        //=======================================================================================
        property long MaxTilePoints
            {
            long get()
                {
                return m_maxTilePoints;
                }
            void set(long value)
                {
                m_maxTilePoints = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/sets the MaxTileDivide.
        /// TODO SPu: ask what is this ?
        /// </summary>                
        //=======================================================================================
        property long MaxTileDivide
            {
            long get()
                {
                return m_maxTileDivide;
                }
            void set(long value)
                {
                m_maxTileDivide = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/sets the tile length.
        /// </summary>                
        //=======================================================================================
        property double TileLength
            {
            double get()
                {
                return m_tileLength;
                }
            void set(double value)
                {
                m_tileLength = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the TileFilterCriteria class.
        /// </summary>                
        //=======================================================================================
        TileFilterCriteria()
            {
            m_maxTilePoints = 5;
            m_maxTileDivide = 5;
            m_tileLength = 0;
            m_zTolerance = 0.25;
            }

    private:

        long m_maxTilePoints;
        long m_maxTileDivide;
        double m_tileLength;
        double m_zTolerance;
    };

//=======================================================================================
/// <summary>
/// Holds crossing features browsing criteria.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>03/2008</date>
//=======================================================================================
public ref class CrossingFeaturesBrowsingCriteria
    {
    public:

        //=======================================================================================
        /// <summary>
        /// Gets/Sets the feature list
        /// </summary>                
        //=======================================================================================
        property array<DTMFeatureType>^ FeatureTypes
            {
            array<DTMFeatureType>^ get()
                {
                return m_featureList;
                }
            void set(array<DTMFeatureType>^ value)
                {
                m_featureList = value;
                }
            }

    private:

        array<DTMFeatureType>^ m_featureList;
    };

//=======================================================================================
/// <summary>
/// Holds slope indicator browsing criteria.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>02/2008</date>
//=======================================================================================
public ref class SlopeIndicatorsBrowsingCriteria
    {
    public:

        //=======================================================================================
        /// <summary>
        /// Gets/sets major interval.
        /// </summary>                
        //=======================================================================================
        property double MajorInterval
            {
            double get()
                {
                return _majorInterval;
                }
            void set (double value)
                {
                _majorInterval = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/sets minor interval.
        /// </summary>                
        //=======================================================================================
        property double MinorInterval
            {
            double get()
                {
                return _minorInterval;
                }
            void set (double value)
                {
                _minorInterval = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the SlopeIndicatorsBrowsingCriteria class.
        /// </summary>                
        //=======================================================================================
        SlopeIndicatorsBrowsingCriteria (double majorInterval, double minorInterval)
            {
            _majorInterval = majorInterval;
            _minorInterval = minorInterval;
            }

    private:

        double _majorInterval;
        double _minorInterval;
    };

//=======================================================================================
/// <summary>
/// Holds path tracing criteria.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>02/2008</date>
//=======================================================================================
public ref class PathTracingCriteria
    {
    };

//=======================================================================================
/// <summary>
/// Holds line tracing criteria.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>02/2008</date>
//=======================================================================================
public ref class LineTracingCriteria
    {
    };

//=======================================================================================
/// <summary>
/// Holds browsing criteria.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>02/2008</date>
//=======================================================================================
public ref class BrowsingCriteria
    {
    public:

        //=======================================================================================
        /// <summary>
        /// Sets a fence block.
        /// </summary>                
        //=======================================================================================
        void SetFenceToBlock (BGEO::DRange3d value);

        //=======================================================================================
        /// <summary>
        /// Gets/sets the Cache Size.
        /// TODO SPU: Ask what is this ?
        /// </summary>                
        //=======================================================================================
        property long CacheSize
            {
            long get()
                {
                return _cacheSize;
                }

            void set(long value)
                {
                _cacheSize = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/sets the fence points.
        /// </summary>                
        //=======================================================================================
        property array<BGEO::DPoint3d>^ FencePoints
            {
            array<BGEO::DPoint3d>^ get()
                {
                return _fencePoints;
                }

            void set(array<BGEO::DPoint3d>^ value)
                {
                _fencePoints = gcnew array<BGEO::DPoint3d> (value->Length);
                value->CopyTo (_fencePoints, 0);
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/sets the fence option.
        /// </summary>                
        //=======================================================================================
        property DTMFenceOption FenceOption
            {
            DTMFenceOption get()
                {
                return _fenceOption;
                }
            void set(DTMFenceOption value)
                {
                _fenceOption = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/sets the fence type.
        /// TODO SPU: Ask whether this cannot conflict with the number of fence points.
        /// </summary>                
        //=======================================================================================
        property DTMFenceType FenceType
            {
            DTMFenceType get()
                {
                return _fenceType;
                }
            void set(DTMFenceType value)
                {
                _fenceType = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the BrowsingCriteria class.
        /// </summary>                
        //=======================================================================================
        BrowsingCriteria()
            {
            _fenceOption = DTMFenceOption::Internal;
            _fenceType   = DTMFenceType::Shape;
            _fencePoints = nullptr;
            _cacheSize = 10000;
            }

    private:

        array<BGEO::DPoint3d>^ _fencePoints ;
        DTMFenceOption _fenceOption ;
        DTMFenceType   _fenceType ;
        long _cacheSize;
    };
//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2009
//=======================================================================================

public ref class PointsBrowsingCriteria : public BrowsingCriteria
    {
    public:

        //=======================================================================================
        /// <summary>
        /// Gets/sets maximum points by page.
        /// </summary>                
        //=======================================================================================
        property int MaxPoints
            {
            int get()
                {
                return m_maxPoints;
                }
            void set(int value)
                {
                m_maxPoints = value;
                }
            }

       //=======================================================================================
        /// <summary>
        /// Gets?sets The Points Selection Option
        /// </summary>                
        //=======================================================================================
        property PointSelection SelectionOption
           {
            PointSelection get()
                {
                return m_pointOption ;
                }
            void set( PointSelection value )
                {
                 m_pointOption = value ;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the PointBrowsingCriteria class.
        /// </summary>                
        //=======================================================================================
        PointsBrowsingCriteria ()
            {
             m_maxPoints = 10000;
             m_pointOption = PointSelection::AllPoints ;
            }
    private:

        int m_maxPoints;
        PointSelection m_pointOption;
    };

//=======================================================================================
/// <summary>
/// Holds features browsing criteria.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>04/2008</date>
//=======================================================================================
public ref class LinearFeaturesBrowsingCriteria : public BrowsingCriteria
    {
    public:

        //=======================================================================================
        /// <summary>
        /// Gets/sets max spots.
        /// TODO Spu: Ask what is this ?
        /// </summary>                
        //=======================================================================================
        property int MaxSpots
            {
            int get()
                {
                return m_maxSpots;
                }
            void set(int value)
                {
                m_maxSpots = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the FeaturesBrowsingCriteria class.
        /// </summary>                
        //=======================================================================================
        LinearFeaturesBrowsingCriteria()
            {
            m_maxSpots = 10000;
            }

    private:

        int m_maxSpots;
    };

//=======================================================================================
/// <summary>
/// Holds features browsing criteria.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>04/2008</date>
//=======================================================================================
public ref class PointFeaturesBrowsingCriteria : public BrowsingCriteria
    {
    public:

        //=======================================================================================
        /// <summary>
        /// Gets/sets max spots.
        /// TODO Spu: Ask what is this ?
        /// </summary>                
        //=======================================================================================
        property int MaxPoints
            {
            int get()
                {
                return m_maxSpots;
                }
            void set(int value)
                {
                m_maxSpots = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the PointFeaturesBrowsingCriteria class.
        /// </summary>                
        //=======================================================================================
        PointFeaturesBrowsingCriteria()
            {
            m_maxSpots = 10000;
            }

    private:

        int m_maxSpots;
    };

//=======================================================================================
/// <summary>
/// Holds dynamic features browsing criteria.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>04/2008</date>
//=======================================================================================
public ref class DynamicFeaturesBrowsingCriteria : public BrowsingCriteria
    {
    public:

        //=======================================================================================
        /// <summary>
        /// Gets/sets max spots.
        /// TODO Spu: Ask what is this ?
        /// </summary>                
        //=======================================================================================
        property int MaxSpots
            {
            int get()
                {
                return m_maxSpots;
                }
            void set(int value)
                {
                m_maxSpots = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the DynamicFeaturesBrowsingCriteria class.
        /// </summary>                
        //=======================================================================================
        DynamicFeaturesBrowsingCriteria()
            {
            m_maxSpots = 10000;
            }

    private:

        int m_maxSpots;
    };
//=======================================================================================
/// <summary>
/// Holds Triangle Mesh browsing criteria.
/// </summary>                
/// <author>Rob.Cormack</author>                              <date>06/2009</date>
//=======================================================================================
public ref class TriangleMeshBrowsingCriteria : public BrowsingCriteria
    {
    public:

        //=======================================================================================
        /// <summary>
        /// Gets/sets max Triangles
        /// </summary>                
        //=======================================================================================
        property int MaxTriangles
            {
            int get()
                {
                return m_maxTriangles;
                }
            void set(int value)
                {
                m_maxTriangles = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the TriangleMeshBrowsingCriteria class.
        /// </summary>                
        //=======================================================================================
        TriangleMeshBrowsingCriteria()
            {
            m_maxTriangles = 50000;
            }

    private:

        int m_maxTriangles;
    };

//=======================================================================================
/// <summary>
/// Holds volume browsing criteria.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>03/2008</date>
//=======================================================================================
public ref class VolumeCriteria : public BrowsingCriteria
    {
    public:

        //=======================================================================================
        /// <summary>
        /// Gets and Sets Range Table
        /// </summary>                
        //=======================================================================================
        property array<VolumeRange^>^ RangeTable
            {
            array<VolumeRange^>^ get()
                {
                return m_volumeRange;
                }
            void set(array<VolumeRange^>^ value)
                {
                m_volumeRange= value;
                }
            }
        
        property array<BGEO::DPoint3d>^ VolumePolygon
            {
            array<BGEO::DPoint3d>^ get()
                {
                return m_volumePolygon;
                }
            void set ( array<BGEO::DPoint3d>^ value)
                {
                 m_volumePolygon = value ;
                }
            }

    
        property int Length
           {
            int get()
               {
                return m_volumePolygon->Length ;
               }
            }


    private:

        array<VolumeRange^>^ m_volumeRange;
        array<BGEO::DPoint3d>^ m_volumePolygon ;

    };


//=======================================================================================
/// <summary>
/// Holds catchments browsing criteria.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>02/2008</date>
//=======================================================================================
public ref class CatchmentsBrowsingCriteria : public BrowsingCriteria
    {

    };
//=======================================================================================
/// <summary>
/// Holds catchments browsing criteria.
/// </summary>                
/// <author>Rob.Cormack</author>                              <date>03/2011</date>
//=======================================================================================
public ref class CatchmentsCalculationCriteria : public BrowsingCriteria
    {

   public:

        //=======================================================================================
        /// <summary>
        /// Gets and Sets The Refine Option
        /// </summary>                
        //=======================================================================================
        property bool RefineOption
            {
            bool  get()
                {
                return m_refineOption;
                }
            void set( bool value)
                {
                m_refineOption = value;
                }
            }
        //=======================================================================================
        /// <summary>
        /// Gets and Sets The False Low Depth
        /// </summary>                
        //=======================================================================================
        property double FalseLowDepth
            {
            double  get()
                {
                return m_falseLowDepth;
                }
            void set( double value)
                {
                m_falseLowDepth = value;
                }
            }
  
    private:

        bool  m_refineOption ;
        double m_falseLowDepth ;

    };

//=======================================================================================
/// <summary>
/// Holds low points browsing criteria.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>02/2008</date>
//=======================================================================================
public ref class LowPointsBrowsingCriteria : public BrowsingCriteria
    {
    };

//=======================================================================================
/// <summary>
/// Holds high points browsing criteria.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>02/2008</date>
//=======================================================================================
public ref class HighPointsBrowsingCriteria : public BrowsingCriteria
    {
    };

//=======================================================================================
/// <summary>
/// Holds ridge lines browsing criteria.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>02/2008</date>
//=======================================================================================
public ref class RidgeLinesBrowsingCriteria : public BrowsingCriteria
    {
    };

//=======================================================================================
/// <summary>
/// Holds sump lines browsing criteria.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>02/2008</date>
//=======================================================================================
public ref class SumpLinesBrowsingCriteria : public BrowsingCriteria
    {
    };

//=======================================================================================
/// <summary>
/// Holds triangles browsing criteria.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>02/2008</date>
//=======================================================================================
public ref class TrianglesBrowsingCriteria : public BrowsingCriteria
    {
    };

//=======================================================================================
/// <summary>
/// Holds triangle edges browsing criteria.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>02/2008</date>
//=======================================================================================
public ref class TriangleEdgesBrowsingCriteria : public BrowsingCriteria
    {
    };

//=======================================================================================
/// <summary>
/// Holds analyzing browsing criteria.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>02/2008</date>
//=======================================================================================
public ref class AnalyzingBrowsingCriteria : public BrowsingCriteria
    {
    public:

        //=======================================================================================
        /// <summary>
        /// Gets/sets whether we want polygonized result.
        /// </summary>                
        //=======================================================================================
        property bool PolygonizedResult
            {
            bool get()
                {
                return _polygonizedResult;
                }
            void set(bool value)
                {
                _polygonizedResult = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/sets ranges.
        /// </summary>                
        //=======================================================================================
        property System::Collections::Generic::IEnumerable<BGEO::DRange1d>^ DoubleRange 
            {
            System::Collections::Generic::IEnumerable<BGEO::DRange1d>^ get()
                {
                return _doubleRange;
                }
            void set(System::Collections::Generic::IEnumerable<BGEO::DRange1d>^ value)
                {
                _doubleRange = gcnew System::Collections::Generic::List<BGEO::DRange1d>(value);
                }
            }

    private:

        bool _polygonizedResult;
        System::Collections::Generic::IEnumerable<BGEO::DRange1d>^ _doubleRange;
    };

//=======================================================================================
/// <summary>
/// Holds elevation analyzing browsing criteria.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>02/2008</date>
//=======================================================================================
public ref class ElevationAnalyzingBrowsingCriteria : public AnalyzingBrowsingCriteria
    {
    };

//=======================================================================================
/// <summary>
/// Holds slope analyzing browsing criteria.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>02/2008</date>
//=======================================================================================
public ref class SlopeAnalyzingBrowsingCriteria : public AnalyzingBrowsingCriteria
    {
    };

//=======================================================================================
/// <summary>
/// Holds aspect analyzing browsing criteria
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>02/2008</date>
//=======================================================================================
public ref class AspectAnalyzingBrowsingCriteria : public AnalyzingBrowsingCriteria
    {
    };

//=======================================================================================
/// <summary>
/// Holds contours browsing criteria.
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>02/2008</date>
//=======================================================================================
public ref class ContoursBrowsingCriteria : public BrowsingCriteria
    {
    public:

        //=======================================================================================
        /// <summary>
        /// Gets/sets ZLow value.
        /// </summary>                
        //=======================================================================================
        property double ZLow
            {
            double get()
                {
                return _zLow;
                }
            void set(double value)
                {
                _zLow = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/sets ZHigh value.
        /// </summary>                
        //=======================================================================================
        property double ZHigh
            {
            double get()
                {
                return _zHigh;
                }
            void set(double value)
                {
                _zHigh = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/sets UseOnlyContourValues.
        /// TODO SPU: Ask what is this ?
        /// </summary>                
        //=======================================================================================
        property bool UseOnlyContourValues
            {
            bool get()
                {
                return _useOnlyContourValues;
                }
            void set(bool value)
                {
                _useOnlyContourValues = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/sets the base elevation.
        /// </summary>                
        //=======================================================================================
        property double BaseElevation
            {
            double get()
                {
                return _baseElevation;
                }
            void set(double value)
                {
                _baseElevation = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/sets the contour interval.
        /// </summary>                
        //=======================================================================================
        property double Interval
            {
            double get()
                {
                return _interval;
                }
            void set(double value)
                {
                _interval = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/sets the smoothing option.
        /// </summary>                
        //=======================================================================================
        property DTMContourSmoothingMethod SmoothingOption
            {
            DTMContourSmoothingMethod get()
                {
                return _smoothingOption;
                }
            void set(DTMContourSmoothingMethod value)
                {
                _smoothingOption = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/sets the linear smoothing factor.
        /// </summary>                
        //=======================================================================================
        property double LinearSmoothingFactor
            {
            double get()
                {
                return _linearSmoothingFactor;
                }
            void set(double value)
                {
                _linearSmoothingFactor = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/sets the spline smoothing factor.
        /// </summary>                
        //=======================================================================================
        property double SplineSmoothingFactor
            {
            double get()
                {
                return _splineSmoothingFactor;
                }
            void set(double value)
                {
                _splineSmoothingFactor = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/sets the spline densificaton.
        /// </summary>                
        //=======================================================================================
        property int SplineDensification
            {
            int get()
                {
                return _splineDensification;
                }
            void set(int value)
                {
                _splineDensification = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/ets the contour values.
        /// </summary>                
        //=======================================================================================
        property array<double>^ ContourValues
            {
            array<double>^ get()
                {
                return _contourValues;
                }
            void set(array<double>^ value)
                {
                _contourValues = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/sets Depression Contour Display
        /// </summary>                
        //=======================================================================================
        property bool ShowDepressionContours
            {
            bool get()
                {
                return _highLowOption;
                }
            void set(bool value)
                {
                _highLowOption = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/sets the Depression Contour Interval
        /// </summary>                
        //=======================================================================================
        property double DepressionContourInterval
            {
            double get()
                {
                return _highLowContourInterval;
                }
            void set(double value)
                {
                 _highLowContourInterval = value;
                }
            }

       //=======================================================================================
        /// <summary>
        /// Gets/sets Limit Contour Slope
        /// </summary>                
        //=======================================================================================
        property bool LimitContoursToSlope
            {
            bool get()
                {
                return _maxSlopeOption;
                }
            void set(bool value)
                {
                _maxSlopeOption = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets/sets the Max Slope Value For Contours
        /// </summary>                
        //=======================================================================================
        property double MaxSlopeValue
            {
            double get()
                {
                return _maxSlopeValue ;
                }
            void set(double value)
                {
                 _maxSlopeValue = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the ContoursBrowsingCriteria class.
        /// </summary>                
        //=======================================================================================
        ContoursBrowsingCriteria (double interval)
            {
            _smoothingOption = DTMContourSmoothingMethod::None;
            _zLow = 1e10;
            _zHigh = -1e10;
            _baseElevation = 0;
            _interval = interval;
            _linearSmoothingFactor = 0.3;
            _splineSmoothingFactor = 2.5;
            _splineDensification = 5;
            _contourValues = nullptr;
            _useOnlyContourValues = false;
            _maxSlopeOption = false ;
            _maxSlopeValue  = 0.0 ;
            _highLowOption  = false ;
            _highLowContourInterval = 1.0 ;
            }

    private:

        bool _useOnlyContourValues;
        double _zLow;
        double _zHigh;
        double _baseElevation;
        double _interval;
        double _linearSmoothingFactor;
        double _splineSmoothingFactor;
        bool   _maxSlopeOption ;
        double _maxSlopeValue ;
        bool   _highLowOption;
        double _highLowContourInterval;
        int _splineDensification;
        array<double>^ _contourValues;
        DTMContourSmoothingMethod _smoothingOption;
     };


public value struct TriangulationReport
    {
    bool Success;
    };

//=======================================================================================
/// <summary>
/// Radial View Sheds Browsing Criteria
/// </summary>                
/// <author>Rob.Cormack</author>                              <date>03/2010</date>
//=======================================================================================
public ref class RadialViewShedsBrowsingCriteria : public BrowsingCriteria
    {
    public:

        //=======================================================================================
        /// <summary>
        /// Gets or Sets the Eye Location
        /// </summary>                
        //=======================================================================================
        property BGEO::DPoint3d EyeLocation
            {
            BGEO::DPoint3d get()
                {
                return eyeLocation ;
                }
            void set(BGEO::DPoint3d value)
                {
                eyeLocation = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets or Sets the Radial View Shed Option
        /// </summary>                
        //=======================================================================================
        property RadialViewShedOption RadialViewShedMethod
            {
            RadialViewShedOption get()
                {
                return radialViewShedMethod;
                }
            void set(RadialViewShedOption value)
                {
                radialViewShedMethod = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets or Sets the Number of Radials
        /// </summary>                
        //=======================================================================================
        property int NumberRadials 
            {
            int get()
                {
                return numberRadials;
                }
            void set(int value)
                {
                numberRadials = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets or Sets the Radial Increment In Degrees
        /// </summary>                
        //=======================================================================================
        property double RadialIncrement 
            {
            double get()
                {
                return radialIncrement;
                }
            void set(double value)
                {
                radialIncrement = value;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the ContoursBrowsingCriteria class.
        /// </summary>                
        //=======================================================================================
        RadialViewShedsBrowsingCriteria (BGEO::DPoint3d eye )
           {
            eyeLocation = eye ;
            radialViewShedMethod =  RadialViewShedOption::UseNumberOfRadials ;
            radialIncrement = 1.0 ;
            numberRadials = 360 ;
           }

    private:

        BGEO::DPoint3d       eyeLocation ;            //  Location Of Eye             
        RadialViewShedOption radialViewShedMethod ;   //  Flag To Indicate Whether To Use Radial Increments Or Number Of Radials
        double               radialIncrement ;        //  Radial Increment In Radians
        int                  numberRadials ;          //  Numberof Radials To Use 
    };

//=======================================================================================
/// <summary>
/// Holds draped point information.
/// </summary>                
/// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
//=======================================================================================
public ref struct DTMDrapedPoint
    {
    public :

        //=======================================================================================
        /// <summary>
        /// Gets the code of a draped point.
        /// </summary>                
        //=======================================================================================
        property DTMDrapedPointCode Code
            {
            DTMDrapedPointCode get()
                {
                return static_cast<DTMDrapedPointCode>(_drapedType);
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets the slope of a draped point.
        /// </summary>                
        //=======================================================================================
        property double Slope
            {
            double get()
                {
                DTMDrapedPointCode code = this->Code;
                if (code == DTMDrapedPointCode::Triangle )
                    {
                    return _slope;
                    }
                else
                    return 0.0;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets the aspect of a draped point.
        /// </summary>                
        //=======================================================================================
        property double   Aspect
            {
            double get()
                {
                DTMDrapedPointCode code = this->Code;
                if (code == DTMDrapedPointCode::Triangle )
                    {
                    return _aspect;
                    }
                else
                    return 0.0;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets the coordinates of a draped point.
        /// </summary>                
        //=======================================================================================
        property BGEO::DPoint3d Coordinates
            {
            BGEO::DPoint3d get()
                {
                DTMDrapedPointCode code = this->Code;
                BGEO::DPoint3d dPoint = BGEO::DPoint3d (_point.X, _point.Y, _elevation);
                if (!(code == DTMDrapedPointCode::Triangle || code == DTMDrapedPointCode::PointOrSide))
                    {
                    dPoint.Z = 0.0;
                    }
                return _point;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets the triangles where is a draped point.
        /// </summary>                
        //=======================================================================================
        property array<BGEO::DPoint3d>^ Triangle
            {
            array<BGEO::DPoint3d>^ get()
                {
                DTMDrapedPointCode code = this->Code;
                if (code == DTMDrapedPointCode::Triangle )
                    return _triangle;
                else
                    return nullptr;
                }
            }

    private:

        double _elevation;
        double _slope;
        double _aspect;
        array<BGEO::DPoint3d>^ _triangle;
        int _drapedType;
        BGEO::DPoint3d _point;

    internal:

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the DTMDrapedPoint class.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        DTMDrapedPoint 
            (
            double elevation, 
            double slope, 
            double aspect, 
            array<BGEO::DPoint3d>^ triangle, 
            int drapedType, 
            BGEO::DPoint3d point
            )
            {
            _elevation = elevation;
            _slope = slope;
            _aspect = aspect;
            _triangle = triangle;
            _drapedType = drapedType;
            _point = point;
            }
    };

class _DTM;

ref class DTM;

////=======================================================================================
///// <summary>
///// Defines options for Rollback
///// </summary>    
///// <author>Daryl.Holmwood</author>                              <date>08/2011</date>
////=======================================================================================
//[Flags]
//public enum class RollBackOption
//    {
//    //=======================================================================================
//    /// <summary>
//    /// No Rollback is implemented
//    /// </summary>                
//    //=======================================================================================
//    None = 0,
//
//    //=======================================================================================
//    /// <summary>
//    /// Rollback Features
//    /// </summary>                
//    //=======================================================================================
//    Features= 1,
//
//    //=======================================================================================
//    /// <summary>
//    /// Rollback Features
//    /// </summary>                
//    //=======================================================================================
//    Normal = 1,
//
//    //=======================================================================================
//    /// <summary>
//    /// Rollback Triangles as GraphicBreaks.
//    /// </summary>                
//    //=======================================================================================
//    RollbackTrianglesAsGraphicBreaks = 2,
//    };
//

//=======================================================================================
/// <summary>
/// Class that represents a DTM.
/// </summary>    
/// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
//=======================================================================================
public ref class DTM
    {
    private:
        /// <summary>
        /// Unmanaged DTM definition
        /// </summary>                    
        BcDTMP m_nativeDtm;

        /// <summary>
        /// Unmanaged memory which has been allocated.
        /// </summary>                    
        size_t m_memoryUsed;

        /// <summary>
        /// Track whether Dispose has been called.
        /// </summary>                    
        bool m_disposed;

        /// <summary>
        /// AutoUpdate the Memmory Pressure.
        /// </summary>
        bool m_autoUpdateMemoryPressure;

#ifdef DEBUG
        System::String^ m_stackTrace;
#endif
        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the DTM class.
        /// </summary>   
        /// <category>DTM Creation</category>
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
    public protected:
        DTM (BcDTMP dtm);

    internal:
        virtual property BcDTMP Handle
            {
            BcDTMP get ();
            }

    private: 

        //=======================================================================================
        /// <summary>
        /// Disposes the DTM instances.
        /// </summary>     
        /// <category>DTM Creation</category>
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        virtual ~DTM();

        //=======================================================================================
        /// <summary>
        /// Finalizes the DTM instances (when it is garbage collected)
        /// </summary>      
        /// <category>DTM Creation</category>
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        !DTM();

        void InternalDispose();
        //=======================================================================================
        /// <summary>
        /// Browses Dynamic feature
        /// </summary>
        /// <category>DTM Dynamic Browse</category>
        /// <param name="criteria">Browsing criteria.</param>
        /// <param name="featureType">Feature type.</param>
        /// <param name="hdlP">Delegate called for each dynamic feature.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Claude.Bernard.</author>                              <date>10/2007</date>
        //=======================================================================================
        void BrowseDynamicFeatures (BrowsingCriteria^ criteria, DTMDynamicFeatureType featureType,  DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg);

    internal:

        //=======================================================================================
        /// <summary>
        /// Checks the memory pressure onto the DTM.
        /// </summary>     
        /// <author>Daryl.Holmwood</author>                              <date>08/2005</date>
        //=======================================================================================
        void CheckMemoryPressure();

    public: 


        //=======================================================================================
        /// <summary>
        /// Checks whether the DTM is triangulated.
        /// </summary>     
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        [EditorBrowsable(EditorBrowsableState::Never) ]
        void CheckIsTriangulated();

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the DTM class.
        /// </summary>   
        /// <category>DTM Creation</category>
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        DTM ();

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the DTM class.
        /// </summary>    
        /// <category>DTM Creation</category>
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        DTM (int iniPoint, int incPoint);

        //=======================================================================================
        /// <summary>
        /// Gets the Handle to the unmanaged DTM as a integer.
        /// </summary>  
        /// <category>DTM Creation</category>
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        property IntPtr ExternalHandle
            {
            IntPtr get()
                {
                return IntPtr(Handle);
                }
            }


        //=======================================================================================
        /// <summary>
        /// Triangulates the DTM.
        /// </summary>   
        /// <category>DTM Creation</category>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        TriangulationReport Triangulate ();

        //=======================================================================================
        /// <summary>
        /// Checks the triangulation is valid.
        /// </summary>   
        /// <category>DTM Creation</category>
        /// <author>Daryl.Holmwood</author>                              <date>02/2008</date>
        //=======================================================================================
        bool CheckTriangulation();

        /// <summary>
        /// Browse for duplicate points
        /// </summary>
        /// <param name="hdlP"></param>
        /// <param name="oArg"></param>
        void BrowseDuplicatePoints (DuplicatePointsBrowsingDelegate^ hdlP, System::Object^ oArg);

        //=======================================================================================
        /// <summary>
        /// Browses for crossing features.
        /// </summary>
        /// <param name="criteria">Crossing features criteria.</param>
        /// <param name="hdlP">Browsing delegate.</param>
        /// <param name="oArg">User defined argument forwarded to the delegate.</param>
        //=======================================================================================
        void BrowseCrossingFeatures (CrossingFeaturesBrowsingCriteria^ criteria, CrossingFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg);

        //=======================================================================================
        /// <summary>
        /// Sets the triangulation parameters.
        /// </summary>   
        /// <param name="pointTol">Point tolerance.</param>
        /// <param name="lineTol">Line tolerance.</param>
        /// <param name="edgeOption">Edge option.</param>
        /// <param name="maxSide">Maximum edge side, used only for RemoveMaxSide edge option.</param>
        /// <category>DTM Creation</category>
        /// <author>Daryl.Holmwood</author>                              <date>02/2008</date>
        //=======================================================================================
        bool SetTriangulationParameters (double pointTol, double lineTol, DTMEdgeOption edgeOption, double maxSide);

        //=======================================================================================
        /// <summary>
        /// Gets the triangulation parameters.
        /// </summary>   
        /// <param name="pointTol">Point tolerance.</param>
        /// <param name="lineTol">Line tolerance.</param>
        /// <param name="edgeOption">Edge option.</param>
        /// <param name="maxSide">Maximum edge side, used only for RemoveMaxSide edge option.</param>
        /// <category>DTM Creation</category>
        /// <author>James.Goode</author>                              <date>02/2008</date>
        //=======================================================================================
        void GetTriangulationParameters([Out] double% pointTol, [Out] double% lineTol, [Out] DTMEdgeOption%  edgeOption, [Out] double% maxSide);

        //=======================================================================================
        /// <summary>
        /// Gets the 3D range of the DTM.
        /// </summary>    
        /// <category>DTM Element</category>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        property BGEO::DRange3d Range3d
            {
            BGEO::DRange3d get();
            }

        //=======================================================================================
        /// <summary>
        /// Calculates the statistics of the DTM.
        /// </summary>     
        /// <category>DTM Element</category>
        /// <author>Daryl.Holmwood</author>                              <date>04/2011</date>
        //=======================================================================================
        DTMFeatureStatisticsInfo^ CalculateFeatureStatistics();

        //=======================================================================================
        /// <summary>
        /// Gets the number of points in the DTM.
        /// </summary>                
        /// <category>DTM Element</category>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        property System::Int64 VerticesCount
            {
            System::Int64 get ();
            }

        //=======================================================================================
        /// <summary>
        /// Gets the number of triangles in the DTM.
        /// </summary>                
        /// <category>DTM Element</category>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        property int TrianglesCount
            {
            int get ();
            }

        //=======================================================================================
        /// <summary>
        /// Gets the hull.
        /// </summary>                
        /// <category>DTM Element</category>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        array<BGEO::DPoint3d>^ GetBoundary();

        //=======================================================================================
        /// <summary>
        /// Save the dtm instance in a dtm binary file.
        /// </summary>
        /// <category>DTM Creation</category>
        /// <param name="fileName">Name of the file to save the DTM to.</param>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        void Save (String^ fileName);

        //=======================================================================================
        /// <summary>
        /// Save the dtm instance in a Stream.
        /// </summary>
        /// <category>DTM Creation</category>
        /// <param name="stream">The Stream to save the DTM to.</param>
        /// <author>Daryl.Holmwood</author>                              <date>03/2010</date>
        //=======================================================================================
        void SaveToStream (System::IO::Stream^ stream);

        //=======================================================================================
        /// <summary>
        /// Populates The DTM from a GEOPAK Tin file.
        /// </summary>
        /// <category>DTM Creation</category>
        /// <param name="fileName">Name of the tin file to populate the DTM from.</param>
        /// <author>Rob.Cormack</author>                              <date>07/2008</date>
        //=======================================================================================
        void PopulateFromGeopakTinFile (String^ fileName);

        //=======================================================================================
        /// <summary>
        /// Save the DTM instance to a GEOPAK Tin file.
        /// </summary>
        /// <category>DTM Export</category>
        /// <param name="fileName">Name of the file to save the DTM to.</param>
        /// <author>Rob.Cormack</author>                              <date>07/2008</date>
        //=======================================================================================
        void SaveAsGeopakTinFile (String^ fileName);

        //=======================================================================================
        /// <summary>
        /// Drapes a point onto the DTM.
        /// </summary>   
        /// <category>DTM Drape</category>
        /// <param name="point">Point to drape.</param>
        /// <returns>A DTMDrapedPoint instance that holds draped point information.</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        DTMDrapedPoint^ DrapePoint (BGEO::DPoint3d point);

        //=======================================================================================
        /// <summary>
        /// Drapes a point onto the DTM.
        /// </summary>   
        /// <category>DTM Drape</category>
        /// <param name="points">Points to drape.</param>
        /// <returns>A DTMDrapedLinearElement instance that holds draped information.</returns>
        /// <author>Daryl.Holmwood</author>                              <date>07/2011</date>
        //=======================================================================================
        DTMDrapedLinearElement^ DrapeLinearPoints (array<BGEO::DPoint3d>^ points);

        //=======================================================================================
        /// <summary>
        /// Drapes a point onto the DTM.
        /// </summary>   
        /// <category>DTM Drape</category>
        /// <param name="points">Points to drape.</param>
        /// <returns>A DTMDrapedLinearElement instance that holds draped information.</returns>
        /// <author>Daryl.Holmwood</author>                              <date>07/2011</date>
        //=======================================================================================
        DTMDrapedLinearElement^ DrapeLinearPoints (System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ points);

        //=======================================================================================
        /// <summary>
        /// Gets a feature by ID.
        /// </summary>   
        /// <category>DTM</category>
        /// <param name="featureId">Identifier of the feature.</param>
        /// <returns>The DTMFeature that has the provided Id.</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        DTMFeature^ GetFeatureById (DTMFeatureId featureId);

        //=======================================================================================
        /// <summary>
        /// Adds a point feature with one point.
        /// </summary>  
        /// <category>DTM</category>
        /// <param name="point">Coordinates of the spot.</param>
        /// <param name="userTag">User tag to attach to the feature.</param>
        /// <returns>The Id assigned to the added feature.</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        DTMFeatureId AddPointFeature (BGEO::DPoint3d point, ::DTMUserTag userTag);

        //=======================================================================================
        /// <summary>
        /// Adds a point feature with several points.
        /// </summary>
        /// <category>DTM</category>
        /// <param name="points">Coordinates of the points.</param>
        /// <param name="userTag">User tag to attach to the feature.</param>
        /// <returns>The Id assigned to the added feature.</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        DTMFeatureId AddPointFeature (array<BGEO::DPoint3d>^ points, ::DTMUserTag userTag);

        //=======================================================================================
        /// <summary>
        /// Adds a point feature with several points.
        /// </summary>
        /// <category>DTM</category>
        /// <param name="points">Coordinates of the points.</param>
        /// <param name="userTag">User tag to attach to the feature.</param>
        /// <returns>The Id assigned to the added feature.</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        DTMFeatureId AddPointFeature (System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ points, ::DTMUserTag userTag);

        //=======================================================================================
        /// <summary>
        /// Adds a point feature with one point.
        /// </summary>  
        /// <category>DTM</category>
        /// <param name="point">Coordinates of the point.</param>
        /// <returns>The Id assigned to the added feature.</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        DTMFeatureId AddPointFeature(BGEO::DPoint3d point);

        //=======================================================================================
        /// <summary>
        /// Adds a point feature with several points.
        /// </summary>          
        /// <category>DTM</category> 
        /// <param name="points">Coordinates of the points.</param>
        /// <returns>The Id assigned to the added feature.</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        DTMFeatureId AddPointFeature (array<BGEO::DPoint3d>^ points);

        //=======================================================================================
        /// <summary>
        /// Adds a point feature with several points.
        /// </summary>          
        /// <category>DTM</category> 
        /// <param name="points">Coordinates of the points.</param>
        /// <returns>The Id assigned to the added feature.</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        DTMFeatureId AddPointFeature (System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ points);
        
        //=======================================================================================
        /// <summary>
        /// Adds a random spot. A random spot is not associated to a feature.
        /// <param name="point">Coordinates of the point.</param>
        /// </summary>     
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        void AddPoint (BGEO::DPoint3d point) ;

        //=======================================================================================
        /// <summary>
        /// Adds several random spots. A random spot is not associated to a feature.
        /// <param name="points">Coordinates of the points.</param>
        /// </summary>     
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        void AddPoints (array<BGEO::DPoint3d>^ points) ;

        //=======================================================================================
        /// <summary>
        /// Adds several random spots. A random spot is not associated to a feature.
        /// <param name="points">Coordinates of the points.</param>
        /// </summary>     
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        void AddPoints (System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ points) ;

        //=======================================================================================
        /// <summary>
        /// Adds a linear features.
        /// </summary>                
        /// <category>DTM</category>
        /// <param name="points">Coordinates of the points.</param>
        /// <param name="featureType">Feature type.</param>
        /// <param name="userTag">User tag attached to the feature.</param>
        /// <returns>The Id assigned to the added feature.</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        DTMFeatureId AddLinearFeature (array<BGEO::DPoint3d>^ points, DTMFeatureType featureType, ::DTMUserTag userTag);

        //=======================================================================================
        /// <summary>
        /// Adds a linear features.
        /// </summary>                
        /// <category>DTM</category>
        /// <param name="points">Coordinates of the points.</param>
        /// <param name="featureType">Feature type.</param>
        /// <param name="userTag">User tag attached to the feature.</param>
        /// <returns>The Id assigned to the added feature.</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        DTMFeatureId AddLinearFeature (System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ points, DTMFeatureType featureType, ::DTMUserTag userTag);
        
        //=======================================================================================
        /// <summary>
        /// Adds a linear features.
        /// </summary>
        /// <category>DTM</category>
        /// <param name="points">Coordinates of the points.</param>
        /// <param name="featureType">Feature type.</param>
        /// <returns>The Id assigned to the added feature.</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        DTMFeatureId AddLinearFeature (array<BGEO::DPoint3d>^ points, DTMFeatureType featureType);

        //=======================================================================================
        /// <summary>
        /// Adds a linear features.
        /// </summary>
        /// <category>DTM</category>
        /// <param name="points">Coordinates of the points.</param>
        /// <param name="featureType">Feature type.</param>
        /// <returns>The Id assigned to the added feature.</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        DTMFeatureId AddLinearFeature (System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ points, DTMFeatureType featureType);

        //=======================================================================================
        /// <summary>
        /// Deletes Features With A Given ID. 
        /// </summary>
        /// <category>DTM</category>
        /// <param name="id">ID of the feature to delete.</param>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        void DeleteFeatureById (DTMFeatureId id);
      
        //=======================================================================================
        /// <summary>
        /// Bulk Deletes Features Using An Array Of Feature Ids to be deleted 
        /// </summary>
        /// <category>DTM</category>
        /// <param name="featureIds">Array Of Feature IDs For The Features To Be Deleted.</param>
        /// <author>Rob.Cormack</author>                              <date>08/2011</date>
        //=======================================================================================
        void BulkDeleteFeaturesByFeatureId (array<DTMFeatureId>^ featureIds );
        
       //=======================================================================================
        /// <summary>
        /// Deletes Features With A Given User Tag 
        /// </summary>
        /// <category>DTM</category>
        /// <param name="userTag">UserTag of the features to delete.</param>
        /// <author>Rob.Cormac</author>                                 <date>08/2011</date>
        //=======================================================================================
        void DeleteFeaturesByUserTag (::DTMUserTag userTag);

        //=======================================================================================
        /// <summary>
        /// Deletes Features With A Given User Tag 
        /// </summary>
        /// <category>DTM</category>
        /// <param name="userTag">An Array Of UserTags For The Features To Be Deleted.</param>
        /// <author>Rob.Cormac</author>                                 <date>08/2011</date>
        //=======================================================================================
        void BulkDeleteFeaturesByUserTag(array<::DTMUserTag>^ userTag) ;

        //=======================================================================================
        /// <summary>
        /// Deletes all the features of a given type. 
        /// </summary>
        /// <category>DTM</category>
        /// <param name="featureType">Type of the features to delete.</param>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        void DeleteFeaturesByType (DTMFeatureType featureType);

        //=======================================================================================
        /// <summary>
        /// Joins features of the given type to create closed features.
        /// </summary>
        /// <category>DTM</category>
        /// <param name="featureType">Feature type.</param>
        /// <param name="tolerance">Tolerance.</param>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        void JoinFeatures (DTMFeatureType featureType, double tolerance);

        //=======================================================================================
        /// <summary>
        /// Removes the hull //todo spu: see with Rob what does this mean? 
        /// </summary>
        /// <category>DTM</category>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        void RemoveHull ();


        //=======================================================================================
        /// <summary>
        /// Removes None Feature Lines On The Tin Hull
        /// Assumes There Is A Set Of Contiguous Features Representing The Tin Hull 
        /// </summary>
        /// <category>DTM</category>
        /// <author>Rob.Cormack</author>                              <date>03/2010</date>
        //=======================================================================================
        void RemoveNoneFeatureHullLines () ;

       //=======================================================================================
        /// <summary>
        /// Calculates the DTM slope area
        /// </summary>
        /// <category>DTM</category>
        /// <returns>The slope area of the whole DTM</returns>  
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        SlopeAreaResult^ CalculateSlopeArea ();

        //=======================================================================================
        /// <summary>
        /// Calculate the DTM slope area corresponding to the area inside a polygon
        /// </summary>                
        /// <category>DTM</category>
        /// <param name="polygon">Polygon which restricts slope area</param>
        /// <returns>The slope area inside the polygon</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        SlopeAreaResult^ CalculateSlopeArea (array<BGEO::DPoint3d>^ polygon);

        //=======================================================================================
        /// <summary>
        /// Calculate the DTM slope area corresponding to the area inside a polygon
        /// </summary>                
        /// <category>DTM</category>
        /// <param name="polygon">Polygon which restricts slope area</param>
        /// <returns>The slope area inside the polygon</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        SlopeAreaResult^ CalculateSlopeArea (System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ polygon);

        //=======================================================================================
        /// <summary>
        /// Merges this dtm with another dtm.
        /// </summary>
        /// <category>DTM Creation</category>
        /// <param name="dtm">Other dtm to merge</param>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        void Merge (DTM ^dtm);

        //=======================================================================================
        /// <summary>
        /// Merges two adjacent dtm instances //todo spu: see with Rob what does that mean?
        /// </summary>
        /// <category>DTM Creation</category>
        /// <param name="dtm">Other dtm to merge</param>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        void MergeAdjacent (DTM ^dtm);

        //=======================================================================================
        /// <summary>
        /// Clones And Clips a dtm by a given polygon.
        /// </summary>
        /// <category>DTM Creation</category>
        /// <param name="Polygon">Polygon to use for the clipping.</param>
        /// <param name="clipMethod">Clipping method.</param>
        /// <returns>Clipped dtm instance</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        DTM^ CloneAndClip(array<BGEO::DPoint3d>^ Polygon, DTMClippingMethod clipMethod);

        //=======================================================================================
        /// <summary>
        /// Clones And Clips a dtm by a given polygon.
        /// </summary>
        /// <category>DTM Creation</category>
        /// <param name="Polygon">Polygon to use for the clipping.</param>
        /// <param name="clipMethod">Clipping method.</param>
        /// <returns>Clipped dtm instance</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        DTM^ CloneAndClip(System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ Polygon, DTMClippingMethod clipMethod);

        //=======================================================================================
        /// <summary>
        /// Clips a dtm by a given polygon.
        /// </summary>
        /// <category>DTM Creation</category>
        /// <param name="Polygon">Polygon to use for the clipping.</param>
        /// <param name="clipMethod">Clipping method.</param>
        /// <returns>Clipped dtm instance</returns>
        /// <author>Rob.Cormack</author>                              <date>05/2009</date>
        //=======================================================================================
        void Clip (array<BGEO::DPoint3d>^ Polygon, DTMClippingMethod clipMethod);

        //=======================================================================================
        /// <summary>
        /// Clips a dtm by a given polygon.
        /// </summary>
        /// <category>DTM Creation</category>
        /// <param name="Polygon">Polygon to use for the clipping.</param>
        /// <param name="clipMethod">Clipping method.</param>
        /// <returns>Clipped dtm instance</returns>
        /// <author>Rob.Cormack</author>                              <date>05/2009</date>
        //=======================================================================================
        void Clip (System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ Polygon, DTMClippingMethod clipMethod);

       //=======================================================================================
        /// <summary>
        /// Offsets the dtm by a delta elevation.
        /// </summary>
        /// <category>DTM Creation</category>
        /// <param name="elevation">Elevation to offset the dtm with.</param>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        void OffsetDeltaElevation (double elevation);

        //=======================================================================================
        /// <summary>
        /// Calculates A Pond About A Point
        /// Firstly Traces From The Point To A Sump
        /// And Then Calculates The Pond About The Sump  
        /// </summary>
        /// <category>DTM Drainage</category>
        /// <param name="X">Point X Coordinate</param>
        /// <param name="Y">Point Y Coordinate</param>
        /// <author>Rob.Cormack</author>                              <date>03/2010</date>
        //=======================================================================================
        PondCalculation^ CalculatePond ( double X , double Y) ;


        //=======================================================================================
        /// <summary>
        /// Calculates A Pond About A Point
        /// Firstly Traces From The Point To A Sump
        /// And Then Calculates The Pond About The Sump  
        /// </summary>
        /// <category>DTM Drainage</category>
        /// <param name="X">Point X Coordinate</param>
        /// <param name="Y">Point Y Coordinate</param>
        /// <param name="falseLowDepth">Trace Out Of Ponds Less Than This Depth</param>
        /// <author>Rob.Cormack</author>                              <date>12/2011</date>
        //=======================================================================================
        PondCalculation^ CalculatePond ( double X , double Y , double falseLowDepth) ;

        //=======================================================================================
        /// <summary>
        /// Traces The Catchment For A Point
        /// Firstly Traces From The Point To A Sump
        /// And Then Calculates And Returns The Maximum Ascents From The Sump  
        /// </summary>
        /// <category>DTM Drainage</category>
        /// <param name="X">Point X Coordinate</param>
        /// <param name="Y">Point Y Coordinate</param>
        /// <param name="MaxPondDepth">Maximum Pond Depth For Downstream Tracing</param>
        /// <author>Rob.Cormack</author>                              <date>03/2010</date>
        //=======================================================================================
        PointCatchmentResult^ TraceCatchmentForPoint (double X, double Y, double MaxPondDepth);
        
        //=======================================================================================
        /// <summary>
        /// Appends A DTM To A DTM Instance
        /// </summary>
        /// <category>DTM Creation</category>
        /// <returns>Copy of this instance</returns>
        /// <author>Rob.Cormack</author>                              <date>05/2008</date>
        //=======================================================================================
        void Append (DTM^ dtmToAppend);

 
        //=======================================================================================
        /// <summary>
        /// Clones this dtm instance.
        /// </summary>
        /// <category>DTM Creation</category>
        /// <returns>Copy of this instance</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        DTM^ Clone ();

        //=======================================================================================
        /// <summary>
        /// Creates a not deep copy of this instance. The two DTM share the same unmanaged dtm.
        /// </summary>
        /// <returns>Copy of this instance</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        virtual DTM^ CloneNotDeep ();

        //=======================================================================================
        /// <summary>
        /// Creates a delta dtm from two dtm instances
        /// </summary>
        /// <category>DTM Creation</category>
        /// <param name="dtm">Other dtm to delta</param>
        /// <returns>The delta dtm.</returns>
        /// <author>Claude.Bernard.</author>                              <date>10/2007</date>
        //=======================================================================================
        DTM^ Delta (DTM ^dtm);

        //=======================================================================================
        /// <summary>
        /// Creates a delta dtm from two dtm instances
        /// </summary>
        /// <category>DTM Creation</category>
        /// <param name="polygon">Polygon to create delta from</param>
        /// <param name="dtm">Other dtm to delta</param>
        /// <returns>The delta dtm.</returns>
        /// <author>Claude.Bernard.</author>                              <date>10/2007</date>
        //=======================================================================================
        DTM^ Delta (array<BGEO::DPoint3d>^ polygon, DTM ^dtm);
        
        //=======================================================================================
        /// <summary>
        /// Creates a delta dtm from two dtm instances
        /// </summary>
        /// <category>DTM Creation</category>
        /// <param name="polygon">Polygon to create delta from</param>
        /// <param name="dtm">Other dtm to delta</param>
        /// <returns>The delta dtm.</returns>
        /// <author>Claude.Bernard.</author>                              <date>10/2007</date>
        //=======================================================================================
        DTM^ Delta (System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ polygon, DTM ^dtm);

        //=======================================================================================
        /// <summary>
        /// Creates a delta ( difference ) dtm to an elevation
        /// </summary>
        /// <category>DTM Creation</category>
        /// <param name="elevation">Elevation to delta</param>
        /// <returns>The delta dtm.</returns>
        /// <author>Rob.Cormack.</author>                              <date>10/2011</date>
        //=======================================================================================
        DTM^ DeltaElevation (double elevation );

        //=======================================================================================
        /// <summary>
        /// Creates a delta ( difference ) dtm to an elevation for a defined area
        /// </summary>
        /// <category>DTM Creation</category>
        /// <param name="polygon">Polygon defining the area for the delta calculation</param>
        /// <param name="elevation">Elevation to delta</param>
        /// <returns>The delta dtm.</returns>
        /// <author>Rob.Cormack.</author>                              <date>10/2011</date>
        //=======================================================================================
        DTM^ DeltaElevation (array<BGEO::DPoint3d>^ polygon, double elevation);

        //=======================================================================================
        /// <summary>
        /// Browses contours
        /// </summary>
        /// <category>DTM Browse</category>
        /// <param name="criteria">The Criteria fpr browsin contours.</param>
        /// <param name="hdl">Browsing delegate.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Claude.Bernard.</author>                              <date>10/2007</date>
        //=======================================================================================
        void BrowseContours (ContoursBrowsingCriteria^ criteria, ContoursBrowsingDelegate^ hdl, System::Object^ oArg);

        //=======================================================================================
        /// <summary>
        /// Draw The Contour Through A Point
        /// </summary>
        /// <category>DTM Browse</category>
        /// <param name="criteria">The Criteria for Drawing A Contour Though A Point.</param>
        /// <param name="hdl">Browsing delegate.</param>
        /// <param name="X">X Coordinate Of Point.</param>
        /// <param name="Y">YCoordinate Of Point.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Rob.Cormack.</author>                              <date>07/2010</date>
        //=======================================================================================
        void ContourAtPoint (ContoursBrowsingCriteria^ criteria, ContoursBrowsingDelegate^ hdl,double X , double Y, System::Object^ oArg);

        //=======================================================================================
        /// <summary>
        /// Browses Triangle Mesh
        /// </summary>
        /// <category>DTM Dynamic Browse</category>
        /// <param name="criteria"></param>
        /// <param name="hdl">Delegate called for each triangle mesh.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Rob.Cormack</author>                              <date>06/2009</date>
        //=======================================================================================
        void BrowseTriangleMesh (TriangleMeshBrowsingCriteria^ criteria, TriangleMeshBrowsingDelegate^ hdl, System::Object^ oArg);

        //=======================================================================================
        /// <summary>
        /// Browses Dynamic feature
        /// </summary>
        /// <category>DTM Dynamic Browse</category>
        /// <param name="featureType"></param>
        /// <param name="criteria"></param>
        /// <param name="hdlP">Delegate called for each dynamic feature.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Claude.Bernard.</author>                              <date>10/2007</date>
        //=======================================================================================
        void BrowseDynamicFeatures (DynamicFeaturesBrowsingCriteria^ criteria, DTMDynamicFeatureType featureType,  DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg);

       //=======================================================================================
        /// <summary>
        /// Browse Point Features
        /// </summary>
        /// <category>DTM Browse</category>
        /// <param name="criteria"></param>
        /// <param name="hdlP">Delegate called for each dynamic feature.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Rob.Cormack</author>                              <date>5/2009</date>
        //=======================================================================================
        void BrowsePointFeatures (PointFeaturesBrowsingCriteria^ criteria, PointFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg);


       //=======================================================================================
        /// <summary>
        /// Browse Points
        /// </summary>
        /// <category>DTM Feature Browse</category>
        /// <param name="criteria"></param>
        /// <param name="hdlP">Delegate called for each dynamic feature.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Rob.Cormack</author>                              <date>5/2009</date>
        //=======================================================================================
        void BrowsePoints (PointsBrowsingCriteria^ criteria, PointsBrowsingDelegate^ hdlP, System::Object^ oArg);

        //=======================================================================================
        /// <summary>
        /// Browse features
        /// </summary>
        /// <category>DTM Feature Browse</category>
        /// <param name="featureType"></param>
        /// <param name="criteria"></param>
        /// <param name="hdlP">Delegate called for each dynamic feature.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Claude.Bernard.</author>                              <date>10/2007</date>
        //=======================================================================================
        void BrowseLinearFeatures (LinearFeaturesBrowsingCriteria^ criteria, DTMFeatureType featureType,  LinearFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg);

        //=======================================================================================
        /// <summary>
        /// Browse Features With Tin Errors
        /// </summary>
        /// <category>DTM Feature Browse</category>
        /// <param name="hdlP">Delegate called for each feature with a tin Error.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Rob.Cormack</author>                              <date>03/2010</date>
        //=======================================================================================
        void BrowseFeaturesWithTinErrors ( LinearFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg) ;

        //=======================================================================================
        /// <summary>
        /// Browse Features With The Same User Tag
        /// </summary>
        /// <category>DTM Feature Browse</category>
        /// <param name="userTag">Feature User Tag.</param>
        /// <param name="hdlP">Delegate called for each feature with the user Tag .</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Rob.Cormack</author>                              <date>03/2010</date>
        //=======================================================================================
        void BrowseFeaturesWithUserTag ( ::DTMUserTag userTag , LinearFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg) ;

        //=======================================================================================
        /// <summary>
        /// Browse Features With The Same Feature Id
        /// </summary>
        /// <category>DTM Feature Browse</category>
        /// <param name="featureId">Feature Id.</param>
        /// <param name="hdlP">Delegate called for each feature with the Feature Id.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Rob.Cormack</author>                              <date>03/2010</date>
        //=======================================================================================
        void BrowseFeaturesWithFeatureId ( DTMFeatureId featureId , LinearFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg) ;
 
        //=======================================================================================
        /// <summary>
        /// Browse Ponds
        /// </summary>
        /// <category>DTM Drainage</category>
        /// <param name="hdlP">Delegate called for each dynamic feature.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Rob.Cormack</author>                              <date>01/2011</date>
        //=======================================================================================
        void BrowsePonds( DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg) ;

        //=======================================================================================
        /// <summary>
        /// Browse catchments
        /// </summary>
        /// <category>DTM Drainage</category>
        /// <param name="criteria"></param>
        /// <param name="minDepth"></param>
        /// <param name="hdlP">Delegate called for each dynamic feature.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Claude.Bernard.</author>                              <date>10/2007</date>
        //=======================================================================================
        void BrowseCatchments (CatchmentsBrowsingCriteria^ criteria, double minDepth, DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg);

        //=======================================================================================
        /// <summary>
        /// Calculate catchments
        /// </summary>
        /// <category>DTM Drainage</category>
        /// <param name="criteria"></param>
        /// <param name="hdlP">Delegate called for each catchment feature.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Claude.Bernard.</author>                              <date>10/2007</date>
        //=======================================================================================
        void CalculateCatchments (CatchmentsCalculationCriteria^ criteria,DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg);

        //=======================================================================================
        /// <summary>
        /// Browse low points
        /// </summary>
        /// <category>DTM Static Browse</category>
        /// <param name="criteria"></param>
        /// <param name="minDepth"></param>
        /// <param name="hdlP">Delegate called for each dynamic feature.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Claude.Bernard.</author>                              <date>10/2007</date>
        //=======================================================================================
        void BrowseLowPoints (LowPointsBrowsingCriteria^ criteria, double minDepth, SinglePointFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg);

        //=======================================================================================
        /// <summary>
        /// Browse high points
        /// </summary>
        /// <category>DTM Static Browse</category>
        /// <param name="criteria"></param>
        /// <param name="hdlP">Delegate called for each dynamic feature.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Claude.Bernard.</author>                              <date>10/2007</date>
        //=======================================================================================
        void BrowseHighPoints (HighPointsBrowsingCriteria^ criteria,  SinglePointFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg);

        //=======================================================================================
        /// <summary>
        /// Browse ridge lines
        /// </summary>
        /// <category>DTM Static Browse</category>
        /// <param name="criteria"></param>
        /// <param name="hdlP">Delegate called for each dynamic feature.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Claude.Bernard.</author>                              <date>10/2007</date>
        //=======================================================================================
        void BrowseRidgeLines (RidgeLinesBrowsingCriteria^ criteria, DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg);

        //=======================================================================================
        /// <summary>
        /// Browse sump lines
        /// </summary>
        /// <category>DTM Static Browse</category>
        /// <param name="criteria"></param>
        /// <param name="hdlP">Delegate called for each dynamic feature.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Claude.Bernard.</author>                              <date>10/2007</date>
        //=======================================================================================
        void BrowseSumpLines (SumpLinesBrowsingCriteria^ criteria, DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg);

        //=======================================================================================
        /// <summary>
        /// Trace path from a reference point to a target point at distance and slope 
        /// </summary>
        /// <category>DTM Trace</category>
        /// <param name="startPoint"></param>
        /// <param name="slope"></param>
        /// <param name="distance"></param>
        /// <param name="criteria"></param>
        /// <param name="hdlP">Delegate called for each dynamic feature.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Claude.Bernard.</author>                              <date>10/2007</date>
        //=======================================================================================
        void TracePath (BGEO::DPoint3d startPoint, double slope, double distance, PathTracingCriteria^ criteria, DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg);

        //=======================================================================================
        /// <summary>
        /// Trace line from a reference point to a target point at distance and slope 
        /// </summary>
        /// <category>DTM Trace</category>
        /// <param name="startPoint"></param>
        /// <param name="slope"></param>
        /// <param name="distance"></param>
        /// <param name="criteria"></param>
        /// <param name="hdlP">Delegate called for each dynamic feature.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Claude.Bernard.</author>                              <date>10/2007</date>
        //=======================================================================================
        void TraceLine (BGEO::DPoint3d startPoint, double slope, double distance, LineTracingCriteria^ criteria, DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg);

        //=======================================================================================
        /// <summary>
        /// Analyze elevation
        /// </summary>
        /// <category>DTM Themes</category>
        /// <param name="criteria"></param>
        /// <param name="hdlP">Delegate called for each dynamic feature.</param>
        /// <param name="oArg">.</param>
        /// <author>Claude.Bernard.</author>                              <date>10/2007</date>
        //=======================================================================================
        void AnalyzeElevation (ElevationAnalyzingBrowsingCriteria^ criteria, DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg);

        //=======================================================================================
        /// <summary>
        /// Analyze slope
        /// </summary>
        /// <category>DTM Themes</category>
        /// <param name="criteria"></param>
        /// <param name="hdlP">Delegate called for each dynamic feature.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Claude.Bernard.</author>                              <date>10/2007</date>
        //=======================================================================================
        void AnalyzeSlope (SlopeAnalyzingBrowsingCriteria^ criteria, DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg);

        //=======================================================================================
        /// <summary>
        /// analyze aspect
        /// </summary>
        /// <category>DTM Themes</category>
        /// <param name="criteria"></param>
        /// <param name="hdlP">Delegate called for each dynamic feature.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        //=======================================================================================
        void AnalyzeAspect (AspectAnalyzingBrowsingCriteria^ criteria, DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg);

        //=======================================================================================
        /// <summary>
        /// Browse slope indicators between two Dtm
        /// </summary>
        ///  <category>DTM Static Browse</category>
        /// <param name="targetDtm"></param>
        /// <param name="criteria"></param>
        /// <param name="hdlP">Delegate called for each dynamic feature.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Claude.Bernard.</author>                              <date>10/2007</date>
        //=======================================================================================
        void BrowseSlopeIndicators (DTM^ targetDtm, SlopeIndicatorsBrowsingCriteria^ criteria, SlopeIndicatorsBrowsingDelegate^ hdlP, System::Object^ oArg);

        //=======================================================================================
        /// <summary>
        /// Trace Maximum Descent
        /// </summary>
        ///  <category>DTM Trace</category>
        /// <param name="minLowPointDepth"></param>
        /// <param name="refPoint"></param>
        /// <param name="hdlP">Delegate called for each dynamic feature.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <returns></returns>
        /// <author>Rob.Cormack</author>                              <date>07/2011</date>
        //=======================================================================================
        void TraceMaximumDescent( double minLowPointDepth,BGEO::DPoint3d refPoint,DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg) ;

        //=======================================================================================
        /// <summary>
        /// Trace Maximum Ascent
        /// </summary>
        ///  <category>DTM Trace</category>
        /// <param name="minHighPointElevation"></param>
        /// <param name="refPoint"></param>
        /// <param name="hdlP">Delegate called for each dynamic feature.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <returns></returns>
        /// <author>Rob.Cormack</author>                              <date>07/2011</date>
        //=======================================================================================
        void TraceMaximumAscent( double minHighPointElevation,BGEO::DPoint3d refPoint,DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg) ;

        //=======================================================================================
        /// <summary>
        /// Browse Triangles
        /// </summary>
        ///  <category>DTM Static Browse</category>
        /// <param name="criteria"></param>
        /// <param name="hdlP">Delegate called for each dynamic feature.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Claude.Bernard.</author>                              <date>10/2007</date>
        //=======================================================================================
        void BrowseTriangles (TrianglesBrowsingCriteria^ criteria, DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg);

        //=======================================================================================
        /// <summary>
        /// Browse Triangle edges
        /// </summary>
        ///  <category>DTM Static Browse</category>
        /// <param name="criteria"></param>
        /// <param name="hdlP">Delegate called for each dynamic feature.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Claude.Bernard.</author>                              <date>10/2007</date>
        //=======================================================================================
        void BrowseTriangleEdges (TriangleEdgesBrowsingCriteria^ criteria, DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg);

        //=======================================================================================
        /// <summary>
        /// Filters out Random Spots
        /// </summary>
        ///  <category>DTM Static Browse</category>
        /// <param name="criteria"></param>
        /// <author>Daryl.Holmwood.</author>                              <date>03/2008</date>
        //=======================================================================================
        FilterResult^ TinFilterPoints (TinFilterCriteria^ criteria);

        //=======================================================================================
        /// <summary>
        /// Filters out Random Spots
        /// </summary>
        ///  <category>DTM Static Browse</category>
        /// <param name="criteria"></param>
        /// <author>Daryl.Holmwood.</author>                              <date>03/2008</date>
        //=======================================================================================
        FilterResult^ TileFilterPoints(TileFilterCriteria^ criteria);

        //=======================================================================================
        /// <summary>
        /// Filters out Single Point Group Spots - AASHTO Specific
        /// </summary>
        ///  <category>DTM Static Browse</category>
        /// <param name="criteria"></param>
        /// <author>Rob.Cormack.</author>                              <date>03/2008</date>
        //=======================================================================================
        FilterResult^ TinFilterSinglePointPointFeatures(TinFilterCriteria^ criteria);

        //=======================================================================================
        /// <summary>
        /// Filters out Single Point Group Spots - AASHTO Specific
        /// </summary>
        ///  <category>DTM Static Browse</category>
        /// <param name="criteria"></param>
        /// <author>Rob.Cormack.</author>                              <date>03/2008</date>
        //=======================================================================================
        FilterResult^ TileFilterSinglePointPointFeatures(TileFilterCriteria^ criteria);

        //=======================================================================================
        /// <summary>
        /// Creates a dtm instance from a dtm file or Geopak tin file
        /// </summary>
        /// <category>DTM Import</category>
        /// <param name="fileName">Name of the tin file to open in Read only mode.</param>
        /// <returns>A new DTM instance.</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        static DTM^ CreateFromFile (String^ fileName);

        //=======================================================================================
        /// <summary>
        /// Creates a dtm instance from a dtm stream
        /// </summary>
        /// <category>DTM Import</category>
        /// <param name="stream">The stream to read from.</param>
        /// <returns>A new DTM instance.</returns>
        /// <author>Daryl.Holmwood</author>                              <date>03/2010</date>
        //=======================================================================================
        static DTM^ CreateFromStream (System::IO::Stream^ stream);

        //=======================================================================================
        /// <summary>
        /// Creates a dtm instance from a Geopak tin file
        /// </summary>
        /// <category>DTM Import</category>
        /// <param name="fileName">Name of the Geopak tin file to inport from.</param>
        /// <returns>A new DTM instance.</returns>
        /// <author>Rob.Cormack</author>                              <date>08/2008</date>
        //=======================================================================================
        static DTM^ CreateFromGeopakTinFile (String^ fileName);

        //=======================================================================================
        /// <summary>
        /// Create an un-triangulated dtm instance from a Geopak Dat file
        /// </summary>
        /// <category>DTM Import</category>
        /// <param name="fileName">Name of the dat file to open.</param>
        /// <returns>A new DTM instance.</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        static DTM^ CreateFromGeopakDatFile (String^ fileName);

        //=======================================================================================
        /// <summary>
        /// Create an un-triangulated dtm instance from an XYZ file
        /// </summary>
        /// <category>DTM Import</category>
        /// <param name="fileName">Name of the XYZ file to open.</param>
        /// <returns>A new DTM instance.</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        static DTM^ CreateFromXyzFile (String^ fileName);

        //=======================================================================================
        /// <summary>
        /// Copies a dtm instance to a contiguous memory block
        /// </summary>
        /// <category>DTM Import</category>
        /// <returns>A Contiguous Memory Block </returns>
        /// <author>Rob.Cormack</author>                              <date>5/2009</date>
        //=======================================================================================
        array<char>^ DTM::CopyToMemoryBlock() ;

        //=======================================================================================
        /// <summary>
        /// Create a dtm instance from a dtm stored in a native memory block
        /// </summary>
        /// <category>DTM Import</category>
        /// <param name="memoryBlock"></param>
        /// <returns>A new DTM instance.</returns>
        /// <author>Daryl.Holmwood</author>                              <date>5/2009</date>
        //=======================================================================================
        static DTM^ DTM::CreateFromNativeMemoryBlock (System::IntPtr memoryBlock, unsigned long blockSize );

        //=======================================================================================
        /// <summary>
        /// Create a dtm instance from a dtm stored in a memory block
        /// </summary>
        /// <category>DTM Import</category>
        /// <param name="memoryBlock"></param>
        /// <returns>A new DTM instance.</returns>
        /// <author>Rob.Cormack</author>                              <date>5/2009</date>
        //=======================================================================================
        static DTM^ DTM::CreateFromMemoryBlock (array<char>^ memoryBlock ) ;

        //=======================================================================================
        /// <summary>
        /// Calculates planar volume.
        /// This Method Will Be Deprecated Use  CalculatePrismoidalVolumeToElevation
        /// </summary>
        /// <category>DTM Volume</category>
        /// <param name="elevation"></param>
        /// <returns>The calculated volume.</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        double CalculatePlanarPrismoidalVolume (double elevation, VolumeCriteria^ criteria);

        //=======================================================================================
        /// <summary>
        /// Calculates cut volume from another dtm inside a polygon.
        /// This Method Will Be Deprecated Use  CalculatePrismoidalVolumeToSurface
        /// </summary>
        /// <category>DTM Volume</category>
        /// <param name="dtm">DTM to calculate cut volume from.</param>
        /// <param name="criteria">Criteria.</param>
        /// <returns>The calculated volume.</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        CutFillResult^ CalculateCutFillVolume (DTM^ dtm, VolumeCriteria^ criteria);

        //=======================================================================================
        /// <summary>
        /// Calculates The Cut Fill And Balance Volumes Between A DTM And Set EElevation and will optionally :-
        /// 1. Determine Volumes For A Specified Area Defined By A Polygon
        /// 2. Will Range The Cut and Fill Volumes For User Defined Elevation Ranges
        /// </summary>
        /// <category>DTM Volume</category>
        /// <param name="elevation">Elevation to calculate the cut fill volumes for.</param>
        /// <param name="criteria">Criteria.</param>
        /// <returns>The calculated volume.</returns>
        /// <author>Rob.Cormack</author>                              <date>03/2009</date>
        //=======================================================================================
        VolumeResult^ CalculatePrismoidalVolumeToElevation (double elevation, VolumeCriteria^ criteria);

        //=======================================================================================
        /// <summary>
        /// Calculates The Cut Fill And Balance Volumes Between Two DTMs and will optionally :-
        /// 1. Determine Volumes For A Specified Area Defined By A Polygon
        /// 2. Will Range The Cut and Fill Volumes For User Defined Elevation Ranges
        /// </summary>
        /// <category>DTM Volume</category>
        /// <param name="dtm">DTM Surface to calculate the cut fill volumes for.</param>
        /// <param name="criteria">Criteria.</param>
        /// <returns>The calculated volume.</returns>
        /// <author>Rob.Cormack</author>                              <date>03/2009</date>
        //=======================================================================================
        VolumeResult^ CalculatePrismoidalVolumeToSurface (DTM^ dtm, VolumeCriteria^ criteria);

        //=======================================================================================
        /// <summary>
        /// Calculates The Volume Balance Between Two DTMs and will optionally 
        /// Determine The Volume Balance For A Specified Area Defined By A Polygon.
        /// The purpose of this method is to validate or check the Volume Balance determined by the
        /// CalculatePrismoidalVolumeToSurface Method.  
        /// </summary>
        /// <category>DTM Volume</category>
        /// <param name="dtm">DTM Surface to calculate the volume balance for.</param>
        /// <param name="criteria">Criteria.</param>
        /// <returns>The calculated volume.</returns>
        /// <author>Rob.Cormack</author>                              <date>03/2009</date>
        //=======================================================================================
        VolumeResult^ CalculatePrismoidalVolumeBalanceToSurface (DTM^ dtm, VolumeCriteria^ criteria);

 
        //=======================================================================================
        /// <summary>
        /// Calculates The Cut Fill And Balance Volumes Between A DTM And Set Elevation and will optionally :-
        /// 1. Determine Volumes For A Specified Area Defined By A Polygon
        /// 2. Will Range The Cut and Fill Volumes For User Defined Elevation Ranges
        /// </summary>
        /// <category>DTM Volume</category>
        /// <param name="elevation">Elevation to calculate the cut fill volumes for.</param>
        /// <param name="numGridPoints">Number Of Grid Points To Use For The Calculation</param>
        /// <param name="criteria">Criteria.</param>
        /// <returns>The calculated volume.</returns>
        /// <author>Rob.Cormack</author>                              <date>03/2009</date>
        //=======================================================================================
        VolumeResult^ CalculateGridVolumeToElevation (double elevation, int numGridPoints , VolumeCriteria^ criteria);

        //=======================================================================================
        /// <summary>
        /// Calculates The Cut Fill And Balance Volumes Between Two DTMs and will optionally :-
        /// 1. Determine Volumes For A Specified Area Defined By A Polygon
        /// 2. Will Range The Cut and Fill Volumes For User Defined Elevation Ranges
        /// </summary>
        /// <category>DTM Volume</category>
        /// <param name="dtm">DTM Surface to calculate the cut fill volumes for.</param>
        /// <param name="numGridPoints">Number Of Grid Points To Use For The Calculation</param>
        /// <param name="criteria">Criteria.</param>
        /// <returns>The calculated volume.</returns>
        /// <author>Rob.Cormack</author>                              <date>03/2009</date>
        //=======================================================================================
        VolumeResult^ CalculateGridVolumeToSurface (DTM^ dtm, int numGridPoints , VolumeCriteria^ criteria);

        //=======================================================================================
        /// <summary>
        /// Creates a DTM from a handle.
        /// </summary>
        /// <category>DTM Creation</category>
        /// <param name="handle">Handle.</param>
        /// <author>Daryl.Holmwood</author>                              <date>01/2008</date>
        //=======================================================================================
        static DTM^ FromHandle (IntPtr handle);


        //=======================================================================================
        /// <summary>
        /// Creates a DTM from A Native Dtm Handle.
        /// </summary>
        /// <category>DTM Creation</category>
        /// <param name="handle">Handle.</param>
        /// <author>Rob.Cormack</author>                                 <date>07/2009</date>
        //=======================================================================================
        static DTM^ FromNativeDtmHandle (IntPtr handle);

        //=======================================================================================
        /// <summary>
        /// Gets the mesh that represents this DTM.
        /// <param name="firstCall">Must be true the first time.</param>
        /// <param name="maxMeshSize">Maximum number of meshes per call.</param>
        /// </summary>
        /// <category>DTM</category>
        /// <returns>The mesh representation.</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        Mesh^ GetMesh (bool firstCall, int maxMeshSize);

        //=======================================================================================
        /// <summary>
        /// Gets the edges that represents this DTM.
        /// </summary>
        /// <category>DTM</category>
        /// <returns>The edge representation.</returns>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        Edges^ GetEdges ();

        //=======================================================================================
        /// <summary>
        /// Transforms the DTM
        /// </summary>
        /// <category>DTM Transform</category>
        /// <author>Sylvain.Pucci</author>                              <date>10/2007</date>
        //=======================================================================================
        void Transform (BGEO::DTransform3d transform);

        //=======================================================================================
        /// <summary>
        /// Transforms the DTM Using a Callback for non uniformed transformations.
        /// </summary>
        /// <category>DTM Transform</category>
        /// <author>Daryl.Holmwood</author>                              <date>05/2011</date>
        //=======================================================================================
        void TransformPointsUsingCallback (DTMTransformCallbackDelegate^ hdl, System::Object^ oArg);

        //=======================================================================================
        /// <summary>
        /// Converts the DTM Units
        /// <param name="xyFactor">Unit Conversion Factor For The X any Y Coordinates</param>
        /// <param name="zFactor">Unit Conversion Factor For The Z Coordinates</param>
        /// </summary>
        /// <category>DTM Transform</category>
        /// <author>Rob.Cormack</author>                              <date>11/2009</date>
        //=======================================================================================
        void ConvertUnits(double xyFactor,double zFactor);

        //=======================================================================================
        /// <summary>
        /// Gets the Editor for this Dtm
        /// </summary>
        /// <category>DTM Edit</category>
        /// <author>Daryl.Holmwood</author>                              <date>04/2008</date>
        //=======================================================================================
        DTMTinEditor^ GetTinEditor()
            {
            return gcnew DTMTinEditor (Handle);
            }

        //=======================================================================================
        /// <summary>
        /// Purges the DTM of certain Features.
        /// </summary>
        /// <category>DTM Edit</category>
        /// <author>Daryl.Holmwood</author>                              <date>06/2008</date>
        //=======================================================================================
        void PurgeDTM (DTMPurgeFlags flags);

        //=======================================================================================
        /// <summary>
        /// Gets the point at a given index.
        /// </summary>
        /// <param name="index">Index of the point to return.</param>
        /// <category>DTM</category>
        /// <author>Daryl.Holmwood</author>                              <date>06/2008</date>
        //=======================================================================================
        BGEO::DPoint3d GetPoint (long index);

        //=======================================================================================
        /// <summary>
        /// Reads An DPoint3d Array From An XYZ File
        /// </summary>
        /// <param name="fileName"> Nane Of XYZ File</param>
        /// <category>DTM</category>
        /// <author>Rob.Cormack</author>                              <date>03/2009</date>
        //=======================================================================================
        static  array<BGEO::DPoint3d>^  DTM::ImportDPoint3DArrayFromXYZFile (String ^fileName) ;


        //=======================================================================================
        /// <summary>
        /// Gets The DTM State For API Purposes
        /// </summary>
        /// <category>DTM</category>
        /// <author>Rob.Cormack</author>                              <date>03/08/2009</date>
        //=======================================================================================
         DtmState  GetDtmState() ;


        //=======================================================================================
        /// <summary>
        /// Converts The DTM From A Triangulated State To An Untriangulated Data State
        /// </summary>
        /// <category>DTM</category>
        /// <author>Rob.Cormack</author>                              <date>05/08/2009</date>
        //=======================================================================================
         void  ConvertToDataState() ;


        ///=======================================================================================
        /// <summary>
        /// Overides the default memory allocation parameters set when a DTM is created.
        /// When a DTM is instantiated it is assigned a vlaue of 250000 for the initialPointSize
        /// and a value of 50000 for the incrementalPointSize. When the first DTM feature
        /// is stored in the DTM an initial memory block of size initialPointsSize is allocated. 
        /// After this initial amount of memory is used it allocates memory blocks of size incrementalPointSize.
        /// </summary>
        /// <category>DTM Memory Management</category>
        /// <param name="initialPointSize"> Initial Memory Allocation In Terms Of DTM points.</param>
        /// <param name="incrementalPointSize"> Incremental Memory Allocation In Terms Of DTM points.</param>
        /// <author>Rob.Cormack</author>                              <date>05/08/2009</date>
        ///=======================================================================================
         void  SetMemoryAllocationParameters( int initialPointSize,int incrementalPointSize) ;

        ///=======================================================================================
        /// <summary>
        /// Interpolates Elevations For DTM Features From Points
        /// </summary>
        /// <category>Interpolation</category>
        /// <param name="dtmFeatureType"> Dtm Feature Type To Be Interpolated</param>
        /// <param name="snapTolerance"> Interpolation Snap Tolerance.</param>
        /// <param name="pointsDtm"> DTM Containing The Points To Use For The Interpolation.</param>
        /// <param name="interpolatedDTM"> DTM To Store The Interpolated Features.</param>
        /// <author>Rob.Cormack</author>                              <date>19/01/2010</date>
        ///=======================================================================================
        InterpolationResult^ InterpolateDtmFeatureType( DTMFeatureType dtmFeatureType,double snapTolerance,DTM ^pointsDtm, DTM ^interpolatedDTM) ;

        ///=======================================================================================
        /// <summary>
        /// Determines Whether A Point Is Visible From The Eye
        /// </summary>
        /// <category>Visibility</category>
        /// <param name="Eye"> Location Of The Eye</param>
        /// <param name="Point"> Location Of The Point.</param>
        /// <author>Rob.Cormack</author>                              <date>17/03/2010</date>
        ///=======================================================================================
        bool PointVisibility ( BGEO::DPoint3d Eye , BGEO::DPoint3d Point ) ;

        ///=======================================================================================
        /// <summary>
        /// Determines Whether A Line Is Visible From The Eye
        /// </summary>
        /// <category>Visibility</category>
        /// <param name="Eye"> Location Of The Eye</param>
        /// <param name="linePoint1"> Start Coordinate Of Line.</param>
        /// <param name="linePoint2"> End Coordinate Of Line.</param>
        /// <author>Rob.Cormack</author>                              <date>17/03/2010</date>
        ///=======================================================================================
        VisibilityResult^ LineVisibility ( BGEO::DPoint3d Eye , BGEO::DPoint3d linePoint1 , BGEO::DPoint3d linePoint2  ) ;

        ///=======================================================================================
        /// <summary>
        /// Browses the Visibility Of All Tin Points From The Eye Position 
        /// </summary>
        /// <category>Visibility</category>
        /// <param name="Eye"> Location Of The Eye</param>
        /// <param name="hdlP">Delegate called for each tin Point.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Rob.Cormack</author>                              <date>17/03/2010</date>
        ///=======================================================================================
        void  BrowseTinPointsVisibility (BGEO::DPoint3d Eye,DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg) ;

        ///=======================================================================================
        /// <summary>
        /// Browses the Visibility Of All Tin Lines From The Eye Position 
        /// </summary>
        /// <category>Visibility</category>
        /// <param name="Eye"> Location Of The Eye</param>
        /// <param name="hdlP">Delegate called for each tin Point.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Rob.Cormack</author>                              <date>17/03/2010</date>
        ///=======================================================================================
        void  BrowseTinLinesVisibility (BGEO::DPoint3d Eye,DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg) ;

        ///=======================================================================================
        /// <summary>
        /// Browses the Visibility Of All Radials Emanating From And Around Eye Position 
        /// </summary>
        /// <category>Visibility</category>
        /// <param name="radialViewShedCriteria"> Controls The creation Of The Radials About The Eye</param>
        /// <param name="hdlP">Delegate called for each radial.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Rob.Cormack</author>                              <date>17/03/2010</date>
        ///=======================================================================================
        void  BrowseRadialViewSheds(RadialViewShedsBrowsingCriteria^ radialViewShedCriteria ,DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg) ;

        ///=======================================================================================
        /// <summary>
        /// Browses the Visibility Of All Regions From The Eye Position
        /// Creates Polygons Of Visible And Invisible Regions 
        /// </summary>
        /// <category>Visibility</category>
        /// <param name="eye"> Location Of The Eye</param>
        /// <param name="hdlP">Delegate called for each region.</param>
        /// <param name="oArg">Parameters for the delegate.</param>
        /// <author>Rob.Cormack</author>                              <date>17/03/2010</date>
        ///=======================================================================================
        void  BrowseRegionViewSheds(BGEO::DPoint3d eye,DynamicFeaturesBrowsingDelegate^ hdlP, System::Object^ oArg) ;

        ///=======================================================================================
        /// <summary>
        /// Replaces The DTM Feature Points With The Ones In This Call
        /// </summary>
        /// <category>DTM Modification</category>
        /// <param name="dtmFeatureId"> Feature Id Of Feature For Which The Points Will Be Replaced.</param>
        /// <param name="points">The Replacement Points.</param>
        /// <author>Rob.Cormack</author>                              <date>25/01/2011</date>
        ///=======================================================================================
        void  ReplaceFeaturePoints(DTMFeatureId dtmFeatureId , array<BGEO::DPoint3d>^ points) ;

        ///=======================================================================================
        /// <summary>
        /// Tries to replace The DTM Feature Points With The Ones In This Call
        /// </summary>
        /// <category>DTM Modification</category>
        /// <param name="dtmFeatureId"> Feature Id Of Feature For Which The Points Will Be Replaced.</param>
        /// <param name="points">The Replacement Points.</param>
        /// <returns>true if it was successful in replacing the feature</returns>
        /// <author>Rob.Cormack</author>                              <date>25/01/2011</date>
        ///=======================================================================================
        bool  TryReplaceFeaturePoints(DTMFeatureId dtmFeatureId , array<BGEO::DPoint3d>^ points) ;

        ///=======================================================================================
        /// <summary>
        /// Get/Sets the Stops the Auto update Memory Pressure calls.
        /// </summary>
        /// <category>DTM Memory Management</category>
        /// <author>Daryl.Holmwood</author>                              <date>21/08/2009</date>
        ///=======================================================================================
         property bool AutoUpdateMemoryPressure
             {
             bool get()
                 {
                 return m_autoUpdateMemoryPressure;
                 }
             void set(bool value)
                 {
                 m_autoUpdateMemoryPressure = value;
                 if(value)
                     CheckMemoryPressure();
                 }
             }

        ///=======================================================================================
        /// <summary>
        /// Get the last modified time of the DTM.
        /// </summary>
        /// <author>Daryl.Holmwood</author>                              <date>22/07/2011</date>
        ///=======================================================================================
         property System::DateTime LastModifiedTime
             {
             System::DateTime get();
             }

        ///=======================================================================================
        /// <summary>
        /// Commits changes to the DTM, to the DTM Element.
        /// </summary>
        /// <author>Daryl.Holmwood</author>                              <date>22/07/2011</date>
        ///=======================================================================================
         void Commit();

        ///=======================================================================================
        /// <summary>
        /// Creates A Stock Pile From The Conveyor Head To The Ground TM
        /// </summary>
        /// <author>Rob.Cormack</author>                              <date>19/10/2012</date>
        ///=======================================================================================
        StockPileResult^ CreateStockPile (StockPileCriteria^ stockPileCriteria ) ;

        ///=======================================================================================
        /// <summary>
        /// Intersect a Vector to the DTM.
        /// </summary>
        /// <author>Daryl.Holmwood</author>                              <date>27/05/2014</date>
        ///=======================================================================================
        bool IntersetVector ([System::Runtime::InteropServices::Out]BGEO::DPoint3d% intersectionPoint, BGEO::DPoint3d startPoint, BGEO::DPoint3d endPoint);

        /// <summary>
        /// Cleans the DTM and compacts the arrays.
        /// </summary>
        void Clean ();

        /// <summary>
        /// Adds multipe segmented linear feature to the terrain model
        /// </summary>
        /// <param name="features">The features to add.</param>
        /// <param name="featureType">The feature Type</param>
        /// <param name="userTag">The UserTag</param>
        /// <returns>The feature id of the new feature</returns>
        DTMFeatureId AddFeatureWithMultipleSegments (System::Collections::Generic::IEnumerable<System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^>^ features, DTMFeatureType featureType, DTMUserTag userTag);

        /// <summary>
        /// Adds multiple segmented linear feature to the terrain model
        /// </summary>
        /// <param name="features">The features to add.</param>
        /// <param name="featureType">The feature Type</param>
        /// <param name="userTag">The UserTag</param>
        /// <returns>The feature id of the new feature</returns>
        DTMFeatureId AddFeatureWithMultipleSegments (System::Collections::Generic::IEnumerable<array<BGEO::DPoint3d>^>^ features, DTMFeatureType featureType, DTMUserTag userTag);

        /// <summary>
        /// Tries and replace a feature with a multisegmented feature
        /// </summary>
        /// <param name="features">The new feature points</param>
        /// <param name="featureId">The feature id to replace</param>
        /// <returns>true if successful</returns>
        bool TryReplaceFeatureWithMultipleSegments (System::Collections::Generic::IEnumerable<System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^>^ features, DTMFeatureId featureId);

        /// <summary>
        /// Tries and replace a feature with a multisegmented feature
        /// </summary>
        /// <param name="features">The new feature points</param>
        /// <param name="featureId">The feature id to replace</param>
        /// <returns>true if successful</returns>
        bool TryReplaceFeatureWithMultipleSegments (System::Collections::Generic::IEnumerable<array<BGEO::DPoint3d>^>^ features, DTMFeatureId featureId);

        /// <summary>
        /// Projects a point onto the Terrainmodel
        /// </summary>
        /// <param name="pointOnDTM">The point on the Terrain Model</param>
        /// <param name="w2vMap">The matrix of the view</param>
        /// <param name="testPoint">The point to project</param>
        /// <returns>true if the projection hit the terrain model</returns>
        bool GetProjectedPointOnDTM ([System::Runtime::InteropServices::Out]BGEO::DPoint3d% pointOnDTM, BGEO::DMatrix4d% w2vMap, BGEO::DPoint3d testPoint);

        /// <summary>
        /// Projects a point onto the Terrainmodel
        /// </summary>
        /// <param name="pointOnDTM">The point on the Terrain Model</param>
        /// <param name="w2vMap">The matrix of the view</param>
        /// <param name="testPoint">The point to project</param>
        /// <returns>true if the projection hit the terrain model</returns>
        bool GetProjectedPointOnDTM ([System::Runtime::InteropServices::Out]BGEO::DPoint3d% pointOnDTM, BGEO::DMap4d% w2vMap, BGEO::DPoint3d testPoint);

        /// <summary>
        /// Filters the number of points in the TerrainModel
        /// </summary>
        /// <param name="numPointsToRemove">Number points to remove</param>
        /// <param name="percentageToRemove">Percentage to remove</param>
        /// <param name="pointsBefore">Number of points before the filter.</param>
        /// <param name="pointsAfter">Number of points after the filter.</param>
        void FilterPoints (long numPointsToRemove, double percentageToRemove, [System::Runtime::InteropServices::Out]long% pointsBefore, [System::Runtime::InteropServices::Out]long% pointsAfter);

        };

END_BENTLEY_TERRAINMODELNET_NAMESPACE
