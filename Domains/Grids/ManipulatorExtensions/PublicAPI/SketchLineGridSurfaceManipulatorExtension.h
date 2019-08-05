/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               04/2018
//=======================================================================================
struct SketchLineGridSurfaceManipulatorExtension : Dgn::IEditManipulatorExtension
    {
    private:
        Dgn::IEditManipulatorPtr GetIEditManipulator(Dgn::DgnElementCR element);

    public:
        virtual Dgn::IEditManipulatorPtr _GetIEditManipulator(Dgn::GeometrySourceCR gelm) override;
        virtual Dgn::IEditManipulatorPtr _GetIEditManipulator(Dgn::HitDetailCR hit) override;

        HANDLER_EXTENSION_DECLARE_MEMBERS(SketchLineGridSurfaceManipulatorExtension, GRIDSMANIPULATORS_EXPORT);

        GRIDSMANIPULATORS_EXPORT static void RegisterDragManipulatorExtensions();
    };

END_GRIDS_NAMESPACE