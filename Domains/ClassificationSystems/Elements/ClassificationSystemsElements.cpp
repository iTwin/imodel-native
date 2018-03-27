/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/ClassificationSystemsElements.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ClassificationSystems/ClassificationSystemsApi.h>


BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(ClassificationSystemClassDefinition)
DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(CIBSEClassDefinition)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CIBSEClassDefinition::CIBSEClassDefinition
(
    CreateParams const& params,
    Utf8CP name,
    Utf8CP Category
) : T_Super(params)
    {
        SetName(name);
        SetCategory(Category);  
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CIBSEClassDefinitionPtr CIBSEClassDefinition::Create
(
    Dgn::DgnDbR db,
    Utf8CP name,
    Utf8CP Category
)
    {
    Dgn::DgnClassId classId = QueryClassId(db);
    Dgn::DgnElement::CreateParams params(
            db,
            db.GetDictionaryModel().GetModelId(),
            classId
    );  
        CIBSEClassDefinitionPtr gridSurface = new CIBSEClassDefinition(params, name, Category);
        return gridSurface;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
/*
void InsertCIBSE
(
    Dgn::DgnDbR db,
    Utf8CP name,
    Utf8CP Category
) 
    {
        CIBSEClassDefinitionPtr classDefinition = CIBSEClassDefinition::Create(db, name,Category);
        classDefinition->Insert();
    }
*/
END_CLASSIFICATIONSYSTEMS_NAMESPACE