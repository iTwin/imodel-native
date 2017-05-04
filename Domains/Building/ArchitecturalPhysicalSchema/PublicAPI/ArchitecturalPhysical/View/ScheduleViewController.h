/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/PublicAPI/ArchitecturalPhysical/View/ScheduleViewController.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "Planning/View/PlanningViewDefinitions.h"

BEGIN_BENTLEY_PLANNING_NAMESPACE

//=======================================================================================
//! View controller used in schedule playback
//! @see SchedulePlayer
//! @ingroup PlanningGroup
//=======================================================================================
struct ScheduleViewController : Dgn::SpatialViewController
    {
    DEFINE_T_SUPER(Dgn::SpatialViewController)

    private:
        ScheduleChannelCR m_scheduleChannel;

        virtual void _OverrideGraphicParams(Dgn::Render::OvrGraphicParamsR overide, Dgn::GeometrySourceCP source) override;

    public:
        //! Constructor
        ScheduleViewController(Dgn::SpatialViewDefinition const& view, ScheduleChannelCR scheduleChannel) : T_Super(view), m_scheduleChannel(scheduleChannel) {}
    };

END_BENTLEY_PLANNING_NAMESPACE

