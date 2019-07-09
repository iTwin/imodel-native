/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Grid Label element used for defining a label near a grid curve.
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridLabel : Dgn::InformationRecordElement
    {
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_GridLabel, Dgn::InformationRecordElement);
    DEFINE_T_SUPER (Dgn::InformationRecordElement);

    public:
        struct CreateParams : T_Super::CreateParams
            {
            DEFINE_T_SUPER(GridLabel::T_Super::CreateParams)
            bool m_labelAtStart;
            bool m_labelAtEnd;
            Dgn::DgnElementId m_ownerSurface;

            //! Used for creating `GridLabel` object.
            //! @param[in] createParams Parent element params
            //! @param[in] labelAtStart true if label should appear at start of the `GridCurve`
            //! @param[in] labelAtEnd true if label should appear at end of the `GridCurve`
            //! @param[in] ownerSurface `GridSurface` owning this label
            GRIDELEMENTS_EXPORT CreateParams(DgnElement::CreateParams const& createParams,
                                             Dgn::DgnElementId ownerSurface = Dgn::DgnElementId(),
                                             bool labelAtStart = false,
                                             bool labelAtEnd = false);
            };

    protected:
        explicit GRIDELEMENTS_EXPORT GridLabel (CreateParams const& params);
        friend struct GridLabelHandler;

        virtual Dgn::DgnDbStatus _OnUpdate (Dgn::DgnElementCR original) override;
        virtual Dgn::DgnDbStatus _OnInsert() override;

        BE_PROP_NAME (HasLabelAtStart)
        BE_PROP_NAME (HasLabelAtEnd)
    public:
        DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridLabel, GRIDELEMENTS_EXPORT)
        
        //! Sets labelAtStart property for `GridLabel`
        //! @param labelAtStart a value to set
        GRIDELEMENTS_EXPORT void SetHasLabelAtStart (bool labelAtStart) { SetPropertyValue (prop_HasLabelAtStart(), labelAtStart); }

        //! Gets labelAtStart property for `GridLabel`
        //! @return true if `GridCurve` should have a bubble at start
        GRIDELEMENTS_EXPORT bool HasLabelAtStart() const { return GetPropertyValueBoolean (prop_HasLabelAtStart()); }

        //! Sets labelAtEnd property for `GridLabel`
        //! @param labelAtEnd a value to set
        GRIDELEMENTS_EXPORT void SetHasLabelAtEnd (bool labelAtEnd) { SetPropertyValue (prop_HasLabelAtEnd(), labelAtEnd); }

        //! Gets labelAtEnd property for `GridLabel`
        //! @return true if `GridCurve` should have a bubble at end
        GRIDELEMENTS_EXPORT bool HasLabelAtEnd() const { return GetPropertyValueBoolean (prop_HasLabelAtEnd()); }

        //! Sets label text for this `GridLabel`
        //! @param label a value to set
        GRIDELEMENTS_EXPORT void SetLabel (Utf8CP label) { SetUserLabel( label); }

        //! Gets label property for `GridLabel`
        //! @return label text for this `GridLabel`
        GRIDELEMENTS_EXPORT Utf8String GetLabel () const { return GetUserLabel (); }

        //! Sets owner for this `GridLabel`
        //! @param owner for this `GridLabel`
        GRIDELEMENTS_EXPORT Dgn::DgnDbStatus SetOwnerId (Dgn::DgnElementId owner);

        //! Sets owner for this `GridLabel`
        //! @param owner for this `GridLabel`
        GRIDELEMENTS_EXPORT Dgn::DgnDbStatus SetOwner (GridSurfaceCR owner);

        //! Gets owner of this `GridLabel`
        //! @return id of `GridSurface` owning this `GridLabel`
        GRIDELEMENTS_EXPORT Dgn::DgnElementId GetOwnerId() const { return GetParentId(); }

        //! Gets owner of this `GridLabel`
        //! @return `GridSurface` owning this `GridLabel`
        GRIDELEMENTS_EXPORT GridSurfaceCPtr GetOwner() const { return GetDgnDb().Elements().Get<GridSurface> (GetOwnerId()); }

        //! Creates `GridLabel` instance
        //! @param[in] ownerSurface id of `GridSurface` owning this label
        //! @param[in] label for the `GridCurve`
        //! @param[in] labelAtStart true if label should appear at start of the `GridCurve`
        //! @param[in] labelAtEnd true if label should appear at end of the `GridCurve`
        GRIDELEMENTS_EXPORT static GridLabelPtr Create (Dgn::DgnDbCR db,
                                                        Dgn::DgnElementId ownerSurface = Dgn::DgnElementId(), 
                                                        Utf8CP label = "", 
                                                        bool labelAtStart = false, 
                                                        bool labelAtEnd = false);

        //! Creates `GridLabel` instance
        //! @param[in] ownerSurface `GridSurface` owning this label
        //! @param[in] label for the `GridCurve`
        //! @param[in] labelAtStart true if label should appear at start of the `GridCurve`
        //! @param[in] labelAtEnd true if label should appear at end of the `GridCurve`
        GRIDELEMENTS_EXPORT static GridLabelPtr Create (GridSurfaceCR ownerSurface, 
                                                        Utf8CP label = "", 
                                                        bool labelAtStart = false, 
                                                        bool labelAtEnd = false);
    };

END_GRIDS_NAMESPACE
