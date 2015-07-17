/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/Commands/AnnotateSpots.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "StdAfx.h"

#define	HORJUSTMODE_LEFT        0
#define HORJUSTMODE_RIGHT       1
#define HORJUSTMODE_DYNAMIC     2
#define HORJUSTMODE_CENTER      3

#define HOR_LEFT(just)          ((just) == TextElementJustification::LeftTop || (just) == TextElementJustification::LeftMiddle || (just) == TextElementJustification::LeftBaseline)

#pragma unmanaged

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_TERRAINMODEL_ELEMENT

#include    "SelectDTMElemTool.h"

#pragma warning ( disable : 4290 )

void startAnnotateSpotsCommand (CommandNumber cmdNumber, int cmdName);
extern void UpdateToolsettings (ElementHandleCR dtm);
extern DMatrix4d GetMatrixWorldToView (int view);
extern void StartSequence (UInt64 cmdNum);
extern void EndSequence (UInt64 cmdNum);
extern bool PrepText (TextBlockPtr& rtb, ElementHandleCR elHandle, DgnButtonEventCR ev);
extern void LoadDependable (void);
extern TextElementJustification ComputeDynamicJustification (DPoint3dP point1P, DPoint3dP point2P, RotMatrixP viewMatrixP);
extern void ConvertToTextBlock (TextBlockPtr& tb, double elevation, ViewportP vp, DgnModelRef* dtmModelRef);
extern bool IsSelfSnap (DgnButtonEventCR ev, ElementHandleCR dtm, DPoint3dR point);

/*----------------------------------------------------------------------------------***
 * @bsimethod                                                    Brien.Bastings  12/05
 +---------------+---------------+---------------+---------------+---------------+------*/    
inline void     UpdateDimension (EditElementHandleR dimElemHandle, MSElementP elmP)
    {
    MSElementDescrP dimEdP = NULL;
    bool OWNED = true;
    bool IS_UNMODIFIED = true;

    mdlElmdscr_new (&dimEdP, NULL, elmP);
    dimElemHandle.SetElementDescr (dimEdP, OWNED, !IS_UNMODIFIED, ACTIVEMODEL);
    }

/// <summary>
/// </summary>
/// <param name="cellElemHandle">cellElemHandle</param>
/// <param name="dimElemHandle">dimElemHandle</param>
/// <param name="originP">originP</param>
/// <param name="isDynamics">isDynamics</param>
/// <remarks> stolen from PlaceNoteTool::CreateNote
/// see $(MSJ)mstn\mdlapps\wordproc\wordnote.cpp </remarks>
/// <returns>SUCCESS if successfull</returns>
/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
struct AnnotateSpotsElemTool : Annotate::SequencedTool, Annotate::DTMHolder
    {
#if defined(DYNAMICS_DEEP_DEBUG)
    virtual void        OnRedrawInit (ViewContextP vp) override { return __super::OnRedrawInit ( vp ); }
    virtual StatusInt   OnRedrawOperation (EditElementHandleR elHandle, ViewContextP vp, bool *canUseCached) override
        { return __super::OnRedrawOperation (elHandle, vp, canUseCached); }
    virtual void        OnResymbolize (ViewContextP vp) override { return __super::OnResymbolize (vp); }
    virtual StatusInt   OnRedrawComplete (ViewContextP vp) override { return __super::OnRedrawComplete (vp); }
#endif // defined(DYNAMICS_DEEP_DEBUG)

    protected:
        DPoint3d                    m_points[100];
        int                         m_nPoints;

        DPoint3d                    m_dynamicPoint;

        AssocPoint                  m_assocPoint;
        bool                        m_assocPtValid;

        bool                        m_isMultiLeader;
        int                         m_firstPointView;
        TextElementJustification    m_horAttachmentSide;
        int                         m_numLeaders;
        DPoint3d                    m_leaderEndTangent;
        UInt32                      m_iGGNum;
        ElementId                   m_cellElementID;

        EditElementHandle           m_existingElemHandle, m_textElemHandle;
        TextBlockPtr                m_pTextBlock;
        DgnButtonEvent              m_ev;
        bool                        m_evIsSelfSnap;
        DPoint3d                    m_elevationPoint;

        DgnButtonEventCR Prepare (DgnButtonEventCR ev)
            {
            m_evIsSelfSnap = IsSelfSnap (m_ev = ev, m_dtm, m_elevationPoint);
            if (m_evIsSelfSnap && !mdlModelRef_is3D(ev.GetViewport()->GetTargetModel()) && mdlModelRef_isReference(m_dtm.GetModelRef()))
                {
                Transform   tr;
                bool SCALE_Z_FOR_2D_REF = true;

                for (DgnAttachmentP rf = mdlRefFile_getInfo(m_dtm.GetModelRef()); NULL != rf; rf = rf->GetParentR().AsDgnAttachmentP())
                    {
                    if (rf == ev.GetViewport()->GetTargetModel())
                        { break; }
                    rf->GetTransformFromParent (tr, !SCALE_Z_FOR_2D_REF);
                    tr.multiply (&m_elevationPoint);
                    }
                }
            return m_ev;
            }

    public:         AnnotateSpotsElemTool (CommandNumber cmdNumber, int cmdName, ElementHandleCR dtm) :
        DTMHolder (dtm), m_evIsSelfSnap (false)
            {
            SetCmdNumber (cmdNumber);
            SetCmdName (cmdName, 0);
            }

        virtual StatusInt       _OnElementModify (EditElementHandleR elHandle) override {return SUCCESS;}

    public: void    SetupPlacementMethod ( void );

    public: bool    IsManualPlacement ( void ) const
                {
                DimensionStylePtr dsP = DimensionStyle::GetActive ();
                int textPos;

                dsP->GetIntegerProp (textPos, DIMSTYLE_PROP_Placement_TextPosition_INTEGER);
                return textPos == DIMSTYLE_VALUE_Placement_TextPosition_Manual;
                }

            /// <author>Piotr.Slowinski</author>                            <date>7/2011</date>
    public: virtual bool _OnInstall (void) override
                {
                return __super::_OnInstall ();
                }
            /*---------------------------------------------------------------------------------****
            * @bsimethod                                    Piotr.Slowinski                 03/2011
            +---------------+---------------+---------------+---------------+---------------+------*/
    public: virtual void _OnCleanup ( void ) override
                {
                __super::_OnCleanup ();
                m_dtm = ElementHandle ();
                m_evIsSelfSnap = false;
                }

            /*----------------------------------------------------------------------------------***
             * @bsimethod                                                    PaulChater      05/01
             +---------------+---------------+---------------+---------------+---------------+------*/    
    public: TextElementJustification NoteGetDynamicJustification
                (
                bool            *bHorJustOverriddenByCursor,
                int	    	    viewNum,
                bool         isDynamics
                );

            /*----------------------------------------------------------------------------------***
             * @bsimethod                                                    SunandSandurkar 07/03
             +---------------+---------------+---------------+---------------+---------------+------*/    
    public: void    AdjustNote
                (
                EditElementHandleR dimElemHandle,
                DimensionStyleP dimStyle,
                bool            isDynamics
                );
            /*---------------------------------------------------------------------------------****
             * @bsimethod                                                    SunandSandurkar  07/03
             +---------------+---------------+---------------+---------------+---------------+------*/
    public: StatusInt CreateNote
                (
                EditElementHandleR cellElemHandle,
                EditElementHandleR dimElemHandle,
                DPoint3d*       originP,
                bool            isDynamics,
                DgnButtonEventCP ev
                );
            /*----------------------------------------------------------------------------------***
             * @bsimethod                                                    SunandSandurkar 07/03
             +---------------+---------------+---------------+---------------+---------------+------*/
    public: void    SetLeaderEndTangent
                (
                EditElementHandleR                  dimElemHandle,
                DPoint3dP                           cursorP,
                DPoint3dP                           prevPointP,
                TextElementJustification            horAttachmentSide,
                int                                 view
                );
            /*---------------------------------------------------------------------------------****
             * @bsimethod                                                    Brien.Bastings  12/05
             +---------------+---------------+---------------+---------------+---------------+------*/
    public: void    DoNoteDynamics ( DgnButtonEventCR ev );
            /*---------------------------------------------------------------------------------****
             * @bsimethod                                                    Brien.Bastings  12/05
             +---------------+---------------+---------------+---------------+---------------+------*/
    public: void    DoMultiLeaderNoteDynamics ( DgnButtonEventCR ev );
            /*---------------------------------------------------------------------------------****
             * @bsimethod                                                    PaulChater      05/01
             +---------------+---------------+---------------+---------------+---------------+------*/
    public: StatusInt WriteDimensionElement ( EditElementHandleR dimElm );

            /*---------------------------------------------------------------------------------****
             * @bsimethod                                                    SunandSandurkar 05/00
             +---------------+---------------+---------------+---------------+---------------+------*/
    public: void    FinishMultiLeaderNote ( DgnButtonEventCR ev );
            /*---------------------------------------------------------------------------------****
             * @bsimethod                                                    SunandSandurkar 05/06
             +---------------+---------------+---------------+---------------+---------------+------*/
    public: void    UpdateCellAssociationOfDim ( EditElementHandleR dimElemHandle )
                {
                MSElement       elm;
                dimElemHandle.GetElementP ()->CopyTo (elm);
                mdlDim_deletePoint (&elm, -1);

                AssocPoint  assocPoint;
                mdlAssoc_createOrigin (&assocPoint, 0, NULL, m_cellElementID);
                mdlDim_insertPoint (&elm, NULL, &assocPoint, -1, POINT_ASSOC);
                UpdateDimension (dimElemHandle, &elm);
                }
            /*---------------------------------------------------------------------------------****
             * @bsimethod                                                    SunandSandurkar 05/06
             +---------------+---------------+---------------+---------------+---------------+------*/
    public: void    UpdateDimDependencyOfCell ( void ) 
                {
                MSElementDescrP cellEdP = NULL;
                if (SUCCESS != mdlAssoc_getElementDescr (&cellEdP, NULL, m_cellElementID, ACTIVEMODEL, false))
                    {
                    BeAssert (("Unable to find note cell", 0));
                    return;
                    }

                EditElementHandle newCellElemHandle (cellEdP, true, false);
                ElementRefP elementRef = cellEdP->h.elementRef;
                if (SUCCESS == mdlNote_addDimDependencyToCell (newCellElemHandle))
                    newCellElemHandle.ReplaceInModel (elementRef);
                }    
            /*---------------------------------------------------------------------------------****
             * @bsimethod                                                    SunandSandurkar 05/06
             +---------------+---------------+---------------+---------------+---------------+------*/
    public: void    UpdateCellAssociationOfInvisibleDim ( void )
                {
                // Find the cell
                MSElementDescrP cellEdP = NULL;
                if (SUCCESS != mdlAssoc_getElementDescr (&cellEdP, NULL, m_cellElementID, ACTIVEMODEL, false))
                    {
                    BeAssert (("Unable to find note cell", 0));
                    return;
                    }

                // Find the invisible dimension        
                MSElementDescrP dimEdP = cellEdP->h.firstElem;
                while   (NULL != dimEdP)
                    {
                    if (DIMENSION_ELM == dimEdP->el.ehdr.type)
                        break;
                    dimEdP = dimEdP->h.next;
                    }

                if (NULL == dimEdP)
                    {
                    BeAssert (("Unable to find invisible dimension", 0));
                    return;
                    }

                // Add an assoc point that points to cell
                EditElementHandle dimElemHandle (dimEdP, false, false);
                UpdateCellAssociationOfDim (dimElemHandle);
                dimElemHandle.GetElementDescrP ()->h.elementRef = dimEdP->h.elementRef;
                mdlElmdscr_replaceDscr (&dimEdP, dimElemHandle.ExtractElementDescr ());

                // Rewrite the entire cell since the invisible dim is its component
                EditElementHandle newCellElemHandle (cellEdP, true, false);
                newCellElemHandle.ReplaceInModel (cellEdP->h.elementRef);
                }

            /*---------------------------------------------------------------------------------****
             * @bsimethod                                                    SunandSandurkar 05/06
             +---------------+---------------+---------------+---------------+---------------+------*/
    public: void    UpdateCellOnlyAssociations ( void )
                {
                // Add an assoc point on the invisible dim that points to cell
                UpdateCellAssociationOfInvisibleDim ();

                // Make the cell dependent on dimension
                UpdateDimDependencyOfCell ();
                }
            /*---------------------------------------------------------------------------------****
             * @bsimethod                                                    PaulChater      05/01
             +---------------+---------------+---------------+---------------+---------------+------*/
    public: void    FinishNote ( DgnButtonEventCR ev )
                {
                if ( 1 >= m_nPoints )
                    return;

                DPoint3d    txtPoint, dataPoint = m_points[m_nPoints-1];
                txtPoint  = dataPoint;

                EditElementHandle  cellElemHandle;
                bool IS_DYNAMICS = true;

                // create note cell
                if ( SUCCESS == CreateNote ( cellElemHandle, m_existingElemHandle, &txtPoint, !IS_DYNAMICS, &ev ) )
                    {
                    m_iGGNum = tcb->graphic;
                    NormalCellHeaderHandler::SetCellRange (cellElemHandle);

                    MSElementDescrP desc = cellElemHandle.ExtractElementDescr ();
                    mdlElmdscr_setProperties (&desc, NULL, &m_iGGNum, NULL, NULL, NULL, NULL, NULL, NULL);
                    cellElemHandle.SetElementDescr (desc, true, false, ACTIVEMODEL);

                    if (SUCCESS == cellElemHandle.AddToModel ())
                        {
                        mdlSystem_updateGraphicGroup ();

                        m_cellElementID = cellElemHandle.GetElementRef ()->GetElementId();

// ToDo Vancouver                        mdlTemplateManager_attachToElement (cellElemHandle.GetElementRef ()); // add active element template
                        }

                    // if the element we added was a text node then increment the node number
                    if (m_textElemHandle.IsValid () && TEXT_NODE_ELM == m_textElemHandle.GetElementType ())
                        {
                        mdlSystem_updateCurrentNode ();

                        // Update text node in current edP for subsequent placements...
                        m_textElemHandle.GetElementP ()->text_node_2d.nodenumber = tcb->canode;
                        }

                    // associate cell with dimension
                    UpdateCellAssociationOfInvisibleDim ();
                    UpdateCellAssociationOfDim (m_existingElemHandle);                    
                    }

                // Find the two points closest to the cell. They will be used to compute
                // the cell rotation angle if rotation is set to Inline.
                Dpoint3d    cursor;
                Dpoint3d    prevPoint;

                cursor    = m_points[m_nPoints-1];
                prevPoint = m_points[m_nPoints-2];

                UInt16  leaderType = 0;
                // prepare and write leader dimension 
                if (mdlDim_getNoteLeaderType (&leaderType, m_existingElemHandle) && 1 == leaderType)
                    {
                    SetLeaderEndTangent (m_existingElemHandle, &cursor, &prevPoint, m_horAttachmentSide, ev.GetViewport ()->GetViewNumber ());
                    mdlDim_segmentGetCurveEndTangent (&m_leaderEndTangent, m_existingElemHandle, 0);
                    }

                MSElementDescrP desc = m_existingElemHandle.ExtractElementDescr();
                mdlElmdscr_setProperties (&desc, NULL, &m_iGGNum, NULL, NULL, NULL, NULL, NULL, NULL);
                m_existingElemHandle.SetElementDescr (desc, true, true);
                WriteDimensionElement (m_existingElemHandle);

                // add a dependency of dimension (root) on the cell (dependent)
                if (cellElemHandle.IsValid ())
                    UpdateDimDependencyOfCell ();

                m_numLeaders++;
                }
            /*---------------------------------------------------------------------------------****
             * @bsimethod                                                    PaulChater      05/00
             +---------------+---------------+---------------+---------------+---------------+------*/
    public: void    NoteDimCompleteNote ( DgnButtonEventCR ev )
                {
                // commit the current note
                if (m_isMultiLeader)
                    FinishMultiLeaderNote (ev);
                else
                    FinishNote (ev);

                // set up for the next leader
                if (m_isMultiLeader = (ev.IsControlKey () && INVALID_ELEMENTID != m_cellElementID))
                    {
                    /* NOTE: Set up the first m_points to correspond to the textpoint
                    because all subsequent multileaders are going to start at text
                    and go backwards. */
                    if ( 1 == m_numLeaders)
                        m_points[0] = m_points[m_nPoints-1];

                    m_nPoints = 1;

                    /* NOTE: The subsequent leaders should use the same settings as the master leader. So simply delete all
                    datapoints from the master dim struct to prepare a new dim. */
                    int         numPoints = mdlDim_getNumberOfPoints (m_existingElemHandle.GetElementP ());
                    MSElement   elm;

                    m_existingElemHandle.GetElementP ()->CopyTo (elm);

                    while (numPoints)
                        mdlDim_deletePoint (&elm, (numPoints--)-1);

                    AssocPoint  assocPoint;

                    mdlAssoc_createOrigin (&assocPoint, 0, NULL, m_cellElementID);
                    mdlDim_insertPoint (&elm, NULL, &assocPoint, 0, POINT_ASSOC);

                    UpdateDimension (m_existingElemHandle, &elm);

                    _SetupAndPromptForNextAction ();
                    }
                else
                    {
                    SetupPlacementMethod ();
                    }
                }
            /*---------------------------------------------------------------------------------****
             * @bsimethod                                                    Brien.Bastings  12/05
             +---------------+---------------+---------------+---------------+---------------+------*/
            virtual void _SetupAndPromptForNextAction ( void ) override
                {
                mdlAccuSnap_enableSnap ( _WantAccuSnap ());
                mdlAccuSnap_enableLocate ( false );

                UInt32      promptId = 0L;

                if ( !m_dtm.IsValid() )
                    promptId = PROMPT_IdentifyDTM;
                else if ( 0 == m_nPoints )
                    promptId = PROMPT_SelectSpot;
                else
                    promptId = PROMPT_AcceptReject;

                //    if ( promptId )
                mdlOutput_rscPrintf ( MSG_PROMPT, 0L, STRINGID_Message_Main, promptId );
                }

            /*---------------------------------------------------------------------------------****
             * @bsimethod                                                    Brien.Bastings  12/05
             +---------------+---------------+---------------+---------------2+---------------+------*/
            virtual void _OnDynamicFrame ( DgnButtonEventCR ev ) override
                {
                if ( m_isMultiLeader )
                    DoMultiLeaderNoteDynamics ( ev );
                //else if ( 0 == m_nPoints )
                //    DoPointDynamics ( ev );
                else
                    DoNoteDynamics ( ev );
                }

            /*---------------------------------------------------------------------------------****
             * @bsimethod                                                    Brien.Bastings  12/05
             +---------------+---------------+---------------+---------------+---------------+------*/
            static void     OnOOPsEvent (int numActions)
                {
                AnnotateSpotsElemTool   *activeTool = dynamic_cast <AnnotateSpotsElemTool *> (DgnTool::GetActivePrimitiveTool ());

                if (!activeTool)
                    return;

                if (1 == activeTool->m_nPoints)
                    activeTool->m_existingElemHandle.Invalidate ();

                if (!activeTool->m_existingElemHandle.IsValid ())
                    return;

                MSElement   elm;

                activeTool->m_existingElemHandle.GetElementP ()->CopyTo (elm);

                if (activeTool->m_numLeaders)
                    mdlDim_deletePoint (&elm, 0);
                else
                    mdlDim_deletePoint (&elm, -1);

                UpdateDimension ( activeTool->m_existingElemHandle, &elm );
                activeTool->m_nPoints--;

                activeTool->_SetupAndPromptForNextAction ();
                }

            /*---------------------------------------------------------------------------------****
             * @bsimethod                                                    Brien.Bastings  12/05
             +---------------+---------------+---------------+---------------+---------------+------*/
            virtual bool            _OnResetButton ( DgnButtonEventCR ev ) override
                {

                if ( 0 == m_nPoints )
                    {
                    startAnnotateSpotsCommand ( GetCmdNumber(), GetToolId () );
                    return false;
                    }

                if ( m_nPoints > 1 && IsManualPlacement () )
                    {
                    m_evIsSelfSnap = false;
                    NoteDimCompleteNote (ev);

                    SetupPlacementMethod ();
                    m_textElemHandle.Invalidate ();
                    return false;
                    }

                if (!(m_isMultiLeader || m_nPoints)) // Leave text and revert to initial state
                    {
                    //m_textElemHandle.Invalidate ();
                    //ResetEditor ();
                    }

                SetupPlacementMethod ();

                m_textElemHandle.Invalidate ();
                m_evIsSelfSnap = false;
                return false;
                }

            /*---------------------------------------------------------------------------------****
             * @bsimethod                                                    Brien.Bastings  12/05
             +---------------+---------------+---------------+---------------+---------------+------*/
            bool _OnDataButtonDTM ( DgnButtonEventCR ev )
                {
                DPoint3d    currPoint = *ev.GetPoint ();
                MSElement   elm;

                if (!m_evIsSelfSnap)
                    {
                    DTMDataRefPtr dtmDataRef;
                    DTMElementHandlerManager::GetDTMDataRef (dtmDataRef, m_dtm);

                    BeAssert (NULL != dtmDataRef.get());
                    if (NULL != dtmDataRef.get())
                        {
                        DMatrix4d w2vMap = GetMatrixWorldToView (ev.GetViewNum());
                        dtmDataRef->GetProjectedPointOnDTM (currPoint, m_dtm, w2vMap, *(ev.GetPoint()));
                        }
                    }

                if (m_existingElemHandle.IsValid () && m_nPoints)
                    {
                    if (mdlVec_pointEqualUOR (&currPoint, &m_points[m_nPoints-1]))
                        return false;
                    }

                if (!m_existingElemHandle.IsValid ())
                    {
                    RotMatrix   rMatrix;

                    m_firstPointView = ev.GetViewport ()->GetViewNumber ();
                    m_nPoints = 0;

                    mdlRMatrix_fromView (&rMatrix, m_firstPointView, true);
                    mdlRMatrix_normalize (&rMatrix, &rMatrix);
                    mdlRMatrix_invert (&rMatrix, &rMatrix);

                    // create dimenion element and insert first point...
                    if (SUCCESS != mdlDim_create ( &elm, NULL, &rMatrix, DimensionType::Note, m_firstPointView) ||
                        SUCCESS != mdlDim_insertPoint ( &elm, &currPoint, NULL, 0, POINT_CHECK) )
                        return true;

                    _BeginDynamics ();

                    StateCallback::SetOopsFunction (OnOOPsEvent);
                    }
                else
                    {
                    m_existingElemHandle.GetElementP ()->CopyTo (elm);
                    mdlDim_insertPoint ( &elm, &currPoint, NULL, m_isMultiLeader ? 0 : -1, POINT_STD );
                    }

                UpdateDimension (m_existingElemHandle, &elm);
                m_points[m_nPoints++] = currPoint;
                m_assocPtValid = mdlAssoc_getCurrent (&m_assocPoint, NULL, 0, ~ASSOC_CREATE_MASK_NOTE);

                // Is note complete?
                if (m_nPoints > 1)
                    {
                    if (!IsManualPlacement () || ev.IsControlKey ())
                        {
                        m_evIsSelfSnap = false;
                        NoteDimCompleteNote (ev);
                        }
                    else
                        _SetupAndPromptForNextAction ();
                    }
                //else if (m_nPoints == 1 && IsCalloutPlacement ())
                //    {
                //    NoteDimCompleteNote (ev);
                //    }
                else
                    {
                    //if ( m_nPoints == 1 )
                        {
                        // TODO: snapping
                        if (!PrepText (m_pTextBlock, m_dtm, ev))
                            {
                            --m_nPoints;
                            m_existingElemHandle.Invalidate ();
                            return false;
                            }
                        }
                        _SetupAndPromptForNextAction ();
                    }

                return false;
                }

            /*---------------------------------------------------------------------------------****
             * @bsimethod                                                    Brien.Bastings  12/05
             +---------------+---------------+---------------+---------------+---------------+------*/
            virtual void            _OnRestartTool( void ) override
                {
                AnnotateSpotsElemTool *tool = new AnnotateSpotsElemTool (GetCmdNumber (), GetToolId (), m_dtm);
                tool->InstallTool ();
                }

            /*---------------------------------------------------------------------------------****
             * @bsimethod                                                    Brien.Bastings  12/05
             +---------------+---------------+---------------+---------------+---------------+------*/
            void            OnRestartTool ( void )
                {
                SetupPlacementMethod ();
                }

            /*---------------------------------------------------------------------------------****
             * @bsimethod                                                    Brien.Bastings  12/05
             +---------------+---------------+---------------+---------------+---------------+------*/
            virtual void            _OnPostInstall (void) override
                {
                __super::_OnPostInstall ();

                SetupPlacementMethod ();
                _BeginDynamics ();
                mdlAccuSnap_enableSnap (_WantAccuSnap ());
                mdlAccuSnap_enableLocate (false);
                UpdateToolsettings (m_dtm);
                //EngageDialogFilter ();
                }

            /*---------------------------------------------------------------------------------****
            * @bsimethod                                    Daryl.Holmwood                  09/2010
            +---------------+---------------+---------------+---------------+---------------+------*/
            virtual bool    _OnPostLocate (HitPathCP path, WStringR cantAcceptReason) override
                {
                if ( !__super::_OnPostLocate ( path, cantAcceptReason ) )
                    return false;

                ElementRefP elmref = mdlDisplayPath_getElem ((DisplayPathP) path, mdlDisplayPath_getCursorIndex((DisplayPathP)path));
                BeAssert(elmref);
                DgnModelRefP modelRef = mdlDisplayPath_getPathRoot ((DisplayPathP)path);
                ElementHandle eh(elmref, modelRef);

                if ( IsValidDTMElement ( eh ))
                    {
                    m_dtm = eh;
                    return true;
                    }

                //char msg[1024] = "";
                //strncpy ( cantAcceptReason, SUCCESS == mdlResource_loadFromStringList ( msg, NULL, STRINGID_Message_Main, ERROR_NotADTM ) ? msg : "Not a DTM", 256 );

                return false;
                }

            /*---------------------------------------------------------------------------------****
            * @bsimethod                                    Piotr.Slowinski                 03/2011
            +---------------+---------------+---------------+---------------+---------------+------*/
            virtual bool _OnDataButton ( DgnButtonEventCR ev ) override
                {
                if ( !m_dtm.IsValid() )
                    {
                    bool result = __super::_OnDataButton ( ev );
                    if ( GetElementAgenda ().GetCount () > 0 )
                        {
                        m_dtm = *GetElementAgenda().GetFirstP();
                        _SetupAndPromptForNextAction ();
                        }
                    return result;
                    }
                return _OnDataButtonDTM ( ev );
                }

            /*---------------------------------------------------------------------------------****
            * @bsimethod                                    Daryl.Holmwood                  09/2010
            +---------------+---------------+---------------+---------------+---------------+------*/
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
// ToDo Vancouver                    if(ICommandUtil::HaveFactory(*curr, CmdDTMAnnotateSpots))
// ToDo Vancouver                        continue;

                    if(!IsValidDTMElement(*curr))
                        {
                        curr->Invalidate();
                        modify=true;
                        }
                    }
                return modify;
                }

            /*---------------------------------------------------------------------------------****
            * @bsimethod                                    Piotr.Slowinski                 03/2011
            +---------------+---------------+---------------+---------------+---------------+------*/
            void            DoPointDynamics ( DgnButtonEventCP ev )
                {
                }

            /*---------------------------------------------------------------------------------****
            * @bsimethod                                    Piotr.Slowinski                 03/2011
            +---------------+---------------+---------------+---------------+---------------+------*/
            virtual void    _HiliteAgendaEntries ( bool changed ) override {}

            /// <author>Piotr.Slowinski</author>                            <date>09/2011</date>
            virtual void        _OnBeginSequence (void) override
                {
                StartSequence (GetCmdNumber());
                }

            virtual void        _OnEndSequence (void) override
                {
                EndSequence (GetCmdNumber());
                }

    }; // End AnnotateSpotsElemTool struct

/// <summary>
/// blah, blah
/// </summary>
/// <param name="cellElemHandle">cellElemHandle</param>
/// <param name="dimElemHandle">dimElemHandle</param>
/// <param name="originP">originP</param>
/// <param name="isDynamics">isDynamics</param>
/// <remarks> stolen from PlaceNoteTool::CreateNote
/// see $(MSJ)mstn\mdlapps\wordproc\wordnote.cpp </remarks>
/// <returns></returns>
/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
void            AnnotateSpotsElemTool::SetupPlacementMethod ( void )
    {
    m_nPoints           = 0;
    m_numLeaders        = 0;
    m_iGGNum            = 0;
    m_firstPointView    = -1;
    m_assocPtValid      = false;
    m_isMultiLeader     = false;
    m_horAttachmentSide = TextElementJustification::LeftTop;
    m_cellElementID     = INVALID_ELEMENTID;

    memset ( &m_leaderEndTangent, 0, sizeof ( m_leaderEndTangent ) );
    m_existingElemHandle.Invalidate ();
    // Open editor dialog right away...
    // OpenEditor ();
    m_textElemHandle.Invalidate ();
    _SetupAndPromptForNextAction ();
    StateCallback::SetOopsFunction (NULL);
    }
/*----------------------------------------------------------------------------------**
* @bsimethod                                                    PaulChater      05/01
+---------------+---------------+---------------+---------------+---------------+------*/    
TextElementJustification AnnotateSpotsElemTool::NoteGetDynamicJustification
    (
    bool* bHorJustOverriddenByCursor,
    int	  viewNum,
    bool  isDynamics
    )
    {
    if (bHorJustOverriddenByCursor)
        *bHorJustOverriddenByCursor = false;

    if (m_nPoints <= 1)
        return TextElementJustification::LeftTop;

    RotMatrix       rMatrix;

    mdlRMatrix_fromView (&rMatrix, viewNum, true);

    TextElementJustification just = TextElementJustification::LeftTop;
    DPoint3d                 pt1, pt2;

    if (IsManualPlacement ())
        {
        pt1 = m_points[m_nPoints -2];
        pt2 = m_points[m_nPoints -1];
        just = ComputeDynamicJustification (&pt1, &pt2, &rMatrix);

        // When completing (resetting) a note in manual placement mode,
        // use the dynamic cursor location to determine dynamic just.
        if (bHorJustOverriddenByCursor && !isDynamics)
            {
            TextElementJustification justByPoints = TextElementJustification::LeftTop;

            pt1  = m_points[m_nPoints-1];
            pt2  = m_dynamicPoint;

            justByPoints = ComputeDynamicJustification (&pt1, &pt2, &rMatrix);
            *bHorJustOverriddenByCursor = (HORJUSTMODE (just) != HORJUSTMODE (justByPoints));
            just = justByPoints;
            }
        }
    else
        {            
        pt1 = m_points[m_nPoints -2];
        pt2 = m_points[m_nPoints -1];

        just = ComputeDynamicJustification (&pt1, &pt2, &rMatrix);
        }        
    return just;
    }

/*----------------------------------------------------------------------------------***
 * @bsimethod                                                    SunandSandurkar 07/03
 +---------------+---------------+---------------+---------------+---------------+------*/    
void            AnnotateSpotsElemTool::AdjustNote
    (
    EditElementHandleR dimElemHandle,
    DimensionStyleP dimStyle,
    bool            isDynamics
    )
    {
    /*-----------------------------------------------------------
    When attachment is dynamic, we provide a way for users to
    specify the attachment side at reset time. We need to set
    the specific shields so a regenerate does not reverse the
    effect.
    -----------------------------------------------------------*/
    TextElementJustification dynamicJust = TextElementJustification::LeftTop;
    int         horizontalJust = 0, verticalJust = 0;
    int         horAttachment = 0;
    bool        bDynamicHorJustByCursor = false, bHorJustOverridden = false, bHorAttachmentOverridden = false;
    MSElement   elm;

    dimElemHandle.GetElementP()->CopyTo (elm);
    dimStyle->GetIntegerProp (horizontalJust, DIMSTYLE_PROP_MLNote_Justification_INTEGER);
    dimStyle->GetIntegerProp (verticalJust, DIMSTYLE_PROP_MLNote_VerticalJustification_INTEGER);

    dynamicJust = NoteGetDynamicJustification (&bDynamicHorJustByCursor, m_firstPointView, isDynamics);

    UpdateDimension (dimElemHandle, &elm);

    if (HORJUSTMODE_DYNAMIC == horizontalJust && bDynamicHorJustByCursor)
        {
        int     just = HOR_LEFT (dynamicJust) ? DIMSTYLE_VALUE_MLNote_Justification_Left : DIMSTYLE_VALUE_MLNote_Justification_Right;

        mdlDim_setNoteHorizontalJustification (dimElemHandle, (DimStyleProp_MLNote_Justification) just);
        bHorJustOverridden = true;
        }

    switch (horAttachment)
        {
        case DIMSTYLE_VALUE_MLNote_HorAttachment_Right:
            {
            m_horAttachmentSide = TextElementJustification::RightTop;
            break;
            }

        case DIMSTYLE_VALUE_MLNote_HorAttachment_Left:
            {
            m_horAttachmentSide = TextElementJustification::LeftTop;
            break;
            }

        case DIMSTYLE_VALUE_MLNote_HorAttachment_Auto:
            {
            m_horAttachmentSide = dynamicJust;

            if (bDynamicHorJustByCursor)
                {
                UInt16  attach = (HOR_LEFT (dynamicJust) ? DIMSTYLE_VALUE_MLNote_HorAttachment_Left : DIMSTYLE_VALUE_MLNote_HorAttachment_Right);

                mdlDim_setNoteHorAttachment (dimElemHandle, &attach);
                bHorAttachmentOverridden = true;
                }
            break;
            }
        }

    // Set any manual overrides and update element handle...
    if (bHorJustOverridden || bHorAttachmentOverridden )
        {
        if (!isDynamics)
            {
            DimStylePropMaskPtr overrides = DimensionHandler::GetInstance().GetOverrideFlags(dimElemHandle);

            if (bHorAttachmentOverridden)
                overrides->SetPropertyBit (DIMSTYLE_PROP_MLNote_HorAttachment_INTEGER, true);

            if (bHorJustOverridden)
                overrides->SetPropertyBit (DIMSTYLE_PROP_MLNote_Justification_INTEGER, true);

            mdlDim_setOverridesDirect (dimElemHandle, overrides.get(), false);
            }
        }
    }
/*---------------------------------------------------------------------------------****
 * @bsimethod                                                    SunandSandurkar  07/03
 +---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       AnnotateSpotsElemTool::CreateNote
    (
    EditElementHandleR cellElemHandle,
    EditElementHandleR dimElemHandle,
    DPoint3d*       originP,
    bool            isDynamics,
    DgnButtonEventCP ev
    )
    {
    // Update the dim element with the current dimstyle settings by synch'ing tcb info
    // NOTE: The reason we are doing this before inserting the last point to the 
    // dimension is because the text node creation logic needs to get the final resolved
    // text height and width values stored in the dimension element. Since there is
    // no notes-specific setting coming from the dimstyle that needs to be stored per
    // point, we can insert the points later.
    DimensionStylePtr   dimStyle = DimensionStyle::GetActive();

    DimensionHandler::GetInstance().ApplyDimensionStyle (dimElemHandle, *dimStyle.get(), true);

    if (!m_textElemHandle.IsValid ()) 
        {
        if (isDynamics)
            {
            if (m_evIsSelfSnap)
                {
                DPoint3d globalOrigin;

                if (ev != NULL)
                    {
                    mdlModelRef_getGlobalOrigin (m_dtm.GetModelRef (), &globalOrigin);

                    double elevation = m_elevationPoint.z - globalOrigin.z;

                    // If the terrain model is an attachment, convert the snapped point to the attached reference model units.
                    if (m_dtm.GetModelRef ()->IsDgnAttachment ())
                        {
                        DPoint3d    refModelPoint = *(ev->GetPoint ());
                        CRefUnitsConverter converter (m_dtm.GetModelRef (), true);
                        converter.FullRootUorsToRefMeters (refModelPoint);

                        elevation = (refModelPoint.z - globalOrigin.z);
                        }

                    ConvertToTextBlock (m_pTextBlock, elevation, ev ? ev->GetViewport () : m_ev.GetViewport (), m_dtm.GetModelRef ());
                    BeAssert (m_pTextBlock.IsValid ());
                    if (m_pTextBlock.IsValid ())
                        m_pTextBlock->SetForceTextNodeFlag (true);
                    }
                }
            else
                { PrepText (m_pTextBlock, m_dtm, ev ? *ev : m_ev); }
            }
        if (m_pTextBlock.IsNull())
            { return ERROR; }

        TextHandlerBase::CreateElement (m_textElemHandle, nullptr, *m_pTextBlock);
        // If created new text node set node number to current text node number!
        if (m_textElemHandle.IsValid () && TEXT_NODE_ELM == m_textElemHandle.GetElementType ())
            {
            if (0 == m_textElemHandle.GetElementP ()->text_node_2d.nodenumber)
                m_textElemHandle.GetElementP ()->text_node_2d.nodenumber = tcb->canode;
            }
        }

    // Adjust the note settings and shields that depend on the current cursor location
    AdjustNote (dimElemHandle, dimStyle.get(), isDynamics);

    if (SUCCESS == mdlNote_create (cellElemHandle, &m_textElemHandle, originP, dimElemHandle, NULL, ACTIVEMODEL, false, NULL, NULL) && NULL != cellElemHandle.PeekElementDescrCP ())
        {
        if (!isDynamics)
            {
            // Set the level of all components to dimension's level
            bool levelOverride;
            dimStyle->GetBooleanProp (levelOverride, DIMSTYLE_PROP_Placement_OverrideLevel_BOOLINT);

            UInt32          level   = tcb->activeLevel;
            if (levelOverride)
                dimStyle->GetLevelProp (level, DIMSTYLE_PROP_Placement_Level_LEVEL);
            EditElementHandle textNode;

            mdlNote_findCellComponent (textNode, TEXT_NODE_ELM, cellElemHandle);
            MSElementDescrP dscr = textNode.GetElementDescrP();

            mdlElement_setProperties (&dscr->el, &level, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
            }

        return SUCCESS;
        }

    return ERROR;
    }

/*----------------------------------------------------------------------------------***
 * @bsimethod                                                    SunandSandurkar 07/03
 +---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotateSpotsElemTool::SetLeaderEndTangent
    (
    EditElementHandleR                  dimElemHandle,
    DPoint3dP                           cursorP,
    DPoint3dP                           prevPointP,
    TextElementJustification            horAttachmentSide,
    int                                 view
    )
    {
    int             rotation = 0;
    DVec3d          endTangent;
    RotMatrix       dimMatrixR, dimMatrixC;

    if (!dimElemHandle.IsValid ())
        return;

    DimensionHandler::GetInstance().GetRotationMatrix (dimElemHandle, dimMatrixC);
    mdlRMatrix_transpose (&dimMatrixR, &dimMatrixC);
    DimensionStylePtr dimStyle = DimensionStyle::GetActive ();
    dimStyle->GetIntegerProp (rotation, DIMSTYLE_PROP_MLNote_TextRotation_INTEGER);

    // Note : Tangents need to point into the curve
    switch (rotation)
        {
        case DIMSTYLE_VALUE_MLNote_TextRotation_Inline:
            {
            mdlVec_computeNormal (&endTangent, prevPointP, cursorP);
            break;
            }

        case DIMSTYLE_VALUE_MLNote_TextRotation_Vertical:
            {
            // Set end tangent left side attachment case
            endTangent.x = 0.0;
            endTangent.y = -1.0;
            endTangent.z = 0.0;

            // Flip end tangent if attachment is on right side
            if (!HOR_LEFT (horAttachmentSide))
                mdlVec_scale (&endTangent, &endTangent, -1.0);

            mdlRMatrix_unrotatePoint (&endTangent, &dimMatrixR);
            break;
            }

        case DIMSTYLE_VALUE_MLNote_TextRotation_Horizontal:
        default:
            {
            // Set end tangent left side attachment case
            endTangent.x = -1.0;
            endTangent.y = 0.0;
            endTangent.z = 0.0;

            // Flip end tangent if attachment is on right side
            if (!HOR_LEFT (horAttachmentSide))
                mdlVec_scale (&endTangent, &endTangent, -1.0);

            mdlRMatrix_unrotatePoint (&endTangent, &dimMatrixR);
            break;
            }
        }

    mdlDim_segmentSetCurveEndTangent (dimElemHandle, 0, &endTangent);
    }

/*---------------------------------------------------------------------------------*
* @bsimethod                                                    Brien.Bastings  12/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotateSpotsElemTool::DoNoteDynamics ( DgnButtonEventCR ev )
    {
    int         view = ev.GetViewport ()->GetViewNumber ();
    DPoint3d    currPoint = *ev.GetPoint ();

    // prepare the leader dimension
    if (2 > m_nPoints)
        {
        RotMatrix   rMatrix;

        mdlRMatrix_fromView (&rMatrix, view, true);
        mdlRMatrix_normalize (&rMatrix, &rMatrix);
        mdlRMatrix_invert (&rMatrix, &rMatrix);

        if (!m_existingElemHandle.IsValid ())
            {
            //if (!IsPlacementReversed ())
            //    return;

            // If there are no data points collected but we got into dynamics, it means
            // we are in Start-At-Text mode. In order to draw the note cell in dynamics,
            // we need to create the corresponding dimension element.
            MSElement elm;
            mdlDim_create (&elm, NULL, &rMatrix, DimensionType::Note, view);
            UpdateDimension (m_existingElemHandle, &elm);
            }
        else
            {
            // User can change accudraw plane after placing the first point. Allow the 
            // changing of dimrotmatrix until the second point is placed.
            mdlDim_setDimRotMatrix (m_existingElemHandle, &rMatrix);
            }

        }

    if (!m_existingElemHandle.IsValid ())
        return;

    if (m_nPoints && mdlVec_pointEqualUOR (&m_points[m_nPoints-1], &currPoint))
        return;

    m_points[m_nPoints++] = currPoint;

    // Since the cursor point cannot be obtained at reset time, save the most recent
    // dynamic point. When completing the note, this point will be used to determine
    // the settings that are dependent on the reset point.
    m_dynamicPoint = currPoint;

    int         pointNo = -1;
    bool        placingDim = true;
    DPoint3d    pt = currPoint, cursor, prevPoint;

    cursor	  = currPoint;
    prevPoint = m_points[m_nPoints-2];

    MSElementDescrP dimEdP = NULL;

    mdlElmdscr_duplicate (&dimEdP, m_existingElemHandle.GetElementDescrP ());

    EditElementHandle  dimElemHandle (dimEdP, true, false, ACTIVEMODEL);

    // before creating cell, add the necessary points to leader since they are needed to determine flipping, rotation.
    if (placingDim)
        {
        MSElement elm;
        dimElemHandle.GetElementP ()->CopyTo (elm);
        mdlDim_insertPoint (&elm, &currPoint, NULL, pointNo, POINT_STD);
        UpdateDimension (dimElemHandle, &elm);
        }

    RedrawElems redrawElems;

    redrawElems.SetDrawMode (DRAW_MODE_TempDraw);
    redrawElems.SetDrawPurpose (DrawPurpose::Dynamics);
    redrawElems.SetDynamicsViews (IViewManager::GetActiveViewSet(), ev.GetViewport ());
    redrawElems.SetRedrawOp (this);

    EditElementHandle  cellElemHandle;
    bool IS_DYNAMICS = true;

    // draw note cell
    if (SUCCESS == CreateNote(cellElemHandle, dimElemHandle, &pt, IS_DYNAMICS,  m_nPoints < 2 ? &Prepare(ev) : nullptr))
        {
        redrawElems.DoRedraw (cellElemHandle);
        m_textElemHandle.Invalidate ();
        }

    // draw leader
    if (placingDim)
        {
        UInt16  leaderType = 0;

        if (mdlDim_getNoteLeaderType (&leaderType, dimElemHandle) && 1 == leaderType)
            {
            SetLeaderEndTangent (dimElemHandle, &cursor, &prevPoint, m_horAttachmentSide, view);
            mdlDim_segmentGetCurveEndTangent (&m_leaderEndTangent, dimElemHandle, 0);
            }

        redrawElems.DoRedraw (dimElemHandle);
        }

    m_nPoints--;        
    }

/*---------------------------------------------------------------------------------****
* @bsimethod                                                    Brien.Bastings  12/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotateSpotsElemTool::DoMultiLeaderNoteDynamics ( DgnButtonEventCR ev )
    {

    if (!m_existingElemHandle.IsValid ())
        return;

    DPoint3d    currPoint = *ev.GetPoint ();
    MSElement   elm;

    m_existingElemHandle.GetElementP ()->CopyTo (elm);

    /* NOTE: For multileaders, we make sure that the first m_points is the text point. 
    Irrespective of the reversed flag, all subsequent multileaders are drawn 
    backwards starting at the text point. */

    // insert the point in the dimension leader
    mdlDim_insertPoint (&elm, &currPoint, NULL, 0, POINT_STD);

    EditElementHandle  dimElemHandle;

    UpdateDimension (dimElemHandle, &elm);

    UInt16          leaderType = 0;

    if (mdlDim_getNoteLeaderType (&leaderType, dimElemHandle) && 1 == leaderType)
        mdlDim_segmentSetCurveEndTangent (dimElemHandle, 0, &m_leaderEndTangent);

    RedrawElems redrawElems;

    redrawElems.SetDrawMode (DRAW_MODE_TempDraw);
    redrawElems.SetDrawPurpose (DrawPurpose::Dynamics);
    redrawElems.SetDynamicsViews (IViewManager::GetActiveViewSet(), ev.GetViewport ());
    redrawElems.SetRedrawOp (this);

    redrawElems.DoRedraw (dimElemHandle);
    }

/*---------------------------------------------------------------------------------****
 * @bsimethod                                                    PaulChater      05/01
 +---------------+---------------+---------------+---------------+---------------+------*/
StatusInt AnnotateSpotsElemTool::WriteDimensionElement (EditElementHandleR dimElm)
    {
    bool compatMode;
    StatusInt status = SUCCESS;

    ActiveParams::GetValue (compatMode, ACTIVEPARAM_DIMCOMPAT);

    dimElm.SetModelRef (ACTIVEMODEL);
    /*-------------------------------------------------------------------
    Check the setting of the current dimension compatibility mode.
    If dimension compatibility is ON, write the dimension to the file
    as IGDS 8.8 compatible primitives.
    If dimension compatibility is OFF, validate the dimension element
    and add it to the file.
    -------------------------------------------------------------------*/
    if (!compatMode)
        {
        DimStylePropMaskPtr overrides = DimensionHandler::GetInstance().GetOverrideFlags (dimElm);

        // ToDo Vancouver mdlDim_setShieldsFromActiveSettings (dimElm);
        // ToDo Vancouver DimensionHandler::SetShieldsFromStyle
        if (overrides->AnyBitSet())
            {
            DimStylePropMaskPtr newOverrides = DimensionHandler::GetInstance().GetOverrideFlags (dimElm);

            DimStylePropMask::LogicalOperation (*newOverrides.get(), *overrides.get(), BitMaskOperation::Or);
            mdlDim_setOverridesDirect (dimElm, newOverrides.get(), false);
            }

        
        status = DimensionHandler::GetInstance().ValidateElementRange (dimElm, true);
        }
    if (status == SUCCESS)
        status = dimElm.AddToModel ();

    if (SUCCESS == status)
        {
        //mdlTemplateManager_attachToElement (dimElm.GetElementRef()); // add active element template
        }

    return status;
    }

/*---------------------------------------------------------------------------------****
 * @bsimethod                                                    SunandSandurkar 05/00
 +---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotateSpotsElemTool::FinishMultiLeaderNote ( DgnButtonEventCR ev )
    {
    if (1 >= m_nPoints)
        return;

    MSElement   elm;

    m_existingElemHandle.GetElementP ()->CopyTo (elm);

    // associate dimension with snapped element
    if (m_assocPtValid)
        {
        mdlDim_deletePoint (&elm, 0);
        mdlDim_insertPoint (&elm, NULL, &m_assocPoint, 0, POINT_ASSOC);
        }

    mdlElement_setProperties (&elm, NULL, &m_iGGNum, NULL, NULL, NULL, NULL, NULL, NULL);

    UInt16 leaderType = 0;
    EditElementHandle dimElm (&elm, ACTIVEMODEL);

    if (mdlDim_getNoteLeaderType (&leaderType, dimElm) && 1 == leaderType)
        mdlDim_segmentSetCurveEndTangent (dimElm, 0, &m_leaderEndTangent);

    WriteDimensionElement (dimElm);

    m_numLeaders++;
    }

/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
void StartMainAnnotateSpotsCommand (ElementHandleCR dtm, CommandNumber cmdNumer)
    {
    AnnotateSpotsElemTool*                  tool;

    LoadDependable ();
    tool = new AnnotateSpotsElemTool (cmdNumer, CMDNAME_LabelSpot, dtm);
    tool->InstallTool ();
    }

/*---------------------------------------------------------------------------------****
* @bsimethod                                                    Daryl.Holmwood  07/10
+---------------+---------------+---------------+---------------+---------------+------*/
void startAnnotateSpotsCommand (CommandNumber cmdNumber, int cmdName)
    {
    Annotate::SelectDTMElemTool*    tool;
    bool const                      REQUIRES_CONTOURS = true;

    LoadDependable ();
    tool = new Annotate::SelectDTMElemTool (cmdNumber, cmdName, PROMPT_IdentifyDTM,
        StartMainAnnotateSpotsCommand, !REQUIRES_CONTOURS);
    tool->InstallTool ();
    }
