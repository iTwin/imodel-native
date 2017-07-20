/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralPhysicalSchema/PublicAPI/StructuralPhysical/StructuralPhysicalModel.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "StructuralPhysicalDefinitions.h"


BEGIN_BENTLEY_STRUCTURAL_PHYSICAL_NAMESPACE

//=======================================================================================
//! The PhysicalModel that only contains StructuralPhysicalElements
//=======================================================================================
struct StructuralPhysicalModel : Dgn::PhysicalModel
    {
    // Declare StructuralPhysicalModelHandler as a friend so that it can access the protected StructuralPhysicalModel constructor
    friend struct StructuralPhysicalModelHandler;

    // The DGNMODEL_DECLARE_MEMBERS macro automates some required configuration:
    // Parameter 1) SP_CLASS_StructuralPhysicalModel - the name of the ECEntityClass in the ecschema.xml file
    // Parameter 2) Dgn::PhysicalModel - the superclass of StructuralPhysicalModel
    DGNMODEL_DECLARE_MEMBERS(SP_CLASS_StructuralPhysicalModel, Dgn::PhysicalModel);

    protected:
        //! StructuralPhysicalModel wants custom behavior when elements are inserted
        STRUCTURAL_PHYSICAL_EXPORT virtual Dgn::DgnDbStatus _OnInsertElement(Dgn::DgnElementR) override;

        //! StructuralPhysicalModel constructor used by StructuralPhysicalModelHandler
        explicit StructuralPhysicalModel(CreateParams const& params) : T_Super(params) {}

    public:
        //! Uses to create (in memory) a new StructuralPhysicalModel
        STRUCTURAL_PHYSICAL_EXPORT static StructuralPhysicalModelPtr Create(Dgn::PhysicalPartitionCR);
    };


//=======================================================================================
//! The ModelHandler for StructuralPhysicalModel
//=======================================================================================
struct StructuralPhysicalModelHandler : Dgn::dgn_ModelHandler::Physical
    {
    // The MODELHANDLER_DECLARE_MEMBERS macro automates some required configuration:
    // Parameter 1) SP_CLASS_StructuralPhysicalModel - the name of the ECEntityClass in the ecschema.xml file
    // Parameter 2) StructuralPhysicalModel - the DgnModel subclass which provides the custom behavior for the ECEntityClass
    // Parameter 3) StructuralPhysicalModelHandler - the factory class that will create instances of StructuralPhysicalModel
    // Parameter 4) Dgn::dgn_ModelHandler::Physical - the superclass of StructuralPhysicalModelHandler
    MODELHANDLER_DECLARE_MEMBERS(SP_CLASS_StructuralPhysicalModel, StructuralPhysicalModel, StructuralPhysicalModelHandler, Dgn::dgn_ModelHandler::Physical, STRUCTURAL_PHYSICAL_EXPORT)
    };


END_BENTLEY_STRUCTURAL_PHYSICAL_NAMESPACE

