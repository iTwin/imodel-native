/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/Commands/SelectDTMElem.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include    "StdAfx.h"
#pragma unmanaged

#include    "SelectDTMElemTool.h"

extern bool GetDTMLabelOnToolsettings (MSDialogP& db, int& item);
extern void StartSequence (uint64_t cmdNum);
extern void EndSequence (uint64_t cmdNum);

/// <author>Piotr.Slowinski</author>                            <date>04/2011</date>
Annotate::SelectDTMElemTool::SelectDTMElemTool
(
uint64_t                cmdNumber,
int                     cmdFunctionName,
int                     cmdPrompt,
MainCommandStarter      starter,
bool                    requiresContours
) : m_starter (starter), m_requiresContours(requiresContours)
    {
    BeAssert (m_starter);
    SetCmdNumber (cmdNumber);
    SetCmdName (cmdFunctionName, cmdPrompt);
    m_attemptSS = 1;
    }

/// <author>Piotr.Slowinski</author>                            <date>04/2011</date>
void Annotate::SelectDTMElemTool::_OnPostInstall (void)
    {
    __super::_OnPostInstall ();
    MSDialogP toolSettings   = NULL;
    int         labelIndex      = -1;

    //mdlDialog_
    if (!GetDTMLabelOnToolsettings (toolSettings, labelIndex))
        return;

    wchar_t msg[1024] = L"";
    mdlDialog_itemSetLabel ( toolSettings, labelIndex, SUCCESS == mdlResource_loadFromStringList ( msg, NULL, WMSGLIST_Main, WSTATUS_NotSelected ) ? msg : L"<Not Selected>" );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  09/2007
+---------------+---------------+---------------+---------------+---------------+------*/
bool            Annotate::SelectDTMElemTool::_OnInstall (void)
    {
    return __super::_OnInstall ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            Annotate::SelectDTMElemTool::_OnCleanup (void)
    {
    m_attemptSS = 1;
    return __super::_OnCleanup ();
    }

/// <author>Piotr.Slowinski</author>                            <date>04/2011</date>
void Annotate::SelectDTMElemTool::_OnRestartTool ( void )
    {
    SelectDTMElemTool* tool;
    tool = new SelectDTMElemTool (GetCmdNumber (), GetToolId(), GetToolPrompt (), m_starter, m_requiresContours);
    tool->InstallTool ();
    }

/// <author>DarylHolmwood</author>                              <date>09/2011</date>
bool Annotate::SelectDTMElemTool::_OnPostLocate (HitPathCP path, WStringR cantAcceptReason)
    {
    ElementRefP elmref = mdlDisplayPath_getElem ((DisplayPathP) path, mdlDisplayPath_getCursorIndex((DisplayPathP)path));
    BeAssert(elmref);
    DgnModelRefP modelRef = mdlDisplayPath_getPathRoot ((DisplayPathP)path);
    ElementHandle eh(elmref, modelRef);
 
   if (IsValidDTMElement (eh))
        {
        if (m_requiresContours)
            {
            ElementHandle symbologyElem;
            DTMElementHandlerManager::GetElementForSymbology (eh, symbologyElem, ACTIVEMODEL);
            DTMSubElementIter &iter = *DTMSubElementIter::Create (symbologyElem);

            for (; iter.IsValid(); iter.ToNext())
                {
                if (DTMElementContoursHandler::GetInstance()->GetSubHandlerId() == iter.GetCurrentId().GetHandlerId())
                    {
                    delete &iter;
                    return true;
                    }
                }

            WChar msg[1024] = L"";
            cantAcceptReason = SUCCESS == mdlResource_loadFromStringList ( msg, NULL, STRINGID_Message_Main, ERROR_NoContoursDrawn ) ? msg : L"No contours are Drawn";
            delete &iter;
            return false;
            }
        return true;
        }

        WChar msg[1024] = L"";
        cantAcceptReason = SUCCESS == mdlResource_loadFromStringList ( msg, NULL, STRINGID_Message_Main, ERROR_NotADTM ) ? msg : L"Not a DTM";
    return false;
    }

/// <author>Piotr.Slowinski</author>                            <date>9/2011</date>
void Annotate::SelectDTMElemTool::_OnBeginSequence (void)
    {
    StartSequence (GetCmdNumber());
    }

/// <author>Piotr.Slowinski</author>                            <date>9/2011</date>
void Annotate::SelectDTMElemTool::_OnEndSequence (void)
    {
    EndSequence (GetCmdNumber());
    }

/// <author>Piotr.Slowinski</author>                            <date>9/2011</date>
StatusInt   Annotate::SequencedTool::_InstallToolImplementation (void)
    {
    SequencedTool   *currTool;

    currTool = dynamic_cast <SequencedTool*>(DgnTool::GetActivePrimitiveTool());
    if (currTool)
        {
        if (currTool->GetCmdNumber() != GetCmdNumber())
            {
            BeAssert (currTool->m_callEndSequenceOnCleanup && m_callEndSequenceOnCleanup);
            currTool->CallAndCleanCleanUpSentinel ();
            BeAssert (!currTool->m_callEndSequenceOnCleanup);
            _OnBeginSequence ();
            }
        else
            {
            BeAssert (currTool->m_callEndSequenceOnCleanup && m_callEndSequenceOnCleanup);
            currTool->m_callEndSequenceOnCleanup = false;
            }
        }
    else
        {
        _OnBeginSequence ();
        BeAssert (m_callEndSequenceOnCleanup);
        }

    return __super::_InstallToolImplementation ();
    }
