/*--------------------------------------------------------------------------------------+
|
|     $Source: TypicalSectionPoint.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/TypicalSectionPoint.h>
#include <RoadRailPhysical/TypicalSection.h>
#include <RoadRailPhysical/RoadRailCategory.h>

HANDLER_DEFINE_MEMBERS(TypicalSectionConstraintConstantOffsetHandler)
HANDLER_DEFINE_MEMBERS(TypicalSectionConstraintOffsetHandler)
HANDLER_DEFINE_MEMBERS(TypicalSectionConstraintSourceHandler)
HANDLER_DEFINE_MEMBERS(TypicalSectionConstraintWithOffsetHandler)
HANDLER_DEFINE_MEMBERS(TypicalSectionHorizontalConstraintHandler)
HANDLER_DEFINE_MEMBERS(TypicalSectionOffsetParameterHandler)
HANDLER_DEFINE_MEMBERS(TypicalSectionParameterHandler)
HANDLER_DEFINE_MEMBERS(TypicalSectionPointHandler)
HANDLER_DEFINE_MEMBERS(TypicalSectionPointNameHandler)
HANDLER_DEFINE_MEMBERS(TypicalSectionVerticalConstraintHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionPointName::TypicalSectionPointName(CreateParams const& params): T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecId TypicalSectionPointName::QueryCodeSpecId(DgnDbCR dgndb)
    {
    CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BRRP_CODESPEC_TypicalSectionPointName);
    BeAssert(codeSpecId.IsValid());
    return codeSpecId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode TypicalSectionPointName::CreateCode(DgnModelCR scope, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(BRRP_CODESPEC_TypicalSectionPointName, scope, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionPointNamePtr TypicalSectionPointName::Create(DefinitionModelCR model, Utf8StringCR pointName, Utf8CP userLabel)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, pointName));

    TypicalSectionPointNamePtr retVal(new TypicalSectionPointName(createParams));

    if (userLabel)
        retVal->SetUserLabel(userLabel);

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionPointNameCPtr TypicalSectionPointName::CreateAndInsert(DefinitionModelCR model, Utf8StringCR pointName, Utf8CP userLabel)
    {
    auto ptr = Create(model, pointName, userLabel);
    if (ptr.IsNull())
        return nullptr;

    return ptr->Insert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionPointNameCPtr TypicalSectionPointName::QueryByName(DefinitionModelCR model, Utf8StringCR pointName)
    {
    auto pointId = model.GetDgnDb().Elements().QueryElementIdByCode(CreateCode(model, pointName));
    if (!pointId.IsValid())
        return nullptr;

    return TypicalSectionPointName::Get(model.GetDgnDb(), pointId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionPoint::TypicalSectionPoint(CreateParams const& params, TypicalSectionPointNameCR pointName): T_Super(params)
    {
    SetPointName(&pointName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionPointPtr TypicalSectionPoint::Create(TypicalSectionPortionBreakDownModelCR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), 
        RoadRailCategory::GetTypicalSectionPoint(model.GetDgnDb()), Placement2d(), CreateCode(model));

    return new TypicalSectionPoint(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionPointCPtr TypicalSectionPoint::CreateAndInsert(TypicalSectionPortionBreakDownModelCR model)
    {
    auto ptr = Create(model);
    if (ptr.IsNull())
        return nullptr;

    return ptr->Insert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionPointPtr TypicalSectionPoint::Create(TypicalSectionPortionBreakDownModelCR model, TypicalSectionPointNameCR pointName)
    {
    if (!model.GetModelId().IsValid() || !pointName.GetElementId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), 
        RoadRailCategory::GetTypicalSectionPoint(model.GetDgnDb()), Placement2d(), 
        TypicalSectionPointName::CreateCode(model, pointName.GetCode().GetValueUtf8()));

    return new TypicalSectionPoint(createParams, pointName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionPointCPtr TypicalSectionPoint::CreateAndInsert(TypicalSectionPortionBreakDownModelCR model, TypicalSectionPointNameCR pointName)
    {
    auto ptr = Create(model, pointName);
    if (ptr.IsNull())
        return nullptr;

    return ptr->Insert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecId TypicalSectionPoint::QueryCodeSpecId(DgnDbCR dgndb)
    {
    CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BRRP_CODESPEC_TypicalSectionPoint);
    BeAssert(codeSpecId.IsValid());
    return codeSpecId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode TypicalSectionPoint::CreateCode(TypicalSectionPortionBreakDownModelCR scope)
    {
    return CodeSpec::CreateCode(BRRP_CODESPEC_TypicalSectionPoint, scope, ""); // TODO: Making it sequential
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionParameter::TypicalSectionParameter(CreateParams const& params): T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecId TypicalSectionParameter::QueryCodeSpecId(DgnDbCR dgndb)
    {
    CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BRRP_CODESPEC_TypicalSectionParameter);
    BeAssert(codeSpecId.IsValid());
    return codeSpecId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode TypicalSectionParameter::CreateCode(DefinitionModelCR scope, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(BRRP_CODESPEC_TypicalSectionParameter, scope, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionOffsetParameter::TypicalSectionOffsetParameter(CreateParams const& params, double defaultVal) : T_Super(params)
    {
    SetDefaultValue(defaultVal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionOffsetParameterPtr TypicalSectionOffsetParameter::Create(Dgn::DefinitionModelCR model, Utf8StringCR name, double defaultVal)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, name));

    return new TypicalSectionOffsetParameter(createParams, defaultVal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionConstraintSourceCPtr TypicalSectionConstraintSource::Insert(int priority, DgnDbStatus* stat)
    {
    auto retValCPtr = GetDgnDb().Elements().Insert<TypicalSectionConstraintSource>(*this, stat);
    if (retValCPtr.IsValid())
        {
        auto constraintRelClassCP = GetDgnDb().Schemas().GetClass(BRRP_SCHEMA_NAME, BRRP_REL_TypicalSectionPointConstraint)->GetRelationshipClassCP();
        auto constraintRelEnablerP = constraintRelClassCP->GetDefaultStandaloneEnabler();
        auto constraintRelInstPtr = constraintRelEnablerP->CreateInstance();
        constraintRelInstPtr->SetValue("Priority", ECValue(priority));

        ECInstanceKey constraintKey;
        GetDgnDb().InsertLinkTableRelationship(constraintKey,
            *constraintRelClassCP,
            retValCPtr->GetElementId(), retValCPtr->GetParentId() /*TypicalSectionPointId*/,
            dynamic_cast<IECRelationshipInstanceP>(constraintRelInstPtr.get()));
        }

    return retValCPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionConstraintWithOffset::TypicalSectionConstraintWithOffset(CreateParams const& params) : T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionHorizontalConstraint::TypicalSectionHorizontalConstraint(CreateParams const& params, ITypicalSectionConstraintPointCR pointRef): T_Super(params)
    {
    SetPointRef(&pointRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionHorizontalConstraintPtr TypicalSectionHorizontalConstraint::Create(TypicalSectionPointCR constrainedPoint, ITypicalSectionConstraintPointCR pointRef)
    {
    if (!constrainedPoint.GetModelId().IsValid() || !pointRef.GetConstraintPointId().IsValid())
        return nullptr;

    CreateParams createParams(constrainedPoint.GetDgnDb(), constrainedPoint.GetModelId(), QueryClassId(constrainedPoint.GetDgnDb()));
    createParams.m_parentId = constrainedPoint.GetElementId();
    createParams.m_parentRelClassId = constrainedPoint.GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_TypicalSectionPointOwnsConstraintSource);

    return new TypicalSectionHorizontalConstraint(createParams, pointRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionHorizontalConstraintCPtr TypicalSectionHorizontalConstraint::CreateAndInsert(
    TypicalSectionPointCR constrainedPoint, ITypicalSectionConstraintPointCR pointRef, 
    TypicalSectionConstraintConstantOffsetR offset, int priority)
    {
    // This convenience method assumes offset hasn't been inserted yet.
    if (offset.GetElementId().IsValid())
        return nullptr;

    auto ptr = Create(constrainedPoint, pointRef);
    auto cPtr = ptr->Insert(priority);
    if (cPtr.IsValid())
        {
        offset.SetParentId(cPtr->GetElementId(), 
            constrainedPoint.GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_TypicalSectionConstraintOwnsOffset));
        offset.Insert();
        }

    return cPtr->ToHorizontalConstraint();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionVerticalConstraint::TypicalSectionVerticalConstraint(CreateParams const& params, ITypicalSectionConstraintPointCR pointRef): T_Super(params)
    {
    SetPointRef(&pointRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionVerticalConstraintPtr TypicalSectionVerticalConstraint::Create(TypicalSectionPointCR constrainedPoint, ITypicalSectionConstraintPointCR pointRef)
    {
    if (!constrainedPoint.GetModelId().IsValid() || !pointRef.GetConstraintPointId().IsValid())
        return nullptr;

    CreateParams createParams(constrainedPoint.GetDgnDb(), constrainedPoint.GetModelId(), QueryClassId(constrainedPoint.GetDgnDb()));
    createParams.m_parentId = constrainedPoint.GetElementId();
    createParams.m_parentRelClassId = constrainedPoint.GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_TypicalSectionPointOwnsConstraintSource);

    return new TypicalSectionVerticalConstraint(createParams, pointRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionVerticalConstraintCPtr TypicalSectionVerticalConstraint::CreateAndInsert(
    TypicalSectionPointCR constrainedPoint, ITypicalSectionConstraintPointCR pointRef,
    TypicalSectionConstraintConstantOffsetR offset, int priority)
    {
    // This convenience method assumes offset hasn't been inserted yet.
    if (offset.GetElementId().IsValid())
        return nullptr;

    auto ptr = Create(constrainedPoint, pointRef);
    auto cPtr = ptr->Insert(priority);
    if (cPtr.IsValid())
        {
        offset.SetParentId(cPtr->GetElementId(),
            constrainedPoint.GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_TypicalSectionConstraintOwnsOffset));
        offset.Insert();
        }

    return cPtr->ToVerticalConstraint();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionConstraintOffset::TypicalSectionConstraintOffset(CreateParams const& params) : T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionConstraintConstantOffset::TypicalSectionConstraintConstantOffset(CreateParams const& params, double value) : T_Super(params)
    {
    SetValue(value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionConstraintConstantOffsetPtr TypicalSectionConstraintConstantOffset::Create(TypicalSectionPortionBreakDownModelCR model, double value)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()));

    return new TypicalSectionConstraintConstantOffset(createParams, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionConstraintConstantOffsetCPtr TypicalSectionConstraintConstantOffset::CreateAndInsert(TypicalSectionConstraintWithOffsetCR constraint, double value)
    {
    if (!constraint.GetElementId().IsValid())
        return nullptr;

    auto ptr = Create(*dynamic_cast<TypicalSectionPortionBreakDownModelCP>(constraint.GetModel().get()), value);
    if (ptr.IsNull())
        return nullptr;

    ptr->SetParentId(constraint.GetElementId(),
        constraint.GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_TypicalSectionConstraintOwnsOffset));
    return ptr->Insert();
    }