/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridPortion.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Heapzone.h>
#include <DgnPlatform/Render.h>
#include <DgnPlatform/ClipPrimitive.h>
#include <DgnPlatform/DgnElement.h>
#include "SurfaceSet.h"
#include "GridSurface.h"


GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS (GridPortion)

BEGIN_GRIDS_NAMESPACE

typedef bvector<Grids::GridSurfacePtr> GridElementVector;
typedef bmap<Utf8String, GridElementVector> GridAxisMap;
typedef bmap<Utf8String, GridAxisMap> GridNameMap;

#define DEFAULT_AXIS ""
#define HORIZONTAL_AXIS "AxisX"
#define VERTICAL_AXIS "AxisY"

#define GRIDLINE_LENGTH_COEFF 0.1

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridPortion : SurfaceSet
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_GridPortion, SurfaceSet);
private:
    BE_PROP_NAME (Normal);

protected:
    explicit GRIDELEMENTS_EXPORT GridPortion (T_Super::CreateParams const& params);
    friend struct GridPortionHandler;

    static  GRIDELEMENTS_EXPORT CreateParams        CreateParamsFromModel (Dgn::DgnModelCR model, Dgn::DgnClassId classId);

    //! Inserts all grid elements in given map
    //! @param[in] elements a grid axis map containing grid elements
    //! @return             BentleyStatus::SUCCESS if no error has occured while inserting elements
    GRIDELEMENTS_EXPORT static BentleyStatus InsertGridMapElements(GridAxisMap elements);

    GRIDELEMENTS_EXPORT static GridPortionPtr Create(Dgn::DgnModelCR model);
public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridPortion, GRIDELEMENTS_EXPORT)

    //! Translates grid to given point
    //! @param[in] grid     grid to translate
    //! @param[in] point    point to translate to
    GRIDELEMENTS_EXPORT static void TranslateToPoint(GridElementVector& grid, DPoint3d point);

    //! Rotates grid to given angle
    //! @param[in] grid     grid to rotate
    //! @param[in] theta    angle to rotate to
    GRIDELEMENTS_EXPORT static void RotateToAngleXY(GridElementVector& grid, double theta);

    //! gets the perpendicularity plane of this Grid
    //! @return             perpendicularity plane of this Grid
    GRIDELEMENTS_EXPORT DPlane3d    GetPlane ();

    
    };

END_GRIDS_NAMESPACE