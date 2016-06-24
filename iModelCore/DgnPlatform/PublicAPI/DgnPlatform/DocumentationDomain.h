/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DocumentationDomain.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnDomain.h>

DGNPLATFORM_TYPEDEFS(Document)

DGNPLATFORM_REF_COUNTED_PTR(Document)

#define DOCUMENTATION_DOMAIN_ECSCHEMA_PATH  L"ECSchemas/Domain/Documentation.01.00.ecschema.xml"
#define DOCUMENTATION_DOMAIN_NAME           "Documentation"
#define DOCUMENTATION_SCHEMA(className)     DOCUMENTATION_DOMAIN_NAME "." className

#define DOC_CLASSNAME_Document              "Document"

BEGIN_BENTLEY_DGN_NAMESPACE

namespace doc_ElementHandler {struct Document;};

//=======================================================================================
//! The Documentation DgnDomain
//! @ingroup GROUP_DgnDomain
// @bsiclass                                                    Shaun.Sewall    06/2016
//=======================================================================================
struct DocumentationDomain : DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(DocumentationDomain, DGNPLATFORM_EXPORT)

public:
    DocumentationDomain();
    ~DocumentationDomain();
    
    //! Import the ECSchema for the DocumentationDomain into the specified DgnDb
    DGNPLATFORM_EXPORT static DgnDbStatus ImportSchema(DgnDbR);
};

//=======================================================================================
//! A Document is an InformationElement that identifies the content of a document.
//! The realized form of a document is called a DocumentCopy (different class than Document). 
//! For example, a will is a legal document.  The will published into a PDF file is an ElectronicDocumentCopy.
//! The will printed onto paper is a PrintedDocumentCopy.
//! In this example, the Document only identifies, names, and tracks the content of the will.
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Document : InformationElement
{
    DGNELEMENT_DECLARE_MEMBERS(DOC_CLASSNAME_Document, InformationElement)
    friend struct doc_ElementHandler::Document;

protected:
    explicit Document(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! The namespace that only contains ElementHandlers for the DocumentationDomain
//! @private
//=======================================================================================
namespace doc_ElementHandler
{
    //! The ElementHandler for Document
    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE Document : dgn_ElementHandler::Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DOC_CLASSNAME_Document, Dgn::Document, Document, dgn_ElementHandler::Element, DGNPLATFORM_EXPORT)
    };
}

END_BENTLEY_DGN_NAMESPACE
