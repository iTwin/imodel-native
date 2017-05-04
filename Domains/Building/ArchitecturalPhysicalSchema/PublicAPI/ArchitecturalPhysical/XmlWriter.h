/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/PublicAPI/ArchitecturalPhysical/XmlWriter.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "PlanningDefinitions.h"
#include <BeXml/BeXml.h>

BEGIN_BENTLEY_PLANNING_NAMESPACE

//=======================================================================================
//! Utility to write the planning data model to an XML file
//! @see XmlReader
//! @ingroup GROUP_Planning
//=======================================================================================
struct XmlWriter
{
//! Options to control the writer
struct Options
    {
    private:
        bool m_writeExternalLinks;
        Dgn::DgnDbCP m_referenceDb;
    public:
        Options() : m_writeExternalLinks(false), m_referenceDb(nullptr) {}

        //! Set up to write external links (when writing schedules from a markup db)
        //! @param referenceDb [in] DgnDb containing the elements the links point to. 
        void SetWriteExternalLinks(Dgn::DgnDbCR referenceDb) { m_referenceDb = &referenceDb; m_writeExternalLinks = true; }

        bool GetWriteExternalLinks() const { return m_writeExternalLinks; }

        Dgn::DgnDbCP GetReferenceDb() const { return m_referenceDb; }
    };

private:
    Dgn::DgnDbCR m_dgndb;
    Options m_options;

    BentleyStatus WriteXml(BeXmlDomR xmlDom) const;

    void WriteRoot(BeXmlDomR xmlDom) const;

    void WritePlans(BeXmlNodeR parentNode) const;
    void WritePlan(BeXmlNodeR plansNode, PlanCR plan) const;

    void WriteBaselines(BeXmlNodeR parentNode, PlanCR plan) const;
    void WriteBaseline(BeXmlNodeR parentNode, Baseline::Entry const& baseline) const;
    //void WriteCameraAnimations(BeXmlNodeR parentNode, BaselineId baselineId) const;
    //void WriteCameraAnimation(BeXmlNodeR parentNode, CameraAnimationId cameraAnimationId) const;
    //void WriteCameraKeyFrames(BeXmlNodeR parentNode, CameraAnimationId animationId) const;

    void WriteWorkBreakdowns(BeXmlNodeR parentNode, PlanId planId) const;
    void WriteWorkBreakdown(BeXmlNodeR parentNode, WorkBreakdownCR wb) const;
    void WriteActivities(BeXmlNodeR parentNode, PlanId planId) const;
    void WriteActivity(BeXmlNodeR parentNode, ActivityCR activity) const;
    void WriteActivityPredecessors(BeXmlNodeR parentNode, ActivityId activityId) const;
    void WriteActivityAffectedElements(BeXmlNodeR parentNode, ActivityId activityId) const;
    void WriteExternalLinks(BeXmlNodeR parentNode, Dgn::DgnElementId markupElementId) const;

    void WriteTimeSpans(BeXmlNodeR parentNode, PlanningElementCR planningElement) const;
    void WriteTimeSpan(BeXmlNodeR parentNode, TimeSpan::Entry const& timeSpan) const;
    void WriteDateTime(BeXmlNodeR parentNode, Utf8CP name, DateTimeCR dateTime) const;
    void WriteDuration(BeXmlNodeR parentNode, Utf8CP name, DurationCR duration) const;
    void WritePoint3d(BeXmlNodeR parentNode, Utf8CP name, DPoint3dCR point) const;
    void WritePoint2d(BeXmlNodeR parentNode, Utf8CP name, DPoint2dCR point) const;

    static void AddAttributeInt64Value(BeXmlNodeR xmlNode, Utf8CP name, int64_t value);
    static void AddAttributeUtf8Value(BeXmlNodeR xmlNode, Utf8CP name, Utf8CP value);
    static void AddElementUtf8Value(BeXmlNodeR xmlNode, Utf8CP name, Utf8CP value);
    static void AddElementIntValue(BeXmlNodeR xmlNode, Utf8CP name, int value);
    static void AddElementIdValue(BeXmlNodeR xmlNode, Utf8CP name, BeSQLite::BeBriefcaseBasedId id);
    static void AddElementInt64Value(BeXmlNodeR xmlNode, Utf8CP name, int64_t value);
    static void AddElementDoubleValue(BeXmlNodeR xmlNode, Utf8CP name, double value);

public:
    //! Constructor
    XmlWriter(Dgn::DgnDbCR dgndb, Options options = Options()) : m_dgndb(dgndb), m_options(options) {}

    //! Write an XML file with all the contents of the planning data model
    PLANNING_EXPORT BentleyStatus WriteXml(BeFileNameCR xmlPathname) const;
};

END_BENTLEY_PLANNING_NAMESPACE

