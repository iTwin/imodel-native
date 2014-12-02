/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/QueryModel.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnModel.h"
#include <Bentley/BeThread.h>
#include <DgnPlatform/DgnCore/ViewContext.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
struct DgnDbRTree3dViewFilter;
//__PUBLISH_SECTION_START__
                                                                                                                    
//=======================================================================================
/**
A QueryModel is a virtual DgnModel that holds \ref ElementRefGroup loaded from the database according to a custom query criteria.
A QueryModel caches the results of the query.

A QueryModel is used in conjunction with a QueryViewController to display the results of the query. 
Applications do not directly deal with QueryModel's. Instead, the query that populates them is supplied by a QueryViewController.

The method MobileDgn::MobileDgnApplication::OpenProject creates a default QueryModel for an application. 
Applications may use MobileDgn::MobileDgnApplication::GetQueryModel to retrieve a reference to that QueryModel. 
QueryModel's are associated with a QueryViewController by passing a QueryModel to the QueryViewController constructor.
*/
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct QueryModel : PhysicalModel
{
//__PUBLISH_SECTION_END__
    friend struct QueryViewController;

    //=======================================================================================
    // @bsiclass                                                     JohnGoding      05/12
    //=======================================================================================
    struct Results
    {
        bvector<PersistentElementRefP> m_elements;
        bvector<PersistentElementRefP> m_closeElements;
        bool   m_reachedMaxElements;
        bool   m_eliminatedByLOD;
        UInt32 m_drawnBeforePurge;
        double m_lowestOcclusionScore;

        UInt32 GetCount() const {return (UInt32) m_elements.size();}
        Results() {m_drawnBeforePurge=0; m_reachedMaxElements=false; m_lowestOcclusionScore=0; m_eliminatedByLOD=false;}
    };

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   07/12
    //=======================================================================================
    struct Selector : ICheckStop
    {
        friend struct QueryModel;
        enum class State
            {
            Inactive            = 0,
            ProcessingRequested = 1,
            Processing          = 2,
            HandlerError        = 3,
            AbortRequested      = 4,
            Aborted             = 5,
            Completed           = 6,
            TerminateRequested  = 7,
            Terminated          = 8
            };

    private:
        DgnProjectR         m_project;
        State               m_state;
        BeConditionVariable m_conditionVariable;
        Utf8String          m_elementLoadSql;
        Utf8String          m_searchSql;
        ViewportCP          m_viewport;
        UInt32              m_maxElements;
        double              m_minimumPixels;
        UInt64              m_maxMemory;
        Results*            m_results;
        BeSQLite::DbResult  m_dbStatus;
        bool                m_restartRangeQuery;
        bool                m_inRangeSelectionStep;
        bool                m_noQuery;
        Frustum             m_frustum;
        QueryViewControllerCP m_controller;
        ElementIdSet*       m_alwaysDraw;
        ElementIdSet*       m_neverDraw;
        ClipVectorPtr       m_clipVector;
        UInt32              m_secondaryHitLimit;
        DRange3d            m_secondaryVolume;
       
        virtual bool _CheckStop () override;
        void qt_ProcessRequest();
        void qt_NotifyCompletion();
        void qt_SearchIdSet(ElementIdSet& idList, DgnDbRTree3dViewFilter& filter);
        void qt_SearchRangeTree(DgnDbRTree3dViewFilter& filter);
        void SetState (State newState);

        Selector(QueryModel&);
        DGNPLATFORM_EXPORT virtual ~Selector();

    public:
        void qt_WaitForWork();
        void Reset();
        Frustum const& GetFrustum() {return m_frustum;}
        //  The QueryViewController passed in via qvc is not the same as viewport->GetViewControllerCP when the viewport is associated with a 
        //  PhysicalRedlineViewController.
        void StartProcessing(ViewportCR viewport, QueryViewControllerCR qvc, Utf8CP sql, UInt32 hitLimit, UInt64 maxMemory, double minimumScreenPixels, 
                                ElementIdSet* highPriority, ElementIdSet* neverDraw, bool onlyHighPriority, ClipVectorP clipVector,
                             UInt32 secondaryHitLimit, DRange3dCR secondaryRange);
        void RequestAbort(bool waitUntilFinished);
        State WaitUntilFinished (ICheckStop* checkStop, UInt32 interval, bool stopQueryOnAbort);
        DGNPLATFORM_EXPORT bool IsActive() const;
        State GetState() const {return m_state;}
        bool HasSelectResults () const {return m_state == State::Completed;}
    };

private:
    Selector m_selector;
    Results* m_currQueryResults;
    virtual DgnModelType _GetModelType() const override {return DgnModelType::Query;}
DGNPLATFORM_EXPORT explicit QueryModel (DgnProjectR);

public:
    Selector& GetSelector() {return m_selector;}
    Results* GetCurrentResults() {return m_currQueryResults;}

    void SaveQueryResults();
    void ResizeElementList(UInt32 newCount);
    void ClearQueryResults();

    //! Returns a count of elements held by the QueryModel. This is the count of elements returned by the most recent query.
    UInt32 GetElementCount() const;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
