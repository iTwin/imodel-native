/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

#define GMS_V_SET_PROPERTY_TYPE(value_type) \
    virtual void _SetProperty(Utf8CP key, value_type const& value) {}

#define GMS_V_TRYGET_PROPERTY_TYPE(value_type) \
    virtual BentleyStatus _TryGetProperty(Utf8CP key, value_type& value) const { return BentleyStatus::ERROR; }

#define GMS_V_SET_TRYGET_PROPERTY_TYPE(value_type) \
    GMS_V_SET_PROPERTY_TYPE(value_type) \
    GMS_V_TRYGET_PROPERTY_TYPE(value_type)

#define GMS_SET_PROPERTY_TYPE(value_type) \
    GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void SetProperty(Utf8CP key, value_type const& value);

#define GMS_TRYGET_PROPERTY_TYPE(value_type) \
    GEOMETRYMANIPULATIONSTRATEGIES_EXPORT BentleyStatus TryGetProperty(Utf8CP key, value_type& value) const;

#define GMS_SET_TRYGET_PROPERTY_TYPE(value_type) \
    GMS_SET_PROPERTY_TYPE(value_type) \
    GMS_TRYGET_PROPERTY_TYPE(value_type)

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               04/2018
//=======================================================================================
struct DynamicStateBase : RefCountedBase
    {
    protected:
        DynamicStateBase() {}
        virtual ~DynamicStateBase() {}

        virtual DynamicStateBasePtr _Clone() const = 0;

    public:
        DynamicStateBasePtr Clone() const;
    };

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               04/2018
//=======================================================================================
struct BooleanDynamicState : DynamicStateBase
    {
    private:
        bool m_state;

        BooleanDynamicState(bool state) : m_state(state) {}

    protected:
        DynamicStateBasePtr _Clone() const override { return Create(m_state); }

    public:
        static BooleanDynamicStatePtr Create(bool state) { return new BooleanDynamicState(state); }
        bool GetState() const { return m_state; }
    };

//=======================================================================================
// Interface that is used by the ScopedDynamicKeyPointResetter
// for restoring dynamic key points.
//
// @bsiclass                                     Mindaugas.Butkus               04/2018
//=======================================================================================
struct IResettableDynamic
    {
    protected:
        virtual ~IResettableDynamic() {}

        virtual DynamicStateBaseCPtr _GetDynamicState() const = 0;
        virtual void _SetDynamicState(DynamicStateBaseCR state) = 0;

    public:
        DynamicStateBaseCPtr GetDynamicState() const;
        void SetDynamicState(DynamicStateBaseCR state);
    };

//=======================================================================================
// A base class for custom property types to use with SetProperty and TryGetProperty
// methods of GeometryManipulationStrategyBase.
//
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct GeometryManipulationStrategyProperty
    {
    protected:
        GeometryManipulationStrategyProperty() {}
        virtual ~GeometryManipulationStrategyProperty() {}

        friend struct GeometryManipulationStrategyBase;
    };

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct GeometryManipulationStrategyBase : RefCountedBase, IResettableDynamic
    {
    private:
        bvector<Utf8String> m_registeredBoolProperties;
        bvector<Utf8String> m_registeredIntProperties;
        bvector<Utf8String> m_registeredDoubleProperties;
        bvector<Utf8String> m_registeredDPoint2dProperties;
        bvector<Utf8String> m_registeredDVec3dProperties;
        bvector<Utf8String> m_registeredDPlane3dProperties;
        bvector<Utf8String> m_registeredRotMatrixProperties;
        bvector<Utf8String> m_registeredUtf8StringProperties;
        bvector<Utf8String> m_registeredDoubleVecProperties;
        bvector<Utf8String> m_registeredUtf8StringVecProperties;
        bvector<Utf8String> m_registeredCustomProperties;

        GeometryManipulationStrategyBase() {}

        friend struct GeometryManipulationStrategy;
        friend struct GeometryPlacementStrategy;

        void RegisterProperty(Utf8StringCR propertyName, bvector<Utf8String>& allProperties);
        void UnregisterProperty(Utf8StringCR propertyName, bvector<Utf8String>& allProperties);

        template <typename T> static void CopyPropertiesTo(GeometryManipulationStrategyBaseCR from, GeometryManipulationStrategyBaseR to, bvector<Utf8String> const& propertyNames);

    protected:
        virtual void _OnPropertySet(Utf8CP key) {}

        GMS_V_SET_TRYGET_PROPERTY_TYPE(bool)
        GMS_V_SET_TRYGET_PROPERTY_TYPE(int)
        GMS_V_SET_TRYGET_PROPERTY_TYPE(double)
        GMS_V_SET_TRYGET_PROPERTY_TYPE(DPoint2d)
        GMS_V_SET_TRYGET_PROPERTY_TYPE(DVec3d)
        GMS_V_SET_TRYGET_PROPERTY_TYPE(DPlane3d)
        GMS_V_SET_TRYGET_PROPERTY_TYPE(RotMatrix)
        GMS_V_SET_TRYGET_PROPERTY_TYPE(Utf8String)
        GMS_V_SET_TRYGET_PROPERTY_TYPE(bvector<double>)
        GMS_V_SET_TRYGET_PROPERTY_TYPE(bvector<Utf8String>)
        GMS_V_SET_TRYGET_PROPERTY_TYPE(GeometryManipulationStrategyProperty)

        virtual bvector<DPoint3d> _GetKeyPoints() const = 0;

        virtual bool _IsDynamicKeyPointSet() const = 0;
        virtual void _ResetDynamicKeyPoint() = 0;

        virtual bool _IsComplete() const = 0;
        virtual bool _CanAcceptMorePoints() const = 0;

        virtual IGeometryPtr _FinishGeometry() const = 0;
        virtual bvector<ConstructionGeometry> _FinishConstructionGeometry() const = 0;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _CopyPropertiesTo(GeometryManipulationStrategyBaseR) const;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void RegisterBoolProperty(Utf8StringCR propertyName);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void RegisterIntProperty(Utf8StringCR propertyName);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void RegisterDoubleProperty(Utf8StringCR propertyName);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void RegisterDPoint2dProperty(Utf8StringCR propertyName);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void RegisterDVec3dProperty(Utf8StringCR propertyName);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void RegisterDPlane3dProperty(Utf8StringCR propertyName);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void RegisterRotMatrixProperty(Utf8StringCR propertyName);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void RegisterUtf8StringProperty(Utf8StringCR propertyName);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void RegisterDoubleVecProperty(Utf8StringCR propertyName);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void RegisterUtf8StringVecProperty(Utf8StringCR propertyName);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void RegisterCustomProperty(Utf8StringCR propertyName);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void UnregisterProperty(Utf8StringCR propertyName);
    public:
        //! Set/TryGet a bool property.
        GMS_SET_TRYGET_PROPERTY_TYPE(bool)
        //! Set/TryGet an int property.
        GMS_SET_TRYGET_PROPERTY_TYPE(int)
        //! Set/TryGet a double property.
        GMS_SET_TRYGET_PROPERTY_TYPE(double)
        //! Set/TryGet a DPoint2d property.
        GMS_SET_TRYGET_PROPERTY_TYPE(DPoint2d)
        //! Set/TryGet a DVec3d property.
        GMS_SET_TRYGET_PROPERTY_TYPE(DVec3d)
        //! Set/TryGet a DPlane3d property.
        GMS_SET_TRYGET_PROPERTY_TYPE(DPlane3d)
        //! Set/TryGet a RotMatrix property.
        GMS_SET_TRYGET_PROPERTY_TYPE(RotMatrix)
        //! Set/TryGet a Utf8String property.
        GMS_SET_TRYGET_PROPERTY_TYPE(Utf8String)
        //! Set/TryGet a bvector<double> property.
        GMS_SET_TRYGET_PROPERTY_TYPE(bvector<double>)
        //! Set/TryGet a bvector<Utf8String> property.
        GMS_SET_TRYGET_PROPERTY_TYPE(bvector<Utf8String>)
        //! Set/TryGet a custom type property.
        GMS_SET_TRYGET_PROPERTY_TYPE(GeometryManipulationStrategyProperty)

        //! Copy registered properties from another strategy.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void CopyPropertiesTo(GeometryManipulationStrategyBaseR) const;

        //! Retrieve added key points. If dynamic key point is set - it contains that dynamic key point.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bvector<DPoint3d> GetKeyPoints() const;

        //! Check if the dynamic key point is set.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bool IsDynamicKeyPointSet() const;
        //! Restores the state of key point collection to the one before setting a dynamic key point.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void ResetDynamicKeyPoint();

        //! Check if geometry can be created from added key points.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bool IsComplete() const;
        //! Check if more key points can be added.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bool CanAcceptMorePoints() const;

        //! Create geometry from added key points.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT IGeometryPtr FinishGeometry() const;

        //! Create constructions geometry.
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bvector<ConstructionGeometry> FinishConstructionGeometry() const;
    };

END_BUILDING_SHARED_NAMESPACE