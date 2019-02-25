#pragma once

BEGIN_SPATIALCOMPOSITION_NAMESPACE

struct CompositeElement : Dgn::SpatialLocationElement
    {
    protected:
        explicit CompositeElement(CreateParams const& params) : Dgn::SpatialLocationElement(params) {}
        BE_PROP_NAME(ComposingElement);
        BE_PROP_NAME(FootprintArea);
        Dgn::DgnElementId m_composedElementId;  //cached value from relationship

        void SetFootprintArea(const double area) { dynamic_cast<Dgn::DgnElementP>(this)->SetPropertyValue(prop_FootprintArea(), area); }

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

        SPATIALCOMPOSITION_EXPORT double GetFootprintArea() const { return dynamic_cast<Dgn::DgnElementCP>(this)->GetPropertyValueDouble(prop_FootprintArea()); }
    };

END_SPATIALCOMPOSITION_NAMESPACE
