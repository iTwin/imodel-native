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

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RadialGridPortion : GridPortion
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_RadialGridPortion, GridPortion);

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(RadialGridPortion::T_Super::CreateParams);

        Dgn::SpatialLocationModelCP m_model;    //TODO: Remove, needs to be handled by the tools
        int m_planeCount;
        int m_circularCount;
        double m_planeIterationAngle;
        double m_circularInterval;
        double m_length;
        double m_height;
        DVec3d m_normal;
        bool m_extendHeight;    //TODO: Remove, needs to be handled by the tools

        //! Creates create parameters for radial grid
        //! @param[in] model                model to create the grid in
        //! @param[in] planeCount           angle between first and last grid planes
        //! @param[in] circularCount        grid arcs count
        //! @param[in] planeIterationAngle  angle between each grid plane
        //! @param[in] circularInterval     distance between grid arcs
        //! @param[in] length               length of grid lines
        //! @param[in] height               height of grid lines
        //! @param[in] extendHeight         true if grid should be extended to both ends in Z axis
        CreateParams(Dgn::SpatialLocationModelCP model, Dgn::DgnElementId modeledElementId, int planeCount, int circularCount, double planeIterationAngle,
                               double circularInterval, double length, double height, Utf8CP name, bool extendHeight = false, DVec3d normal = DVec3d::From(0, 0, 1)) :
            T_Super(DgnElement::CreateParams(model->GetDgnDb(), model->GetModelId(), QueryClassId(model->GetDgnDb()), Dgn::DgnCode(model->GetDgnDb().CodeSpecs().QueryCodeSpecId(GRIDS_AUTHORITY_RadialGridPortion), modeledElementId, name))),
            m_model(model),
            m_planeCount(planeCount),
            m_circularCount(circularCount),
            m_planeIterationAngle(planeIterationAngle),
            m_circularInterval(circularInterval),
            m_length(length),
            m_height(height),
            m_extendHeight(extendHeight),
            m_normal(normal)
            {
            }

        //! Constructor from base params. Chiefly for internal use.
        //! @param[in]      params   The base element parameters
        //! @return 
        explicit GRIDELEMENTS_EXPORT CreateParams(Dgn::DgnElement::CreateParams const& params)
            : T_Super(params) { }
        };

protected:
    explicit GRIDELEMENTS_EXPORT RadialGridPortion (T_Super::CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT RadialGridPortion (T_Super::CreateParams const& params, DVec3d normal);


    static GridElementVector CreateGridPreview (CreateParams params, GridAxisPtr planeAxis, GridAxisPtr arcAxis);

    friend struct RadialGridPortionHandler;

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (RadialGridPortion, GRIDELEMENTS_EXPORT)

    //! Creates an empty radial grid
    //! @param[in]  model   model for the radialgridportion
    //! @param[in]  normal  perpendicularity plane of this Grid
    //! @return             Radial grid
    GRIDELEMENTS_EXPORT static RadialGridPortionPtr Create (Dgn::SpatialLocationModelCR model, DVec3d normal);

    //! Creates an empty radial grid
    //! @param[in]  model   model for the radialgridportion
    //! @param[in]  normal  perpendicularity plane of this Grid
    //! @return             Radial grid
    GRIDELEMENTS_EXPORT static RadialGridPortionPtr CreateAndInsert (CreateParams params);

};

END_GRIDS_NAMESPACE