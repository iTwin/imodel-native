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
#include "GridPortion.h"

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS (RadialGridPortion)

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

        Dgn::SpatialLocationModelCP m_model;
        int m_planeCount;
        int m_circularCount;
        double m_planeIterationAngle;
        double m_circularInterval;
        double m_length;
        double m_height;
        bool m_extendHeight;

        //! Creates create parameters for radial grid
        //! @param[in] model                model to create the grid in
        //! @param[in] planeCount           angle between first and last grid planes
        //! @param[in] circularCount        grid arcs count
        //! @param[in] planeIterationAngle  angle between each grid plane
        //! @param[in] circularInterval     distance between grid arcs
        //! @param[in] length               length of grid lines
        //! @param[in] height               height of grid lines
        //! @param[in] extendHeight         true if grid should be extended to both ends in Z axis
        CreateParams(Dgn::SpatialLocationModelCP model, int planeCount, int circularCount, double planeIterationAngle,
                               double circularInterval, double length, double height, bool extendHeight = false) :
            T_Super(Dgn::DgnElement::CreateParams(model->GetDgnDb(), model->GetModelId(), QueryClassId(model->GetDgnDb()))),
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

protected:
    explicit GRIDELEMENTS_EXPORT RadialGridPortion (T_Super::CreateParams const& params);
    friend struct RadialGridPortionHandler;

    GRIDELEMENTS_EXPORT static RadialGridPortionPtr Create(Dgn::SpatialLocationModelCR model);
public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (RadialGridPortion, GRIDELEMENTS_EXPORT)


    //! Creates radial grid and returns it as a map where DEFAULT_AXIS maps to grid elements
    //! @param[in] params   grid parameters containing information about the grid. For more info look up CreateParams
    //! @return             GridAxisMap containing the grid surfaces
    GRIDELEMENTS_EXPORT static GridElementVector CreateGridPreview(CreateParams params);

    //! Creates radial grid and inserts its elements to db
    //! @param[out] grid    created grid elements
    //! @param[in]  params  grid parameters containing information about the grid. For more info look up CreateParams
    //! @return             BentleyStatus::SUCCESS if no error has occured while inserting elements.
    GRIDELEMENTS_EXPORT static BentleyStatus CreateAndInsert(GridAxisMap& grid, CreateParams params);
};

END_GRIDS_NAMESPACE