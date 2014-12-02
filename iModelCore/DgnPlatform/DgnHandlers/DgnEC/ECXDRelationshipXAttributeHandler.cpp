/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnEC/ECXDRelationshipXAttributeHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#include <RmgrTools/Tools/DataExternalizer.h>

#define CURRTXN(a) a->GetDgnProject()->GetTxnManager().GetCurrentTxn()
#define XATTRIBUTEID_ECXDRelationship 0xECD9

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace Bentley::DgnPlatform::LoggingHelpers;

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_LOGGING
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace LoggingHelpers
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                      01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString         Label (ElementHandleCR eh)
    {
    Handler& handler = eh.GetHandler();
    WString typeName, description;
    handler.GetTypeName (typeName, 32); 
    handler.GetDescription (eh, description, 32);
    wchar_t elementId[32];
    BeStringUtilities::Snwprintf (elementId, _countof(elementId), L"%llu", eh.GetElementId());

    WString displayLabel = L"{" + typeName + L"(" + elementId + L")";
    
    if ( ! description.Equals (typeName.c_str()) )
        displayLabel += L": " + description;
        
    displayLabel += L"}";
    
    return displayLabel;
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                      01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString         Label (ElementRefP elementRef)
    {
    if (NULL == elementRef)
        return L"";
    ElementHandle eh(elementRef);
    return Label (eh);
    }
    
} // namespace LogggingHelpers

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDRelationshipXAttributeHandler& ECXDRelationshipXAttributeHandler::GetHandler ()
    {
    static ECXDRelationshipXAttributeHandler s_this;
    
    return s_this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeHandlerId ECXDRelationshipXAttributeHandler::_GetId ()
    {
    return XAttributeHandlerId (XATTRIBUTEID_ECXDRelationship, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
WString         ECXDRelationshipXAttributeHandler::_GetLabel ()
    {
    return L"ECXDRelationship";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                      11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECXDRelationshipXAttributeHandler::Register()
    {
    XAttributeHandlerManager::RegisterHandler                 (GetId(), &GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                      02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDRelationshipEnablerP GetRelationshipEnabler (XAttributeHandleCR xa, DgnModelP homeDgnModel)
    {
    InstanceHeader const& header = MemoryInstanceSupport::PeekInstanceHeader (xa.PeekData());

    SchemaClassIndexPair indexPair (header.m_schemaIndex, header.m_classIndex);
    
    BeAssert (NULL != homeDgnModel && NULL != homeDgnModel->GetDgnProject() && "Assuming that homeModel will never be NULL and it will always have a file. It is OUR file, after all.");
    DgnProjectR dgnFile = *homeDgnModel->GetDgnProject();
    
    ECXDProviderR provider = ECXDProvider::GetProvider();
    ECXDPerFileCacheR perFileCache = provider.GetPerFileCache (dgnFile);
    ECXDRelationshipEnablerP relationshipEnabler = provider.ObtainRelationshipEnabler (indexPair, perFileCache);
    if (NULL == relationshipEnabler) 
        {
        DgnECManager::GetManager().GetLogger().errorv(L"We were unable to obtain RelationshipEnabler for on %ls for XAttribute #%d... is ECSchema out of synch with the file?", Label(xa.GetElementRef()).c_str(), xa.GetId());
        BeAssert (false && "We were unable to obtain RelationshipEnabler... is ECSchema out of synch with the file?");
        return NULL;
        }

    return relationshipEnabler;
    }


END_BENTLEY_DGNPLATFORM_NAMESPACE
