/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ArcManipulationStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ArcManipulationStrategy)

BEGIN_BUILDING_SHARED_NAMESPACE

#define KEY_POINT_ACCESSOR_DECL(name, index) \
    DPoint3dCR Get##name() const; \
    GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bool Is##name##Set() const; \
    void Set##name(DPoint3dCR newValue); \
    void Reset##name(); \
    void SetDynamic##name(DPoint3dCR newValue); \
    bool Is##name##Dynamic() const;

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct ArcManipulationStrategy : public EllipseManipulationStrategy
    {
    DEFINE_T_SUPER(EllipseManipulationStrategy)

    private:
        static const size_t s_startIndex = 0;
        static const size_t s_midPointIndex = 1;
        static const size_t s_endIndex = 2;
        static const size_t s_centerIndex = 3;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ArcManipulationStrategy();

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual ICurvePrimitivePtr _FinishPrimitive() const override;

        virtual void _OnKeyPointsChanged() override;

        virtual bool _IsComplete() const override;

    public:
        static ArcManipulationStrategyPtr Create() { return new ArcManipulationStrategy(); }

        KEY_POINT_ACCESSOR_DECL(Start, s_startIndex)
        KEY_POINT_ACCESSOR_DECL(Center, s_centerIndex)
        KEY_POINT_ACCESSOR_DECL(Mid, s_midPointIndex)
        KEY_POINT_ACCESSOR_DECL(End, s_endIndex)
    };

END_BUILDING_SHARED_NAMESPACE