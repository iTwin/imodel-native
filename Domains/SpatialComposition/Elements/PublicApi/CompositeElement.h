/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <SpatialComposition/Domain/SpatialCompositionMacros.h>
#include <DgnPlatform/DgnPlatformApi.h>

SPATIALCOMPOSITION_REFCOUNTED_PTR_AND_TYPEDEFS(CompositeElement)

BEGIN_SPATIALCOMPOSITION_NAMESPACE

struct CompositeElement : Dgn::SpatialLocationElement
    {
    typedef Dgn::SpatialLocationElement T_Super;
    protected:
        explicit CompositeElement(CreateParams const& params) : Dgn::SpatialLocationElement(params) {}
        BE_PROP_NAME(ComposingElement);
        BE_PROP_NAME(FootprintArea);
        Dgn::DgnElementId m_composedElementId;  //cached value from relationship

        void SetFootprintArea(const double area) { dynamic_cast<Dgn::DgnElementP>(this)->SetPropertyValue(prop_FootprintArea(), area); }

        virtual SPATIALCOMPOSITION_EXPORT void CalculateProperties ();
        virtual SPATIALCOMPOSITION_EXPORT Dgn::DgnDbStatus _LoadFromDb ();
        virtual SPATIALCOMPOSITION_EXPORT void _CopyFrom (Dgn::DgnElementCR source) override;
        virtual SPATIALCOMPOSITION_EXPORT Dgn::DgnDbStatus _OnInsert () override;
        virtual SPATIALCOMPOSITION_EXPORT Dgn::DgnDbStatus _OnUpdate (Dgn::DgnElementCR original) override;
        virtual SPATIALCOMPOSITION_EXPORT Dgn::DgnDbStatus _OnDelete() const override;
        virtual SPATIALCOMPOSITION_EXPORT Dgn::Render::GeometryParams _CreateGeometryParameters();

    public:
        //---------------------------------------------------------------------------------------
        // Getters and setters
        //---------------------------------------------------------------------------------------

        //! Gets composing element's that contains this CompositeElement id
        //! @return element id of the composing element
        SPATIALCOMPOSITION_EXPORT Dgn::DgnElementId GetComposedElementId () const 
            {
            return m_composedElementId;
            }

        //! Sets CompositeElement composingElementId Id value
        //! @param composingElementId a value to set
        SPATIALCOMPOSITION_EXPORT void SetComposedElementId (Dgn::DgnElementId composingElementId)
            {
            m_composedElementId = composingElementId;
            if (GetElementId().IsValid()) 
                SetPropertyValue (prop_ComposingElement (), composingElementId, GetDgnDb().Schemas().GetClassId(SPATIALCOMPOSITION_SCHEMA_NAME, SPATIALCOMPOSITION_REL_CompositeComposesSubComposites));
            }

        //! Make an iterator over composing elements
        //! @return iterator over composing elements
        SPATIALCOMPOSITION_EXPORT Dgn::ElementIterator MakeIterator() const;

        //! Make an iterator over elements of the specified ECClass in this DgnDb that compose this element
        //! @param[in] className The <i>full</i> ECClass name of the composing element's class. For example: BUILDING_SPACEPLANNING_SCHEMA(BUILDING_SPACEPLANNING_CLASS_Story) to make iterator over Stories that compose this element.
        //! @return              iterator over elements with given class name
        SPATIALCOMPOSITION_EXPORT Dgn::ElementIterator MakeIterator(Utf8CP className) const;

        //! Make an iterator over overlaped elements
        //! @return iterator over overlaped elements
        SPATIALCOMPOSITION_EXPORT Dgn::ElementIterator MakeOverlapedElementsIterator() const;

        //! Add an overlaped element
        //! @return status
        SPATIALCOMPOSITION_EXPORT Dgn::DgnDbStatus AddOverlapedElement (Dgn::DgnElementId overlapedElementId);

        //! Remove an overlaped element
        //! @return status
        SPATIALCOMPOSITION_EXPORT Dgn::DgnDbStatus RemoveOverlapedElement (Dgn::DgnElementId overlapedElementId);

        //! Get element's footprint area
        //! @return   element's footprint area
        SPATIALCOMPOSITION_EXPORT double GetFootprintArea() const;

        //! Change element's shape with a new curve
        //! @param[in] curveVector             curve vector that will be set as element's new shape
        //! @param[in] updatePlacementOrigin   true if origin of this extrusion should be updated
        //! @return                            true if there were no errors while updating element shape
        virtual SPATIALCOMPOSITION_EXPORT bool SetFootprintShape(CurveVectorCPtr pCurve, bool updatePlacementOrigin = true);

        //! Gets name of this composite element
        //! @return name of this composite element
        SPATIALCOMPOSITION_EXPORT Utf8String GetName() const { return GetUserLabel(); }

        //! Sets name of this composite element
        //! @param[in]  name    new name for this composite element
        SPATIALCOMPOSITION_EXPORT void SetName(Utf8CP name) { SetUserLabel(name); };

        //---------------------------------------------------------------------------------------
        // Queries
        //---------------------------------------------------------------------------------------
        //! Get the model containing conflicts related to this element
        //! @return confilting model
        virtual SPATIALCOMPOSITION_EXPORT Dgn::DgnModelPtr GetConflictsModel() const { return nullptr; }

        //---------------------------------------------------------------------------------------
        // Other
        //---------------------------------------------------------------------------------------
        //! Draws outline for given geometry and graphic builder
        //! @param[in]  curves  geometry to draw outline from
        //! @param[in]  graphic graphic builder to draw outline with
        SPATIALCOMPOSITION_EXPORT static void DrawOutline(CurveVectorCR curves, Dgn::Render::GraphicBuilderR graphic);
    };

END_SPATIALCOMPOSITION_NAMESPACE
