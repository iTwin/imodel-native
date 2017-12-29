/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/RadialGridPortion.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Heapzone.h>
#include <DgnPlatform/Render.h>
#include <DgnPlatform/ClipPrimitive.h>
#include <DgnPlatform/DgnElement.h>
#include <Grids/gridsApi.h>

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

        int m_planeCount;
        int m_circularCount;
        double m_planeIterationAngle;
        double m_circularInterval;
        double m_length;
        double m_height;
        bool m_extendHeight;    //TODO: Remove, needs to be handled by the tools

        //! Creates create parameters for radial grid
        //! @param[in] model                model to create the grid in
        //! @param[in] planeCount           grid planes count
        //! @param[in] circularCount        grid arcs count
        //! @param[in] planeIterationAngle  angle between each grid plane in radians
        //! @param[in] circularInterval     distance between grid arcs
        //! @param[in] length               length of grid planes
        //! @param[in] height               height of grid surfaces
        //! @param[in] extendHeight         true if grid should be extended to both ends in Z axis
        CreateParams(Dgn::SpatialLocationModelCR model, Dgn::DgnElementId modeledElementId, int planeCount, int circularCount, double planeIterationAngle,
                               double circularInterval, double length, double height, Utf8CP name, double defaultstaElevation, double defaultendElevation, bool extendHeight = false) :
            T_Super (model, modeledElementId, name, QueryClassId (model.GetDgnDb ()), defaultstaElevation, defaultendElevation),
            m_planeCount(planeCount),
            m_circularCount(circularCount),
            m_planeIterationAngle(planeIterationAngle),
            m_circularInterval(circularInterval),
            m_length(length),
            m_height(height),
            m_extendHeight(extendHeight)
            {
            }

        //! Constructor from base params. Chiefly for internal use.
        //! @param[in]      params   The base element parameters
        //! @return 
        explicit GRIDELEMENTS_EXPORT CreateParams(Dgn::DgnElement::CreateParams const& params)
            : T_Super(params) { }
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