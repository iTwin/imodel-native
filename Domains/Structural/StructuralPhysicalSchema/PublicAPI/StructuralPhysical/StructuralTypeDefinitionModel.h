/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralPhysicalSchema/PublicAPI/StructuralPhysical/StructuralTypeDefinitionModel.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "StructuralPhysicalDefinitions.h"


BEGIN_BENTLEY_STRUCTURAL_PHYSICAL_NAMESPACE

//=======================================================================================
//! The DefinitionModel for StructuralTypeDefinitionModel
//=======================================================================================
struct StructuralTypeDefinitionModel : Dgn::DefinitionModel
    {
    // Declare StructuralTypeDefinitionModelHandler as a friend so that it can access the protected StructuralTypeDefinitionModel constructor
    friend struct StructuralTypeDefinitionModelHandler;

    // The DGNMODEL_DECLARE_MEMBERS macro automates some required configuration:
    // Parameter 1) BP_CLASS_StructuralTypeDefinitionModel - the name of the ECEntityClass in the ecschema.xml file
    // Parameter 2) Dgn::DefinitionModel - the superclass of StructuralTypeDefinitionModel
    DGNMODEL_DECLARE_MEMBERS(BP_CLASS_StructuralTypeDefinitionModel, Dgn::DefinitionModel);

    protected:
        //! StructuralTypeDefinitionModel wants custom behavior when elements are inserted
        STRUCTURAL_PHYSICAL_EXPORT virtual Dgn::DgnDbStatus _OnInsertElement(Dgn::DgnElementR) override;

        //! StructuralTypeDefinitionModel constructor used by StructuralTypeDefinitionModelHandler
        explicit StructuralTypeDefinitionModel(CreateParams const& params) : T_Super(params) {}

    public:
        //! Uses to create (in memory) a new StructuralTypeDefinitionModel
        STRUCTURAL_PHYSICAL_EXPORT static StructuralTypeDefinitionModelPtr Create(Dgn::DefinitionPartitionCR);
    };


//=======================================================================================
//! The ModelHandler for StructuralTypeDefinitionModel
//=======================================================================================
struct StructuralTypeDefinitionModelHandler : Dgn::dgn_ModelHandler::Definition
    {
    // The MODELHANDLER_DECLARE_MEMBERS macro automates some required configuration:
    // Parameter 1) BP_CLASS_StructuralTypeDefinitionModel - the name of the ECEntityClass in the ecschema.xml file
    // Parameter 2) StructuralTypeDefinitionModel - the DgnModel subclass which provides the custom behavior for the ECEntityClass
    // Parameter 3) TStructuralTypeDefinitionModelHandler - the factory class that will create instances of StructuralTypeDefinitionModel
    // Parameter 4) Dgn::dgn_ModelHandler::Definition - the superclass of StructuralTypeDefinitionModelHandler
    MODELHANDLER_DECLARE_MEMBERS(BP_CLASS_StructuralTypeDefinitionModel, StructuralTypeDefinitionModel, StructuralTypeDefinitionModelHandler, Dgn::dgn_ModelHandler::Definition, STRUCTURAL_PHYSICAL_EXPORT)
    };


END_BENTLEY_STRUCTURAL_PHYSICAL_NAMESPACE

