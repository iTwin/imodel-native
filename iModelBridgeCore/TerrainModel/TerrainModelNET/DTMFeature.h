/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTMFeature.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include ".\Bentley.Civil.DTM.h"
#include <vcclr.h>

BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

//=======================================================================================
/// <summary>
/// Defines DTM feature types.
/// </summary>
/// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
//=======================================================================================
public enum class DTMFeatureType
    {
    /// <summary>
    /// Point
    /// Defines one or more points that is stored in the DTM without
    /// an identifier. Points can not be uniquely browsed. They are browsed in total.  
    /// A Point is the preferred feature type to store large quantities of point data in the DTM
    /// For Example Lidar points should be stored as Points.
    /// </summary>
    Point = (long)::DTMFeatureType::RandomSpots,
    /// <summary>
    /// PointFeature
    /// Defines a Point feature of one or more points that is stored in the DTM with an identifier.
    /// PointFeatures can be uniquely browsed. PointFeatures are used to group a collection of points for 
    /// some future purpose. For example a set of survey control points can be stored as one PointFeature. 
    /// </summary>
    PointFeature = (long)::DTMFeatureType::GroupSpots,
    /// <summary>
    /// BreakLine  
    /// Defines a break line feature of two or more points that is used to constrain the triangulation.
    /// BreakLines are persisted in the Tin and can be individually browsed.
    /// </summary>
    Breakline = (long)::DTMFeatureType::Breakline,
    /// <summary>
    /// Soft Break Line  
    /// Defines a soft break line feature of two or more points that is used to constrain the triangulation.
    /// Soft BreakLines are persisted in the Tin and can be individually browsed. Contours are smoothed 
    /// over soft break lines 
    /// </summary>
    SoftBreakline = (long)::DTMFeatureType::SoftBreakline,
    /// <summary>
    /// Contour
    /// Defines a contour feature of two or more points that is used to constrain the triangulation.
    /// Contours are persisted in the Tin and can be individually browsed. Additional processing, beyond break line processing,
    /// is applied to contour features to remove zero slope triangles along the contour.
    /// For removal of zero slope triangles it is best to store a contour as one contour feature
    /// rather than multiple contour features representing contour segments.  
    /// </summary>
    Contour = (long)::DTMFeatureType::ContourLine,
    /// <summary>
    /// Void
    /// Defines a polygonal void feature of three or more points that is used to delineate areas in the triangulation
    /// for which no data exists. Voids are persisted in the Tin and can be individually browsed.
    /// Void feature points are included in the triangulation and the void segments joining void points
    /// are draped over the tin surface. 
    /// The triangulation engine assumes that no voids, breakvoids , drapeVoids or islands touch or intersect. 
    /// </summary>
    Void = (long)::DTMFeatureType::Void,
    /// <summary>
    /// BreakVoid
    /// Defines a polygonal break void feature of three or more points that is used to delineate areas in the triangulation
    /// for which no data exists. Break voids are persisted in the Tin as voids and can be individually browsed.
    /// Break Void feature points are included in the triangulation and the void segments joining the break void points
    /// modify the tin surface elevation by inserting the segements as break lines. For example, a break void would be the
    /// best way to represent a lake boundary of constant elevation.
    /// The triangulation engine assumes that no voids, breakvoids , drapeVoids or islands touch or intersect. 
    /// </summary>
    BreakVoid = (long)::DTMFeatureType::BreakVoid,
    /// <summary>
    /// DrapeVoid
    /// Defines a polygonal drape void feature of three or more points that is used to delineate areas in the triangulation
    /// for which no data exists. Drape voids are persisted in the Tin as voids and can be individually browsed.
    /// Drape voids are post inserted into the Tin after triangulation by draping the drape void over the Tin surface.
    /// Therefore a drape void does not require elevation values and provides a convenient method for voiding areas
    /// in the triangulation directly from a 2D polygon. 
    /// The triangulation engine assumes that no voids, breakvoids , drapeVoids or islands touch or intersect. 
    /// </summary>
    DrapeVoid = (long)::DTMFeatureType::DrapeVoid,
    /// <summary>
    /// Island  
    /// Defines a polygonal island feature of three or more points that is used to delineate areas inside a void in the triangulation
    /// for which data exists. Islands are persisted in the Tin and can be individually browsed.
    /// Islands must be internal to a void and voids can be internal to an island.
    /// The triangulation engine assumes that no voids, breakvoids , drapeVoids or islands touch or intersect. 
    /// </summary>
    Island = (long)::DTMFeatureType::Island,
    /// <summary>
    /// Hole Feature
    /// Defines a polygonal hole feature of three or more points that is used to delineate areas in the triangulation
    /// for which no data exists. Holes are persisted in the Tin and can be individually browsed.
    /// A hole is a variant of a void feature and the difference between a void and a hole is only apparent
    /// when one triangulation is merged into another. Consider merging surface B into surface A when part of
    /// surface A is replaced by surface B. If surface B has voids then the surface B voids will
    /// become part of the merged surface. If surface B has holes then the merged surface will contain areas of 
    /// of surface A that correspond to the surface B holes.
    /// </summary>
    Hole = (long)::DTMFeatureType::Hole,
    /// <summary>
    /// Hull
    /// Defines a polygonal hull feature of three or more points that is used to delineate the triangulation hull.
    /// There can only be one hull per DTM. The Hull is persisted in the Tin and can be browsed.
    /// A triangualtion hull is always convex. The hull feature modifies or constrains the triangulation hull
    /// to one that is more representative of the surface data. If there is more than one hull feature 
    /// the triangulation engine will use the first one it finds.
    /// </summary>
    Hull = (long)::DTMFeatureType::Hull,
    /// <summary>
    /// Drape Hull
    /// Defines a polygonal hull feature of three or more points that is used to delineate the triangulation hull.
    /// There can only be one hull per DTM. The Hull is persisted in the Tin and can be browsed.
    /// A drape hull is post inserted after the initial triangulation. It is draped onto the Tin surface and 
    /// then inserted into the Tin. All features external to the inserted drape hull are clipped from the Tin.
    /// Once inserted into the Tin it is renamed to a Hull.
    /// </summary>
    DrapeHull = (long)::DTMFeatureType::DrapeHull,
    /// <summary>
    /// HullLine
    /// Defines a HullLine feature of two or more points that is used to represent the extermity of the triangulation.
    /// at distinct locations. HullLines are a variant of Hull features. HullLine features are not persisted in the Tin but can be browsed prior to
    /// triangulation. HullLine features are used to constrain the triangulation.
    /// A Hullline feature is a special 3D modelling feature and normally should not be used out of this context
    /// and is included here for completeness
    /// </summary>
    HullLine = (long)::DTMFeatureType::HullLine,
    /// <summary>
    /// VoidLine
    /// Defines a VoidLine feature of two or more points that is used to represent the extermity of voids
    /// at distinct locations. VoidLines are a variant of Void features. VoidLine features are not persisted in the Tin but can be browsed prior to
    /// triangulation. VoidLine features are used to construct voids.
    /// A VoidLine feature is a special 3D modelling feature and normally should not be used out of this context
    /// and is included here for completeness
    /// </summary>
    VoidLine = (long)::DTMFeatureType::VoidLine,
    /// <summary>
    /// HoleLine
    /// Defines a HoleLine feature of two or more points that is used to represent the extermity of holes.
    /// at distinct locations. HoleLines are a variant of Hole features. HoleLine features are not persisted in the Tin but can be browsed prior to
    /// triangulation. HoleLine features are used to construct holes.
    /// A HoleLine feature is a special 3D modelling feature and normally should not be used out of this context
    /// and is included here for completeness
    /// </summary>
    HoleLine = (long)::DTMFeatureType::HoleLine,
    /// <summary>
    /// Slope Toe
    /// Defines a SlopeToe feature of two or more points that is used to represent the intersection of a sideslope
    /// with the Tin surface. SlopeToe features are used to communicate information between the DTM and other applications.
    /// SlopeToe features are not included in the triangualtion and need to be converted to some other DTM feature if they
    /// are to be include in the triangulation.
    /// A SlopeToe feature is a special 3D modelling feature and normally should not be used out of this context
    /// and is included here for completeness
    /// </summary>
    SlopeToe = (long)::DTMFeatureType::SlopeToe,
    /// <summary>
    /// Graphic Break 
    /// A GraphicBreak consisting of two or more points is used to constrain the triangulation in 
    /// the same manner as a break line or contour but is not persisted in the Tin. Its primary use
    /// is to reproduce identical triangulations. For example a triangle is stored as a Graphic Break.     
    /// </summary>
    GraphicBreak = (long)::DTMFeatureType::GraphicBreak,
    /// <summary>
    /// Region 
    /// A Region is a polygon that is persisted in the Tin for the purpose of browsing DTM features
    /// within a region.
    ///     
    /// </summary>
    Region = (long)::DTMFeatureType::Region,
    };

//=======================================================================================
/// <summary>
/// Defines dynamic DTM feature types.
/// </summary>
/// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
//=======================================================================================
public enum class DTMDynamicFeatureType:long
    {
    /// <summary>
    /// Defines contour feature
    /// </summary>
    Contour = (long)::DTMFeatureType::Contour,
    /// <summary>
    /// Defines low point feature
    /// </summary>
    LowPoint  = (long)::DTMFeatureType::LowPoint,
    /// <summary>
    /// Defines high point feature
    /// </summary>
    HighPoint  = (long)::DTMFeatureType::HighPoint,
    /// <summary>
    /// Defines sump line feature
    /// </summary>
    SumpLine = (long)::DTMFeatureType::SumpLine,
    /// <summary>
    /// Defines ridge line feature
    /// </summary>
    RidgeLine = (long)::DTMFeatureType::RidgeLine,
    /// <summary>
    /// Defines descent trace feature
    /// </summary>
    DescentTrace = (long)::DTMFeatureType::DescentTrace,
    /// <summary>
    /// Defines ascent trace feature
    /// </summary>
    AscentTrace = (long)::DTMFeatureType::AscentTrace,
    /// <summary>
    /// Defines catchment feature
    /// </summary>
    Catchment = (long)::DTMFeatureType::Catchment,
    /// <summary>
    /// Defines A Pond Feature
    /// The Boundary Of A Pond 
    /// </summary>
    Pond = (long)::DTMFeatureType::LowPointPond,
    /// <summary>
    /// Defines A Pond Island Feature
    /// An Island Within A Pond
    /// </summary>
    PondIsland = (long)::DTMFeatureType::PondIsland,
    /// <summary>
    /// Defines A Visible Point Feature
    /// A Point That Can Be Seen From The Eye Location
    /// </summary>
    VisiblePoint = (long)::DTMFeatureType::VisiblePoint,
    /// <summary>
    /// Defines An Invisible Point Feature
    /// A Point That Can Not Be Seen From The Eye Location
    /// </summary>
    InvisiblePoint = (long)::DTMFeatureType::InvisiblePoint,
    /// <summary>
    /// Defines A Visible Line Feature
    /// A Line That Can Be Seen From The Eye Location
    /// </summary>
    VisibleLine = (long)::DTMFeatureType::VisibleLine,
    /// <summary>
    /// Defines An Invisible Line Feature
    /// A Line That Can Not Be Seen From The Eye Location
    /// </summary>
    InvisibleLine = (long)::DTMFeatureType::InvisibleLine,
    /// <summary>
    /// Defines theme feature
    /// </summary>
    Theme = (long)::DTMFeatureType::Theme,
    /// <summary>
    /// Defines Grade Slope feature
    /// </summary>
    GradeSlope = (long)::DTMFeatureType::GradeSlope,
    /// <summary>
    /// Traingle 
    /// </summary>
    Triangle = (long)::DTMFeatureType::Triangle,
    /// <summary>
    /// Traingle 
    /// </summary>
    TriangleInfo = (long)::DTMFeatureType::TriangleInfo,
    /// <summary>
    /// Traingle Edge
    /// </summary>
    TriangleEdge = (long)::DTMFeatureType::TriangleEdge,
    /// <summary>
    /// Traingle Vertex
    /// </summary>
    TriangleVertex = (long)::DTMFeatureType::TinPoint,
    };

//=======================================================================================
/// <summary>
/// Represents a DTM Feature Id
/// </summary>                
/// <author>Sylvain.Pucci</author>                              <date>2/2008</date>
//=======================================================================================
public value struct DTMFeatureId
    {
    private:

        ::DTMFeatureId m_id;

    public:
        static initonly DTMFeatureId NullId = DTMFeatureId (DTM_NULL_FEATURE_ID);

    internal:
        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the DTMFeatureId class.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>2/2008</date>
        //=======================================================================================
        DTMFeatureId (::DTMFeatureId id)
            {
            m_id = id;
            }

    public:

        //=======================================================================================
        /// <summary>
        /// Gets the feature Id.
        /// </summary>                
        /// <author>Daryl.Holmwood</author>                              <date>4/2014</date>
        //=======================================================================================
        static DTMFeatureId FromId (::DTMFeatureId id)
            {
            return DTMFeatureId (id);
            }

        //=======================================================================================
        /// <summary>
        /// Gets the internal Id.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>2/2008</date>
        //=======================================================================================
        property ::DTMFeatureId Id
            {
            ::DTMFeatureId get ()
                {
                return m_id;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets the internal Id.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>2/2008</date>
        //=======================================================================================
        virtual System::String^ ToString () override
            {
            return m_id.ToString ();
            }

        //=======================================================================================
        /// <summary>
        /// Gets a DTMFeatureId from a string.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>2/2008</date>
        //=======================================================================================
        static DTMFeatureId FromString (System::String^ string)
            {
            Int64 id = Int64::Parse (string);
            return DTMFeatureId (id);
            }

        //=======================================================================================
        /// <summary>
        /// Gets a DTMFeatureId from a string.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>2/2008</date>
        //=======================================================================================
        static DTMFeatureId FromStorage (array<Byte>^ bytes)
            {
            Int64 fromStorage = 0;
            return DTMFeatureId (fromStorage);
            }

        property bool IsNullId
            {
            bool get()
                {
                return (m_id == DTM_NULL_FEATURE_ID);
                }
            }

    };

//=======================================================================================
/// <summary>
/// Represents a DTM feature.
/// </summary>
/// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
//=======================================================================================
public ref class DTMFeature 
#if (_MSC_VER >= 1400)
abstract
#endif
    {
    protected:
        BcDTMFeature* m_dtmFeature;

    internal:
        DTMFeature(BcDTMFeature* dtmFeature);
        virtual ~DTMFeature();
        !DTMFeature();


    public:

        [EditorBrowsable(EditorBrowsableState::Never) ]
        property BcDTMFeature* ExternalHandle
            {
            BcDTMFeature* get()
                {
                return m_dtmFeature;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Gets the ID of the feature.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        property virtual DTMFeatureId Id
            {
            DTMFeatureId get();
            }

        //=======================================================================================
        /// <summary>
        /// Gets the user tag of the feature.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        property virtual ::DTMUserTag UserTag
            {
            ::DTMUserTag get();
            }

       //=======================================================================================
        /// <summary>
        /// Gets the DTM Feature Type of the feature.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        property virtual DTMFeatureType FeatureType
            {
             DTMFeatureType get();
            }

        //=======================================================================================
        /// <summary>
        /// Gets the count of elements of the feature.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        property virtual int ElementsCount
            {
            int get();
            }

        //=======================================================================================
        /// <summary>
        /// Gets the points of the feature.
        /// </summary>                
        /// <author>Daryl.Holmwood</author>                              <date>03/2010</date>
        //=======================================================================================
        virtual array<BGEO::DPoint3d>^ GetElementPoints (int element) abstract;

    };

//=======================================================================================
/// <summary>
/// Represents a spot feature.
/// </summary>
/// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
//=======================================================================================
public ref class DTMSpot: public DTMFeature
    {
    internal:

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of a spot feature.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        DTMSpot (BcDTMSpot* dtmSpot);

    public:

        //=======================================================================================
        /// <summary>
        /// Gets the points of the feature.
        /// </summary>                
        /// <author>Daryl.Holmwood</author>                              <date>03/2010</date>
        //=======================================================================================
        array<BGEO::DPoint3d>^  GetPoints ();

        //=======================================================================================
        /// <summary>
        /// Gets the points of the feature.
        /// </summary>                
        /// <author>Daryl.Holmwood</author>                              <date>03/2010</date>
        //=======================================================================================
        virtual array<BGEO::DPoint3d>^ GetElementPoints (int element) override
            {
            if(element == 0)
                return GetPoints();
            return nullptr;
            }
    };

//=======================================================================================
/// <summary>
/// Represents a linear feature.
/// </summary>
/// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
//=======================================================================================
public ref class DTMLinearFeature : public DTMFeature
    {
    internal:

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of a linear feature.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        DTMLinearFeature (BcDTMFeature* dtmLinearFeature);

    public:

        //=======================================================================================
        /// <summary>
        /// Gets the elements of the feature.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
//DHToDo        virtual array<LinearElement^>^ GetElements();

        //=======================================================================================
        /// <summary>
        /// Gets the points of the feature.
        /// </summary>                
        /// <author>Daryl.Holmwood</author>                              <date>03/2010</date>
        //=======================================================================================
        virtual array<BGEO::DPoint3d>^ GetElementPoints (int element) override;
    };

//=======================================================================================
/// <summary>
/// Represents a linear feature.
/// </summary>
/// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
//=======================================================================================
public ref class DTMComplexLinearFeature: public DTMLinearFeature
    {
    internal:

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of a linear feature.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        DTMComplexLinearFeature (BcDTMComplexLinearFeature* DTMComplexLinearFeature);

    public:

        //=======================================================================================
        /// <summary>
        /// Gets the count of elements of the feature.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        property int ElementsCount
            {
            virtual int get() override;
            }

        //=======================================================================================
        /// <summary>
        /// Gets the elements of the feature.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
//DHToDo        virtual array<LinearElement^>^ GetElements() override;

        //=======================================================================================
        /// <summary>
        /// Gets the points of the feature.
        /// </summary>                
        /// <author>Daryl.Holmwood</author>                              <date>03/2010</date>
        //=======================================================================================
        virtual array<BGEO::DPoint3d>^ GetElementPoints (int element) override;
    };

END_BENTLEY_TERRAINMODELNET_NAMESPACE
