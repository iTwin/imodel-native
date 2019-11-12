/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicAPI\StructuralElement.h"
#include "PublicAPI\StructuralPhysicalDefinitions.h"
#include "PublicAPI\StructuralDomainApi.h"

HANDLER_DEFINE_MEMBERS(StructuralMemberHandler)

StructuralMemberPtr StructuralMember::Create(Dgn::PhysicalModelCR model)
    {
    Dgn::DgnModelId modelId = model.GetModelId();

    BeAssert(modelId.IsValid());

    if (!modelId.IsValid())
        {
        return nullptr;
        }

    Dgn::DgnDbR db = model.GetDgnDb();

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

#ifdef _EXCLUDED_FROM_EAP_BUILD_
bool StructuralMember::IsCurveMember()
    {
    bool bRet(false);
    ECN::ECBaseClassesList baseClasses = GetElementClass()->GetBaseClasses();

    for (auto i = baseClasses.begin(); i != baseClasses.end(); ++i)
        {
        ECN::ECClassCP baseClass = *i;

        Utf8String name = baseClass->GetName();

        bRet = (0 == name.CompareTo(STRUCTURAL_PHYSICAL_CLASS_ICURVE_MEMBER));

        if (false != bRet)
            {
            break;
            }
        }

    return bRet;
    }

bool StructuralMember::IsSurfaceMember()
    {
    bool bRet(false);
    ECN::ECBaseClassesList baseClasses = GetElementClass()->GetBaseClasses();

    for (auto i = baseClasses.begin(); i != baseClasses.end(); ++i)
        {
        ECN::ECClassCP baseClass = *i;

        Utf8String name = baseClass->GetName();

        bRet = (0 == name.CompareTo(STRUCTURAL_PHYSICAL_CLASS_ISURFACE_MEMBER));

        if (false != bRet)
            {
            break;
            }
        }

    return bRet;
    }
#endif
