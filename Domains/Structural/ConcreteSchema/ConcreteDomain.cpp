/*--------------------------------------------------------------------------------------+
|
|     $Source: ConcreteSchema/ConcreteDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StructuralDomainInternal.h"


BEGIN_BENTLEY_CONCRETE_NAMESPACE

DOMAIN_DEFINE_MEMBERS(ConcreteDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Jiang                     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ConcreteDomain::ConcreteDomain() : Dgn::DgnDomain(BENTLEY_CONCRETE_SCHEMA_NAME, "Bentley Concrete Domain", 1)
    {
    //RegisterHandler(ConcreteElementHandler::GetHandler());

    //RegisterHandler(FrameElementHandler::GetHandler());
    //RegisterHandler(BeamHandler::GetHandler());
    //RegisterHandler(ColumnHandler::GetHandler());

    //RegisterHandler(SurfaceElementHandler::GetHandler());
    //RegisterHandler(SlabHandler::GetHandler());
    //RegisterHandler(WallHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Jiang                     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ConcreteDomain::InsertDomainCodeSpecs(Dgn::DgnDbR db)
    {
    Dgn::CodeSpecPtr codeSpec = Dgn::CodeSpec::Create(db, BENTLEY_CONCRETE_AUTHORITY, Dgn::CodeScopeSpec::CreateModelScope());
    if (codeSpec.IsValid())
        {
        codeSpec->Insert();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Jiang                     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ConcreteDomain::_OnSchemaImported(Dgn::DgnDbR dgndb) const
    {
    Dgn::DgnSubCategory::Appearance defaultApperance;
    defaultApperance.SetInvisible(false);

    InsertDomainCodeSpecs(dgndb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Jiang                     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ConcreteDomain::_OnDgnDbOpened(Dgn::DgnDbR db) const
    {
    }

END_BENTLEY_CONCRETE_NAMESPACE
