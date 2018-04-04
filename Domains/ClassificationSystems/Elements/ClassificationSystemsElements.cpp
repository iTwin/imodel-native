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
DEFINE_CLASSIFICATIONSYSTEMS_ELEMENT_BASE_METHODS(OmniClassClassDefinition)
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
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
OmniClassClassDefinition::OmniClassClassDefinition
(
    CreateParams const& params,
    Utf8CP name, 
    Utf8CP omniClassID, 
    Utf8CP description
) : T_Super(params)
    {
    SetName(name);
    SetOmniClassID(omniClassID);
    SetDescription(description);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Martynas.Saulius               04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
OmniClassClassDefinitionPtr OmniClassClassDefinition::Create
(
    Dgn::DgnDbR db,
    Utf8CP name, 
    Utf8CP omniClassID, 
    Utf8CP description
)
    {
    Dgn::DgnClassId classId = QueryClassId(db);
    Dgn::DgnElement::CreateParams params(
        db,
        db.GetDictionaryModel().GetModelId(),
        classId
    );
    OmniClassClassDefinitionPtr gridSurface = new OmniClassClassDefinition(params, name, omniClassID, description);
    return gridSurface;
    }
END_CLASSIFICATIONSYSTEMS_NAMESPACE