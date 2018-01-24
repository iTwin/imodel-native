/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/LineStringMetesAndBoundsPlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct LineStringMetesAndBoundsPlacementStrategy : public LineStringPlacementStrategy
    {
    DEFINE_T_SUPER(LineStringPlacementStrategy)

    private:
        DPlane3d m_workingPlane;

        MetesAndBounds m_metesAndBounds;

    private:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT LineStringMetesAndBoundsPlacementStrategy();

        bvector<DPoint3d> CalculateKeyPoints() const;

    protected:
        virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) override;
        virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;

        virtual bool _IsComplete() const override;
        virtual bool _CanAcceptMorePoints() const override;

        virtual void _SetProperty(Utf8CP key, DPlane3dCR value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, DPlane3dR value) const override;

        virtual void _SetProperty(Utf8CP key, GeometryManipulationStrategyProperty const& value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, GeometryManipulationStrategyProperty& value) const override;

        virtual void _OnPropertySet(Utf8CP key) override;

    public:
        static LineStringMetesAndBoundsPlacementStrategyPtr Create() { return new LineStringMetesAndBoundsPlacementStrategy(); }

        static constexpr Utf8CP prop_WorkingPlane() { return "WorkingPlane"; }
        static constexpr Utf8CP prop_MetesAndBounds() { return "MetesAndBounds"; }
    };

END_BUILDING_SHARED_NAMESPACE