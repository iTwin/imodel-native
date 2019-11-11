/*--------------------------------------------------------------------------------------+
|
|     $Source$
|
|  $Copyright$
|
+--------------------------------------------------------------------------------------*/
#include "BridgeStructuralPhysicalInternal.h"
#include <BridgeStructuralPhysical/BridgeStructuralPhysicalDomain.h>
DOMAIN_DEFINE_MEMBERS(BridgeStructuralPhysicalDomain)
#include <BridgeStructuralPhysical/BridgeCategory.h>
#include <BridgeStructuralPhysical/Bridge.h>
#include <BridgeStructuralPhysical/BridgeSuperstructure.h>
#include <BridgeStructuralPhysical/BridgeSubstructure.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nick.Purcell                    05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BridgeStructuralPhysicalDomain::BridgeStructuralPhysicalDomain() : DgnDomain(BBP_SCHEMA_NAME, "Bentley BridgeStructuralPhysical Domain", 1)
    {
    }

void createCodeSpecs(DgnDbR dgndb)
    {
    auto codeSpecPtr = CodeSpec::Create(dgndb, BBP_CODESPEC_Bridge, CodeScopeSpec::CreateModelScope());
    BeAssert(codeSpecPtr.IsValid());
    if (codeSpecPtr.IsValid())
        {
        codeSpecPtr->Insert();
        BeAssert(codeSpecPtr->GetCodeSpecId().IsValid());
        }

    codeSpecPtr = CodeSpec::Create(dgndb, BBP_CODESPEC_StructuralSystem, CodeScopeSpec::CreateModelScope());
    BeAssert(codeSpecPtr.IsValid());
    if (codeSpecPtr.IsValid())
        {
        codeSpecPtr->Insert();
        BeAssert(codeSpecPtr->GetCodeSpecId().IsValid());
        }
    }

Dgn::DgnDbStatus BridgeStructuralPhysicalDomain::SetUpModelHierarchy(BridgeCR bridgeCR)
	{
	if (!bridgeCR.GetElementId().IsValid())
		return DgnDbStatus::InvalidId;

	auto& physicalModelHandlerR = dgn_ModelHandler::Physical::GetHandler();
	auto multiDisciplinaryModelPtr = physicalModelHandlerR.Create(DgnModel::CreateParams(bridgeCR.GetDgnDb(), bridgeCR.GetDgnDb().Domains().GetClassId(physicalModelHandlerR), bridgeCR.GetElementId()));
   

	if (multiDisciplinaryModelPtr.IsNull())
		return DgnDbStatus::InvalidId;

    multiDisciplinaryModelPtr->SetIsPrivate(true);
    multiDisciplinaryModelPtr->Insert();
	PhysicalModelPtr physicalModelPtr;
	StructuralSystemCPtr structSystemCPtr = StructuralSystem::Insert(*multiDisciplinaryModelPtr->ToPhysicalModel(), bridgeCR);

    if(structSystemCPtr.IsNull())
        return DgnDbStatus::InvalidId;

	return DgnDbStatus::Success;
	}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void BridgeStructuralPhysicalDomain::_OnSchemaImported(DgnDbR dgndb) const
    {
    createCodeSpecs(dgndb);
    BridgeCategory::SetUp(dgndb);
    }


StructuralSystemCPtr StructuralSystem::Insert(PhysicalModelCR parentModel, BridgeCR parentBridgeCR)
	{
	GeometricElement3d::CreateParams createParams(parentModel.GetDgnDb(), parentModel.GetModelId(), QueryClassId(parentModel.GetDgnDb()),
		BridgeCategory::Get(parentModel.GetDgnDb()));

    auto bridgeCodeValue = parentBridgeCR.GetCode().GetValueUtf8();
    auto structuralCodeValue = bridgeCodeValue.append("- StructuralSystem");
    DgnCode structuralCode = StructuralSystem::CreateCode(parentModel, structuralCodeValue);
    if(structuralCode.IsValid())
        createParams.m_code = structuralCode;

	StructuralSystemPtr newPtr(new StructuralSystem(*Create(parentModel.GetDgnDb(), createParams)));
	auto structSystemCPtr = parentModel.GetDgnDb().Elements().Insert<PhysicalElement>(*newPtr->getP());
	if (structSystemCPtr.IsNull())
		return nullptr;
	
	auto& physicalModelHandlerR = dgn_ModelHandler::Physical::GetHandler();
	auto newDgnModelPtr = physicalModelHandlerR.Create(DgnModel::CreateParams(parentModel.GetDgnDb(), parentModel.GetDgnDb().Domains().GetClassId(physicalModelHandlerR),
		structSystemCPtr->GetElementId()));

	if (newDgnModelPtr.IsValid())
	    {
        if (DgnDbStatus::Success != newDgnModelPtr->Insert())
            return nullptr;
	    }
    return new StructuralSystem(*structSystemCPtr);
	}

DgnElementId StructuralSystem::Query(PhysicalModelCR physicalModel)
    {
    auto stmtPtr = physicalModel.GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BBP_SCHEMA(BBP_CLASS_StructuralSystem) " WHERE Model.Id = ?");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, physicalModel.GetModelId());
    if (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        return stmtPtr->GetValueId<DgnElementId>(0);

    return DgnElementId();
    }

DgnCode StructuralSystem::CreateCode(PhysicalModelCR scopeModel, Utf8StringCR codeValue)
    {
    return CodeSpec::CreateCode(BBP_CODESPEC_StructuralSystem, scopeModel, codeValue);
    }
