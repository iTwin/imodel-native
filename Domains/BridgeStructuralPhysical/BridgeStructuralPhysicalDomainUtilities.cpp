/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "BridgeStructuralPhysicalInternal.h"
#include <BridgeStructuralPhysical/BridgeStructuralPhysicalDomainUtilities.h>
#include <BridgeStructuralPhysical/BridgeStructuralPhysicalDomain.h>
//#include <BridgeStructuralPhysical/PhysicalModel.h>
#include <BridgeStructuralPhysical/BridgeCategory.h>

BEGIN_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE
BE_JSON_NAME(BridgeStructuralPhysicalDomain)

//---------------------------------------------------------------------------------------
// @bsimethod                                  Nick.Purcell                  05/2018
//---------------------------------------------------------------------------------------
Utf8String BridgeStructuralPhysicalDomainUtilities::BuildPhysicalModelCode(Utf8StringCR modelCodeName)
    {
    return modelCodeName + ":Physical";
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                  Nick.Purcell                  05/2018
//---------------------------------------------------------------------------------------
PhysicalModelPtr BridgeStructuralPhysicalDomainUtilities::GetPhysicalModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject)
    {
    if (parentSubject.IsNull())
        {
        parentSubject = db.Elements().GetRootSubject();
        }

    Dgn::DgnCode partitionCode = Dgn::PhysicalPartition::CreateCode(*parentSubject, BuildPhysicalModelCode(modelCodeName));
    Dgn::DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
    Dgn::PhysicalPartitionCPtr partition = db.Elements().Get<Dgn::PhysicalPartition>(partitionId);
    if (!partition.IsValid())
        return nullptr;
    return (PhysicalModelP)(partition->GetSubModel().get());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                  Nick.Purcell                  05/2018
//---------------------------------------------------------------------------------------
PhysicalModelPtr BridgeStructuralPhysicalDomainUtilities::CreatePhysicalModel(Utf8StringCR modelCodeName, Dgn::DgnDbR db, Dgn::SubjectCPtr parentSubject)
    {
    if (!parentSubject.IsValid())
        {
        parentSubject = db.Elements().GetRootSubject();
        }

    // Create the partition and the StructuralPhysicalModel.
    Utf8String phyModelCode = BuildPhysicalModelCode(modelCodeName);

    Dgn::PhysicalPartitionCPtr partition = Dgn::PhysicalPartition::CreateAndInsert(*parentSubject, phyModelCode);

    if (!partition.IsValid())
        return nullptr;

    PhysicalModelPtr physicalModel = PhysicalModel::CreateAndInsert(*partition);

    return physicalModel;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Nick.Purcell                  05/2018
//---------------------------------------------------------------------------------------
Dgn::PhysicalElementPtr BridgeStructuralPhysicalDomainUtilities::CreatePhysicalElement(Utf8StringCR schemaName, Utf8StringCR className, PhysicalModelCPtr model, Utf8CP categoryName)
    {
    Dgn::DgnDbR db = model->GetDgnDb();
    Dgn::DgnModelId modelId = model->GetModelId();

    // Find the class

    ECN::ECClassCP structuralClass = db.GetClassLocater().LocateClass(schemaName.c_str(), className.c_str());

    if (nullptr == structuralClass)
        return nullptr;

    ECN::ECClassId classId = structuralClass->GetId();

    Dgn::ElementHandlerP elmHandler = Dgn::dgn_ElementHandler::Element::FindHandler(db, classId);
    if (NULL == elmHandler)
        return nullptr;

    Utf8String localCategoryName = structuralClass->GetDisplayLabel();

    if (nullptr != categoryName)
        localCategoryName = categoryName;

    Dgn::DgnCategoryId categoryId = BridgeCategory::Get(db);

    Dgn::GeometricElement3d::CreateParams params(db, modelId, classId, categoryId);

    Dgn::DgnElementPtr element = elmHandler->Create(params);

    Dgn::PhysicalElementPtr structuralElement = dynamic_pointer_cast<Dgn::PhysicalElement>(element);

    auto geomSource = structuralElement->ToGeometrySourceP();

    if (nullptr == geomSource)
        return nullptr;

    geomSource->SetCategoryId(categoryId);

    return structuralElement;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                  Nick.Purcell                  05/2018
//---------------------------------------------------------------------------------------
StructuralMemberPtr BridgeStructuralPhysicalDomainUtilities::CreateStructuralMember(Utf8StringCR schemaName, Utf8StringCR className,
	PhysicalModelCPtr model, Utf8CP categoryName,
	DgnElementId parentId , DgnClassId parentRelClassId)
	{
	Dgn::DgnDbR db = model->GetDgnDb();
	Dgn::DgnModelId modelId = model->GetModelId();

	// Find the class

	ECN::ECClassCP structuralClass = db.GetClassLocater().LocateClass(schemaName.c_str(), className.c_str());

	if (nullptr == structuralClass)
		return nullptr;

	ECN::ECClassId classId = structuralClass->GetId();

	Dgn::ElementHandlerP elmHandler = Dgn::dgn_ElementHandler::Element::FindHandler(db, classId);
	if (NULL == elmHandler)
		return nullptr;

	Utf8String localCategoryName = structuralClass->GetDisplayLabel();

	if (nullptr != categoryName)
		localCategoryName = categoryName;

	Dgn::DgnCategoryId categoryId = BridgeCategory::Get(db);

	Dgn::GeometricElement3d::CreateParams params(db, modelId, classId, categoryId);
	params.m_parentId = parentId;
	params.m_parentRelClassId = parentRelClassId;
	Dgn::DgnElementPtr element = elmHandler->Create(params);

	StructuralMemberPtr structuralMember = dynamic_pointer_cast<StructuralMember>(element);

	auto geomSource = structuralMember->ToGeometrySourceP();

	if (nullptr == geomSource)
		return nullptr;

	geomSource->SetCategoryId(categoryId);
	return structuralMember;
}

//---------------------------------------------------------------------------------------
// @bsimethod                                  Nick.Purcell                  05/2018
//---------------------------------------------------------------------------------------
PhysicalModelPtr BridgeStructuralPhysicalDomainUtilities::QueryPhysicalModel(SubjectCPtr parentSubject, Utf8StringCR name)
    {
    DgnDbR db = parentSubject->GetDgnDb();
    DgnCode partitionCode = PhysicalPartition::CreateCode(*parentSubject, name);
    DgnModelId modelId = db.Models().QuerySubModelId(partitionCode);
    return db.Models().Get<PhysicalModel>(modelId);
    }

END_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE 

