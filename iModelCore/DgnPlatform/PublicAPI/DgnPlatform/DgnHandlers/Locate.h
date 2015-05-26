/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/Locate.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include    <DgnPlatform/DgnCore/HitPath.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
enum class ComponentMode
    {
    None            = 0, //! Set cursor index to head of path (0).
    Innermost       = 1, //! Set cursor index to last entry in path (-1).
//__PUBLISH_SECTION_END__
    InnermostHeader = 2, //! Set cursor index to innermost complex header (legacy mode of extremely limited usefulness)...
//__PUBLISH_SECTION_START__
    NormalChild     = 3, //! Set cursor index to innermost public component of a complex element (same as None if no normal cells in path)...
    SharedChild     = 4, //! Set cursor index to innermost public shared component (same as NormalChild if no shared cells in path)...
    };

/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
enum TestPathStatus
    {
    TESTPATH_NotOnPath      = 0,
    TESTPATH_IsOnPath       = 1,
    TESTPATH_TestAborted    = 2,
    };

/*=================================================================================**//**
* The possible actions for which a LocateFunc_locateFilter filter can be called.
* @bsiclass                                                     KeithBentley    03/01
+===============+===============+===============+===============+===============+======*/
enum LOCATE_Action
    {
    GLOBAL_LOCATE_IDENTIFY          = 1,
    GLOBAL_LOCATE_SELECTIONSET      = 2,
    GLOBAL_LOCATE_FENCE             = 3,
    GLOBAL_LOCATE_FENCECLIP         = 4,
    GLOBAL_LOCATE_SNAP              = 5,
    GLOBAL_LOCATE_AUTOLOCATE        = 6,
    };

/*=================================================================================**//**
* Values to return from ~tLocateFunc_locateFilter.
* @note It would be rare and extreme for a ~tLocateFunc_locateFilter to ever return LOCATE_FILTER_STATUS_Accept.
* <p>Usually, filters will return LOCATE_FILTER_STATUS_Reject to indicate the element is unacceptable, or LOCATE_FILTER_STATUS_Neutral to
* indicate that the element is acceptable <i>as far as this filter is concerned.</i> By returning LOCATE_FILTER_STATUS_Accept, a
* single filter can cause the element to be accepted, <b>without calling other filters</b> that might otherwise reject the element.
* Indicates the reason an element was rejected by a filter.
* @bsiclass                                                     KeithBentley    03/01
+===============+===============+===============+===============+===============+======*/
enum LocateFilterStatus
    {
    LOCATE_FILTER_STATUS_Reject     = 0,
    LOCATE_FILTER_STATUS_Neutral    = 1,
    LOCATE_FILTER_STATUS_Accept     = 2
    };

/*=================================================================================**//**
* Indicates the reason an element was rejected by a filter.
* @bsiclass                                                     KeithBentley    03/01
+===============+===============+===============+===============+===============+======*/
enum LocateFailureValue
    {
    LOCATE_FAILURE_None              = 0,
    LOCATE_FAILURE_NoElements        = 1,
    LOCATE_FAILURE_LockedFile        = 2,
    LOCATE_FAILURE_LevelLock         = 3,
    LOCATE_FAILURE_LockedElem        = 4,
    LOCATE_FAILURE_LockedLevel       = 5,
    LOCATE_FAILURE_ViewOnly          = 6,
    LOCATE_FAILURE_ByApp             = 7,
    LOCATE_FAILURE_ByCommand         = 8,
    LOCATE_FAILURE_ByType            = 9,
    LOCATE_FAILURE_ByProperties      = 10,
    LOCATE_FAILURE_Transient         = 11,
    LOCATE_FAILURE_FileNotAllowed    = 12,
    LOCATE_FAILURE_FileReadOnly      = 13,
    LOCATE_FAILURE_RefernceFile      = 14,
    LOCATE_FAILURE_NotSnappable      = 15,
    LOCATE_FAILURE_NonSnappableRef   = 16,
    LOCATE_FAILURE_ParentNoLocate    = 17,
    LOCATE_FAILURE_RefNoRights       = 18,
    LOCATE_FAILURE_ParentRefNoRights = 19,
    LOCATE_FAILURE_LockedComponent   = 20,
    LOCATE_FAILURE_NotInWorkingSet   = 21,
    LOCATE_FAILURE_RejectedByElement = 22,
    LOCATE_FAILURE_NotInActiveRef    = 23,
    LOCATE_FAILURE_RefNotNowActive   = 24,  // ref is read/write, but is not currently the active ref.
    };

enum SnapTypeEnum 
    {
    SNAP_TYPE_Points=1, 
    SNAP_TYPE_Constraints=2
    };

//__PUBLISH_SECTION_END__
//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/10
//=======================================================================================
struct StopLocateTest
{
virtual ~StopLocateTest (){}
virtual bool _CheckStopLocate() = 0;
};

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  10/14
+===============+===============+===============+===============+===============+======*/
struct LocateOptions
{
private:

bool                m_disableProjectFilter;
bool                m_wantSortHitsByClass;
uint32_t            m_maxHits;
HitSource           m_hitSource;
LocateSurfacesPref  m_locateSurface;

public:
    
LocateOptions ()
    {
    m_disableProjectFilter  = false;
    m_wantSortHitsByClass   = true;
    m_maxHits               = 20;
    m_hitSource             = HitSource::DataPoint;
    m_locateSurface         = LocateSurfacesPref::ByView;
    }

LocateOptions (HitSource hitSource, uint32_t maxHits)
    {
    m_disableProjectFilter  = false;
    m_wantSortHitsByClass   = true;
    m_maxHits               = maxHits;
    m_hitSource             = hitSource;
    m_locateSurface         = LocateSurfacesPref::ByView;
    }

void SetDisableDgnProjectFilter (bool disableProjectFilter) {m_disableProjectFilter = disableProjectFilter;}
void SetHitsSortedByClass (bool sortByClass) {m_wantSortHitsByClass = sortByClass;}
void SetMaxHits (uint32_t maxHits) {m_maxHits = maxHits;}
void SetHitSource (HitSource hitSource) {m_hitSource = hitSource;}
void SetLocateSurfaces (LocateSurfacesPref locateSurface) {m_locateSurface = locateSurface;}

bool GetDisableDgnProjectFilter () const {return m_disableProjectFilter;}
bool GetHitsSortedByClass () const {return m_wantSortHitsByClass;}
uint32_t GetMaxHits () const {return m_maxHits;}
HitSource GetHitSource () const {return m_hitSource;}
LocateSurfacesPref GetLocateSurfaces () const {return m_locateSurface;}

}; // LocateOptions

/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    03/01
+===============+===============+===============+===============+===============+======*/
struct  ElementPicker
{
private:
    DgnViewportP       m_viewport;
    DPoint3d        m_pickPointWorld;
    HitList*        m_hitList;
    bool            m_lastPickAborted;

public:
    DGNVIEW_EXPORT ElementPicker ();

    DGNVIEW_EXPORT void      Empty ();
    DGNVIEW_EXPORT void      ClearFrom (DgnModelP modelRef);
    DGNVIEW_EXPORT HitPath*  GetNextHit ();
    DGNVIEW_EXPORT HitPath*  GetHit (int i);
    DGNVIEW_EXPORT HitList*  GetHitList (bool takeOwnership);
    DGNVIEW_EXPORT void      ResetCurrentHit ();
    
    DGNVIEW_EXPORT int DoPickElements (bool* wasAborted, DgnViewportR vp, DPoint3dCR pickPointWorld, double pickApertureScreen, StopLocateTest*, LocateOptions const& options);
    DGNVIEW_EXPORT TestPathStatus TestHit (HitPathCR hit, HitListP hitList, DgnViewportR vp, DPoint3dCR pickPointWorld, double pickApertureScreen, StopLocateTest*, LocateOptions const& options);

}; // ElementPicker

//__PUBLISH_SECTION_START__
/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    03/01
+===============+===============+===============+===============+===============+======*/
struct  ElementLocateManager
{
//__PUBLISH_SECTION_END__
    typedef enum LocateFilterStatus (*LocateFilterFunc)(Utf8StringP cantAcceptExplanation, GeometricElementCP elm, HitPathCP path);

    friend struct ElementPicker;
    friend struct SnapContext;
    friend struct TentativePoint;
    friend struct AccuSnap;

private:
    HitPathP GetPreLocatedPath ();

protected:
    typedef bvector<RefCountedPtr<HitPath>> HilitedArray;

    HilitedArray    m_hilitedElems;
    HitList*        m_hitList;
    HitPath*        m_currHit;
    ElementPicker   m_picker;
    LocateOptions   m_options;

    DGNVIEW_EXPORT virtual void _Clear ();
    DGNVIEW_EXPORT virtual HitPathCP _DoLocateHitPath (LocateFailureValue* reasonCode, Utf8StringP cantAcceptExplanation, bool newSearch, DPoint3dCR, DgnViewportP, ComponentMode complexComponent, bool filterHits);
    DGNVIEW_EXPORT virtual bool _FilterHit (HitPathP* swapPathP, HitPathCP, LocateFailureValue*, Utf8StringP cantAcceptExplanation, ComponentMode complexComponent, LOCATE_Action);
    DGNVIEW_EXPORT virtual void _InitToolLocate ();
    DGNVIEW_EXPORT virtual void _InitLocateOptions (); // Called from _InitToolLocate to establish defaults.
    DGNVIEW_EXPORT virtual double _GetAperture(); // in graphite, we need the help of the tool admin to set the aperture
    virtual int _GetCursorColor() {return 7;}
    virtual int _GetKeypointDivisor() {return 1;}
    DGNVIEW_EXPORT virtual void _GetLocateError (Utf8StringR reasonString, int reason);
    DGNVIEW_EXPORT virtual void _AdjustSnapPath (SnapContext&);
    virtual bool _IsSnappableDgnAttachment (DgnModelP modelRef) {return true;}
    virtual HitPathCP _GetAppFilterPath () {return nullptr;}
    virtual LocateFilterStatus _AppFilterHit (LocateFailureValue*, Utf8StringP explanation, HitPathCP path, LocateFilterFunc internalFilter, bool preLocate, LOCATE_Action action) {return LOCATE_FILTER_STATUS_Neutral;}
    virtual SnapStatus _PerformConstraintSnap(SnapPathP, double hotDistance, HitSource snapSource) {return SnapStatus::Success;}
    DGNVIEW_EXPORT virtual void _GetPreferredPointSnapModes (bvector<SnapMode>& snaps, HitSource source);
    virtual bool _IsConstraintSnapActive () {return false;}
    virtual void _SetChosenSnapMode (SnapTypeEnum snapType, SnapMode snapMode) {}
    virtual void _SynchSnapMode() {}
    virtual bool _WantSnapLock() {return true;}
    virtual void _OnFlashHit(SnapPathP) {}
    virtual void _OnAccuSnapMotion(SnapPathP, bool wasHot, DgnButtonEventCR) {}

public:
    DGNVIEW_EXPORT ElementLocateManager();
    virtual ~ElementLocateManager() {}

    HitPathCP GetCurrPath () {return m_currHit;}
    HitListCP GetHitList () {return m_hitList;}
    ElementPicker& GetElementPicker() {return m_picker;}
    LocateOptions& GetLocateOptions() {return m_options;}
    DGNVIEW_EXPORT double ComputeSnapTolerance (DgnViewportCR, double);
    int GetKeypointDivisor();
    DGNVIEW_EXPORT HitPathCP GetAppFilterPath ();
    LocateFilterStatus AppFilterHit (LocateFailureValue*, Utf8StringP, HitPathCP, LocateFilterFunc, bool, LOCATE_Action);
    void DoHilite (HitPathCP, bool doGroups);
    DGNVIEW_EXPORT bool IsElementHilited () const;
    DGNVIEW_EXPORT HitPathCP GetHilitedPath (int index) const;
    DGNVIEW_EXPORT void Clear ();
    DGNVIEW_EXPORT void SetCurrHit (HitPathCP);
    DGNVIEW_EXPORT HitPathP GetNextHit ();
    DGNVIEW_EXPORT void ClearHilited (bool redraw);
    DGNVIEW_EXPORT void DropFromHilited (HitPathCP);
    DGNVIEW_EXPORT void SetHitList (HitList* list);
    DGNVIEW_EXPORT void HiliteAndSave (HitPathCP, bool doHilite, bool checkHilited);
    DGNVIEW_EXPORT void ClearFrom (DgnModelP);
    DGNVIEW_EXPORT void DropFromHilited (DgnModelP modelRef);
    DGNVIEW_EXPORT bool FilterHit (HitPathP* swapPathP, HitPathCP, LocateFailureValue*, Utf8StringP cantAcceptExplanation, ComponentMode complexComponent, LOCATE_Action);
    DGNVIEW_EXPORT double GetAperture();
    DGNVIEW_EXPORT void GetLocateError (Utf8StringR reasonString, int reason);
    DGNVIEW_EXPORT void InitToolLocate ();
    DGNVIEW_EXPORT void ShowErrorExplanation (Utf8CP cantAcceptExplanation, int reason);
    DGNVIEW_EXPORT void ShowPathInfo (HitPathCP);
    DGNVIEW_EXPORT void SetPathIndex (HitPathP hit, ComponentMode complexComponent);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    DGNVIEW_EXPORT HitPathCP DoLocateHitPath (LocateFailureValue* reasonCode, Utf8StringP cantAcceptExplanation, bool newSearch, DPoint3dCR, DgnViewportP, ComponentMode complexComponent, bool filterHits=true);
    DGNVIEW_EXPORT static ElementLocateManager& GetManager();

}; // ElementLocateManager

END_BENTLEY_DGNPLATFORM_NAMESPACE

BEGIN_BENTLEY_API_NAMESPACE
typedef DgnPlatform::LocateFailureValue LocateFailure;
END_BENTLEY_API_NAMESPACE
