/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/Commands/AnnotateContours.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "StdAfx.h"

#pragma unmanaged

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_TERRAINMODEL_ELEMENT

#include    "SelectDTMElemTool.h"
//#include    "dtmcommandstool.h"
//#include    <DTMElementHandler.h>

//#include    <dimUtilities.fdf>

void startAnnotateContoursCommand (CommandNumber cmdNumber, int cmdName);
extern void UpdateToolsettings (ElementHandleCR dtm);
extern void CollectIntersections (bvector<ElementHandle>& geometryCollector, DPoint3dCR p1, DPoint3dCR p2, ElementHandleCR eh, ViewportP vp);
extern void StartSequence (CommandNumber cmdNum);
extern void EndSequence (CommandNumber cmdNum);

/*=================================================================================****
* @bsiclass                                                     Daryl.Holmwood   07/10
+===============+===============+===============+===============+===============+======*/
struct  AnnotateContoursElemTool : Annotate::DTMHolder, Annotate::SequencedTool
{
    DgnButtonEvent m_ev1;
    DgnButtonEvent m_ev2;
    int m_state;
    bvector<ElementHandle>  m_geomCollector;

#if defined(DYNAMICS_DEEP_DEBUG)
    virtual void        OnRedrawInit (ViewContextP vp) override { return __super::OnRedrawInit ( vp ); }
    virtual StatusInt   OnRedrawOperation (EditElementHandleR elHandle, ViewContextP vp, bool *canUseCached) override
        { return __super::OnRedrawOperation (elHandle, vp, canUseCached); }
    virtual void        OnResymbolize (ViewContextP vp) override { return __super::OnResymbolize (vp); }
    virtual StatusInt   OnRedrawComplete (ViewContextP vp) override { return __super::OnRedrawComplete (vp); }
#endif // defined(DYNAMICS_DEEP_DEBUG)

    /*---------------------------------------------------------------------------------****
    * @bsimethod                                    Daryl.Holmwood                  09/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    AnnotateContoursElemTool (CommandNumber cmdNumber, int cmdName, ElementHandleCR dtm) : DTMHolder (dtm)
        {
        m_state = 0;
        SetCmdNumber (cmdNumber);
        SetCmdName (cmdName, 0);
        }

    virtual ~AnnotateContoursElemTool ( void ) {}

    virtual UsesSelection   _AllowSelection (void) override {return USES_SS_None;}
    virtual bool            _DoGroups (void) override {return false;}
    virtual bool            _AcceptIdentifiesNext (void) override {return true;}
    virtual bool            _WantDynamics (void) override {return m_state != 0;}
    virtual bool    _WantAdditionalLocate (DgnButtonEventCR ev) { return false; }
    virtual RefLocateOption _GetReferenceLocateOptions (void) {return (RefLocateOption) (REF_LOCATE_Normal | REF_LOCATE_SelfAttachment | REF_LOCATE_Editable);}

    /*---------------------------------------------------------------------------------****
    * @bsimethod                                    Daryl.Holmwood                  09/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool            DoOperator (ElementHandleCR elHandle, bool isDynamics)
        {
        DTMDataRefPtr dtmDataRef;

        m_geomCollector.clear();
        DTMElementHandlerManager::GetDTMDataRef (dtmDataRef, elHandle);
        if (NULL == dtmDataRef.get())
            { return false; }

//        m_geomCollector->SetUseFastAlgorithm (isDynamics);
//        m_geomCollector->SetGeometryPreference (IGeometryCollector::IGeometryCollector_PreferDescriptor);
        CollectIntersections (m_geomCollector, *m_ev1.GetPoint(), *m_ev2.GetPoint(), elHandle, m_ev1.GetViewport());
        return true;
        }

    /*---------------------------------------------------------------------------------****
    * @bsimethod                                    Daryl.Holmwood                  09/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual StatusInt       _OnElementModify (EditElementHandleR elHandle) override
        {
        return ERROR;
        }

    /*---------------------------------------------------------------------------------****
    * @bsimethod                                    Daryl.Holmwood                  09/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool    _SetupForModify ( DgnButtonEventCR ev, bool isDynamics ) override
        {
        if (!m_dtm.IsValid () || !_GetAnchorPoint (NULL))
            return false;

        return DoOperator (m_dtm, isDynamics);
        }

    /*---------------------------------------------------------------------------------****
    * @bsimethod                                                    Daryl.Holmwood  07/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void    _SetupAndPromptForNextAction (void) override
        {
        mdlAccuSnap_enableSnap (_WantAccuSnap ());
        mdlAccuSnap_enableLocate (false);

        ::UInt32    promptId = 0L;

        if ( m_state != 2 )
            ++m_state;
        if (!m_dtm.IsValid ())
            promptId = PROMPT_IdentifyDTM;
        else if (m_state == 1)
            promptId = PROMPT_SelectFromPoint;
        else
            promptId = PROMPT_SelectToPoint;

    //    if ( promptId )
            mdlOutput_rscPrintf (MSG_PROMPT, 0L, STRINGID_Message_Main, promptId);
        }

    virtual void    _SetLocateCriteria (void) override
        {
        mdlLocate_allowAllModels (true);
        }

    /*---------------------------------------------------------------------------------****
    * @bsimethod                                    Daryl.Holmwood                  09/2010
    +---------------+---------------+---------------+---------------+---------------+------*/

    virtual bool    _FilterAgendaEntries (void) override
        {
        if (1 != GetElementAgenda().GetCount ())
            {
            assert (false);
            EditElementHandleP curr = GetElementAgenda().GetFirstP (); 
            EditElementHandleP end  = curr + GetElementAgenda().GetCount (); 
            for ( ; curr < end ; curr++)
                curr->Invalidate ();
            return true;
            }

        bool modify=false;
        EditElementHandleP curr = GetElementAgenda().GetFirstP();
        EditElementHandleP end  = curr + GetElementAgenda().GetCount();

        for (; curr < end ; curr++)
            {
            if(!IsValidDTMElement(*curr))
                {
                curr->Invalidate();
                modify=true;
                }
            }
        return modify;
        }

    /*---------------------------------------------------------------------------------****
    * @bsimethod                                                    Daryl.Holmwood  07/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void    _OnReinitialize (void) override {}

    /*---------------------------------------------------------------------------------****
    * @bsimethod                                                    Daryl.Holmwood  07/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void    _OnPostInstall (void) override
        {
        __super::_OnPostInstall ();
        UpdateToolsettings (m_dtm);
        }

    /*---------------------------------------------------------------------------------****
    * @bsimethod                                    Daryl.Holmwood                  09/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool    _OnInstall (void) override
        {
        if (!__super::_OnInstall ()) return false;
        LocateCallback::SetGlobalPostLocateFunction (LocateFilter);
        return true;
        }

    /*---------------------------------------------------------------------------------****
    * @bsimethod                                    Daryl.Holmwood                  09/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool _OnModifierKeyTransition (bool wentDown, int key) override
        {
        if(key == ALTKEY && wentDown) return true;
        else                          return false;
        }

    /*---------------------------------------------------------------------------------****
    * @bsimethod                                                    Daryl.Holmwood  07/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void    _OnDynamicFrame (DgnButtonEventCR ev) override
        {
        if (m_state != 2 || !m_dtm.IsValid())
            return;

        m_ev2 = ev;
        DPoint3d pts[2] = { *m_ev1.GetPoint(), *m_ev2.GetPoint() };

#ifndef DRAWDEBUG
        UInt32 color = ev.GetViewport()->AdjustColorForContrast (ev.GetViewport()->GetHiliteColor(), ev.GetViewport()->GetBackgroundColor());
        ev.GetViewport()->SetSymbologyRgb (color, color, 1, -1);
        ev.GetViewport()->GetIViewDraw()->DrawLineString3d (2, pts, NULL);
#endif
        if (!_SetupForModify (ev, true))
            return;

        struct ElementSet : IElementSet
            {
            bvector<ElementHandle>& m_elements;
            size_t m_index;
            ElementSet(bvector<ElementHandle>& elements) : m_elements (elements)
                {
                m_index = 0;
                }

            virtual size_t GetCount () { return m_elements.size(); }

            virtual bool GetFirst (ElementHandleR elHandle) { m_index = 0; if (m_index >= m_elements.size()) return false; elHandle = m_elements[m_index]; return true; }
            virtual bool GetNext(ElementHandleR elHandle) { m_index++; if (m_index >= m_elements.size()) return false; elHandle = m_elements[m_index]; return true; }
            };

        ElementSet set (m_geomCollector);
        IViewManager::GetActiveViewSet().DoElementSetDynamics (&set, NULL, NULL);
        }


    /*---------------------------------------------------------------------------------****
    * @bsimethod                                                    Daryl.Holmwood  07/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool    _OnDataButton (DgnButtonEventCR ev) override
        {

        if (!m_dtm.IsValid())
            {
            bool result = __super::_OnDataButton (ev);
            if ( GetElementAgenda().GetCount () > 0 )
                {
                m_dtm = *GetElementAgenda().GetFirstP ();
                _SetupAndPromptForNextAction ();
                }
            return result;
            }

        switch (m_state)
            {
            case 1:
                {
                m_ev1 = ev;
                _SetAnchorPoint (const_cast<DPoint3d*>(m_ev1.GetPoint ()));
                _BeginDynamics ();
                break;
                }

            case 2:
                {
                bool const IS_DYNAMICS = true;

                m_ev2 = ev;
                DoOperator (m_dtm, !IS_DYNAMICS);
                if (!m_geomCollector.empty())
                    {
                    for (size_t i = 0; i < m_geomCollector.size(); i++)
                        {
                        EditElementHandle curr (m_geomCollector[i], true);
                        curr.SetModelRef (ACTIVEMODEL);
//                        dimUtil_writeDimensionElement (curr->GetElementP());
                        curr.AddToModel ();
                        }
                    }

                if(!ev.IsAltKey())
                    {
                    m_ev1 = m_ev2;
//ToDo                    SetAnchorPoint (const_cast<DPoint3dP>(m_ev1.GetPoint()));
                    }
                break;
                }
            }

        _SetupAndPromptForNextAction ();
        return false;
        };

    /*---------------------------------------------------------------------------------****
    * @bsimethod                                                    Daryl.Holmwood  07/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool    _OnResetButton (DgnButtonEventCR ev) override
        {
        if (!m_dtm.IsValid())
            {
            _ExitTool ();
            return true;
            }
        if (m_state == 2)
            {
            // m_state is incremented in SetupAndPromptForNextAction()
            m_state = 0;
            _SetupAndPromptForNextAction ();
            }
        else
            _OnRestartTool ();
        return true;
        }
    /*---------------------------------------------------------------------------------****
    * @bsimethod                                                    Daryl.Holmwood  07/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void    _HiliteAgendaEntries (bool changed) override {}

    /*---------------------------------------------------------------------------------****
    * @bsimethod                                                    Daryl.Holmwood  07/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void    _OnRestartTool (void) override
        {
        startAnnotateContoursCommand (GetCmdNumber(), GetToolId ());
        }

    virtual void    AnnotateContoursElemTool::_OnCleanup (void) override
        {
        __super::_OnCleanup ();
        m_dtm = ElementHandle ();
        LocateCallback::SetGlobalPostLocateFunction (NULL);
        }

    /*---------------------------------------------------------------------------------****
    * @bsimethod                                                    Daryl.Holmwood  07/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void    _ExitTool (void) override
        {
        if (_CheckSingleShot ())
            mdlState_startDefaultCommand ();
        else
            _OnRestartTool();
        }

        /// <author>Piotr.Slowinski</author>                            <date>09/2011</date>
    virtual void        _OnBeginSequence (void) override
        {
        StartSequence (GetCmdNumber ());
        }

    virtual void        _OnEndSequence (void) override
        {
        EndSequence (GetCmdNumber ());
        }

}; // AnnotateContoursElemTool

extern void LoadDependable (void);

/// <summary>
/// </summary>
/// <returns></returns>
/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
DgnPlatform::LocateFilterStatus Annotate::DTMHolder::LocateFilter
    (
    DgnPlatform::LOCATE_Action  action,
    MSElementCP                 pElement,
    DgnModelRefP                modelRef,
    UInt32                      filePosition,
    DPoint3dCP                  pPoint,
    int                         viewNumber,
    HitPathCP                   hitPath,
    WStringR                    rejectReason
    )
    {
    Annotate::DTMHolder *activeTool = dynamic_cast <Annotate::DTMHolder*> ( DgnTool::GetActivePrimitiveTool () );

    BeAssert ( activeTool );
    if ( !activeTool )
        return LOCATE_FILTER_STATUS_Neutral;

    if( action == GLOBAL_LOCATE_SNAP )
        {
        if( mdlDisplayPath_getElem (const_cast<HitPathP>(hitPath),0) == activeTool->GetDtm().GetElementRef () )
            return LOCATE_FILTER_STATUS_Reject;
        }
    return LOCATE_FILTER_STATUS_Neutral;
    }


/// <summary>
/// </summary>
/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
void StartMainAnnotateContoursCommand (ElementHandleCR dtm, CommandNumber cmdNumber)
    {
    AnnotateContoursElemTool *tool;

    LoadDependable ();
    tool = new AnnotateContoursElemTool (cmdNumber, CMDNAME_LabelContours, dtm);
    tool->InstallTool ();
    }

/*---------------------------------------------------------------------------------****
* @bsimethod                                                    Daryl.Holmwood  07/10
+---------------+---------------+---------------+---------------+---------------+------*/
void startAnnotateContoursCommand (CommandNumber cmdNumber, int cmdName)
    {
    Annotate::SelectDTMElemTool*    tool;
    bool const                      REQUIRES_CONTOURS = true;

    LoadDependable ();
    tool = new Annotate::SelectDTMElemTool (cmdNumber, cmdName, PROMPT_IdentifyDTM, StartMainAnnotateContoursCommand, REQUIRES_CONTOURS);
    tool->InstallTool ();
    }
