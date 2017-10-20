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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool extractTimeSpan(double& start, double& end, TimeSpanCR timeSpan, DateType dateType)
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
    return start > 0.0 && end > 0.0 && end > start;
    }

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
    Byte                    m_beforeAlpha;
    Byte                    m_activeAlpha;
    Byte                    m_afterAlpha;
    Flags                   m_flags;

public:
    struct Fade
        {
        Byte        m_start;
        Byte        m_end;
        double      m_time;

        Json::Value GetJson() const
            {
            Json::Value     value = Json::objectValue;

            value["start"] = m_start/255.0;
            value["delta"] = ((double) m_end - (double) m_start) / 255.0 * m_time;

            return value;
            }
        };

    bool    GetShowBefore() const { return m_flags.m_showBefore; }
    bool    GetShowAfter() const { return m_flags.m_showAfter; }

    void    SetActiveColor(ColorDefCR color) { m_activeColor.SetColorNoAlpha(color); m_flags.m_activeColor = true; }
    void    SetBeforeColor(ColorDefCR color) { m_beforeColor.SetColorNoAlpha(color); m_flags.m_beforeColor = true; }
    void    SetAfterColor(ColorDefCR color)  { m_afterColor.SetColorNoAlpha(color);  m_flags.m_afterColor = true; }

    void    SetActiveAlpha(Byte alpha)      { m_activeAlpha = alpha; m_flags.m_activeAlpha = true; }
    void    SetBeforeAlpha(Byte alpha)      { m_beforeAlpha = alpha; m_flags.m_beforeAlpha = true; }
    void    SetAfterAlpha(Byte alpha)       { m_afterAlpha  = alpha;  m_flags.m_afterAlpha  = true; }
    
    ColorDefCP GetBeforeColor() const       { return m_flags.m_beforeColor ? &m_beforeColor : nullptr; } 
    ColorDefCP GetActiveColor() const       { return m_flags.m_activeColor ? &m_activeColor : nullptr; } 
    ColorDefCP GetAfterColor() const        { return m_flags.m_afterColor  ? &m_afterColor  : nullptr; } 
    Byte const* GetBeforeAlpha() const      { return m_flags.m_beforeAlpha ? &m_beforeAlpha : nullptr; }
    Byte const* GetActiveAlpha() const      { return m_flags.m_activeAlpha ? &m_activeAlpha : nullptr; }
    Byte const* GetAfterAlpha() const       { return m_flags.m_afterAlpha  ? &m_afterAlpha : nullptr; }

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
        SetActiveAlpha(255 - alpha);

    if (displaySettings.GetBeforeTransparency(alpha))
        SetBeforeAlpha(255-alpha);

    if (displaySettings.GetAfterTransparency(alpha))
        SetAfterAlpha(255-alpha);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Fade* GetFade (Fade& fade, double startTime, double endTime) const
    {
    if (!m_flags.m_showFade)
        return nullptr;

    fade.m_start = m_beforeAlpha;
    fade.m_end   = m_afterAlpha;
    fade.m_time  = endTime - startTime;

    BeAssert(fade.m_time > 1.0E-5);

    return &fade;
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
    ElementScheduleEntry (double startTime, double endTime, ElementAppearanceProfileId appearanceId) : m_startTime(startTime), m_endTime(endTime), m_appearanceId(appearanceId) { BeAssert(endTime > startTime); }

    double GetStart() const { return m_startTime; }
    double GetEnd() const { return m_endTime; }
    ElementAppearanceProfileId GetAppearanceId() const { return m_appearanceId; }

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

    struct State
        {
        bool        m_show;
        bool        m_setColor;
        ColorDef    m_color;
        double      m_time;

        State(double time) : m_time(time), m_show(false), m_setColor(false) { }
        };
        
    bool operator < (struct ElementSchedule const& rhs) const { return m_entries < rhs.m_entries; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void    AppendJsonEntry(Json::Value& jsonEntries, State& state, double time, bool show, ColorDefCP color, Byte const* alpha, ScheduleAppearance::Fade const* fade)
    {
    if (show != state.m_show ||
        state.m_setColor != (nullptr != color || nullptr != alpha) ||
        (nullptr != color && *color != state.m_color) ||
        (nullptr != alpha && *alpha != state.m_color.GetAlpha()))
        {
        Json::Value jsonEntry;

        state.m_show = show;
        jsonEntry["show"] = show;
        
        if (show)
            {
            Json::Value     jsonColor = Json::arrayValue;
            if (nullptr != color)
                {

                jsonColor.append(color->GetRed()/255.0);
                jsonColor.append(color->GetGreen()/255.0);
                jsonColor.append(color->GetBlue()/255.0);
                jsonEntry["color"] = jsonColor;
                state.m_color.SetColorNoAlpha(*color);
                }
            else
                {
                jsonColor.append(1.0);
                jsonColor.append(1.0);
                jsonColor.append(1.0);
                }

            if (nullptr != alpha)
                {
                state.m_color.SetAlpha(*alpha);
                jsonColor.append(*alpha/255.0);
                }
            else
                {
                state.m_color.SetAlpha(255);
                jsonColor.append(1.0);
                }

            if (nullptr != fade)
                jsonEntry["fade"] = fade->GetJson();
                
            if (true == (state.m_setColor = (nullptr != color || nullptr != alpha)))
                jsonEntry["color"] = jsonColor;
            }

        jsonEntry["time"] = time;
        jsonEntries.append(std::move(jsonEntry));
        }
        
    state.m_time = time;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value     GetJson(uint32_t id, double start, double end, DgnDbCR db)
    {
    double          current = start;
    State           state(start);

    Json::Value     json = Json::objectValue;
    Json::Value     jsonEntries = Json::arrayValue;

    json["id"] = id;
    for (auto entry = m_entries.begin(); entry != m_entries.end(); entry++)
        {
        ScheduleAppearance  appearance(entry->GetAppearanceId(), db);

        if (entry->GetStart() > state.m_time)
            AppendJsonEntry (jsonEntries, state, state.m_time, appearance.GetShowBefore(), appearance.GetBeforeColor(), appearance.GetBeforeAlpha(), nullptr);
            
        ScheduleAppearance::Fade        fade;
        AppendJsonEntry (jsonEntries, state, entry->GetStart(), true, appearance.GetActiveColor(), appearance.GetActiveAlpha(), appearance.GetFade(fade, entry->GetStart(), entry->GetEnd()));

        auto next = entry;
        next++;
        if (next == m_entries.end() || entry->GetEnd() < next->GetStart())
            AppendJsonEntry (jsonEntries, state, entry->GetEnd(), appearance.GetShowAfter(), appearance.GetAfterColor(), appearance.GetAfterAlpha(), nullptr);
        
        }
    json["entries"] = std::move(jsonEntries);

    return json;
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
    
struct CompareSchedules
    {
    bool operator() (ElementSchedulePtr const& schedule0, ElementSchedulePtr const& schedule1) const
        {
        return *schedule0 < *schedule1;
        }
    };

/*---------------------------------------------------------------------------------**//**                                                                                                                                                 
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value     GetJson(PublisherContext::T_ScheduleEntryMap& entryMap, double start, double end, DgnDbCR db)
    {
    Json::Value scheduleJson = Json::arrayValue;
    uint32_t nextEntry = 0;
    bmap<ElementSchedulePtr, uint32_t, CompareSchedules>     mergedSchedules;

    for (auto const& curr : m_elementSchedules)
        {
        auto        insertPair = mergedSchedules.Insert(curr.second, nextEntry);

        if (insertPair.second)
            nextEntry++;
        
        entryMap[curr.first] = insertPair.first->second;
        }

    for (auto const& curr : mergedSchedules)
        scheduleJson.append(curr.first->GetJson(curr.second, start, end, db));
    
    return scheduleJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Schedule (Planning::Plan::Entry const& planEntry, Planning::Plan const& plan, Planning::Baseline::Entry const& baselineEntry, DgnDbCR markupDb, DgnDbCR db, DateType dateType)
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
            SUCCESS != readStmt.GetValueDateTime(2).ToJulianDay(endJulian) ||
            endJulian <= startJulian)
            {
            //BeAssert(false);
            continue;
            }

        for (Activity::AffectedElementsEntry const& entry : Activity::MakeAffectedElementsIterator(markupDb, activityId))
            {
            DgnElementIdSet         affectedElements = PlanningUtility::GatherAffectedElements(entry.GetElementId(), markupDb, &db);
            ElementScheduleEntry    scheduleEntry(startJulian, endJulian, entry.GetAppearanceProfileId());

            for (auto& elementId : affectedElements)
                AddEntry(elementId, scheduleEntry);
            }
        }
    }
};  // Schedule


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void  PublisherContext::ExtractSchedules()
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

    DateType                            scheduleTypes[] = { DateType::Planned, DateType::Actual, DateType::Early, DateType::Late };
    int                                 scheduleId = 0;  
    bmap<T_ScheduleEntryMap, uint32_t>  entryMaps;
                                                                                                                                                                                      

    for (auto& scheduleType : scheduleTypes)
        {
        for (auto const& planEntry : Planning::Plan::MakePlanIterator(*markupDb))
            {
            PlanCPtr        plan = Planning::Plan::Get(*markupDb, planEntry.GetId());

            if (!plan.IsValid())
                {
                BeAssert(false);
                continue;
                }
            for (auto& baseline : plan->MakeBaselineIterator())
                {
                TimeSpanCP      timeSpan;

                if (nullptr != (timeSpan = plan->GetTimeSpan(baseline.GetId())))
                    {
                    double      start, end;

                    if (!extractTimeSpan (start, end, *timeSpan, scheduleType))
                        continue;

                    Schedule             rawSchedule (planEntry, *plan, baseline, *markupDb, GetDgnDb(), scheduleType);
                    T_ScheduleEntryMap   entryMap;
                    Json::Value          entriesJson = rawSchedule.GetJson(entryMap, start, end, *markupDb);

                    if (entriesJson.size() > 0)
                        {
                        Json::Value     scheduleJson = Json::objectValue;

                        scheduleJson["label"] = plan->GetUserLabel();
                        scheduleJson["type"]  = PlanningUtility::ConvertDateTypeToString(scheduleType).c_str();
                        scheduleJson["start"] = start;
                        scheduleJson["end"]   = end;
                        scheduleJson["elements"] = std::move(entriesJson);

                        auto insertPair = entryMaps.Insert(entryMap, entryMaps.size());
                        if (insertPair.second)
                            m_scheduleEntryMaps.push_back(entryMap);
                            
                        scheduleJson["index"] = insertPair.first->second;
                        m_schedulesJson.append(scheduleJson);
                        }
                    }
                }
            }
        }
    }

