/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/PublicAPI/ArchitecturalPhysical/ArchitecturalPhysicalUtility.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "PlanningDefinitions.h"
#include "Duration.h"
#include "Activity.h"
#include "CameraAnimation.h"

BEGIN_BENTLEY_PLANNING_NAMESPACE

//=======================================================================================
//! ScheduleDateType 
//! @ingroup GROUP_Planning
//=======================================================================================
enum class DateType
{
    Invalid = -1,
    Planned = 0,
    Actual = 1,
    Early = 2,
    Late = 3
};

//=======================================================================================
//! PlanningUtility
//! @ingroup GROUP_Planning
//=======================================================================================
struct PlanningUtility
{
private:
    static void AddMemberElements(Dgn::DgnElementIdSet& elementIdSet, Dgn::DgnElementCR element, Dgn::DgnDbCR dgnDb);

public:
    PLANNING_EXPORT static Utf8String ConvertDateTypeToString(DateType dateType);
    PLANNING_EXPORT static DateType ConvertStringToDateType(Utf8CP dateTypeStr);

    PLANNING_EXPORT static Utf8String ConvertElementAppearanceProfileTypeToString(ElementAppearanceProfile::Type appearanceProfileType);
    PLANNING_EXPORT static ElementAppearanceProfile::Type ConvertStringToElementAppearanceProfileType(Utf8CP appearanceProfileTypeStr);

    PLANNING_EXPORT static Utf8String ConvertDurationFormatToString(Duration::Format durationFormat);
    PLANNING_EXPORT static Duration::Format ConvertStringToDurationFormat(Utf8CP durationFormatStr);
    
    PLANNING_EXPORT static Utf8String ConvertConstraintTypeToString(Activity::ConstraintType constraintType);
    PLANNING_EXPORT static Activity::ConstraintType ConvertStringToConstraintType(Utf8CP constraintTypeStr);

    PLANNING_EXPORT static Utf8String ConvertInterpolationTypeToString(CameraAnimation::InterpolationType interpolationType);
    PLANNING_EXPORT static CameraAnimation::InterpolationType ConvertStringToInterpolationType(Utf8CP interpolationTypeStr);

    PLANNING_EXPORT static void QueryElementAndMembers(Dgn::DgnElementIdSet& elementIdSet, Dgn::DgnElementId elementId, Dgn::DgnDbCR dgnDb);
    PLANNING_EXPORT static void ResolveMarkupExternalLinks(Dgn::DgnElementIdSet& linkedIds, Dgn::DgnElementIdSet const& linkIds, Dgn::DgnDbCR markupDb);
};

END_BENTLEY_PLANNING_NAMESPACE

