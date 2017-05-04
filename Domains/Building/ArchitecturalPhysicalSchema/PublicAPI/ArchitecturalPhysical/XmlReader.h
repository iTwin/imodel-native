/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/PublicAPI/ArchitecturalPhysical/XmlReader.h $
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
//! Utility to read the planning data model from an XML file
//! @see XmlWriter
//! @ingroup GROUP_Planning
//=======================================================================================
struct XmlReader
{
    enum class Status
        {
        Success = SUCCESS,
        Empty = 1,
        Error = ERROR
        };

    //! Options to control the reader
    struct Options
        {
        private:
        bool m_readExternalLinks;
        Dgn::DgnDbCP m_referenceDb;
        Dgn::LinkModelP m_linkModel;
        public:
            Options() : m_readExternalLinks(false), m_referenceDb(nullptr), m_linkModel(nullptr) {}

            //! Set up read of external links (when reading schedules to a markup db)
            //! @param linkModel [in] Model (in the markup db) where the external links should be imported
            //! @param referenceDb [in] DgnDb containing the elements the links point to. 
            void SetReadExternalLinks(Dgn::LinkModelR linkModel, Dgn::DgnDbCR referenceDb) { m_linkModel = &linkModel; m_referenceDb = &referenceDb; m_readExternalLinks = true; }

            bool GetReadExternalLinks() const { return m_readExternalLinks; }

            Dgn::DgnDbCP GetReferenceDb() const { return m_referenceDb; }

            Dgn::LinkModelP GetLinkModel() const { return m_linkModel; }
        };

private:
    PlanningModelR m_model;
    Dgn::DgnDbR m_dgndb;
    Options m_options;
    bmap<int64_t, Dgn::DgnElementId> m_elementIdMap;
    bmap<int64_t, BaselineId> m_baselineIdMap;

    static int64_t GetAttributeInt64Value(BeXmlNodeR node, Utf8CP name);
    static int64_t GetContentInt64Value(BeXmlNodeR node, Utf8CP name);

    static XmlReader::Status ReadDateTime(DateTimeR dateTime, BeXmlNodeR parentNode, Utf8CP name);
    static XmlReader::Status ReadDuration(DurationR duration, BeXmlNodeR parentNode, Utf8CP name);
    static XmlReader::Status ReadPoint3d(DPoint3dR point, BeXmlNodeR parentNode, Utf8CP name);
    static XmlReader::Status ReadPoint2d(DPoint2dR point, BeXmlNodeR parentNode, Utf8CP name);

    XmlReader::Status ReadPlans(BeXmlNodeR parentNode);
    XmlReader::Status ReadPlan(BeXmlNodeR planNode);
    XmlReader::Status ReadWorkBreakdowns(BeXmlNodeR planNode, PlanR plan);
    XmlReader::Status ReadWorkBreakdown(BeXmlNodeR workBreakdownNode, PlanR plan);
    XmlReader::Status ReadActivities(BeXmlNodeR planNode, PlanR plan);
    XmlReader::Status ReadActivity(BeXmlNodeR activityNode, PlanR plan);
    XmlReader::Status ReadBaselines(BeXmlNodeR planNode, PlanR plan);
    XmlReader::Status ReadBaseline(BeXmlNodeR baselineNode, PlanR plan);

    //XmlReader::Status ReadCameraAnimations(BeXmlNodeR baselineNode, BaselineId baselineId);
    //XmlReader::Status ReadCameraAnimation(BeXmlNodeR parentNode, BaselineId baselineId, DateType dateType);
    //XmlReader::Status ReadCameraKeyFrames(BeXmlNodeR animationNode, CameraAnimationId animationId);
    //XmlReader::Status ReadCameraKeyFrame(BeXmlNodeR keyframeNode, CameraAnimationId animationId);

    XmlReader::Status ReadTimeSpans(BeXmlNodeR parentNode, PlanningElementR planningElement);
    XmlReader::Status ReadTimeSpan(BeXmlNodeR timeSpanNode, PlanningElementR planningElement);
    XmlReader::Status ReadParent(BeXmlNodeR elementNode, Dgn::DgnElementId elementId);
    XmlReader::Status ReadActivityConstraints(BeXmlNodeR activityNode, ActivityId activityId);
    XmlReader::Status ReadAffectedElements(BeXmlNodeR activityNode, ActivityId activityId);
    Dgn::DgnElementId ReadExternalLinks(BeXmlNodeR affectedElementNode);

public:
    //! Constructor
    PLANNING_EXPORT XmlReader(PlanningModelR model, Options options = Options()) : m_model(model), m_dgndb(model.GetDgnDb()), m_options(options) {}
    
    //! Read an XML file containing the Planning Data Model
    PLANNING_EXPORT BentleyStatus ReadXml(BeFileNameCR xmlPathname);
};

END_BENTLEY_PLANNING_NAMESPACE



