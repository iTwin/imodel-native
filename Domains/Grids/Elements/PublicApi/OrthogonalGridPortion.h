/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

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

        double      m_defaultCoordinateIncrementX;
        double      m_defaultCoordinateIncrementY;
        double      m_defaultStartExtentX;
        double      m_defaultEndExtentX;
        double      m_defaultStartExtentY;
        double      m_defaultEndExtentY;

        //! Creates create parameters for orthogonal grid
        //! @param[in] model                        model to create the grid in
        //! @param[in] modeledElementId             element id of the modeled element
        //! @param[in] name                         name for this grid
        //! @param[in] defaultCoordIncX             default coordinate increment in X axis
        //! @param[in] defaultCoordIncY             default coordinate increment in Y axis
        //! @param[in] defaultStaExtX               default start extent in X axis
        //! @param[in] defaultEndExtX               default end extent in X axis
        //! @param[in] defaultStaExtY               default start extent in Y axis
        //! @param[in] defaultEndExtY               default end extent in Y axis
        //! @param[in] defaultStaElevation          default start(bottom) elevation for the surfaces
        //! @param[in] defaultEndElevation          default end(top) elevation for the surfaces
        CreateParams (Dgn::SpatialLocationModelCR model, Dgn::DgnElementId modeledElementId, Utf8CP name, double defaultCoordIncX, double defaultCoordIncY, double defaultStaExtX, double defaultEndExtX, double defaultStaExtY, double defaultEndExtY, double defaultStaElevation, double defaultEndElevation) :
            T_Super (model, modeledElementId, name, QueryClassId (model.GetDgnDb ()), defaultStaElevation, defaultEndElevation)
            {
            m_defaultCoordinateIncrementX = defaultCoordIncX;
            m_defaultCoordinateIncrementY = defaultCoordIncY;
            m_defaultStartExtentX = defaultStaExtX;
            m_defaultEndExtentX = defaultEndExtX;
            m_defaultStartExtentY = defaultStaExtY;
            m_defaultEndExtentY = defaultEndExtY;
            }

        //! Constructor from base params. Chiefly for internal use.
        //! @param[in]      params   The base element parameters
        //! @return 
        explicit GRIDELEMENTS_EXPORT CreateParams (Dgn::DgnElement::CreateParams const& params)
            : T_Super (params)
            {
            //initialize with zeros, this is not supposed to be a valid element.
            m_defaultCoordinateIncrementX = m_defaultCoordinateIncrementY = m_defaultStartExtentX =
                m_defaultEndExtentX = m_defaultStartExtentY = m_defaultEndExtentY = 0.0;
            }
        };

private:

    BE_PROP_NAME(DefaultCoordinateIncrementX)
    BE_PROP_NAME(DefaultCoordinateIncrementY)
    BE_PROP_NAME(DefaultStartExtentX)
    BE_PROP_NAME(DefaultEndExtentX)
    BE_PROP_NAME(DefaultStartExtentY)
    BE_PROP_NAME(DefaultEndExtentY)


protected:
    explicit GRIDELEMENTS_EXPORT OrthogonalGrid (CreateParams const& params);
    friend struct OrthogonalGridHandler;

    //! Calculates translation for grid planed needed for grid to be orthogonal
    //! @param[in] elementIndex     index of the grid plane
    //! @param[in] interval         distance between grid planes
    //! @param[in] rotationAngle    angle grid is being rotated
    //! @param[in] isHorizontal     true if grid is horizontal
    //! @returns vector as translation for grid
    GRIDELEMENTS_EXPORT static DVec3d FindOrthogonalFormTranslation(int elementIndex, double interval, double rotationAngle, bool isHorizontal);

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (OrthogonalGrid, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Creation
    //---------------------------------------------------------------------------------------
    //! Creates orthogonal grid
    //! @param[in]  params          orthogonalgrid parameters containing information about the grid. For more info look up CreateParams
    //! @return                     grid with gridsurfaces in submodel
    GRIDELEMENTS_EXPORT static OrthogonalGridPtr Create(CreateParams const& params);

    //! Creates orthogonal grid and inserts in database
    //! @param[in]  params          orthogonalgrid parameters containing information about the grid. For more info look up CreateParams
    //! @return                     grid with gridsurfaces in submodel
    GRIDELEMENTS_EXPORT static OrthogonalGridPtr CreateAndInsert (CreateParams const& params);

    //! Creates orthogonal grid with a specified number of surfaces in axes, with default parameters
    //! @param[in]  params          orthogonalgrid parameters containing information about the grid. For more info look up CreateParams
    //! @param[in]  xSurfaceCount   count of surfaces to create on X axis
    //! @param[in]  ySurfaceCount   count of surfaces to create on Y axis
    //! @return                     grid with gridsurfaces in submodel
    GRIDELEMENTS_EXPORT static OrthogonalGridPtr CreateAndInsertWithSurfaces(CreateParams const& params, int xSurfaceCount, int ySurfaceCount);




    //---------------------------------------------------------------------------------------
    // getters/setters
    //---------------------------------------------------------------------------------------
    //! Gets X axis
    //! @return DefaultCoordinateIncrementX of this OrthogonalGrid
    GRIDELEMENTS_EXPORT OrthogonalAxisXCPtr      GetXAxis() const;

    //! Gets Y axis
    //! @return DefaultCoordinateIncrementX of this OrthogonalGrid
    GRIDELEMENTS_EXPORT OrthogonalAxisYCPtr      GetYAxis() const;

    //! Gets Default X coordinate increment of this OrthogonalGrid
    //! @return DefaultCoordinateIncrementX of this OrthogonalGrid
    GRIDELEMENTS_EXPORT double      GetDefaultCoordinateIncrementX() const { return GetPropertyValueDouble(prop_DefaultCoordinateIncrementX()); }

    //! Sets Default X coordinate increment of this OrthogonalGrid
    //! @param[in]  coordIncX   new DefaultCoordinateIncrementX for this OrthogonalGrid
    GRIDELEMENTS_EXPORT void        SetDefaultCoordinateIncrementX(double coordIncX) { SetPropertyValue(prop_DefaultCoordinateIncrementX(), coordIncX); };

    //! Gets Default Y coordinate increment of this OrthogonalGrid
    //! @return DefaultCoordinateIncrementY of this OrthogonalGrid
    GRIDELEMENTS_EXPORT double      GetDefaultCoordinateIncrementY() const { return GetPropertyValueDouble(prop_DefaultCoordinateIncrementY()); }

    //! Sets Default Y coordinate increment of this OrthogonalGrid
    //! @param[in]  coordIncY   new DefaultCoordinateIncrementY for this OrthogonalGrid
    GRIDELEMENTS_EXPORT void        SetDefaultCoordinateIncrementY(double coordIncY) { SetPropertyValue(prop_DefaultCoordinateIncrementY(), coordIncY); };

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