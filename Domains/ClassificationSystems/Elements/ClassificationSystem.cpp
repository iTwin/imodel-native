/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/ClassificationSystem.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/Classification.h"
#include "PublicApi/ClassificationGroup.h"
#include "PublicApi/ClassificationSystem.h"

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

namespace BS = BENTLEY_BUILDING_SHARED_NAMESPACE_NAME;

DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationSystem)


//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              04/2018
//---------------------------------------------------------------------------------------
Dgn::DgnCode ClassificationSystem::GetSystemCode
(
    Dgn::DgnDbR db,
    Utf8CP name
)
    {
    return Dgn::DgnCode(db.CodeSpecs().QueryCodeSpecId(CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystem), db.Elements().GetRootSubjectId(), name);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationSystem::ClassificationSystem
(
    CreateParams const& params,
    Utf8CP name
) : T_Super(params)
    {
        Dgn::DgnCode code = GetSystemCode(params.m_dgndb, name);
        SetCode(code);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aurimas.Laureckis               06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassificationSystem::_SerializeProperties
(
    Json::Value& elementData
) const
    {
    elementData["name"] = GetName();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Aurimas.Laureckis              06/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::ElementIterator ClassificationSystem::MakeIterator(Dgn::DgnDbR dgnDbR)
    {
    return dgnDbR.Elements().MakeIterator(CLASSIFICATIONSYSTEMS_SCHEMA(CLASSIFICATIONSYSTEMS_CLASS_ClassificationSystem));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aurimas.Laureckis               06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassificationSystem::_FormatSerializedProperties
(
    Json::Value& elementData
) const
    {
    elementData["name"] = GetName();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ClassificationSystemPtr ClassificationSystem::Create
(
    Dgn::DgnDbR db,
    Utf8CP name
)
    {
    Dgn::DgnClassId classId = QueryClassId(db);
    Dgn::DgnElement::CreateParams params(db, BS::BuildingUtils::GetOrCreateDefinitionModel(db, "ClassificationSystems")->GetModelId(), classId);
    ClassificationSystemPtr system = new ClassificationSystem(params, name);
    return system;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              04/2018
//---------------------------------------------------------------------------------------
ClassificationSystemCPtr ClassificationSystem::TryGet
(
    Dgn::DgnDbR db,
    Utf8CP name
)
    {
    Dgn::DgnCode code = GetSystemCode(db, name);
    Dgn::DgnElementId id = db.Elements().QueryElementIdByCode(code);
    if (id.IsValid())
        {
        ClassificationSystemCPtr system = db.Elements().Get<ClassificationSystem>(id);
        return system;
        }
    else {
        return nullptr;
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Martynas.Saulius              05/2018
//---------------------------------------------------------------------------------------
Utf8CP ClassificationSystem::GetName() const
    {
    return GetCode().GetValueUtf8CP();
    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE
