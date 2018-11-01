/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/DataCaptureSchema/XmlWriter.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "DataCaptureSchemaDefinitions.h"
#include <BeXml/BeXml.h>

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE

//=======================================================================================
//! Utility to write the planning data model to an XML file
//! @see XmlReader
//! @ingroup PlanningGroup
//=======================================================================================
struct XmlWriter
{
private:
    Dgn::DgnDbCR m_dgndb;

    BentleyStatus WriteXml(BeXmlDomR xmlDom) const;

    void WriteRoot(BeXmlDomR xmlDom) const;

//     void WritePlans(BeXmlNodeR parentNode) const;
//     void WritePlan(BeXmlNodeR plansNode, PlanCR plan) const;

    void WritePoint3d(BeXmlNodeR parentNode, Utf8CP name, DPoint3dCR point) const;
    void WritePoint2d(BeXmlNodeR parentNode, Utf8CP name, DPoint2dCR point) const;

    static void AddAttributeInt64Value(BeXmlNodeR xmlNode, Utf8CP name, int64_t value);
    static void AddElementUtf8Value(BeXmlNodeR xmlNode, Utf8CP name, Utf8CP value);
    static void AddElementIntValue(BeXmlNodeR xmlNode, Utf8CP name, int value);
    static void AddElementIdValue(BeXmlNodeR xmlNode, Utf8CP name, BeSQLite::BeBriefcaseBasedId id);
    static void AddElementInt64Value(BeXmlNodeR xmlNode, Utf8CP name, int64_t value);
    static void AddElementDoubleValue(BeXmlNodeR xmlNode, Utf8CP name, double value);

public:
    //! Constructor
    XmlWriter(Dgn::DgnDbCR dgndb) : m_dgndb(dgndb) {}

    //! Write an XML file with all the contents of the Data Capture model
    DATACAPTURE_EXPORT BentleyStatus WriteXml(BeFileNameCR xmlPathname) const;
};

END_BENTLEY_DATACAPTURE_NAMESPACE

