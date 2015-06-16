/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/Locate.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnCore/HitPath.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
enum class TestHitStatus
    {
    NotOn       = 0,
    IsOn        = 1,
    Aborted     = 2
    };

/*=================================================================================**//**
* The possible actions for which a locate filter can be called.
* @bsiclass                                                     KeithBentley    03/01
+===============+===============+===============+===============+===============+======*/
enum class LocateAction
    {
    Identify    = 0,
    AutoLocate  = 1
    };

/*=================================================================================**//**
* Values to return from a locate filter.
* @note It would be rare and extreme for a locate filter to ever return Accept.
* <p>Usually, filters will return Reject to indicate the element is unacceptable, or LNeutral to
* indicate that the element is acceptable <i>as far as this filter is concerned.</i> By returning Accept, a
* single filter can cause the element to be accepted, <b>without calling other filters</b> that might otherwise reject the element.
* Indicates the reason an element was rejected by a filter.
* @bsiclass                                                     KeithBentley    03/01
+===============+===============+===============+===============+===============+======*/
enum class LocateFilterStatus
    {
    Reject      = 0,
    Neutral     = 1,
    Accept      = 2
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class SnapType 
    {
    Points      = 1, 
    Constraints = 2
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

bool                m_disableDgnDbFilter;
uint32_t            m_maxHits;
HitSource           m_hitSource;
LocateSurfacesPref  m_locateSurface;

public:
    
LocateOptions ()
    {
    m_disableDgnDbFilter    = false;
    m_maxHits               = 20;
    m_hitSource             = HitSource::DataPoint;
    m_locateSurface         = LocateSurfacesPref::ByView;
    }

LocateOptions (HitSource hitSource, uint32_t maxHits)
    {
    m_disableDgnDbFilter    = false;
    m_maxHits               = maxHits;
    m_hitSource             = hitSource;
    m_locateSurface         = LocateSurfacesPref::ByView;
    }

void SetDisableDgnDbFilter (bool disableDgnDbFilter) {m_disableDgnDbFilter = disableDgnDbFilter;}
void SetMaxHits (uint32_t maxHits) {m_maxHits = maxHits;}
void SetHitSource (HitSource hitSource) {m_hitSource = hitSource;}
void SetLocateSurfaces (LocateSurfacesPref locateSurface) {m_locateSurface = locateSurface;}

bool GetDisableDgnDbFilter () const {return m_disableDgnDbFilter;}
uint32_t GetMaxHits() const {return m_maxHits;}
HitSource GetHitSource() const {return m_hitSource;}
LocateSurfacesPref GetLocateSurfaces() const {return m_locateSurface;}

}; // LocateOptions

/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    03/01
+===============+===============+===============+===============+===============+======*/
struct  ElementPicker
{
private:
    DgnViewportP    m_viewport;
    DPoint3d        m_pickPointWorld;
    HitList*        m_hitList;
    bool            m_lastPickAborted;

public:
    DGNVIEW_EXPORT ElementPicker ();

    DGNVIEW_EXPORT void       Empty ();
    DGNVIEW_EXPORT void       ClearFrom (DgnModelP modelRef);
    DGNVIEW_EXPORT HitDetail* GetNextHit ();
    DGNVIEW_EXPORT HitDetail* GetHit (int i);
    DGNVIEW_EXPORT HitList*   GetHitList (bool takeOwnership);
    DGNVIEW_EXPORT void       ResetCurrentHit ();
    
    DGNVIEW_EXPORT int DoPick (bool* wasAborted, DgnViewportR vp, DPoint3dCR pickPointWorld, double pickApertureScreen, StopLocateTest*, LocateOptions const& options);
    DGNVIEW_EXPORT TestHitStatus TestHit (HitDetailCR hit, HitListP hitList, DgnViewportR vp, DPoint3dCR pickPointWorld, double pickApertureScreen, StopLocateTest*, LocateOptions const& options);

}; // ElementPicker

//__PUBLISH_SECTION_START__
/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    03/01
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ElementLocateManager
{
//__PUBLISH_SECTION_END__
    friend struct ElementPicker;
    friend struct SnapContext;
    friend struct TentativePoint;
    friend struct AccuSnap;

private:
    HitDetailP GetPreLocatedHit ();

protected:
    typedef bvector<RefCountedPtr<HitDetail>> HilitedArray;

    HilitedArray    m_hilitedElems;
    HitList*        m_hitList;
    HitDetail*      m_currHit;
    ElementPicker   m_picker;
    LocateOptions   m_options;

    DGNVIEW_EXPORT virtual void _Clear ();
    DGNVIEW_EXPORT virtual HitDetailCP _DoLocate (LocateFailureValue* reasonCode, Utf8StringP cantAcceptExplanation, bool newSearch, DPoint3dCR, DgnViewportP, ComponentMode mode, bool filterHits);
    DGNVIEW_EXPORT virtual bool _FilterHit (HitDetailCP, LocateFailureValue*, Utf8StringP cantAcceptExplanation, ComponentMode mode, LocateAction);
    DGNVIEW_EXPORT virtual void _InitToolLocate ();
    DGNVIEW_EXPORT virtual void _InitLocateOptions (); // Called from _InitToolLocate to establish defaults.
    DGNVIEW_EXPORT virtual double _GetAperture(); // in graphite, we need the help of the tool admin to set the aperture
    DGNVIEW_EXPORT virtual void _GetLocateError (Utf8StringR reasonString, int reason);
    DGNVIEW_EXPORT virtual void _AdjustSnapDetail (SnapContext&);
    virtual bool _IsSnappableModel(DgnModelP modelRef) {return true;}
    virtual SnapStatus _PerformConstraintSnap(SnapDetailP, double hotDistance, HitSource snapSource) {return SnapStatus::Success;}
    DGNVIEW_EXPORT virtual void _GetPreferredPointSnapModes (bvector<SnapMode>& snaps, HitSource source);
    virtual uint32_t _GetKeypointDivisor() {return 2;}
    virtual bool _IsConstraintSnapActive () {return false;}
    virtual void _SetChosenSnapMode (SnapType snapType, SnapMode snapMode) {}
    virtual void _SynchSnapMode() {}
    virtual void _OnFlashHit(SnapDetailP) {}
    virtual void _OnAccuSnapMotion(SnapDetailP, bool wasHot, DgnButtonEventCR) {}

public:
    DGNVIEW_EXPORT ElementLocateManager();
    virtual ~ElementLocateManager() {}

    HitDetailCP GetCurrHit () {return m_currHit;}
    HitListCP GetHitList () {return m_hitList;}
    ElementPicker& GetElementPicker() {return m_picker;}
    LocateOptions& GetLocateOptions() {return m_options;}
    DGNVIEW_EXPORT double ComputeSnapTolerance (DgnViewportCR, double);
    uint32_t GetKeypointDivisor();
    void DoHilite (HitDetailCP, bool doGroups);
    DGNVIEW_EXPORT bool IsElementHilited () const;
    DGNVIEW_EXPORT HitDetailCP GetHilitedHit (int index) const;
    DGNVIEW_EXPORT void Clear ();
    DGNVIEW_EXPORT void SetCurrHit (HitDetailCP);
    DGNVIEW_EXPORT HitDetailP GetNextHit ();
    DGNVIEW_EXPORT void ClearHilited (bool redraw);
    DGNVIEW_EXPORT void DropFromHilited (HitDetailCP);
    DGNVIEW_EXPORT void SetHitList (HitList* list);
    DGNVIEW_EXPORT void HiliteAndSave (HitDetailCP, bool doHilite, bool checkHilited);
    DGNVIEW_EXPORT void ClearFrom (DgnModelP);
    DGNVIEW_EXPORT void DropFromHilited (DgnModelP modelRef);
    DGNVIEW_EXPORT bool FilterHit (HitDetailCP, LocateFailureValue*, Utf8StringP cantAcceptExplanation, ComponentMode mode, LocateAction);
    DGNVIEW_EXPORT double GetAperture();
    DGNVIEW_EXPORT void GetLocateError (Utf8StringR reasonString, int reason);
    DGNVIEW_EXPORT void InitToolLocate ();
    DGNVIEW_EXPORT void ShowErrorExplanation (Utf8CP cantAcceptExplanation, int reason);
    DGNVIEW_EXPORT void ShowHitInfo (HitDetailCP);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    DGNVIEW_EXPORT HitDetailCP DoLocate (LocateFailureValue* reasonCode, Utf8StringP cantAcceptExplanation, bool newSearch, DPoint3dCR, DgnViewportP, ComponentMode mode, bool filterHits=true);
    DGNVIEW_EXPORT static ElementLocateManager& GetManager();

}; // ElementLocateManager

END_BENTLEY_DGNPLATFORM_NAMESPACE

BEGIN_BENTLEY_NAMESPACE
typedef Dgn::LocateFailureValue LocateFailure;
END_BENTLEY_NAMESPACE
