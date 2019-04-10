/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/RadialGridPortion.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

#define CIRCULAR_GRID_EXTEND_LENGTH 5

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RadialGrid : PlanGrid
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_RadialGrid, PlanGrid);

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(RadialGrid::T_Super::CreateParams);

        double m_defaultAngleIncrement;
        double m_defaultRadiusIncrement;
        double m_defaultStartAngle;
        double m_defaultEndAngle;
        double m_defaultStartRadius;
        double m_defaultEndRadius;

        //! Creates create parameters for radial grid
        //! @param[in] model                    model to create the grid in
        //! @param[in] scopeElementId           scope element id
        //! @param[in] name                     grid name, must be unique
        //! @param[in] defaultAngleIncrement    default angle increment
        //! @param[in] defaultRadiusIncrement   default radius increment
        //! @param[in] defaultStartAngle        default start angle
        //! @param[in] defaultEndAngle          default end angle
        //! @param[in] defaultStartRadius       default start radius
        //! @param[in] defaultEndRadius         default end radius
        //! @param[in] defaultstaElevation      default start elevation
        //! @param[in] defaultendElevation      default end elevation
        CreateParams(Dgn::SpatialLocationModelCR model, Dgn::DgnElementId scopeElementId, Utf8CP name, double defaultAngleIncrement,
                     double defaultRadiusIncrement, double defaultStartAngle, double defaultEndAngle, double defaultStartRadius, 
                     double defaultEndRadius, double defaultstaElevation, double defaultendElevation) :
            T_Super (model, scopeElementId, name, QueryClassId (model.GetDgnDb ()), defaultstaElevation, defaultendElevation)
            {
            m_defaultAngleIncrement = defaultAngleIncrement;
            m_defaultRadiusIncrement = defaultRadiusIncrement;
            m_defaultStartAngle = defaultStartAngle;
            m_defaultEndAngle = defaultEndAngle;
            m_defaultStartRadius = defaultStartRadius;
            m_defaultEndRadius = defaultEndRadius;
            }

        //! Constructor from base params. Chiefly for internal use.
        //! @param[in]      params   The base element parameters
        //! @return 
        explicit GRIDELEMENTS_EXPORT CreateParams(Dgn::DgnElement::CreateParams const& params)
            : T_Super(params) 
                {
                m_defaultAngleIncrement = m_defaultRadiusIncrement = m_defaultStartAngle =
                    m_defaultEndAngle = m_defaultStartRadius = m_defaultEndRadius = 0.0; //initialize with zeros, this is not supposed to be valid..
                }
        };

private:

    BE_PROP_NAME(DefaultAngleIncrement)
    BE_PROP_NAME(DefaultRadiusIncrement)
    BE_PROP_NAME(DefaultStartAngle)
    BE_PROP_NAME(DefaultEndAngle)
    BE_PROP_NAME(DefaultStartRadius)
    BE_PROP_NAME(DefaultEndRadius)

protected:
    explicit GRIDELEMENTS_EXPORT RadialGrid (CreateParams const& params);

    static BentleyStatus CreateAndInsertGridSurfaces (CreateParams params, Dgn::SpatialLocationModelCPtr model, GridAxisCR planeAxis, GridAxisCR arcAxis);

    friend struct RadialGridHandler;

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (RadialGrid, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Creation
    //---------------------------------------------------------------------------------------
    //! Creates an empty radial grid
    //! @param[in]  model   model for the radialgridportion
    //! @return             Radial grid
    GRIDELEMENTS_EXPORT static RadialGridPtr Create (CreateParams const& params);

    //! Creates an empty radial grid
    //! @param[in]  params  create params for this grid portion. See CreateParams
    //! @return             Radial grid
    GRIDELEMENTS_EXPORT static RadialGridPtr CreateAndInsert (CreateParams const& params);

    //! Creates radial grid with a specified number of surfaces in axes, with default parameters
    //! @param[in]  params          radial grid parameters containing information about the grid. For more info look up CreateParams
    //! @param[in]  radialSurfaceCount   count of surfaces to create on radial axis
    //! @param[in]  circumferentialSurfaceCount   count of surfaces to create on circular axis
    //! @return                     grid with gridsurfaces in submodel
    GRIDELEMENTS_EXPORT static RadialGridPtr CreateAndInsertWithSurfaces(CreateParams const& params, int radialSurfaceCount, int circumferentialSurfaceCount);

    //---------------------------------------------------------------------------------------
    // getters/setters
    //---------------------------------------------------------------------------------------
    //! Gets X axis
    //! @return DefaultCoordinateIncrementX of this OrthogonalGrid
    GRIDELEMENTS_EXPORT RadialAxisCPtr        GetRadialAxis() const;

    //! Gets Y axis
    //! @return DefaultCoordinateIncrementX of this OrthogonalGrid
    GRIDELEMENTS_EXPORT CircularAxisCPtr      GetCircularAxis() const;

    //! Gets default angle increment of this RadialGrid
    //! @return DefaultAngleIncrement of this RadialGrid
    GRIDELEMENTS_EXPORT double      GetDefaultAngleIncrement() const { return GetPropertyValueDouble(prop_DefaultAngleIncrement()); }

    //! Sets default angle increment of this RadialGrid
    //! @param[in]  angleInc   new DefaultAngleIncrement for this RadialGrid
    GRIDELEMENTS_EXPORT void        SetDefaultAngleIncrement(double angleInc) { SetPropertyValue(prop_DefaultAngleIncrement(), angleInc); };

    //! Gets default radius increment of this RadialGrid
    //! @return DefaultRadiusIncrement of this RadialGrid
    GRIDELEMENTS_EXPORT double      GetDefaultRadiusIncrement() const { return GetPropertyValueDouble(prop_DefaultRadiusIncrement()); }

    //! Sets default radius increment of this RadialGrid
    //! @param[in]  radiusInc   new DefaultRadiusIncrement for this RadialGrid
    GRIDELEMENTS_EXPORT void        SetDefaultRadiusIncrement(double radiusInc) { SetPropertyValue(prop_DefaultRadiusIncrement(), radiusInc); };

    //! Gets default start angle of this RadialGrid
    //! @return DefaultStartAngle of this RadialGrid
    GRIDELEMENTS_EXPORT double      GetDefaultStartAngle () const { return GetPropertyValueDouble (prop_DefaultStartAngle ()); }

    //! Sets default start angle of this RadialGrid
    //! @param[in]  staAngle   new DefaultStartAngle for this RadialGrid
    GRIDELEMENTS_EXPORT void        SetDefaultStartAngle (double staAngle) { SetPropertyValue (prop_DefaultStartAngle (), staAngle); };

    //! Gets default end angle of this RadialGrid
    //! @return DefaultEndAngle of this RadialGrid
    GRIDELEMENTS_EXPORT double      GetDefaultEndAngle () const { return GetPropertyValueDouble (prop_DefaultEndAngle ()); }

    //! Sets default end angle of this RadialGrid
    //! @param[in]  endAngle   new DefaultEndAngle for this RadialGrid
    GRIDELEMENTS_EXPORT void        SetDefaultEndAngle (double endAngle) { SetPropertyValue (prop_DefaultEndAngle (), endAngle); };

    //! Gets default start radius of this RadialGrid
    //! @return DefaultStartRadius of this RadialGrid
    GRIDELEMENTS_EXPORT double      GetDefaultStartRadius () const { return GetPropertyValueDouble (prop_DefaultStartRadius ()); }

    //! Sets default start radius of this RadialGrid
    //! @param[in]  staRadius   new DefaultStartRadius for this RadialGrid
    GRIDELEMENTS_EXPORT void        SetDefaultStartRadius (double staRadius) { SetPropertyValue (prop_DefaultStartRadius (), staRadius); };

    //! Gets default end radius of this RadialGrid
    //! @return DefaultEndRadius of this RadialGrid
    GRIDELEMENTS_EXPORT double      GetDefaultEndRadius () const { return GetPropertyValueDouble (prop_DefaultEndRadius ()); }

    //! Sets default end radius of this RadialGrid
    //! @param[in]  endRadius   new DefaultEndRadius for this RadialGrid
    GRIDELEMENTS_EXPORT void        SetDefaultEndRadius (double endRadius) { SetPropertyValue (prop_DefaultEndRadius (), endRadius); };
};

END_GRIDS_NAMESPACE