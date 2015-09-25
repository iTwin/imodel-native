/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/QueryModel.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnModel.h"
#include <Bentley/BeThread.h>
#include <DgnPlatform/DgnCore/ViewContext.h>

BEGIN_BENTLEY_DGN_NAMESPACE

struct DgnDbRTree3dViewFilter;
                                                                                                                    
//=======================================================================================
/**
A QueryModel is a virtual DgnModel that holds @ref DgnElementGroup loaded from the database according to a custom query criteria.
A QueryModel caches the results of the query.

A QueryModel is used in conjunction with a QueryViewController to display the results of the query. 
Applications do not directly deal with QueryModel's. Instead, the query that populates them is supplied by a QueryViewController.

The method MobileDgn::MobileDgnApplication::OpenDgnDb creates a default QueryModel for an application. 
Applications may use MobileDgn::MobileDgnApplication::GetQueryModel to retrieve a reference to that QueryModel. 
QueryModels are associated with a QueryViewController by passing a QueryModel to the QueryViewController constructor.
*/
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct QueryModel : PhysicalModel
{
    friend struct QueryViewController;

    //=======================================================================================
    // @bsiclass                                                     JohnGoding      05/12
    //=======================================================================================
    struct Results
    {
        bvector<DgnElementCP> m_elements;
        bvector<DgnElementCP> m_closeElements;
        bool   m_reachedMaxElements;
        bool   m_eliminatedByLOD;
        uint32_t m_drawnBeforePurge;
        double m_lowestOcclusionScore;

        uint32_t GetCount() const {return (uint32_t) m_elements.size();}
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
        DgnDbR              m_dgndb;
        State               m_state;
        BeConditionVariable m_conditionVariable;
        Utf8String          m_elementLoadSql;
        Utf8String          m_searchSql;
        DgnViewportCP       m_viewport;
        uint32_t            m_maxElements;
        double              m_minimumPixels;
        uint64_t            m_maxMemory;
        Results*            m_results;
        BeSQLite::DbResult  m_dbStatus;
        bool                m_restartRangeQuery;
        bool                m_inRangeSelectionStep;
        bool                m_noQuery;
        Frustum             m_frustum;
        QueryViewControllerCP m_controller;
        DgnElementIdSet*    m_alwaysDraw;
        DgnElementIdSet*    m_neverDraw;
        ClipVectorPtr       m_clipVector;
        uint32_t            m_secondaryHitLimit;
        DRange3d            m_secondaryVolume;
       
        virtual bool _CheckStop () override;
        void qt_ProcessRequest();
        void qt_NotifyCompletion();
        void qt_SearchIdSet(DgnElementIdSet& idList, DgnDbRTree3dViewFilter& filter);
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
        void StartProcessing(DgnViewportCR viewport, QueryViewControllerCR qvc, Utf8CP sql, uint32_t hitLimit, uint64_t maxMemory, double minimumScreenPixels, 
                                DgnElementIdSet* highPriority, DgnElementIdSet* neverDraw, bool onlyHighPriority, ClipVectorP clipVector,
                             uint32_t secondaryHitLimit, DRange3dCR secondaryRange);
        void RequestAbort(bool waitUntilFinished);
        State WaitUntilFinished (ICheckStop* checkStop, uint32_t interval, bool stopQueryOnAbort);
        DGNPLATFORM_EXPORT bool IsActive() const;
        State GetState() const {return m_state;}
        bool HasSelectResults () const {return m_state == State::Completed;}
    };

private:
    Selector m_selector;
    Results* m_currQueryResults;
    void ResetResults(){ ReleaseAllElements(); ClearRangeIndex(); m_filled=true;}
    DGNPLATFORM_EXPORT explicit QueryModel (DgnDbR);
    virtual void _FillModel() override {} // QueryModels are never filled.

public:
    Selector& GetSelector() {return m_selector;}
    Results* GetCurrentResults() {return m_currQueryResults;}

    void SaveQueryResults();
    void ResizeElementList(uint32_t newCount);
    void ClearQueryResults();

    //! Returns a count of elements held by the QueryModel. This is the count of elements returned by the most recent query.
    uint32_t GetElementCount() const;
};

END_BENTLEY_DGN_NAMESPACE
