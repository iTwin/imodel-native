/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DocumentationDomain.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DocumentationDomain.h>

//=======================================================================================
//  Handler definitions
//=======================================================================================
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

DOMAIN_DEFINE_MEMBERS(DocumentationDomain)

namespace doc_ElementHandler
    {
    HANDLER_DEFINE_MEMBERS(Document)
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    06/2016
//---------------------------------------------------------------------------------------
DocumentationDomain::DocumentationDomain() : DgnDomain(DOCUMENTATION_DOMAIN_NAME, "Documentation Domain", 1) 
    {
    RegisterHandler(doc_ElementHandler::Document::GetHandler());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    06/2016
//---------------------------------------------------------------------------------------
DocumentationDomain::~DocumentationDomain()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    06/2016
//---------------------------------------------------------------------------------------
DgnDbStatus DocumentationDomain::ImportSchema(DgnDbR db)
    {
    BeFileName domainSchemaFile = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    domainSchemaFile.AppendToPath(DOCUMENTATION_DOMAIN_ECSCHEMA_PATH);
    BeAssert(domainSchemaFile.DoesPathExist());

    DgnDomainR domain = DocumentationDomain::GetDomain();
    DgnDbStatus importSchemaStatus = domain.ImportSchema(db, domainSchemaFile, ImportSchemaOptions::ImportOnly);
    BeAssert(DgnDbStatus::Success == importSchemaStatus);
    return importSchemaStatus;
    }
