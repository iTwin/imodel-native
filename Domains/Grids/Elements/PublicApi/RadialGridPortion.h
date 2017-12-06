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

        Dgn::SpatialLocationModelCP m_model;    //TODO: Remove, needs to be handled by the tools
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
        CreateParams(Dgn::SpatialLocationModelCP model, Dgn::DgnElementId modeledElementId, int planeCount, int circularCount, double planeIterationAngle,
                               double circularInterval, double length, double height, Utf8CP name, bool extendHeight = false) :
            T_Super(DgnElement::CreateParams(model->GetDgnDb(), model->GetModelId(), QueryClassId(model->GetDgnDb()), Dgn::DgnCode(model->GetDgnDb().CodeSpecs().QueryCodeSpecId(GRIDS_AUTHORITY_Grid), modeledElementId, name))),
            m_model(model),
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

    BE_PROP_NAME (DefaultStartAngle)
    BE_PROP_NAME (DefaultEndAngle)
    BE_PROP_NAME (DefaultStartRadius)
    BE_PROP_NAME (DefaultEndRadius)

protected:
    explicit GRIDELEMENTS_EXPORT RadialGrid (T_Super::CreateParams const& params);


    static BentleyStatus CreateAndInsertGridSurfaces (CreateParams params, Dgn::SpatialLocationModelCPtr model, GridAxisPtr planeAxis, GridAxisPtr arcAxis);

    friend struct RadialGridHandler;

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (RadialGrid, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Creation
    //---------------------------------------------------------------------------------------
    //! Creates an empty radial grid
    //! @param[in]  model   model for the radialgridportion
    //! @return             Radial grid
    GRIDELEMENTS_EXPORT static RadialGridPtr Create (Dgn::SpatialLocationModelCR model);

    //! Creates an empty radial grid
    //! @param[in]  params  create params for this grid portion. See CreateParams
    //! @return             Radial grid
    GRIDELEMENTS_EXPORT static RadialGridPtr CreateAndInsert (CreateParams params);

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