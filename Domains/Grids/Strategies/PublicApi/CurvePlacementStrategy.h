/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/PublicApi/CurvePlacementStrategy.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(CurvePlacementStrategy)

BEGIN_GRIDS_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE ICurvePlacementStrategy
    {
    protected:
        // TODO move to base strategy
        virtual BentleyStatus _SetPropertyValueBoolean   (Utf8CP propertyName, const bool              & value) = 0;
        virtual BentleyStatus _SetPropertyValueInteger   (Utf8CP propertyName, const int               & value) = 0;
        virtual BentleyStatus _SetPropertyValueLong      (Utf8CP propertyName, const long              & value) = 0;
        virtual BentleyStatus _SetPropertyValueDouble    (Utf8CP propertyName, const double            & value) = 0;
        virtual BentleyStatus _SetPropertyValueBinary    (Utf8CP propertyName, const Byte *            & value) = 0;
        virtual BentleyStatus _SetPropertyValueUtf8      (Utf8CP propertyName, const Utf8CP            & value) = 0;
        virtual BentleyStatus _SetPropertyValueUtf16     (Utf8CP propertyName, const Utf16CP           & value) = 0;
        virtual BentleyStatus _SetPropertyValueWChar     (Utf8CP propertyName, const WCharCP           & value) = 0;
        virtual BentleyStatus _SetPropertyValuePoint2d   (Utf8CP propertyName, const DPoint2d          & value) = 0;
        virtual BentleyStatus _SetPropertyValuePoint3d   (Utf8CP propertyName, const DPoint3d          & value) = 0;
        virtual BentleyStatus _SetPropertyValueElementId (Utf8CP propertyName, const Dgn::DgnElementId & value) = 0;

        // TODO move to base strategy
        virtual BentleyStatus _GetPropertyValueBoolean   (Utf8CP propertyName, bool              & value) const = 0;
        virtual BentleyStatus _GetPropertyValueInteger   (Utf8CP propertyName, int               & value) const = 0;
        virtual BentleyStatus _GetPropertyValueLong      (Utf8CP propertyName, long              & value) const = 0;
        virtual BentleyStatus _GetPropertyValueDouble    (Utf8CP propertyName, double            & value) const = 0;
        virtual BentleyStatus _GetPropertyValueBinary    (Utf8CP propertyName, Byte *            & value) const = 0;
        virtual BentleyStatus _GetPropertyValueUtf8      (Utf8CP propertyName, Utf8CP            & value) const = 0;
        virtual BentleyStatus _GetPropertyValueUtf16     (Utf8CP propertyName, Utf16CP           & value) const = 0;
        virtual BentleyStatus _GetPropertyValueWChar     (Utf8CP propertyName, WCharCP           & value) const = 0;
        virtual BentleyStatus _GetPropertyValuePoint2d   (Utf8CP propertyName, DPoint2d          & value) const = 0;
        virtual BentleyStatus _GetPropertyValuePoint3d   (Utf8CP propertyName, DPoint3d          & value) const = 0;
        virtual BentleyStatus _GetPropertyValueElementId (Utf8CP propertyName, Dgn::DgnElementId & value) const = 0;

        virtual void _Reset() = 0;

        virtual ICurvePrimitivePtr  _GetCurvePrimitive() = 0;
        virtual ICurvePrimitivePtr  _Finish() = 0;
    };

struct EXPORT_VTABLE_ATTRIBUTE CurvePlacementStrategy : public ICurvePlacementStrategy, RefCountedBase
    {
    bvector<DPoint3d> m_points;
    DPoint3d m_dynamicPoint;
    bool m_useDynamicPoint = false;

    protected:
        // TODO move to base strategy
        virtual BentleyStatus _SetPropertyValueBoolean   (Utf8CP propertyName, const bool              & value) override {return BentleyStatus::ERROR;}
        virtual BentleyStatus _SetPropertyValueInteger   (Utf8CP propertyName, const int               & value) override {return BentleyStatus::ERROR;}
        virtual BentleyStatus _SetPropertyValueLong      (Utf8CP propertyName, const long              & value) override {return BentleyStatus::ERROR;}
        virtual BentleyStatus _SetPropertyValueDouble    (Utf8CP propertyName, const double            & value) override {return BentleyStatus::ERROR;}
        virtual BentleyStatus _SetPropertyValueBinary    (Utf8CP propertyName, const Byte *            & value) override {return BentleyStatus::ERROR;}
        virtual BentleyStatus _SetPropertyValueUtf8      (Utf8CP propertyName, const Utf8CP            & value) override {return BentleyStatus::ERROR;}
        virtual BentleyStatus _SetPropertyValueUtf16     (Utf8CP propertyName, const Utf16CP           & value) override {return BentleyStatus::ERROR;}
        virtual BentleyStatus _SetPropertyValueWChar     (Utf8CP propertyName, const WCharCP           & value) override {return BentleyStatus::ERROR;}
        virtual BentleyStatus _SetPropertyValuePoint2d   (Utf8CP propertyName, const DPoint2d          & value) override {return BentleyStatus::ERROR;}
        virtual BentleyStatus _SetPropertyValuePoint3d   (Utf8CP propertyName, const DPoint3d          & value) override {return BentleyStatus::ERROR;}
        virtual BentleyStatus _SetPropertyValueElementId (Utf8CP propertyName, const Dgn::DgnElementId & value) override {return BentleyStatus::ERROR;}
        
        // TODO move to base strategy
        virtual BentleyStatus _GetPropertyValueBoolean   (Utf8CP propertyName, bool              & value) const override {return BentleyStatus::ERROR;}
        virtual BentleyStatus _GetPropertyValueInteger   (Utf8CP propertyName, int               & value) const override {return BentleyStatus::ERROR;}
        virtual BentleyStatus _GetPropertyValueLong      (Utf8CP propertyName, long              & value) const override {return BentleyStatus::ERROR;}
        virtual BentleyStatus _GetPropertyValueDouble    (Utf8CP propertyName, double            & value) const override {return BentleyStatus::ERROR;}
        virtual BentleyStatus _GetPropertyValueBinary    (Utf8CP propertyName, Byte *            & value) const override {return BentleyStatus::ERROR;}
        virtual BentleyStatus _GetPropertyValueUtf8      (Utf8CP propertyName, Utf8CP            & value) const override {return BentleyStatus::ERROR;}
        virtual BentleyStatus _GetPropertyValueUtf16     (Utf8CP propertyName, Utf16CP           & value) const override {return BentleyStatus::ERROR;}
        virtual BentleyStatus _GetPropertyValueWChar     (Utf8CP propertyName, WCharCP           & value) const override {return BentleyStatus::ERROR;}
        virtual BentleyStatus _GetPropertyValuePoint2d   (Utf8CP propertyName, DPoint2d          & value) const override {return BentleyStatus::ERROR;}
        virtual BentleyStatus _GetPropertyValuePoint3d   (Utf8CP propertyName, DPoint3d          & value) const override {return BentleyStatus::ERROR;}
        virtual BentleyStatus _GetPropertyValueElementId (Utf8CP propertyName, Dgn::DgnElementId & value) const override {return BentleyStatus::ERROR;}

        virtual BentleyStatus _AddPoint(DPoint3d point)         { m_points.push_back(point); return BentleyStatus::SUCCESS; }
        virtual void          _SetDynamicPoint(DPoint3d point)  { m_dynamicPoint = point; m_useDynamicPoint = true; }
        virtual void          _UnsetDynamicPoint()              { m_useDynamicPoint = false; }
        virtual void          _AcceptDynamicPoint()             { m_points.push_back(m_dynamic_point); UnsetDynamicPoint(); }
        virtual bool          _IsInDynamics() const             { return m_useDynamicPoint; }

        virtual void                _Reset() override   { m_points = {}; UnsetDynamicPoint(); }
        virtual ICurvePrimitivePtr  _Finish()           { ICurvePrimitivePtr finalCurve = GetCurvePrimitive(); Reset(); return finalCurve; }
    public:
        // TODO move to base strategy
        //! Sets a boolean property with given value. If given property is not found or is not a boolean value, returns BentleyStatus::ERROR
        //! @param[in]  propertyName    name of property to set
        //! @param[in]  value           value to set
        //! @return BentleyStatus::ERROR if given property is not found or its type does not match given value type
        GRIDSTRATEGIES_EXPORT BentleyStatus SetPropertyValueBoolean   (Utf8CP propertyName, const bool              & value) {return _SetPropertyValueBoolean(propertyName, value);}
        
        //! Sets an integer property with given value. If given property is not found or is not an integer value, returns BentleyStatus::ERROR
        //! @param[in]  propertyName    name of property to set
        //! @param[in]  value           value to set
        //! @return BentleyStatus::ERROR if given property is not found or its type does not match given value type
        GRIDSTRATEGIES_EXPORT BentleyStatus SetPropertyValueInteger   (Utf8CP propertyName, const int               & value) {return _SetPropertyValueInteger(propertyName, value);}
        
        //! Sets a long property with given value. If given property is not found or is not a long value, returns BentleyStatus::ERROR
        //! @param[in]  propertyName    name of property to set
        //! @param[in]  value           value to set
        //! @return BentleyStatus::ERROR if given property is not found or its type does not match given value type
        GRIDSTRATEGIES_EXPORT BentleyStatus SetPropertyValueLong      (Utf8CP propertyName, const long              & value) {return _SetPropertyValueLong(propertyName, value);}
        
        //! Sets a double property with given value. If given property is not found or is not a double value, returns BentleyStatus::ERROR
        //! @param[in]  propertyName    name of property to set
        //! @param[in]  value           value to set
        //! @return BentleyStatus::ERROR if given property is not found or its type does not match given value type
        GRIDSTRATEGIES_EXPORT BentleyStatus SetPropertyValueDouble    (Utf8CP propertyName, const double            & value) {return _SetPropertyValueDouble(propertyName, value);}
        
        //! Sets a binary property with given value. If given property is not found or is not a binary value, returns BentleyStatus::ERROR
        //! @param[in]  propertyName    name of property to set
        //! @param[in]  value           value to set
        //! @return BentleyStatus::ERROR if given property is not found or its type does not match given value type
        GRIDSTRATEGIES_EXPORT BentleyStatus SetPropertyValueBinary(Utf8CP propertyName, const Byte *            & value) { return _SetPropertyValueBinary(propertyName, value); }

        //! Sets an Utf8 property with given value. If given property is not found or is not an Utf8 value, returns BentleyStatus::ERROR
        //! @param[in]  propertyName    name of property to set
        //! @param[in]  value           value to set
        //! @return BentleyStatus::ERROR if given property is not found or its type does not match given value type
        GRIDSTRATEGIES_EXPORT BentleyStatus SetPropertyValueUtf8      (Utf8CP propertyName, const Utf8CP            & value) {return _SetPropertyValueUtf8(propertyName, value);}
        
        //! Sets an Utf16 property with given value. If given property is not found or is not a Utf16 value, returns BentleyStatus::ERROR
        //! @param[in]  propertyName    name of property to set
        //! @param[in]  value           value to set
        //! @return BentleyStatus::ERROR if given property is not found or its type does not match given value type
        GRIDSTRATEGIES_EXPORT BentleyStatus SetPropertyValueUtf16     (Utf8CP propertyName, const Utf16CP           & value) {return _SetPropertyValueUtf16(propertyName, value);}
        
        //! Sets a WChar property with given value. If given property is not found or is not a WChar value, returns BentleyStatus::ERROR
        //! @param[in]  propertyName    name of property to set
        //! @param[in]  value           value to set
        //! @return BentleyStatus::ERROR if given property is not found or its type does not match given value type
        GRIDSTRATEGIES_EXPORT BentleyStatus SetPropertyValueWChar     (Utf8CP propertyName, const WCharCP           & value) {return _SetPropertyValueWChar(propertyName, value);}
        
        //! Sets a DPoint2d property with given value. If given property is not found or is not a DPoint2d value, returns BentleyStatus::ERROR
        //! @param[in]  propertyName    name of property to set
        //! @param[in]  value           value to set
        //! @return BentleyStatus::ERROR if given property is not found or its type does not match given value type
        GRIDSTRATEGIES_EXPORT BentleyStatus SetPropertyValuePoint2d   (Utf8CP propertyName, const DPoint2d          & value) {return _SetPropertyValuePoint2d(propertyName, value);}
        
        //! Sets a DPoint3d property with given value. If given property is not found or is not a DPoint3d value, returns BentleyStatus::ERROR
        //! @param[in]  propertyName    name of property to set
        //! @param[in]  value           value to set
        //! @return BentleyStatus::ERROR if given property is not found or its type does not match given value type
        GRIDSTRATEGIES_EXPORT BentleyStatus SetPropertyValuePoint3d   (Utf8CP propertyName, const DPoint3d          & value) {return _SetPropertyValuePoint3d(propertyName, value);}
        
        //! Sets a DgnElementId property with given value. If given property is not found or is not a DgnElementId value, returns BentleyStatus::ERROR
        //! @param[in]  propertyName    name of property to set
        //! @param[in]  value           value to set
        //! @return BentleyStatus::ERROR if given property is not found or its type does not match given value type
        GRIDSTRATEGIES_EXPORT BentleyStatus SetPropertyValueElementId (Utf8CP propertyName, const Dgn::DgnElementId & value) {return _SetPropertyValueElementId(propertyName, value);}
        
        // TODO move to base strategy
        //! Gets a boolean property value. If given property is not found or is not a boolean value, returns BentleyStatus::ERROR
        //! @param[in]  propertyName    name of property to get
        //! @param[out] value           returned value
        //! @return BentleyStatus::ERROR if given property is not found or its type does not match given value type
        GRIDSTRATEGIES_EXPORT BentleyStatus GetPropertyValueBoolean   (Utf8CP propertyName, bool              & value) const {return _GetPropertyValueBoolean(propertyName, value);}
        
        //! Gets an integer property value. If given property is not found or is not an integer value, returns BentleyStatus::ERROR
        //! @param[in]  propertyName    name of property to get
        //! @param[out] value           returned value
        //! @return BentleyStatus::ERROR if given property is not found or its type does not match given value type
        GRIDSTRATEGIES_EXPORT BentleyStatus GetPropertyValueInteger   (Utf8CP propertyName, int               & value) const {return _GetPropertyValueInteger(propertyName, value);}
        
        //! Gets a long property value. If given property is not found or is not a long value, returns BentleyStatus::ERROR
        //! @param[in]  propertyName    name of property to get
        //! @param[out] value           returned value
        //! @return BentleyStatus::ERROR if given property is not found or its type does not match given value type
        GRIDSTRATEGIES_EXPORT BentleyStatus GetPropertyValueLong      (Utf8CP propertyName, long              & value) const {return _GetPropertyValueLong(propertyName, value);}
        
        //! Gets a double property value. If given property is not found or is not a double value, returns BentleyStatus::ERROR
        //! @param[in]  propertyName    name of property to get
        //! @param[out] value           returned value
        //! @return BentleyStatus::ERROR if given property is not found or its type does not match given value type
        GRIDSTRATEGIES_EXPORT BentleyStatus GetPropertyValueDouble    (Utf8CP propertyName, double            & value) const {return _GetPropertyValueDouble(propertyName, value);}

        //! Gets a binary property value. If given property is not found or is not a binary value, returns BentleyStatus::ERROR
        //! @param[in]  propertyName    name of property to get
        //! @param[out] value           returned value
        //! @return BentleyStatus::ERROR if given property is not found or its type does not match given value type
        GRIDSTRATEGIES_EXPORT BentleyStatus GetPropertyValueBinary(Utf8CP propertyName, Byte *            & value) const { return _GetPropertyValueBinary(propertyName, value); }

        //! Gets an Utf8 property value. If given property is not found or is not an Utf8 value, returns BentleyStatus::ERROR
        //! @param[in]  propertyName    name of property to get
        //! @param[out] value           returned value
        //! @return BentleyStatus::ERROR if given property is not found or its type does not match given value type
        GRIDSTRATEGIES_EXPORT BentleyStatus GetPropertyValueUtf8      (Utf8CP propertyName, Utf8CP            & value) const {return _GetPropertyValueUtf8(propertyName, value);}
        
        //! Gets an utf16 property value. If given property is not found or is not an utf16 value, returns BentleyStatus::ERROR
        //! @param[in]  propertyName    name of property to get
        //! @param[out] value           returned value
        //! @return BentleyStatus::ERROR if given property is not found or its type does not match given value type
        GRIDSTRATEGIES_EXPORT BentleyStatus GetPropertyValueUtf16     (Utf8CP propertyName, Utf16CP           & value) const {return _GetPropertyValueUtf16(propertyName, value);}
        
        //! Gets a WChar property value. If given property is not found or is not a WChar value, returns BentleyStatus::ERROR
        //! @param[in]  propertyName    name of property to get
        //! @param[out] value           returned value
        //! @return BentleyStatus::ERROR if given property is not found or its type does not match given value type
        GRIDSTRATEGIES_EXPORT BentleyStatus GetPropertyValueWChar     (Utf8CP propertyName, WCharCP           & value) const {return _GetPropertyValueWChar(propertyName, value);}
        
        //! Gets a DPoint2d property value. If given property is not found or is not a DPoint2d value, returns BentleyStatus::ERROR
        //! @param[in]  propertyName    name of property to get
        //! @param[out] value           returned value
        //! @return BentleyStatus::ERROR if given property is not found or its type does not match given value type
        GRIDSTRATEGIES_EXPORT BentleyStatus GetPropertyValuePoint2d   (Utf8CP propertyName, DPoint2d          & value) const {return _GetPropertyValuePoint2d(propertyName, value);}
        
        //! Gets a DPoint3d property value. If given property is not found or is not a DPoint3d value, returns BentleyStatus::ERROR
        //! @param[in]  propertyName    name of property to get
        //! @param[out] value           returned value
        //! @return BentleyStatus::ERROR if given property is not found or its type does not match given value type
        GRIDSTRATEGIES_EXPORT BentleyStatus GetPropertyValuePoint3d   (Utf8CP propertyName, DPoint3d          & value) const {return _GetPropertyValuePoint3d(propertyName, value);}
        
        //! Gets a DgnElementId property value. If given property is not found or is not a DgnElementId value, returns BentleyStatus::ERROR
        //! @param[in]  propertyName    name of property to get
        //! @param[out] value           returned value
        //! @return BentleyStatus::ERROR if given property is not found or its type does not match given value type
        GRIDSTRATEGIES_EXPORT BentleyStatus GetPropertyValueElementId (Utf8CP propertyName, Dgn::DgnElementId & value) const {return _GetPropertyValueElementId(propertyName, value);}

        //! Returns created curve primitive
        //! @return curve primitive created by strategy
        GRIDSTRATEGIES_EXPORT ICurvePrimitivePtr GetCurvePrimitive() { return _GetCurvePrimitive(); }
    
        //! Adds point to point pool
        //! @param[in] point    point to add
        //! @return BentleyStatus::SUCCESS if no error has occured when adding a point
        GRIDSTRATEGIES_EXPORT BentleyStatus AddPoint(DPoint3d point) { return _AddPoint(point); }

        //! Sets dynamic point for the tool
        //! @param[in]  point   point to set as dynamic point
        GRIDSTRATEGIES_EXPORT void SetDynamicPoint(DPoint3d) { _SetDynamicPoint(); }

        //! Discards current dynamic point
        GRIDSTRATEGIES_EXPORT void UnsetDynamicPoint() { _UnsetDynamicPoint(); }

        //! Accepts current dynamic point to the point pool
        GRIDSTRATEGIES_EXPORT void AcceptDynamicPoint() { _AcceptDynamicPoint(); }

        //! Checks if current dynamic point is in use
        GRIDSTRATEGIES_EXPORT bool IsInDynamics() const { return _IsInDynamics(); }

        //! Resets strategy to initial state
        GRIDSTRATEGIES_EXPORT void Reset() { _Reset(); };

        //! Resets strategy to initial state and returns curve created in the last state
        //! @return curve created in the last state
        GRIDSTRATEGIES_EXPORT ICurvePrimitivePtr Finish() { return _Finish(); }
};

END_GRIDS_NAMESPACE