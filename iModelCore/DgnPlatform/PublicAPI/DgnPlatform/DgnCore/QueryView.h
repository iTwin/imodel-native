/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/QueryView.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnCore/QueryModel.h>
#include <DgnPlatform/DgnCore/ViewController.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Populates a QueryModel with \ref ElementRefGroup from a SQL query. The query can combine 
//! spatial criteria with business and graphic criteria.
//!
//! @remarks QueryViewController is also used to produce graphics for picking and for purposes other than display.
// @bsiclass                                                    Keith.Bentley   07/12
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE QueryViewController : CameraViewController, BeSQLite::VirtualSet
{
    DEFINE_T_SUPER (CameraViewController)

#if !defined (DOCUMENTATION_GENERATOR)
//__PUBLISH_SECTION_END__
    friend struct QueryModel::Selector;
//__PUBLISH_SECTION_START__
protected:

    bool        m_notifyOnViewUpdated;
    bool        m_forceNewQuery;    //!< If true, before doing the next view update, repopulate the QueryModel with the result of the query 
    bool        m_noQuery;          //!< If true, *only* draw the "always drawn" list - do not query for other elements
    bool        m_selectProcessingActive;
    UInt64      m_lastUpdateTime;
    double      m_lastQueryTime;
    double      m_fps;
    DrawPurpose m_lastUpdateType;
    DRange3d    m_secondaryVolume;  //  ignored unless m_secondaryHitLimit > 0
    UInt32      m_secondaryHitLimit;
    UInt32      m_intermediatePaintsThreshold;
    UInt32      m_maxToDrawInDynamicUpdate;
    UInt32      m_maxDrawnInDynamicUpdate;
    Frustum     m_startQueryFrustum;
    Frustum     m_saveQueryFrustum;
    QueryModelR m_queryModel;
    ElementIdSet m_alwaysDrawn;
    ElementIdSet m_neverDrawn;

    void ComputeFps ();
    DGNPLATFORM_EXPORT void EmptyQueryModel();
    void QueryModelExtents(DRange3dR, ViewportR);

    //! Populate the QueryModel with the results of the query.
    void LoadElementsForUpdate (ViewportR viewport, DrawPurpose updateType, ICheckStopP checkStop, bool needNewQuery, bool waitForQueryToFinish, bool stopQueryOnAbort);
    void SaveSelectResults();
    void StartSelectProcessing (ViewportR, DrawPurpose updateType);
    DGNPLATFORM_EXPORT virtual bool _IsInSet (int nVal, BeSQLite::DbValue const*) const override;
    virtual DRange3d _ShowTxnSummary(TxnSummaryCR summary) override {m_forceNewQuery=true; return T_Super::_ShowTxnSummary(summary);}
    virtual void _FillModels() override {} // query models do not load elements in advance
    DGNPLATFORM_EXPORT virtual void _OnAttachedToViewport(ViewportR) override;

protected:
    //! Called at the beginning of a healing update to populate the QueryModel.
    //! @param[in]  viewport    The viewport that will display the graphics
    //! @param[in]  context     The context that is processing the graphics.
    //! @param[in]  fullHeal    if true, this heal is of the entire viewport. Otherwise, just a portion of the viewport is being healed.
    //! @remarks Applications that override this method normally perform any additional work that is required and then 
    //! call QueryViewController::_OnHealUpdate to let it decide if is necessary to repopulate the QueryModel.
    //! @remarks An application may use this and _OnFullUpdate to decide when to display some indication such as a spinner to 
    //! let the user know that the update is in progress.  The application can override PhysicalViewController::_OnUpdateComplete to stop the spinner.
    DGNPLATFORM_EXPORT virtual void _OnHealUpdate (ViewportR viewport, ViewContextR context, bool fullHeal) override;

    //! Called at the beginning of a full update to populate the QueryModel.
    //! @param[in] viewport    The viewport that will display the graphics
    //! @param[in] context     The context that is processing the graphics.
    //! @param[in] info        Options
    //! @remarks Applications that override this method normally perform any additional work that is required and then call QueryViewController::_OnFullUpdate to 
    //!  let it decide if is necessary to repopulate the QueryModel.
    //! @remarks An application may use this and _OnFullUpdate to decide when to display some indication such as a spinner to 
    //! let the user know that the update is in progress.  The application can override PhysicalViewController::_OnUpdateComplete
    //! to know when to stop the spinner.
    DGNPLATFORM_EXPORT virtual void _OnFullUpdate (ViewportR viewport, ViewContextR context, FullUpdateInfo& info) override;

    //! Called at the beginning of a dynamic update to populate the QueryModel.
    //! @param[in]  viewport    The viewport that will display the graphics
    //! @param[in]  context     The context that is processing the graphics.
    //! @param[in]  info        Options
    //! @remarks  Although an application can override this method, the decision on whether or not to repopulate the QueryModel in a dynamic update is typically left to
    //! QueryViewController::_OnDynamicUpdate. It in turn defers the decision to _WantElementLoadStart.
    DGNPLATFORM_EXPORT virtual void _OnDynamicUpdate (ViewportR viewport, ViewContextR context, DynamicUpdateInfo& info) override;

    //! QueryViewController uses this to determine if it should start another background query to repopulate the query model.
    //! QueryViewController calls this from _OnDynamicUpdate and when it detects that the background element query processing is idle during a dynamic update.
    //! @param[in] viewport    The viewport that will display the graphics
    //! @param[in] currentTime The current time in seconds.
    //! @param[in] lastQueryTime The time the last query was started.
    //! @param[in] maxElementsDrawnInDynamicUpdate The maximum number of elements drawn in any dynamic frame since the QueryModel was last populated.
    //! @param[in] queryFrustum The frustum used in the last range query used to populate the QueryModel.
    //! @returns  Return true to start another round.
    //! @remarks It is very rare than an application needs to override this method.
    DGNPLATFORM_EXPORT virtual bool _WantElementLoadStart (ViewportR viewport, double currentTime, double lastQueryTime, UInt32 maxElementsDrawnInDynamicUpdate, Frustum const& queryFrustum);

    //! Called when the visibility of a level is changed.
    DGNPLATFORM_EXPORT virtual void _OnLevelChange(bool singleEnabled) override;

    //! Called when the display of a model is changed on or off
    //! @param modelId  The model to turn on or off.
    //! @param onOff    If true, elements in the model are candidates for display; else elements in the model are not displayed.
    DGNPLATFORM_EXPORT virtual void _ChangeModelDisplay (DgnModelId modelId, bool onOff) override;

    //! Draw the elements in the query model.
    //! @param context The context that is processing the graphics. Sometimes this is a ViewContext, when the output is a Viewport. Sometimes, this is a PickContext, when
    //! the purpose is to identify an element or snap location.
    //! @remarks It not normally necessary for apps to override this function.
    DGNPLATFORM_EXPORT virtual void _DrawView (ViewContextR context) override;

    //! Return the default maximum number of elements to load. This is then scaled by the value returned from _GetMaxElementFactor.
    virtual UInt32 _GetMaxElementsToLoad () {return 5000;}

    DGNPLATFORM_EXPORT virtual void _RestoreFromSettings (JsonValueCR) override;
#endif

protected:
    //! The premise of a QueryModel is that it holds only a small subset of the potential elements in a DgnProject, limited to a maximum number of 
    //! elements and bytes. Obviously, the criteria that determines which elements are loaded at a given time must combine business logic (e.g. elements that meet 
    //! a certain property test), display logic (e.g. which models and levels are turned on), plus spatial criteria (i.e. the position of the camera).
    //! Further, assuming more than the maximum number of elements meet all the search criteria, the candidate elements should be sorted such that the "best"
    //! set of elements are returned.
    //! <p> This method is used to obtain an SQL statement to achieve that goal. The "best set" of elements are determined 
    //! using a spatial scoring algorithm that traverses the persistent range tree (an RTree in SQLite), and scores elements based on an approximate number of
    //! pixels occluded by its axis aligned bounding box (AABB - aka "range box"). The SQL returned by the base-class implementation of this method contains 
    //! logic to affect that purpose, plus filters for level and models. To add additional, application-specific criteria to the query, override this method, call
    //! T_Super::_GetRTreeMatchSql, and append your filters as additional "AND" clauses on that string. Then, return the new combined SQL statement.
    //! @param viewport The viewport where the query model is to be displayed.
    //! @note It is very important that the "rTreeMatch" function be the primary discriminator in the query plan. To ensure that, you must always precede
    //! column names in your SQL with a "+" to force SQLite to ignore any indexes on that column.
    /**
       $SAMPLECODE_BEGIN[QueryView_GetRTreeMatchSql,Example]
__PUBLISH_INSERT_FILE__  QueryView_GetRTreeMatchSql.sampleCode
       $SAMPLECODE_END
     */
    DGNPLATFORM_EXPORT virtual Utf8String _GetRTreeMatchSql (ViewportR viewport);

    DGNPLATFORM_EXPORT void BindModelAndLevel (BeSQLiteStatementR stmt) const;

    //! Compute the range of the elements and graphics in the QueryModel.
    //! @remarks This function may also load elements to determine the range.
    //! @param[out] range    the computed range 
    //! @param[in]  viewport the viewport that will display the graphics
    //! @param[in]  params   options for computing the range.
    //! @return \a true if the returned \a range is complete. Otherwise the caller will compute the tightest fit for all loaded elements.
    DGNPLATFORM_EXPORT virtual bool _OnComputeFitRange (DRange3dR range, ViewportR viewport, FitViewParamsR params) override;

    //! Return a value in the range -100 (fewest) to 100 (most) to determine the maximum number of elements loaded by the query.
    //! 0 means the "default" number of elements.
    virtual Int32 _GetMaxElementFactor() {return 0;}

    //! Return the size in pixels of the smallest element that should be displayed.
    virtual double _GetMinimumSizePixels (DrawPurpose updateType) {return 0.1;}

    //! Return the maximum number of bytes of memory that should be used to hold loaded element data. Element data may exceed this limit at times and is trimmed back at intervals.
    //! It is recommended that applications use this default implementation and instead control memory usage by overriding _GetMaxElementFactor
    virtual UInt64 _GetMaxElementMemory () {return GetMaxElementMemory();}

public:
    //! Construct the view controller.                          
    //! @param project  The project of the view
    //! @param viewId   Identifies the view that will be displayed
    DGNPLATFORM_EXPORT QueryViewController (DgnProjectR project, DgnViewId viewId);
    DGNPLATFORM_EXPORT ~QueryViewController();

//__PUBLISH_SECTION_END__
    void SetForceNewQuery(bool newValue) { m_forceNewQuery = newValue; }
//__PUBLISH_SECTION_START__

    //! Return the maximum number of bytes of memory that should be used to hold loaded element data. Element data may exceed this limit at times and is trimmed back at intervals.
    DGNPLATFORM_EXPORT UInt64 GetMaxElementMemory();

    //! Return the maximum number of elements to hold in the associated QueryModel.
    DGNPLATFORM_EXPORT UInt32 GetMaxElementsToLoad ();

    //! Get the list of elements that are always drawn
    ElementIdSet const& GetAlwaysDrawn() {return m_alwaysDrawn;}

    //! Establish a set of elements that are always drawn in the view.
    DGNPLATFORM_EXPORT void SetAlwaysDrawn(ElementIdSet const&, bool exclusive);

    DGNPLATFORM_EXPORT void ClearAlwaysDrawn();

    //! Get the list of elements that are never drawn.
    //! @remarks An element in the never-draw list is excluded regardless of whether or not it is 
    //! in the always-draw list. That is, the never-draw list gets priority over the always-draw list.
    ElementIdSet const& GetNeverDrawn() {return m_neverDrawn;}

    DGNPLATFORM_EXPORT void SetNeverDrawn(ElementIdSet const&);
    DGNPLATFORM_EXPORT void ClearNeverDrawn();

    //! Gets the QueryModel that this QueryViewController uses.
    QueryModelR GetQueryModel() const {return m_queryModel;}

    //! Enables a secondary range query.
    DGNPLATFORM_EXPORT void EnableSecondaryQueryRange(UInt32 hitLimit, DRange3dCR volume);

    //! Disables secondary range query.
    void DisableSecondaryQueryRange(){m_secondaryHitLimit=0;}

    //! Return a counter that indicates when the last query was run. This counter is relative to some unspecified start time, 
    //! but can conceptually be thought of as the number of seconds since the process started.
    //! @see BeTimeUtilities::QuerySecondsCounter
    //! @note this method can be used to determine if related caches need to be updated
    double GetLastQueryCounter() {return m_lastQueryTime;}
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
