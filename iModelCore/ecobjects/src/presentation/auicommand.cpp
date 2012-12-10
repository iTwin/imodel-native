/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/auicommand.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <EcPresentation/auipresentationapi.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   IUICommand::ExecuteCmd (IAUIDataContextCP instance)
    {
    ECPresentationManager::GetManager().JournalCmd (*this, instance);
    return _ExecuteCmd(instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void   ECPresentationCommandProvider::GetCommand (bvector<IUICommandPtr>& commands, IAUIDataContextCR instance, int purpose)
    {
    return _GetCommand(commands, instance, purpose);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            IUICommand::Journal (IJournalItemR journalEntry)
    {
    _Journal(journalEntry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         IUICommand::GetCommandId () const
    {
    return _GetCommandId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WString         IUICommand::GetLabel ()  const
    {
    return _GetLabel();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            IUICommand::SetLabel (WStringCR label)
    {
    _SetLabel(label);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         IUICommand::GetDescription ()  const
    {
    return _GetDescription();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            IUICommand::SetDescription (WStringCR description)
    {
    _SetDescription(description);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECImageKeyCP    IUICommand::GetImageId ()  const
    {
    return _GetImageId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            IUICommand::SetImageId (ECImageKeyCR key)
    {
    _SetImageId(key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IUICommand::IsSeparator ()  const
    {
    return _IsSeparator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IUICommand::EditActionMenuMark IUICommand::GetMenuMark() const
    {
    return _GetMenuMark();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            IUICommand::SetMenuMark(EditActionMenuMark mark)
    {
    _SetMenuMark(mark);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IUICommand::GetIsEnabled () const
    {
    return _GetIsEnabled();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            IUICommand::SetIsEnabled (bool enabled)
    {
    _SetIsEnabled(enabled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IUICommand* IUICommand::GetParent() const
    {
    return _GetParent();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IUICommand::EditActionPriority IUICommand::GetPriority () const
    {
    return _GetPriority();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            IUICommand::SetPriority (EditActionPriority priority)
    {
    _SetPriority(priority);
    }