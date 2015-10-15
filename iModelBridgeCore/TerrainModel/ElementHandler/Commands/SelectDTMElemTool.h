/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/Commands/SelectDTMElemTool.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_TERRAINMODEL_ELEMENT

inline bool IsValidDTMElement (ElementHandleR element)
    {    
    return DTMElementHandlerManager::IsDTMElement (element) &&
        (ElementHandle::XAttributeIter (element, Bentley::DgnPlatform::XAttributeHandlerId (TMElementMajorId, XATTRIBUTES_SUBID_DTM_REFERENCE)).IsValid() ||
        ElementHandle::XAttributeIter (element, Bentley::DgnPlatform::XAttributeHandlerId (TMElementMajorId, XATTRIBUTES_SUBID_DTM_HEADER)).IsValid());
    }


namespace Annotate {

/// <summary>
///
/// </summary>
/// <param name=""></param>
/// <remarks>
/// 
/// </remarks>
/// <returns></returns>
/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
struct DTMHolder
{
protected: Bentley::DgnPlatform::ElementHandle m_dtm;
protected: DTMHolder::DTMHolder ( ElementHandleCR dtm ) : m_dtm ( dtm ) {}
public: ElementHandleCR DTMHolder::GetDtm ( void ) const { return m_dtm; }
protected: static DgnPlatform::LocateFilterStatus LocateFilter
(
DgnPlatform::LOCATE_Action  action,
MSElementCP                 pElement,
DgnModelRefP                modelRef,
uint32_t                      filePosition,
DPoint3dCP                  pPoint,
int                         viewNumber,
HitPathCP                   hitPath,
WStringR                    rejectReason
);

}; // End DTMHolder struct

/// <author>Piotr.Slowinski</author>                            <date>8/2011</date>
struct SequencedTool : DgnElementSetTool
{
protected:
    uint64_t   m_cmdNumber;

    bool m_callEndSequenceOnCleanup;

    void CallAndCleanCleanUpSentinel (void)
        {
        if (m_callEndSequenceOnCleanup)
            {
            m_callEndSequenceOnCleanup = false;
            _OnEndSequence ();
            }
        }
    virtual ElemSource  _GetPreferredElemSource (void) override {return SOURCE_Pick;}
    void     mdlLocate_allowAllModels (bool allowRefs)
        {
        int     iView;
        int     typeMask = MRLIST_IncludeRoot;

        if (allowRefs)
            typeMask |= MRLIST_IncludePrimaryRefs;

        tcb->searchModelList->empty();

        for (iView = 0; iView < MAX_VIEWS; iView++)
            {
            DgnModelRefP    rootModelRef = mdlView_getRootModel (iView);

            if (INVALID_MODELREF != rootModelRef)
                mdlModelRef_appendChildList (&tcb->searchModelList, rootModelRef, typeMask, -1);
            }
        }
public:
    SequencedTool (void) : m_callEndSequenceOnCleanup (true)
        {}

    virtual StatusInt   _InstallToolImplementation (void) override;
    virtual void        _OnCleanup (void) override
        {
        CallAndCleanCleanUpSentinel ();
        __super::_OnCleanup ();
        }
    virtual void        _OnBeginSequence (void) = 0;
    virtual void        _OnEndSequence (void) = 0;

    void SetCmdNumber (uint64_t cmdNumber) {m_cmdNumber = cmdNumber;}
    uint64_t GetCmdNumber () {return m_cmdNumber;}

}; // End SequencedTool struct

/// <summary>
/// </summary>
/// <param name=""></param>
/// <remarks>
/// </remarks>
/// <returns></returns>
/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
struct SelectDTMElemTool : SequencedTool
    {
    typedef void (*MainCommandStarter) (ElementHandleCR dtm, uint64_t cmdNumber);

    MainCommandStarter const    m_starter;
    bool m_attemptSS;
    bool m_requiresContours;

    SelectDTMElemTool::SelectDTMElemTool
    (
    uint64_t    cmdNumber,
    int         cmdFunctionName,
    int         cmdPrompt,
    MainCommandStarter starter,
    bool        requiresContours = false
    );

    virtual ~SelectDTMElemTool () {}

    virtual void _OnRestartTool () override;
    virtual void _SetupAndPromptForNextAction () override
        {
        mdlAccuSnap_enableSnap ( false );
        mdlAccuSnap_enableLocate ( true );
        mdlOutput_rscPrintf ( MSG_PROMPT, NULL, 0, GetToolPrompt () );  
        }
    virtual bool _OnResetButton (DgnButtonEventCR ev) override { return false; }
    virtual bool _OnPostLocate (HitPathCP path, WStringR cantAcceptReason) override;
    virtual void _SetLocateCriteria () override
        {
        // Commented out implementation browses all models for all views.
        // That might interfere with Civil multiple model approach of work,
        // hence direct manipulation on tcb->searchModelList
        //BoolInt const ALLOW_REFS = TRUE;

        //mdlLocate_allowAllModels (ALLOW_REFS);
        if (tcb->searchModelList)
            {
            tcb->searchModelList->clear();
            mdlModelRef_appendChildList (&tcb->searchModelList, ACTIVEMODEL, MRLIST_IncludeRoot | MRLIST_IncludePrimaryRefs, -1);
            }
        }
//    virtual ElemSource  GetPreferredElemSource () {

    virtual DgnElementSetTool::UsesSelection   _AllowSelection ()
        {
        if (m_attemptSS)
            return DgnElementSetTool::USES_SS_Check;
        return DgnElementSetTool::USES_SS_None;
        }
    virtual bool    NeedPointForSelection () {return false;}
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Tom.Jiang                       07/2008
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual StatusInt   _ProcessAgenda (DgnButtonEventCR ev) override
            {
            return SUCCESS;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Tom.Jiang                       07/2008
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual StatusInt   _OnElementModify (EditElementHandleR elHandle) override
            {
            return ERROR;
            }

    virtual bool _OnModifyComplete (DgnButtonEventCR evCP) override
        {
        if (SOURCE_SelectionSet == _GetElemSource ())
            {
            if ( GetElementAgenda().size() > 0 )
                {
                m_starter ( *GetElementAgenda ().GetFirstP (), GetCmdNumber() );
                }
            }
        return false;
        }


    virtual bool    _NeedAcceptPoint () override { return false; }

    virtual void    _BuildAgenda (DgnButtonEventCR ev)override
        {
        __super::_BuildAgenda (ev);

        if (SOURCE_SelectionSet == _GetElemSource ())
            {
            if ( GetElementAgenda().size() > 0 )
                {
                return;
                }
            else
                m_attemptSS = false;
            }
            _SetElemSource (SOURCE_Pick);
            _BeginPickElements ();
        }
    virtual bool    _FilterAgendaEntries ( void ) override
        {
        if (1 != GetElementAgenda ().GetCount ())
            {
            BeAssert (false);
            EditElementHandleP curr = GetElementAgenda ().GetFirstP (); 
            EditElementHandleP end  = curr + GetElementAgenda ().GetCount (); 
            for ( ; curr < end ; curr++)
                curr->Invalidate ();
            return true;
            }

        bool modify=false;
        EditElementHandleP curr = GetElementAgenda().GetFirstP();
        EditElementHandleP end  = curr + GetElementAgenda().GetCount();

        for (; curr < end ; curr++)
            {
            if ( !IsValidDTMElement ( *curr ))
                {
                curr->Invalidate();
                modify=true;
                }
            else if (m_requiresContours)
                {
                bool hasContours = false;
                ElementHandle symbologyElem;
                DTMElementHandlerManager::GetElementForSymbology (*curr, symbologyElem, ACTIVEMODEL);
                DTMSubElementIter &iter = *DTMSubElementIter::Create (symbologyElem);
                for (; iter.IsValid(); iter.ToNext())
                    {
                    if (DTMElementContoursHandler::GetInstance()->GetSubHandlerId() == iter.GetCurrentId().GetHandlerId())
                        {
                        hasContours = true;
                        break;
                        }
                    }
                delete &iter;

                if (!hasContours)
                    {
                    curr->Invalidate();
                    modify=true;
                    }
                return modify;
                }
            }
        return modify;
        }

    //virtual StatusInt   OnPreElementModify (EditElementHandleR elHandle) override {return SUCCESS;} 
    //virtual bool        OnModifyComplete (DgnButtonEventCP evCP) override;
//    virtual EditElementHandleP BuildLocateAgenda (HitPathCP path, DgnButtonEventCP ev) override;
//    virtual HitPathCP   DoLocate (DgnButtonEventCP ev, bool newSearch, int complexComponent) override;
    //virtual RefLocateOption GetReferenceLocateOptions () override {return (RefLocateOption) (REF_LOCATE_Normal | REF_LOCATE_SelfAttachment | REF_LOCATE_TreatAsElement);}
    
    virtual void    _HiliteAgendaEntries ( bool changed ) override {}

    virtual bool _OnDataButton ( DgnButtonEventCR ev ) override
        {
        bool rtVal = __super::_OnDataButton ( ev );
        if ( GetElementAgenda().size() > 0 )
            {
            m_starter ( *GetElementAgenda ().GetFirstP (), GetCmdNumber() );
            }
        else
            m_attemptSS = 0;

        return rtVal;
        }

    virtual bool    SelectDTMElemTool::_OnInstall ( void ) override;
    virtual void    SelectDTMElemTool::_OnCleanup ( void ) override;

    /// <author>Piotr.Slowinski</author>                            <date>04/2011</date>
    virtual void    SelectDTMElemTool::_OnPostInstall ( void ) override;

    /// <author>Piotr.Slowinski</author>                            <date>09/2011</date>
    virtual void        _OnBeginSequence (void) override;
    virtual void        _OnEndSequence (void) override;

    }; // End SelectDTMElemTool struct

} // end Annotate namespace
