/*--------------------------------------------------------------------------------------+                                                                                                                                                                                                                                          class
|
|     $Source: TilePublisher/lib/SchedulePublisher.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <TilePublisher/TilePublisher.h>
#include <DgnPlatform/DgnMarkupProject.h>
#include <Planning/PlanningApi.h>
#include <BeSqLite/BeSqLite.h>
#include "Constants.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_TILEPUBLISHER
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_PLANNING
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

#ifdef WIP  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void getStartAndEnd(double& start, double& end, TimeSpanCR timeSpan, DateType dateType)
    {
    switch (dateType)
        {
        default:
        case DateType::Planned:
            timeSpan.GetPlannedStart().ToJulianDay(start);
            timeSpan.GetPlannedFinish().ToJulianDay(end);
            break;

        case DateType::Actual:
            timeSpan.GetActualStart().ToJulianDay(start);
            timeSpan.GetActualFinish().ToJulianDay(end);
            break;
        case DateType::Early:
            timeSpan.GetEarlyStart().ToJulianDay(start);
            timeSpan.GetEarlyFinish().ToJulianDay(end);
            break;
        case DateType::Late:
            timeSpan.GetLateStart().ToJulianDay(start);
            timeSpan.GetLateFinish().ToJulianDay(end);
            break;
        }
    }
#endif

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     09/2017
//=======================================================================================
struct ScheduleAppearance
{
private:
    struct Flags
        {
        unsigned            m_showBefore:1;
        unsigned            m_showAfter:1;
        unsigned            m_showFade:1;
        unsigned            m_beforeColor:1;
        unsigned            m_activeColor:1;
        unsigned            m_afterColor:1;
        unsigned            m_beforeAlpha:1;
        unsigned            m_activeAlpha:1;
        unsigned            m_afterAlpha:1;

        Flags() { memset(this, 0, sizeof(*this)); }
        };                   

    ColorDef                m_beforeColor;
    ColorDef                m_activeColor;
    ColorDef                m_afterColor;
    Flags                   m_flags;
    void    SetActiveColor(ColorDefCR color) { m_activeColor.SetColorNoAlpha(color); m_flags.m_activeColor = true; }
    void    SetBeforeColor(ColorDefCR color) { m_beforeColor.SetColorNoAlpha(color); m_flags.m_beforeColor = true; }
    void    SetAfterColor(ColorDefCR color)  { m_afterColor.SetColorNoAlpha(color);  m_flags.m_afterColor = true; }

    void    SetActiveAlpha(Byte alpha)      { m_activeColor.SetAlpha(alpha); m_flags.m_activeAlpha = true; }
    void    SetBeforeAlpha(Byte alpha)      { m_beforeColor.SetAlpha(alpha); m_flags.m_beforeAlpha = true; }
    void    SetAfterAlpha(Byte alpha)       { m_afterColor.SetAlpha(alpha);  m_flags.m_afterAlpha  = true; }
                                                                                                            
public: 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ScheduleAppearance(ElementAppearanceProfileId appearanceId, DgnDbCR markupDb)
    {
    ElementAppearanceProfileCPtr appearanceProfile = ElementAppearanceProfile::Get(markupDb, appearanceId);

    if (!appearanceProfile.IsValid())
        {
        BeAssert(false);
        return;;
        }

    ElementAppearanceProfile::DisplaySettings const& displaySettings = appearanceProfile->GetDisplaySettings();

    bool showBefore, showAfter, showFade;
    displaySettings.GetShowBefore(showBefore);
    displaySettings.GetShowAfter(showAfter);
    displaySettings.GetShowFade(showFade);

    m_flags.m_showBefore = showBefore;
    m_flags.m_showAfter  = showAfter;
    m_flags.m_showFade   = showFade;


    ColorDef        color;
    Byte            alpha;

    if (displaySettings.GetActiveColor(color))
        SetActiveColor(color);

    if (displaySettings.GetBeforeColor(color))
        SetBeforeColor(color);

    if (displaySettings.GetAfterColor(color))
        SetAfterColor(color);

    if (displaySettings.GetActiveTransparency(alpha))
        SetActiveAlpha(alpha);

    if (displaySettings.GetBeforeTransparency(alpha))
        SetBeforeAlpha(alpha);

    if (displaySettings.GetAfterTransparency(alpha))
        SetAfterAlpha(alpha);
    }
};  // ScheduleAppearance

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     09/2017
//=======================================================================================
struct  ElementScheduleEntry 
{
private:
    double                      m_startTime;    // Julian days.
    double                      m_endTime;      // Julian days.
    ElementAppearanceProfileId  m_appearanceId;

public:
    ElementScheduleEntry() {}
    ElementScheduleEntry (double startTime, double endTime, ElementAppearanceProfileId appearanceId) : m_startTime(startTime), m_endTime(endTime), m_appearanceId(appearanceId) { }

    double GetStartTime () const { return m_startTime; }
    double GetEndTime() const { return m_endTime; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool operator < (struct ElementScheduleEntry const& rhs) const
    {
    if (m_startTime != rhs.m_startTime)
        return m_startTime < rhs.m_startTime;

    if (m_endTime != rhs.m_endTime)
        return m_endTime < rhs.m_endTime;

    return m_appearanceId < rhs.m_appearanceId;
    }
    
};      // ElementScheduleEntry    


//=======================================================================================
// @bsistruct                                                   Ray.Bentley     09/2017
//=======================================================================================
struct ElementSchedule : RefCountedBase
{
private:
    bset<ElementScheduleEntry>  m_entries;

public:
    ElementSchedule (ElementScheduleEntry const& entry)     { AddEntry(entry); }
    void AddEntry (ElementScheduleEntry const& entry)       { m_entries.insert(entry); }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value     GetJson(DgnElementId id, double start, double end)
    {
    }

 };  // ElementSchedule

typedef RefCountedPtr<ElementSchedule> ElementSchedulePtr;
//=======================================================================================
// @bsistruct                                                   Ray.Bentley     09/2017
//=======================================================================================
struct Schedule
{
    bmap<DgnElementId, ElementSchedulePtr> m_elementSchedules;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void    AddEntry (DgnElementId elementId, ElementScheduleEntry const& entry)
    {
    auto const&   found = m_elementSchedules.find(elementId);

    if (found == m_elementSchedules.end())
        m_elementSchedules.Insert(elementId, new ElementSchedule(entry));
    else
        found->second->AddEntry(entry);
    }
    
    
/*---------------------------------------------------------------------------------**//**                                                                                                                                                 
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value     GetJson(TimeSpanCR timeSpan, DateType dateType)
    {
    Json::Value scheduleJson = Json::objectValue;
#ifdef WIP
    double      start, end;

    getStartAndEnd (start, end, timeSpan, dateType);

    for (auto const& curr : m_elementSchedules)
        scheduleJson.append(curr.send.GetJson(curr.first, start, end));
          
#endif                  
    return scheduleJson;
    }

};  // Schedule

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus   extractRawSchedule (Schedule& schedule, Planning::Plan::Entry const& planEntry, Planning::Plan const& plan, Planning::Baseline::Entry const& baselineEntry, DgnDbCR markupDb, DgnDbCR db, DateType dateType)
    {
    Utf8CP              ecSqlFormatStr =
        "SELECT Activity.ECInstanceId, TimeSpan.%sStart, TimeSpan.%sFinish " \
        "FROM bp.Activity " \
        "JOIN bp.TimeSpan USING bis.ElementOwnsMultiAspects " \
        "WHERE Activity.Plan.Id = :planId AND TimeSpan.Baseline.Id = :baselineId " \
        "ORDER BY TimeSpan.%sStart, TimeSpan.%sFinish";

    Utf8String          dateTypeStr = PlanningUtility::ConvertDateTypeToString(dateType);
    Utf8PrintfString    readECSql(ecSqlFormatStr, dateTypeStr, dateTypeStr, dateTypeStr, dateTypeStr);

    ECSqlStatement      readStmt;
    ECSqlStatus         ecSqlStatus = readStmt.Prepare(markupDb, readECSql.c_str());
    BeAssert(ecSqlStatus == ECSqlStatus::Success);

    readStmt.BindId(1, planEntry.GetId());
    readStmt.BindId(2, baselineEntry.GetId());

    DbResult ecStepStatus;
    while ((ecStepStatus = readStmt.Step()) == BE_SQLITE_ROW)
        {
        double          startJulian, endJulian;

        ActivityId activityId = readStmt.GetValueId<ActivityId>(0);

        if (SUCCESS != readStmt.GetValueDateTime(1).ToJulianDay(startJulian) ||
            SUCCESS != readStmt.GetValueDateTime(2).ToJulianDay(endJulian))
            {
            BeAssert(false);
            continue;
            }

        for (Activity::AffectedElementsEntry const& entry : Activity::MakeAffectedElementsIterator(markupDb, activityId))
            {
            DgnElementIdSet         affectedElements = PlanningUtility::GatherAffectedElements(entry.GetElementId(), markupDb, &db);
            ElementScheduleEntry    scheduleEntry(startJulian, endJulian, entry.GetAppearanceProfileId());

            for (auto& elementId : affectedElements)
                schedule.AddEntry(elementId, scheduleEntry);
            }
        }
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus extractSchedules(Json::Value& schedules, DgnDbCR markupDb, DgnDbCR db, DateType dateType)
    {
    schedules = Json::arrayValue;

    for (auto const& planEntry : Planning::Plan::MakePlanIterator(markupDb))
        {
        PlanCPtr        plan = Planning::Plan::Get(markupDb, planEntry.GetId());

        if (!plan.IsValid())
            {
            BeAssert(false);
            continue;
            }
        for (auto& baseline : plan->MakeBaselineIterator())
            {
            Schedule        rawSchedule;
            TimeSpanCP      timeSpan;

            if (nullptr != (timeSpan = plan->GetTimeSpan(baseline.GetId())) &&
                SUCCESS == extractRawSchedule(rawSchedule, planEntry, *plan, baseline, markupDb, db, dateType))
                schedules.append(rawSchedule.GetJson(*timeSpan, dateType));
            }
        }

    return 0 == schedules.size() ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void   PublisherContext::PublishScheduleSimulations()
    {
    DgnDomains::RegisterDomain(PlanningDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    DgnDomains::RegisterDomain(MarkupDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);

    DgnDb::OpenParams       openParams(Db::OpenMode::Readonly);
    WString                 device, directory, name;

    GetDgnDb().GetFileName().ParseName (&device, &directory, &name, nullptr);

    DgnMarkupProjectPtr     markupDb = DgnMarkupProject::OpenDgnDb(nullptr, BeFileName(device.c_str(), directory.c_str(), name.c_str(), nullptr), openParams);

    if (!markupDb.IsValid())
        return;

    if (!PlanningDomain::GetDomain().IsSchemaImported(*markupDb))
        return;

    Json::Value     scheduleJson;
    DateType        scheduleTypes[] = { DateType::Planned, DateType::Actual, DateType::Early, DateType::Late };

    for (auto& scheduleType : scheduleTypes)
        extractSchedules(scheduleJson, *markupDb, GetDgnDb(), scheduleType);
    }

