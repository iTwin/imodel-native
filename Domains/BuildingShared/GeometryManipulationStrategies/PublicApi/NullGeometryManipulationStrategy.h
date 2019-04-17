/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               02/2018
//=======================================================================================
struct NullGeometryManipulationStrategy final : public GeometryManipulationStrategy
    {
    DEFINE_T_SUPER(GeometryManipulationStrategy)

    private:
        struct NullDynamicState : DynamicStateBase
            {
            private:
                NullDynamicState() {}

            protected:
                DynamicStateBasePtr _Clone() const override { return Create(); }

            public:
                static RefCountedPtr<NullDynamicState> Create() { return new NullDynamicState(); }
            };

        NullGeometryManipulationStrategy() : T_Super() {}

    protected:
        virtual void _AppendDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override {}
        virtual void _AppendDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override {}
        virtual void _InsertDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) override {}
        virtual void _InsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override {}
        virtual void _UpdateDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) override {}
        virtual void _UpdateDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override {}
        virtual void _UpsertDynamicKeyPoint(DPoint3d newDynamicKeyPoint, size_t index) override {}
        virtual void _UpsertDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints, size_t index) override {}

        virtual void _AppendKeyPoint(DPoint3dCR newKeyPoint) override {}
        virtual void _AppendKeyPoints(bvector<DPoint3d> const& newKeyPoints) override {}
        virtual void _InsertKeyPoint(DPoint3dCR newKeyPoint, size_t index) override {}
        virtual void _ReplaceKeyPoint(DPoint3dCR newKeyPoint, size_t index) override {}
        virtual void _PopKeyPoint() override {}
        virtual void _RemoveKeyPoint(size_t index) override {}
        virtual void _Clear() override {}

        virtual bvector<DPoint3d> _GetKeyPoints() const override { return bvector<DPoint3d>(); }

        virtual bool _IsDynamicKeyPointSet() const override { return false; }
        virtual void _ResetDynamicKeyPoint() override {}

        virtual bool _IsComplete() const override { return false; }
        virtual bool _CanAcceptMorePoints() const override { return false; }

        virtual IGeometryPtr _FinishGeometry() const override { return nullptr; }
        virtual bvector<ConstructionGeometry> _FinishConstructionGeometry() const override { return bvector<ConstructionGeometry>(); }

        // IRessetableDynamic
        virtual void _SetDynamicState(DynamicStateBaseCR state) override {}
        virtual DynamicStateBaseCPtr _GetDynamicState() const override { return DynamicStateBaseCPtr(NullDynamicState::Create()); }

    public:
        static NullGeometryManipulationStrategyPtr Create() { return new NullGeometryManipulationStrategy(); }
    };

END_BUILDING_SHARED_NAMESPACE