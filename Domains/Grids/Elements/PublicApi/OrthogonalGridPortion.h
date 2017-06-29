/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/OrthogonalGridPortion.h $
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

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS (OrthogonalGridPortion)

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE OrthogonalGridPortion : GridPortion
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_OrthogonalGridPortion, GridPortion);

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(OrthogonalGridPortion::T_Super::CreateParams);

        Dgn::SpatialLocationModelCP m_model;
        int m_horizontalCount;
        int m_verticalCount;
        double m_horizontalInterval;
        double m_verticalInterval;
        double m_length;
        double m_height;
        DVec3d m_horizontalExtendTranslation;
        DVec3d m_verticalExtendTranslation;

        //! Creates create parameters for orthogonal grid
        //! @param[in] model                        model to create the grid in
        //! @param[in] horizontalCount              horizontal lines count
        //! @param[in] verticalCount                vertical lines count
        //! @param[in] horizontalInterval           distance between horizontal lines
        //! @param[in] verticalInterval             distance between vertical lines
        //! @param[in] length                       length of grid lines
        //! @param[in] height                       height of grid lines
        //! @param[in] horizontalExtendTranslation  translation of horizontal lines to be extended
        //! @param[in] verticalExtendTranslation    translation of vertical lines to be extended
        CreateParams(Dgn::SpatialLocationModelCP model, int horizontalCount, int verticalCount, double horizontalInterval, double verticalInterval,
                     double length, double height, DVec3d horizontalExtendTranslation, DVec3d verticalExtendTranslation) :
            T_Super(DgnElement::CreateParams(model->GetDgnDb(), model->GetModelId(), QueryClassId(model->GetDgnDb()))),
            m_model(model),
            m_horizontalCount(horizontalCount),
            m_verticalCount(verticalCount),
            m_horizontalInterval(horizontalInterval),
            m_verticalInterval(verticalInterval),
            m_length(length),
            m_height(height),
            m_horizontalExtendTranslation(horizontalExtendTranslation),
            m_verticalExtendTranslation(verticalExtendTranslation)
            {
            }

        //! Constructor from base params. Chiefly for internal use.
        //! @param[in]      params   The base element parameters
        //! @return 
        explicit GRIDELEMENTS_EXPORT CreateParams(Dgn::DgnElement::CreateParams const& params)
            : T_Super(params) { }
        };

    //! Creates horizontal or vertical orthogonal grid portion
    //! @param[in] params       parameters for creating the grid portion
    //! @param[in] isHorizontal true if horizontal elements should be created, false if vertical
    //! @return                 vector containing horizontal or vertical orthogonal grid elements
    static GridElementVector CreateGridElements(CreateParams params, bool isHorizontal);

protected:
    explicit GRIDELEMENTS_EXPORT OrthogonalGridPortion (T_Super::CreateParams const& params);
    friend struct OrthogonalGridPortionHandler;

    //! Calculates translation for grid planed needed for grid to be orthogonal
    //! @param[in] elementIndex     index of the grid plane
    //! @param[in] interval         distance between grid planes
    //! @param[in] rotationAngle    angle grid is being rotated
    //! @param[in] isHorizontal     true if grid is horizontal
    //! @returns vector as translation for grid
    GRIDELEMENTS_EXPORT static DVec3d FindOrthogonalFormTranslation(int elementIndex, double interval, double rotationAngle, bool isHorizontal);

    //! Adds dimensions between elements
    //! @param[in] element1 first element for dimension 
    //! @param[in] element2 second element for dimension
    //! @param[in] distance distance between elements
    GRIDELEMENTS_EXPORT static void AddDimensionsToOrthogonalGrid(Grids::GridSurfacePtr element1, Grids::GridSurfacePtr element2, double distance);

    //! Creates orthogonal grid axis map from grid element vector
    //! @param[out] axisMap GridAxisMap containing grid surfaces HORIZONTAL_AXIS and VERTICAL_AXIS
    //! @param[in] elements grid element vector containing perpendicular grid surfaces
    //! @return             BentleyStatus::SUCCESS if no error occured while sorting elements
    GRIDELEMENTS_EXPORT static BentleyStatus ElementVectorToAxisMap(GridAxisMap& axisMap, GridElementVector elements);


    GRIDELEMENTS_EXPORT static OrthogonalGridPortionPtr Create(Dgn::SpatialLocationModelCR model);
public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (OrthogonalGridPortion, GRIDELEMENTS_EXPORT)

    //! Creates orthogonal grid and returns it as a map where HORIZONTAL_AXIS maps to horizontal grid planes and VERTICAL_AXIS maps to vertical grid planes
    //! @param[in] params    grid parameters containing information about the grid. For more info look up CreateParams
    //! @return              GridAxisMap containing the grid planes
    GRIDELEMENTS_EXPORT static GridElementVector CreateGridPreview(CreateParams params);

    //! Creates orthogonal grid and inserts its elements and dimensions to db
    //! @param[in] params   grid parameters containing information about the grid. For more info look up CreateParams
    //! @return             BentleyStatus::SUCCESS if no error has occured while inserting elements/dimensions
    GRIDELEMENTS_EXPORT static BentleyStatus CreateAndInsert(CreateParams params);

    //! Rotates orthogonal grid by given angle in radians on XY plane
    //! @param[in] grid             orthogonal grid to rotate
    //! @param[in] theta            angle to rotate on XY plane
    //! @param[in] updateDimensions true if dimensions are to be updated. Expensive in dynamics because dimensions need are updated in db
    GRIDELEMENTS_EXPORT static void RotateToAngleXY(GridAxisMap& grid, double theta, bool updateDimensions = false);

    //! Rotates orthogonal grid by given angle in radians on XY plane
    //! @param[in] grid     orthogonal grid to rotate
    //! @param[in] theta    angle to rotate on XY plane
    GRIDELEMENTS_EXPORT static void RotateToAngleXY(GridElementVector& grid, double theta, bool updateDimensions = false);
};

END_GRIDS_NAMESPACE