/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralPhysical/StructuralMember.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <StructuralDomain/StructuralPhysical/StructuralMember.h>
#include <StructuralDomain/StructuralCommon/StructuralCommonDefinitions.h>
#include <StructuralDomain/StructuralDomainApi.h>

HANDLER_DEFINE_MEMBERS(StructuralMemberHandler)

StructuralMemberPtr StructuralMember::Create(Structural::StructuralPhysicalModelCPtr model)
{
    Dgn::DgnModelId modelId = model.get()->GetModelId();

    BeAssert(modelId.IsValid());

    if (!modelId.IsValid())
    {
        return nullptr;
    }

    Dgn::DgnDbR db = model.get()->GetDgnDb();

    ECN::ECClassCP structuralClass = db.GetClassLocater().LocateClass(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, StructuralMember::MyHandlerECClassName());

    BeAssert(nullptr != structuralClass);

    if (nullptr == structuralClass)
    {
        return nullptr;
    }

    Dgn::DgnCategoryId categoryId = Structural::StructuralPhysicalCategory::QueryStructuralPhysicalCategoryId(db, structuralClass->GetDisplayLabel().c_str());

    BeAssert(categoryId.IsValid());

    CreateParams createParams(db, modelId, QueryClassId(db), categoryId);

    return new StructuralMember(createParams);
}

StructuralMemberCPtr StructuralMember::Insert(Dgn::DgnDbStatus* insertStatusOut)
{
    Dgn::DgnDbStatus ALLOW_NULL_OUTPUT(insertStatus, insertStatusOut);
    StructuralMemberCPtr structMember = GetDgnDb().Elements().Insert<StructuralMember>(*this, &insertStatus);

    BeAssert(structMember.IsValid());

    return structMember;
}

StructuralMemberCPtr StructuralMember::Update(Dgn::DgnDbStatus* updateStatusOut)
{
    Dgn::DgnDbStatus ALLOW_NULL_OUTPUT(updateStatus, updateStatusOut);
    StructuralMemberCPtr structMember = GetDgnDb().Elements().Update<StructuralMember>(*this, &updateStatus);

    BeAssert(structMember.IsValid());

    return structMember;
}


