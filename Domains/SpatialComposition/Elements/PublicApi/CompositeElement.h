#pragma once

#include <SpatialComposition/Domain/SpatialCompositionMacros.h>
#include <DgnPlatform/DgnPlatformApi.h>

BEGIN_SPATIALCOMPOSITION_NAMESPACE

struct CompositeElement : Dgn::SpatialLocationElement
    {
    protected:
        explicit CompositeElement(CreateParams const& params) : Dgn::SpatialLocationElement(params) {}
        BE_PROP_NAME(ComposingElement)

    public:
        //---------------------------------------------------------------------------------------
        // Getters and setters
        //---------------------------------------------------------------------------------------

        //! Gets composing element's that contains this CompositeElement id
        //! @return element id of the composing element
        Dgn::DgnElementId GetComposedElementId () const 
            {
            return GetPropertyValueId<Dgn::DgnElementId> (prop_ComposingElement ());
            }

        //! Sets CompositeElement composingElementId Id value
        //! @param composingElementId a value to set
        void SetComposedElementId (Dgn::DgnElementId composingElementId)
            {
            SetPropertyValue (prop_ComposingElement (), composingElementId, GetDgnDb().Schemas().GetClassId(SPATIALCOMPOSITION_SCHEMA_NAME, SPATIALCOMPOSITION_REL_CompositeComposesSubComposites));
            }

        //! Make an iterator over composing elements
        //! @return iterator over composing elements
        SPATIALCOMPOSITION_EXPORT Dgn::ElementIterator MakeIterator() const;

        //! Make an iterator over overlaped elements
        //! @return iterator over overlaped elements
        SPATIALCOMPOSITION_EXPORT Dgn::ElementIterator MakeOverlapedElementsIterator() const;

        //! Add an overlaped element
        //! @return status
        SPATIALCOMPOSITION_EXPORT Dgn::DgnDbStatus AddOverlapedElement (Dgn::DgnElementId overlapedElementId);

        //! Remove an overlaped element
        //! @return status
        SPATIALCOMPOSITION_EXPORT Dgn::DgnDbStatus RemoveOverlapedElement (Dgn::DgnElementId overlapedElementId);
    };

END_SPATIALCOMPOSITION_NAMESPACE
