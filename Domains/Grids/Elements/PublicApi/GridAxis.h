/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridAxis.h $
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
struct EXPORT_VTABLE_ATTRIBUTE GridAxis : Dgn::DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_GridAxis, Dgn::DefinitionElement);
    DEFINE_T_SUPER(Dgn::DefinitionElement);

protected:
    explicit GRIDELEMENTS_EXPORT GridAxis (CreateParams const& params);
    friend struct GridAxisHandler;

    static CreateParams CreateParamsFromModel (Dgn::DgnModelCR model, Dgn::DgnClassId classId);

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

    private:

    protected:
        DECLARE_GRIDS_ELEMENT_BASE_METHODS(OrthogonalAxisX, GRIDELEMENTS_EXPORT)
        friend struct OrthogonalAxisXHandler;

        explicit GRIDELEMENTS_EXPORT OrthogonalAxisX(CreateParams const& params);

    public:

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

    private:

    protected:
        DECLARE_GRIDS_ELEMENT_BASE_METHODS(OrthogonalAxisY, GRIDELEMENTS_EXPORT)
        friend struct OrthogonalAxisYHandler;

        explicit GRIDELEMENTS_EXPORT OrthogonalAxisY(CreateParams const& params);

    public:

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

    private:

    protected:
        DECLARE_GRIDS_ELEMENT_BASE_METHODS(CircularAxis, GRIDELEMENTS_EXPORT)
        friend struct CircularAxisHandler;

        explicit GRIDELEMENTS_EXPORT CircularAxis(CreateParams const& params);

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

    private:

    protected:
        DECLARE_GRIDS_ELEMENT_BASE_METHODS(RadialAxis, GRIDELEMENTS_EXPORT)
        friend struct RadialAxisHandler;

        explicit GRIDELEMENTS_EXPORT RadialAxis(CreateParams const& params);

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

    private:

    protected:
        DECLARE_GRIDS_ELEMENT_BASE_METHODS(GeneralGridAxis, GRIDELEMENTS_EXPORT)
        friend struct GeneralGridAxisHandler;

        explicit GRIDELEMENTS_EXPORT GeneralGridAxis(CreateParams const& params);

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