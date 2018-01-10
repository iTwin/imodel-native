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

        bvector<Utf8String> m_directionStrings;
        bvector<double> m_lengths;

    private:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT LineStringMetesAndBoundsPlacementStrategy();

    protected:
        virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) override;

        virtual bool _IsComplete() const override;
        virtual bool _CanAcceptMorePoints() const override;

        virtual void _SetProperty(Utf8CP key, DPlane3dCR value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, DPlane3dR value) const override;

        virtual void _SetProperty(Utf8CP key, bvector<Utf8String> const& value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, bvector<Utf8String>& value) const override;

        virtual void _SetProperty(Utf8CP key, bvector<double> const& value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, bvector<double>& value) const override;

    public:
        static LineStringMetesAndBoundsPlacementStrategyPtr Create() { return new LineStringMetesAndBoundsPlacementStrategy(); }

        static const Utf8CP prop_WorkingPlane;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static const Utf8CP prop_DirectionStrings;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static const Utf8CP prop_Lengths;
    };

END_BUILDING_SHARED_NAMESPACE