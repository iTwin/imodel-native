/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridAxis.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridAxis : Dgn::GroupInformationElement
{
    DEFINE_T_SUPER(Dgn::GroupInformationElement);

    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(GridAxis::T_Super::CreateParams);
        Dgn::DgnElementId m_gridId;

        //! Creates create parameters for orthogonal grid
        //! @param[in] model              model for the PlanCartesianGridSurface
        CreateParams(Dgn::DgnModelCR model, Dgn::DgnClassId classId, Dgn::DgnElementId gridId) :
            T_Super::CreateParams(model.GetDgnDb(), model.GetModelId(), classId), m_gridId(gridId)
            {}

        //! Constructor from base params. Chiefly for internal use.
        //! @param[in]      params   The base element parameters
        //! @return 
        explicit CreateParams(Dgn::DgnElement::CreateParams const& params)
            : T_Super(params), m_gridId(Dgn::DgnElementId())
            {}
        };

protected:
    explicit GRIDELEMENTS_EXPORT GridAxis (CreateParams const& params);
    friend struct GridAxisHandler;

    BE_PROP_NAME(Grid)
    BE_PROP_NAME(Name)

    virtual Dgn::DgnDbStatus _Validate () const;

    //! Called when an element is about to be inserted into the DgnDb.
    //! @return DgnDbStatus::Success to allow the insert, otherwise it will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnInsert, forwarding its status.
    virtual Dgn::DgnDbStatus _OnInsert () override;

    //! Called when this element is about to be replace its original element in the DgnDb.
    //! @param [in] original the original state of this element.
    //! Subclasses may override this method to control whether their instances are updated.
    //! @return DgnDbStatus::Success to allow the update, otherwise the update will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnUpdate, forwarding its status.
    virtual Dgn::DgnDbStatus _OnUpdate (Dgn::DgnElementCR original) override;

    virtual void _SetName(Utf8CP name) { SetPropertyValue(prop_Name(), name); SetUserLabel(name); };

    virtual bool _IsOrthogonalAxisX() const { return false; }
    virtual bool _IsOrthogonalAxisY() const { return false; }
    virtual bool _IsCircularAxis() const { return false; }
    virtual bool _IsRadialAxis() const { return false; }
    virtual bool _IsGeneralGridAxis() const { return false; }

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridAxis, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Getters and setters
    //---------------------------------------------------------------------------------------
    //! Sets gridAxis grid Id value
    //! @param gridId a value to set
    void SetGridId (Dgn::DgnElementId gridId) { SetPropertyValue (prop_Grid (), gridId, GetDgnDb().Schemas().GetClassId(GRIDS_SCHEMA_NAME, GRIDS_REL_GridHasAxes)); };

    //! Gets grid's that contains this axis id
    //! @return element id of the grid
    Dgn::DgnElementId GetGridId () const { return GetPropertyValueId<Dgn::DgnElementId> (prop_Grid ()); };

    //! Gets name of this gridAxis
    //! @return name of this gridAxis
    GRIDELEMENTS_EXPORT Utf8String  GetName() const { return GetPropertyValueString(prop_Name()); }

    //! Sets name of this gridAxis
    //! @param[in]  name    new name for this gridAxis
    GRIDELEMENTS_EXPORT void        SetName(Utf8CP name) { _SetName(name); };

    //! returns true if this is orthogonalAxisX
    //! @return true if this is orthogonalAxisX
    GRIDELEMENTS_EXPORT bool        IsOrthogonalAxisX() const { return _IsOrthogonalAxisX(); }

    //! returns true if this is OrthogonalAxisY
    //! @return true if this is OrthogonalAxisY
    GRIDELEMENTS_EXPORT bool        IsOrthogonalAxisY() const { return _IsOrthogonalAxisY(); }

    //! returns true if this is CircularAxis
    //! @return true if this is CircularAxis
    GRIDELEMENTS_EXPORT bool        IsCircularAxis() const { return _IsCircularAxis(); }

    //! returns true if this is RadialAxis
    //! @return true if this is RadialAxis
    GRIDELEMENTS_EXPORT bool        IsRadialAxis() const { return _IsRadialAxis(); }

    //! returns true if this is GeneralGridAxis
    //! @return true if this is GeneralGridAxis
    GRIDELEMENTS_EXPORT bool        IsGeneralGridAxis() const { return _IsGeneralGridAxis(); }

    //---------------------------------------------------------------------------------------
    // Queries
    //---------------------------------------------------------------------------------------
    //! Make an iterator over gridSurfaces that compose this GridAxis
    //! @returns an iterator over grid surfaces that this grid axis contains
    GRIDELEMENTS_EXPORT Dgn::ElementIterator MakeIterator () const;
};

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE OrthogonalAxisX : GridAxis
    {
    DGNELEMENT_DECLARE_MEMBERS(GRIDS_CLASS_OrthogonalAxisX, GridAxis);
    DEFINE_T_SUPER(GridAxis);

    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(OrthogonalAxisX::T_Super::CreateParams);

        //! Creates create parameters for orthogonal grid
        //! @param[in] model              model for the PlanCartesianGridSurface
        CreateParams(Dgn::DgnModelCR model, OrthogonalGridCR grid) :
            T_Super::CreateParams(model, QueryClassId(model.GetDgnDb()), grid.GetElementId())
            {
            }

        //! Constructor from base params. Chiefly for internal use.
        //! @param[in]      params   The base element parameters
        //! @return 
        explicit GRIDELEMENTS_EXPORT CreateParams(Dgn::DgnElement::CreateParams const& params)
            : T_Super(params)
            {
            }
        };

    private:

    protected:
        friend struct OrthogonalAxisXHandler;

        explicit GRIDELEMENTS_EXPORT OrthogonalAxisX(CreateParams const& params);

        virtual bool _IsOrthogonalAxisX() const { return true; }
    public:
        DECLARE_GRIDS_ELEMENT_BASE_METHODS(OrthogonalAxisX, GRIDELEMENTS_EXPORT)

        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates an empty grid axis
        //! @param[in]  model   model for the axis
        //! @param[in]  grid    grid this axis belongs to
        //! @return             sketch grid
        GRIDELEMENTS_EXPORT static OrthogonalAxisXPtr Create(Dgn::DgnModelCR model, OrthogonalGridCR grid);

        //! Creates and inserts an empty grid axis
        //! @param[in]  model   model for the axis
        //! @param[in]  grid    grid this axis belongs to
        //! @return             sketch grid
        GRIDELEMENTS_EXPORT static OrthogonalAxisXPtr CreateAndInsert(Dgn::DgnModelCR model, OrthogonalGridCR grid);
    };

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE OrthogonalAxisY : GridAxis
    {
    DGNELEMENT_DECLARE_MEMBERS(GRIDS_CLASS_OrthogonalAxisY, GridAxis);

    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(OrthogonalAxisY::T_Super::CreateParams);

        //! Creates create parameters for orthogonal grid
        //! @param[in] model              model for the PlanCartesianGridSurface
        CreateParams(Dgn::DgnModelCR model, OrthogonalGridCR grid) :
            T_Super::CreateParams(model, QueryClassId(model.GetDgnDb()), grid.GetElementId())
            {}

        //! Constructor from base params. Chiefly for internal use.
        //! @param[in]      params   The base element parameters
        //! @return 
        explicit GRIDELEMENTS_EXPORT CreateParams(Dgn::DgnElement::CreateParams const& params)
            : T_Super(params)
            {}
        };

    private:

    protected:
        friend struct OrthogonalAxisYHandler;

        explicit GRIDELEMENTS_EXPORT OrthogonalAxisY(CreateParams const& params);

        virtual bool _IsOrthogonalAxisY() const { return true; }
    public:
        DECLARE_GRIDS_ELEMENT_BASE_METHODS(OrthogonalAxisY, GRIDELEMENTS_EXPORT)

        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates an empty grid axis
        //! @param[in]  model   model for the axis
        //! @param[in]  grid    grid this axis belongs to
        //! @return             sketch grid
        GRIDELEMENTS_EXPORT static OrthogonalAxisYPtr Create(Dgn::DgnModelCR model, OrthogonalGridCR grid);

        //! Creates and inserts an empty grid axis
        //! @param[in]  model   model for the axis
        //! @param[in]  grid    grid this axis belongs to
        //! @return             sketch grid
        GRIDELEMENTS_EXPORT static OrthogonalAxisYPtr CreateAndInsert(Dgn::DgnModelCR model, OrthogonalGridCR grid);
    };

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CircularAxis : GridAxis
    {
    DGNELEMENT_DECLARE_MEMBERS(GRIDS_CLASS_CircularAxis, GridAxis);

    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(CircularAxis::T_Super::CreateParams);

        //! Creates create parameters for orthogonal grid
        //! @param[in] model              model for the PlanCartesianGridSurface
        CreateParams(Dgn::DgnModelCR model, RadialGridCR grid) :
            T_Super::CreateParams(model, QueryClassId(model.GetDgnDb()), grid.GetElementId())
            {}

        //! Constructor from base params. Chiefly for internal use.
        //! @param[in]      params   The base element parameters
        //! @return 
        explicit GRIDELEMENTS_EXPORT CreateParams(Dgn::DgnElement::CreateParams const& params)
            : T_Super(params)
            {}
        };

    private:

    protected:
        DECLARE_GRIDS_ELEMENT_BASE_METHODS(CircularAxis, GRIDELEMENTS_EXPORT)
        friend struct CircularAxisHandler;

        explicit GRIDELEMENTS_EXPORT CircularAxis(CreateParams const& params);

        virtual bool _IsCircularAxis() const { return true; }
    public:

        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates an empty grid axis
        //! @param[in]  model   model for the axis
        //! @param[in]  grid    grid this axis belongs to
        //! @return             sketch grid
        GRIDELEMENTS_EXPORT static CircularAxisPtr Create(Dgn::DgnModelCR model, RadialGridCR grid);

        //! Creates and inserts an empty grid axis
        //! @param[in]  model   model for the axis
        //! @param[in]  grid    grid this axis belongs to
        //! @return             sketch grid
        GRIDELEMENTS_EXPORT static CircularAxisPtr CreateAndInsert(Dgn::DgnModelCR model, RadialGridCR grid);
    };

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RadialAxis : GridAxis
    {
    DGNELEMENT_DECLARE_MEMBERS(GRIDS_CLASS_RadialAxis, GridAxis);

    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(RadialAxis::T_Super::CreateParams);

        //! Creates create parameters for orthogonal grid
        //! @param[in] model              model for the PlanCartesianGridSurface
        CreateParams(Dgn::DgnModelCR model, RadialGridCR grid) :
            T_Super::CreateParams(model, QueryClassId(model.GetDgnDb()), grid.GetElementId())
            {}

        //! Constructor from base params. Chiefly for internal use.
        //! @param[in]      params   The base element parameters
        //! @return 
        explicit GRIDELEMENTS_EXPORT CreateParams(Dgn::DgnElement::CreateParams const& params)
            : T_Super(params)
            {}
        };

    private:

    protected:
        DECLARE_GRIDS_ELEMENT_BASE_METHODS(RadialAxis, GRIDELEMENTS_EXPORT)
        friend struct RadialAxisHandler;

        explicit GRIDELEMENTS_EXPORT RadialAxis(CreateParams const& params);

        virtual bool _IsRadialAxis() const { return true; }

    public:

        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates an empty grid axis
        //! @param[in]  model   model for the axis
        //! @param[in]  grid    grid this axis belongs to
        //! @return             sketch grid
        GRIDELEMENTS_EXPORT static RadialAxisPtr Create(Dgn::DgnModelCR model, RadialGridCR grid);

        //! Creates and inserts an empty grid axis
        //! @param[in]  model   model for the axis
        //! @param[in]  grid    grid this axis belongs to
        //! @return             sketch grid
        GRIDELEMENTS_EXPORT static RadialAxisPtr CreateAndInsert(Dgn::DgnModelCR model, RadialGridCR grid);
    };

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeneralGridAxis : GridAxis
    {
    DGNELEMENT_DECLARE_MEMBERS(GRIDS_CLASS_GeneralGridAxis, GridAxis);

    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(GeneralGridAxis::T_Super::CreateParams);

        //! Creates create parameters for orthogonal grid
        //! @param[in] model              model for the PlanCartesianGridSurface
        CreateParams(Dgn::DgnModelCR model, GridCR grid) :
            T_Super::CreateParams(model, QueryClassId(model.GetDgnDb()), grid.GetElementId())
            {}

        //! Constructor from base params. Chiefly for internal use.
        //! @param[in]      params   The base element parameters
        //! @return 
        explicit GRIDELEMENTS_EXPORT CreateParams(Dgn::DgnElement::CreateParams const& params)
            : T_Super(params)
            {}
        };

    private:

    protected:
        DECLARE_GRIDS_ELEMENT_BASE_METHODS(GeneralGridAxis, GRIDELEMENTS_EXPORT)
        friend struct GeneralGridAxisHandler;

        explicit GRIDELEMENTS_EXPORT GeneralGridAxis(CreateParams const& params);

        virtual bool _IsGeneralGridAxis() const { return true; }

    public:

        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates an empty grid axis
        //! @param[in]  model   model for the axis
        //! @param[in]  grid    grid this axis belongs to
        //! @return             sketch grid
        GRIDELEMENTS_EXPORT static GeneralGridAxisPtr Create(Dgn::DgnModelCR model, GridCR grid);

        //! Creates and inserts an empty grid axis
        //! @param[in]  model   model for the axis
        //! @param[in]  grid    grid this axis belongs to
        //! @return             sketch grid
        GRIDELEMENTS_EXPORT static GeneralGridAxisPtr CreateAndInsert(Dgn::DgnModelCR model, GridCR grid);
    };
END_GRIDS_NAMESPACE