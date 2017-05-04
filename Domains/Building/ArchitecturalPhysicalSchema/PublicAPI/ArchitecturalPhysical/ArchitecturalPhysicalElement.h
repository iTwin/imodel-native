/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/PublicAPI/ArchitecturalPhysical/ArchitecturalPhysicalElement.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "PlanningDefinitions.h"
#include <DgnPlatform/ECSqlClassParams.h>
#include "TimeSpan.h"

BEGIN_BENTLEY_PLANNING_NAMESPACE

//=======================================================================================
//! Base class for Planning constructs
//! @ingroup GROUP_Planning
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PlanningElement : Dgn::InformationContentElement
{
    friend struct PlanningElementHandler;
    friend struct PlanningElement;

    DGNELEMENT_DECLARE_MEMBERS(BP_CLASS_PlanningElement, Dgn::InformationContentElement);

private:
    Dgn::DgnDbStatus BindParameters(BeSQLite::EC::ECSqlStatement& statement);
    TimeSpanId QueryTimeSpanId(BaselineId baselineId) const;
    TimeSpanPtr CreateTimeSpan(BaselineId baselineId);

protected:
    int m_outlineIndex = 0;
    PlanId m_planId;
    Utf8String m_wbsCode;

    explicit PlanningElement(DgnElement::CreateParams const& params, PlanId planId = PlanId()) : T_Super(params), m_planId(planId), m_wbsCode("") {}

    void SetPlanId(PlanId planId) { m_planId = planId; }
    static PlanId GetParentPlanId(Dgn::DgnDbCR dgndb, Dgn::DgnElementId parentId);
   
    virtual Dgn::DgnClassId _GetParentRelClassId() const { return Dgn::DgnClassId(); }
    virtual PlanCP _ToPlan() const { return nullptr; }
    virtual WorkBreakdownCP _ToWorkBreakdown() const { return nullptr; }
    virtual ActivityCP _ToActivity() const { return nullptr; }

    //! Get the plan id that this WorkBreakdown/Activity
    PLANNING_EXPORT virtual PlanId _GetPlanId() const { return m_planId; }

    //! Change the parent (owner) of this WorkBreakdown/Activity.
    //! @return DgnDbStatus::Success if the parentId was changed, error status otherwise.
    //! Override to validate the parent/child relationship and return a value other than DgnDbStatus::Success to reject proposed new parent.
    //! @note If you override this method, you @b must call T_Super::_SetParentId, forwarding its status (that is, only return DgnDbStatus::Success if both your
    //! implementation and your superclass succeed.)
    PLANNING_EXPORT Dgn::DgnDbStatus _SetParentId(Dgn::DgnElementId parentId, Dgn::DgnClassId parentRelClassId) override;

    //! Called to bind the parameters when updating or inserting a new Activity into the DgnDb. Override to save subclass properties.
    //! @note If you override this method, you should bind your subclass properties
    //! to the supplied ECSqlStatement, using statement.GetParameterIndex with your property's name.
    //! And then you @em must call T_Super::_BindWriteParams
    PLANNING_EXPORT void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;

    PLANNING_EXPORT Dgn::DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, Dgn::ECSqlClassParams const& selectParams) override;

    //! Called when an element is about to be inserted into the DgnDb.
    //! @return DgnDbStatus::Success to allow the insert, otherwise it will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnInsert, forwarding its status.
    PLANNING_EXPORT Dgn::DgnDbStatus _OnInsert() override;

    //! Virtual assignment method. If your subclass has member variables, it @b must override this method and copy those values from @a source.
    //! @param[in] source The element from which to copy
    //! @note If you override this method, you @b must call T_Super::_CopyFrom, forwarding its status (that is, only return DgnDbStatus::Success if both your
    //! implementation and your superclass succeed.)
    PLANNING_EXPORT void _CopyFrom(Dgn::DgnElementCR source) override;

public:
    //! Get the id of the (root) Plan this WorkBreakdown/Activity belongs to
    PlanId GetPlanId() const { return _GetPlanId(); }

    //! Set the Parent WorkBreakdown of this planning element
    //! @param[in] parentWbId Parent WorkBreakdown Id
    //! @remarks Plan-s cannot have a parent, but only WorkBreakdown and Activities can. Call Update() to make the change to the Db.
    Dgn::DgnDbStatus SetParentWorkBreakdownId(WorkBreakdownId parentWbId) { return SetParentId(parentWbId, _GetParentRelClassId()); }

    //! Get the outline index of this WorkBreakdown/Activity. 
    //! @remarks The outline index indicates the order of the WorkBreakdown within it's parent. It is 
    //! typically used for presentation purposes.
    //! @see SetOutlineIndex
    int GetOutlineIndex() const { return m_outlineIndex; }

    //! Set the outline index of this WorkBreakdown/Activity. 
    //! @see GetOutlineIndex
    void SetOutlineIndex(int outlineIndex) { m_outlineIndex = outlineIndex; }

    //! Get the WBS Code of this WorkBreakdown/Activity. 
    //! @see SetOutlineIndex
    Utf8StringCR GetWbsCode() const { return m_wbsCode; }

    //! @see GetOutlineIndex
    void SetWbsCode(Utf8CP wbsCode) { m_wbsCode.AssignOrClear(wbsCode); }

    //! Get the TimeSpan for the Plan/WorkBreakdown/Activity to make updates
    //! @param [in] baselineId Id of the baseline
    //! @remarks For persisting the changes to the Db, call Update() afterwards.
    //! @see GetTimeSpan()
    PLANNING_EXPORT TimeSpanP GetTimeSpanP(BaselineId baselineId);

    //! Get the TimeSpan for the Plan/WorkBreakdown/Activity to make updates
    //! @param [in] baselineLabel Optional name of the baseline (pass nullptr for retrieving the primary timespan)
    //! @remarks Method first finds the BaselineId from the supplied label. For persisting the changes to the Db, call Update() afterwards.
    //! @see GetTimeSpan()
    PLANNING_EXPORT TimeSpanP GetTimeSpanP(Utf8CP baselineLabel = nullptr);

    //! @param [in] baselineId Id of the baseline
    //! @see GetTimeSpanP()
    //! @remarks The returned pointer will be freed on the next Insert() or Update() of the element. Use TimeSpanCPtr to hold the object in memory. 
    PLANNING_EXPORT TimeSpanCP GetTimeSpan(BaselineId baselineId) const;

    //! Get the TimeSpan for the Plan/WorkBreakdown/Activity to read the values
    //! @param [in] baselineLabel Optional name of the baseline (pass nullptr for retrieving the primary timespan)
    //! @see GetTimeSpanP()
    //! @remarks The returned pointer will be freed on the next Insert() or Update() of the element. Use TimeSpanCPtr to hold the object in memory. 
    PLANNING_EXPORT TimeSpanCP GetTimeSpan(Utf8CP baselineLabel = nullptr) const;

    //! Make an iterator over all TimeSpan-s in this planning elements
    PLANNING_EXPORT TimeSpan::Iterator MakeTimeSpanIterator() const;

    PlanP ToPlanP() { return const_cast<PlanP>(_ToPlan()); }  //!< more efficient substitute for dynamic_cast<PlanP>(el)
    PlanCP ToPlan() const { return _ToPlan(); }    //!< more efficient substitute for dynamic_cast<PlanCP>(el)
    bool IsPlan() const { return nullptr != ToPlan(); }     //!< Determine whether this element is a Plan or not

    WorkBreakdownP ToWorkBreakdownP() { return const_cast<WorkBreakdownP>(_ToWorkBreakdown()); }  //!< more efficient substitute for dynamic_cast<WorkBreakdownP>(el)
    WorkBreakdownCP ToWorkBreakdown() const { return _ToWorkBreakdown(); }    //!< more efficient substitute for dynamic_cast<WorkBreakdownCP>(el)
    bool IsWorkBreakdown() const { return nullptr != ToWorkBreakdown(); }     //!< Determine whether this element is a WorkBreakdown or not

    ActivityP ToActivityP() { return const_cast<ActivityP>(_ToActivity()); }  //!< more efficient substitute for dynamic_cast<ActivityP>(el)
    ActivityCP ToActivity() const { return _ToActivity(); }    //!< more efficient substitute for dynamic_cast<ActivityCP>(el)
    bool IsActivity() const { return nullptr != ToActivity(); }     //!< Determine whether this element is a Activity or not
};

//=================================================================================
//! ElementHandler for PlanningElement-s
//! @ingroup GROUP_Planning
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PlanningElementHandler : Dgn::dgn_ElementHandler::InformationContent
{
    ELEMENTHANDLER_DECLARE_MEMBERS(BP_CLASS_PlanningElement, PlanningElement, PlanningElementHandler, Dgn::dgn_ElementHandler::InformationContent, PLANNING_EXPORT)
    PLANNING_EXPORT void _RegisterPropertyAccessors(Dgn::ECSqlClassInfo&, ECN::ClassLayoutCR) override;
};

END_BENTLEY_PLANNING_NAMESPACE
