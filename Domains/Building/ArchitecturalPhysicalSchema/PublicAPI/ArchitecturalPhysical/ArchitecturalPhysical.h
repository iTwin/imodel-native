/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "PlanningDefinitions.h"
#include "PlanningUtility.h"
#include "WorkBreakdown.h"
#include "Scheduler.h"
#include "Baseline.h"
#include <DgnPlatform/ECSqlClassParams.h>

BEGIN_BENTLEY_PLANNING_NAMESPACE

#define PLAN_ECSQL_PREFIX "plan"

//=======================================================================================
//! Represents a calendar used in Planning
//! @see Duration, Plan
//! @ingroup GROUP_Planning
//=======================================================================================
struct Calendar
{
private:
    int m_minutesPerDay = 480;
    int m_minutesPerWeek = 2400;
    int m_daysPerMonth = 20;

public:
    //! Create a calendar
    Calendar() {}

    //! Create a calendar
    Calendar(int minutesPerDay, int minutesPerWeek, int daysPerMonth) : m_minutesPerDay(minutesPerDay), m_minutesPerWeek(minutesPerWeek), m_daysPerMonth(daysPerMonth) {}

    //! Get the minutes per day  (defaults to 480, i.e., a 8 hour day)
    int GetMinutesPerDay() const { return m_minutesPerDay; }

    //! Set the minutes per day  (defaults to 480, i.e., a 8 hour day)
    void SetMinutesPerDay(int minutesPerDay) { m_minutesPerDay = minutesPerDay; }

    //! Get the minutes per day  (defaults to 2400, i.e., a 5 day week and 8 hour days)
    int GetMinutesPerWeek() const { return m_minutesPerWeek; }

    //! Set the minutes per day  (defaults to 2400, i.e., a 5 day week and 8 hour days)
    void SetMinutesPerWeek(int minutesPerWeek) { m_minutesPerWeek = minutesPerWeek; }

    //! Get the days per month (defaults to 20, i.e., 4 weeks and 5 hour days)
    int GetDaysPerMonth() const { return m_daysPerMonth; }

    //! Set the days per month (defaults to 20, i.e., 4 weeks and 5 day weeks)
    void SetDaysPerMonth(int daysPerMonth) { m_daysPerMonth = daysPerMonth; }

    //! Equality operator for calendars
    bool operator==(CalendarCR rhs) const
        {
        return m_minutesPerDay == rhs.m_minutesPerDay && m_minutesPerWeek == rhs.m_minutesPerWeek && m_daysPerMonth == rhs.m_daysPerMonth;
        }

    //! Inequality operator for calendars
    bool operator!=(CalendarCR rhs) const
        {
        return !(*this == rhs);
        }
};

//=======================================================================================
//! Data access helper functions for the Plan class. Plan represents some work done to achieve
//! some result. A plan usually contains a hierarchical WorkBreakdown and Activity-s.
//! @ingroup GROUP_Planning
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Plan : WorkBreakdown
{
    friend struct PlanElementHandler;

    //! Entry in Plan iterator
    struct Entry : Dgn::ECSqlStatementEntry
    {
        friend struct Dgn::ECSqlStatementIterator < Plan::Entry >;
        friend struct Plan;
    private:
        Entry(BeSQLite::EC::ECSqlStatement* statement = nullptr) : Dgn::ECSqlStatementEntry(statement) {}
    public:
        PlanId GetId() const { return m_statement->GetValueId<PlanId>(0); }

        //! Get the outline index of this Plan. 
        //! @remarks The outline index indicates the order of the Plan within the DgnDb. It is 
        //! typically used for presentation purposes.
        int GetOutlineIndex() const { return m_statement->GetValueInt(1); }

        //! Get the calendar
        Calendar GetCalendar() const
            {
            return Calendar(m_statement->GetValueInt(2), m_statement->GetValueInt(3), m_statement->GetValueInt(4));
            }

        //! Get the code
        Dgn::DgnCode GetCode() const { return Dgn::DgnCode(m_statement->GetValueId<Dgn::CodeSpecId>(5), m_statement->GetValueText(6), m_statement->GetValueText(7)); }

        //! Get the plan's display label
        Utf8CP GetUserLabel() const { return m_statement->GetValueText(8); }
    };

    //! Iterator over plans
    struct Iterator : Dgn::ECSqlStatementIterator < Plan::Entry >
    {
    };

private:
    Calendar m_calendar;
    BaselinePtr m_primaryBaseline;

    Dgn::DgnDbStatus _LoadFromDb() override;
    PlanId _GetPlanId() const final { return PlanId(GetElementId().GetValueUnchecked()); }
    Dgn::DgnClassId _GetParentRelClassId() const override { return Dgn::DgnClassId(); }
    PlanCP _ToPlan() const override { return this; }

    DGNELEMENT_DECLARE_MEMBERS(BP_CLASS_Plan, WorkBreakdown);

protected:
    explicit Plan(DgnElement::CreateParams const& params) : T_Super(params, PlanId()), m_primaryBaseline(nullptr) {}
    
    //! @see DgnElement::_ReadSelectParams()
    PLANNING_EXPORT Dgn::DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, Dgn::ECSqlClassParams const& selectParams) override;

    //! @see DgnElement::_BindWriteParams()
    PLANNING_EXPORT void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;

    //! @see DgnElement::_CopyFrom()
    PLANNING_EXPORT void _CopyFrom(Dgn::DgnElementCR source, CopyFromOptions const& opts) override;

public:
    //! Create a new plan
    //! @param[in] model PlanningModel that has been already written to the database, and has a valid id
    //! @param[in] label Label of the new plan
    PLANNING_EXPORT static PlanPtr Create(PlanningModelR model, Utf8CP label);

    //! Insert the Plan in the DgnDb
    PLANNING_EXPORT PlanCPtr Insert(Dgn::DgnDbStatus* stat = nullptr);

    //! Get a read only copy of the Plan from the DgnDb
    //! @return Invalid if the Plan does not exist
    PLANNING_EXPORT static PlanCPtr Get(Dgn::DgnDbCR dgnDb, Dgn::DgnElementId planId);

    //! Get an editable copy of the Plan from the DgnDb
    //! @return Invalid if the Plan does not exist, or if it cannot be edited.
    PLANNING_EXPORT static PlanPtr GetForEdit(Dgn::DgnDbR dgnDb, Dgn::DgnElementId planId);

    //! Update the persistent state of the Plan in the DgnDb from this modified copy of it. 
    PLANNING_EXPORT PlanCPtr Update(Dgn::DgnDbStatus* stat = nullptr);

    //! Query the DgnClassId of the planning.Plan ECClass in the specified DgnDb.
    //! @note This is a static method that always returns the DgnClassId of the planning.Plan class - it does @em not return the class of a specific instance.
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbR dgndb) { return Dgn::DgnClassId(dgndb.Schemas().GetClassId(BENTLEY_PLANNING_SCHEMA_NAME, BP_CLASS_Plan)); }

    //! Get the id of this Plan
    PlanId GetId() const { return PlanId(GetElementId().GetValueUnchecked()); }

    //! Get an editable reference to the the Calendar used by this Plan
    CalendarR GetCalendarR() { return m_calendar; }

    //! Get a read only reference to the Calendar used by this Plan
    CalendarCR GetCalendar() const { return m_calendar; }

    //! Create a new baseline for the plan
    //! @remarks For persisting the baseline in the Db, call @ref Plan::Insert() or @ref Plan::Update() after creating the baseline. 
    PLANNING_EXPORT BaselineP CreateBaseline(Utf8CP baselineLabel);

    //! Get an existing baseline from the plan to make updates
    //! @remarks For persisting the baseline changes in the Db, call @ref Plan::Update() after updating the baseline
    //! @see GetBaseline()
    PLANNING_EXPORT BaselineP GetBaselineP(Utf8CP baselineLabel);

    //! Get an existing baseline from the plan to read the values
    //! @see GetBaselineP()
    //! @remarks The returned pointer will be freed on the next Insert() or Update() of the element. Use BaselineCPtr to hold the object in memory. 
    PLANNING_EXPORT BaselineCP GetBaseline(Utf8CP baselineLabel) const;

    //! Check if the plan contains a baseline with the specified label
    bool ContainsBaseline(Utf8CP baselineLabel) const;

    //! Make an iterator over Plan-s given a ECSQL where and orderBy clause
    //! @remarks Use the macro PLAN_ECSQL_PREFIX as the alias for Plan in the where and orderBy clause.
    PLANNING_EXPORT static Plan::Iterator MakePlanIterator(Dgn::DgnDbCR dgndb, Utf8CP whereClause = nullptr, Utf8CP orderByClause = nullptr);

    //! Make an iterator over all Baseline-s in this plan
    PLANNING_EXPORT Baseline::Iterator MakeBaselineIterator() const;

    //! Query for an Plan (Id) by label
    //! @return Id of the Plan or invalid Id if an Plan was not found
    PLANNING_EXPORT static PlanId QueryForIdByLabel(Dgn::DgnDbR dgndb, Utf8CP planLabel);
};

//=================================================================================
//! ElementHandler for Plan Elements
//! @ingroup GROUP_Planning
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PlanElementHandler : WorkBreakdownElementHandler
{
    ELEMENTHANDLER_DECLARE_MEMBERS(BP_CLASS_Plan, Plan, PlanElementHandler, WorkBreakdownElementHandler, PLANNING_EXPORT)
    PLANNING_EXPORT void _RegisterPropertyAccessors(Dgn::ECSqlClassInfo&, ECN::ClassLayoutCR) override;
    PLANNING_EXPORT bool _IsRestrictedAction(uint64_t action) const override;
};

END_BENTLEY_PLANNING_NAMESPACE
