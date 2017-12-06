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
#include <Grids/gridsApi.h>

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE OrthogonalGrid : PlanGrid
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_OrthogonalGrid, PlanGrid);
protected:

public:     //public - non-exported

    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER (OrthogonalGrid::T_Super::CreateParams);

        Dgn::SpatialLocationModelCP m_model;    //TODO: remove, instead use modelId of base CreateParams
        bool    m_createDimensions;

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
        //! @param[in] createDimensions             true to create dimensions between planes
        //! @param[in] extendHeight                 true if grid should be extended to both ends in Z axis
        CreateParams (Dgn::SpatialLocationModelCP model, Dgn::DgnElementId modeledElementId, bool createDimensions, Utf8CP name) :
            T_Super (DgnElement::CreateParams (model->GetDgnDb (), model->GetModelId (), QueryClassId (model->GetDgnDb ()), Dgn::DgnCode (model->GetDgnDb ().CodeSpecs ().QueryCodeSpecId (GRIDS_AUTHORITY_Grid), modeledElementId, name))),
            m_model (model),
            m_createDimensions (createDimensions)
            {}

        //! Constructor from base params. Chiefly for internal use.
        //! @param[in]      params   The base element parameters
        //! @return 
        explicit GRIDELEMENTS_EXPORT CreateParams (Dgn::DgnElement::CreateParams const& params)
            : T_Super (params)
            {}
        };

    struct StandardCreateParams : CreateParams
        {
        DEFINE_T_SUPER (OrthogonalGrid::CreateParams);

        int m_horizontalCount;
        int m_verticalCount;
        double m_horizontalInterval;
        double m_verticalInterval;
        double m_length;
        double m_height;
        DVec3d m_horizontalExtendTranslation;
        DVec3d m_verticalExtendTranslation;
        bool m_extendHeight;        //TODO: Remove, needs to be handled by the tools

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
                                    //! @param[in] createDimensions             true to create dimensions between planes
                                    //! @param[in] extendHeight                 true if grid should be extended to both ends in Z axis
        StandardCreateParams (Dgn::SpatialLocationModelCP model, Dgn::DgnElementId modeledElementId, int horizontalCount, int verticalCount, double horizontalInterval, double verticalInterval,
                      double length, double height, DVec3d horizontalExtendTranslation, DVec3d verticalExtendTranslation, bool createDimensions, bool extendHeight, Utf8CP name) :
            T_Super (model, modeledElementId, createDimensions, name),
            m_horizontalCount (horizontalCount),
            m_verticalCount (verticalCount),
            m_horizontalInterval (horizontalInterval),
            m_verticalInterval (verticalInterval),
            m_length (length),
            m_height (height),
            m_horizontalExtendTranslation (horizontalExtendTranslation),
            m_verticalExtendTranslation (verticalExtendTranslation),
            m_extendHeight (extendHeight)
            {}

        //! Constructor from base params. Chiefly for internal use.
        //! @param[in]      params   The base element parameters
        //! @return 
        explicit GRIDELEMENTS_EXPORT StandardCreateParams (Dgn::DgnElement::CreateParams const& params)
            : T_Super (params)
            {}
        };
private:

    BE_PROP_NAME (DefaultStartExtentX)
    BE_PROP_NAME (DefaultEndExtentX)
    BE_PROP_NAME (DefaultStartExtentY)
    BE_PROP_NAME (DefaultEndExtentY)

    static BentleyStatus            ValidateBySurfacesParams (bvector<CurveVectorPtr> const& xSurfaces, bvector<CurveVectorPtr> const& ySurfaces, CreateParams const& params);
    static bool                     AreSurfacesCoplanar (bvector<CurveVectorPtr> const& surfaces);

    //! Creates horizontal and vertical orthogonal grid surfaces and inserts them
    //! @param[in] params               parameters for creating the grid portion
    //! @param[in] model                model to create grid surfaces in
    //! @param[in] horizontalGridAxis   axis for horizontal elements
    //! @param[in] verticalGridAxis     axis for vertical elements
    //! @return                         BentleyStatus::SUCCESS if no error has occured when creating and inserting elements
    static BentleyStatus CreateAndInsertSurfaces(StandardCreateParams params, Dgn::SpatialLocationModelCPtr model, GridAxisPtr horizontalGridAxis, GridAxisPtr verticalGridAxis);

    static BentleyStatus CreateSurfaces(bvector<GridSurfacePtr>& allSurfaces, Dgn::SpatialLocationModelCPtr model, int count, double interval, double rotAngle, double length, double height, bool extendHeight, DVec3d extendTranslation, GridAxisPtr gridAxis, bool isHorizontal);

protected:
    explicit GRIDELEMENTS_EXPORT OrthogonalGrid (T_Super::CreateParams const& params);
    friend struct OrthogonalGridHandler;

    //! Calculates translation for grid planed needed for grid to be orthogonal
    //! @param[in] elementIndex     index of the grid plane
    //! @param[in] interval         distance between grid planes
    //! @param[in] rotationAngle    angle grid is being rotated
    //! @param[in] isHorizontal     true if grid is horizontal
    //! @returns vector as translation for grid
    GRIDELEMENTS_EXPORT static DVec3d FindOrthogonalFormTranslation(int elementIndex, double interval, double rotationAngle, bool isHorizontal);

    //! Adds dimensions between elements
    //! @param[in] element1 first element for dimension 
    //! @param[in] element2 second element for 
    //! @param[in] distance distance between elements
    GRIDELEMENTS_EXPORT static void AddDimensionsToOrthogonalGrid(Grids::GridSurfacePtr element1, Grids::GridSurfacePtr element2, double distance);

    BentleyStatus   CreateCoplanarGridPlanes (bvector<CurveVectorPtr> const& surfaces, GridAxisPtr gridAxis, CreateParams const& params);

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (OrthogonalGrid, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Creation
    //---------------------------------------------------------------------------------------
    //! Creates orthogonal grid
    //! @param[in]  params          grid parameters containing information about the grid. For more info look up CreateParams
    //! @return                     grid with gridsurfaces in submodel
    GRIDELEMENTS_EXPORT static OrthogonalGridPtr CreateAndInsert (StandardCreateParams const& params);

    //! Creates orthogonal grid
    //! @param[in]  xSurfaces   gridsurfaces in the X direction
    //! @param[in]  ySurfaces   gridsurfaces in the Y direction
    //! @param[in]  params      grid parameters containing information about the grid. For more info look up CreateParams
    //! @return     grid with gridsurfaces in submodel
    GRIDELEMENTS_EXPORT static OrthogonalGridPtr CreateAndInsertBySurfaces (bvector<CurveVectorPtr> const& xSurfaces, bvector<CurveVectorPtr> const& ySurfaces, CreateParams const& params);

    //---------------------------------------------------------------------------------------
    // Geometry modification
    //---------------------------------------------------------------------------------------
    //!Rotates orthogonal grid by given angle in radians on XY plane
    //! @param[in] theta            angle to rotate on XY plane
    //! @param[in] updateDimensions true if dimensions are to be updated. Expensive in dynamics because dimensions need are updated in db
    //! @return                     Dgn::RepositoryStatus::Success if no error has occured when rotating the grid
    GRIDELEMENTS_EXPORT Dgn::RepositoryStatus RotateToAngleXY(double theta, bool updateDimensions = false);

    //! Gets Default X start extent of this OrthogonalGrid
    //! @return DefaultStartExtentX of this OrthogonalGrid
    GRIDELEMENTS_EXPORT double      GetDefaultStartExtentX () const { return GetPropertyValueDouble (prop_DefaultStartExtentX ()); }

    //! Sets Default X start extent of this OrthogonalGrid
    //! @param[in]  staExtentX   new DefaultStartExtentX for this OrthogonalGrid
    GRIDELEMENTS_EXPORT void        SetDefaultStartExtentX (double staExtentX) { SetPropertyValue (prop_DefaultStartExtentX (), staExtentX); };

    //! Gets Default X end extent of this OrthogonalGrid
    //! @return DefaultEndExtentX of this OrthogonalGrid
    GRIDELEMENTS_EXPORT double      GetDefaultEndExtentX () const { return GetPropertyValueDouble (prop_DefaultEndExtentX ()); }

    //! Sets Default X end extent of this OrthogonalGrid
    //! @param[in]  endExtentX   new DefaultEndExtentX for this OrthogonalGrid
    GRIDELEMENTS_EXPORT void        SetDefaultEndExtentX (double endExtentX) { SetPropertyValue (prop_DefaultEndExtentX (), endExtentX); };

    //! Gets Default Y start extent of this OrthogonalGrid
    //! @return DefaultStartExtentY of this OrthogonalGrid
    GRIDELEMENTS_EXPORT double      GetDefaultStartExtentY () const { return GetPropertyValueDouble (prop_DefaultStartExtentY ()); }

    //! Sets Default Y start extent of this OrthogonalGrid
    //! @param[in]  staExtentY   new DefaultStartExtentY for this OrthogonalGrid
    GRIDELEMENTS_EXPORT void        SetDefaultStartExtentY (double staExtentY) { SetPropertyValue (prop_DefaultStartExtentY (), staExtentY); };

    //! Gets Default Y end extent of this OrthogonalGrid
    //! @return DefaultEndExtentY of this OrthogonalGrid
    GRIDELEMENTS_EXPORT double      GetDefaultEndExtentY () const { return GetPropertyValueDouble (prop_DefaultEndExtentY ()); }

    //! Sets Default Y end extent of this OrthogonalGrid
    //! @param[in]  endExtentY   new DefaultEndExtentY for this OrthogonalGrid
    GRIDELEMENTS_EXPORT void        SetDefaultEndExtentY (double endExtentY) { SetPropertyValue (prop_DefaultEndExtentY (), endExtentY); };

};

END_GRIDS_NAMESPACE