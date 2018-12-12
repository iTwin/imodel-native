/*--------------------------------------------------------------------------------------+
|
|     $Source: DataCaptureSchema/XmlWriter.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DataCaptureSchemaInternal.h"

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                Marc.Bedard                          11/2016
//---------------------------------------------------------------------------------------
BentleyStatus XmlWriter::WriteXml(BeFileNameCR xmlPathname) const
    {
    BeXmlDomPtr xmlDomPtr = BeXmlDom::CreateEmpty();
    WriteRoot(*xmlDomPtr);
    BeXmlStatus xmlStatus = xmlDomPtr->ToFile(xmlPathname, (BeXmlDom::ToStringOption) (BeXmlDom::TO_STRING_OPTION_Formatted | BeXmlDom::TO_STRING_OPTION_Indent), BeXmlDom::FILE_ENCODING_Utf8);
    return (xmlStatus == BEXML_Success) ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Marc.Bedard                          11/2016
//---------------------------------------------------------------------------------------
void XmlWriter::WriteRoot(BeXmlDomR xmlDom) const
    {
    BeXmlNodeP rootNode = xmlDom.GetRootElement();
    BeAssert(!rootNode && "Can only write to an empty XML document");

//     rootNode = xmlDom.AddNewElement("DgnDb-Data capture", nullptr, nullptr);
// 
//     WritePlans(*rootNode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Marc.Bedard                          11/2016
//---------------------------------------------------------------------------------------
void XmlWriter::AddAttributeInt64Value(BeXmlNodeR xmlNode, Utf8CP name, int64_t value)
    {
    Utf8PrintfString idStr("%lld", value);
    xmlNewProp((xmlNodePtr) &xmlNode, (xmlChar const*) name, (xmlChar const*) idStr.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Marc.Bedard                          11/2016
//---------------------------------------------------------------------------------------
void XmlWriter::AddElementUtf8Value(BeXmlNodeR xmlNode, Utf8CP name, Utf8CP value)
    {
    BeXmlDomP xmlDom = xmlNode.GetDom();
    xmlNodePtr childNode = xmlNewDocRawNode(&(xmlDom->GetDocument()), nullptr, (xmlChar const*) name, (xmlChar const*) value);
    BeAssert(childNode != nullptr);

    xmlAddChild((xmlNodePtr) &xmlNode, childNode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Marc.Bedard                          11/2016
//---------------------------------------------------------------------------------------
void XmlWriter::AddElementIntValue(BeXmlNodeR xmlNode, Utf8CP name, int value)
    {
    xmlNode.AddElementInt32Value(name, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Marc.Bedard                          11/2016
//---------------------------------------------------------------------------------------
void XmlWriter::AddElementDoubleValue(BeXmlNodeR xmlNode, Utf8CP name, double value)
    {
    xmlNode.AddElementDoubleValue(name, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Marc.Bedard                          11/2016
//---------------------------------------------------------------------------------------
void XmlWriter::AddElementInt64Value(BeXmlNodeR xmlNode, Utf8CP name, int64_t value)
    {
    Utf8PrintfString idStr("%lld", value);
    AddElementUtf8Value(xmlNode, name, idStr.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Marc.Bedard                          11/2016
//---------------------------------------------------------------------------------------
// void XmlWriter::WritePlans(BeXmlNodeR parentNode) const
//     {
//     BeXmlNodeP plansNode = parentNode.AddEmptyElement("Plans");
//     for (Plan::Entry const& planEntry : Plan::MakePlanIterator(m_dgndb))
//         {
//         PlanCPtr plan = Plan::Get(m_dgndb, planEntry.GetId());
//         BeAssert(plan.IsValid());
// 
//         WritePlan(*plansNode, *plan);
//         }
//     }


//---------------------------------------------------------------------------------------
// @bsimethod                                Marc.Bedard                          11/2016
//---------------------------------------------------------------------------------------
// void XmlWriter::WritePlan(BeXmlNodeR parentNode, PlanCR plan) const
//     {
//     BeXmlNodeP planNode = parentNode.AddEmptyElement("Plan");
// 
//     AddAttributeInt64Value(*planNode, "Id", plan.GetId().GetValueUnchecked());
//     AddElementUtf8Value(*planNode, "Label", plan.GetLabel());
//     AddElementUtf8Value(*planNode, "Code", plan.GetCode().GetValueCP());
//     AddElementIntValue(*planNode, "OutlineIndex", plan.GetOutlineIndex());
// 
//     CalendarCR calendar = plan.GetCalendar();
//     AddElementIntValue(*planNode, "MinutesPerDay", calendar.GetMinutesPerDay());
//     AddElementIntValue(*planNode, "MinutesPerWeek", calendar.GetMinutesPerWeek());
//     AddElementIntValue(*planNode, "DaysPerMonth", calendar.GetDaysPerMonth());
// 
//     WriteTimelines(*planNode, plan.GetId());
//     WriteWorkBreakdowns(*planNode, plan.GetId());
//     WriteActivities(*planNode, plan.GetId());
//     WriteTimeSpans(*planNode, plan.GetId());
//     }


//---------------------------------------------------------------------------------------
// @bsimethod                                Marc.Bedard                          11/2016
//---------------------------------------------------------------------------------------
void XmlWriter::WritePoint3d(BeXmlNodeR parentNode, Utf8CP name, DPoint3dCR point) const
    {
    BeXmlNodeP pointNode = parentNode.AddEmptyElement(name);
    AddElementDoubleValue(*pointNode, "X", point.x);
    AddElementDoubleValue(*pointNode, "Y", point.y);
    AddElementDoubleValue(*pointNode, "Z", point.z);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Marc.Bedard                          11/2016
//---------------------------------------------------------------------------------------
void XmlWriter::WritePoint2d(BeXmlNodeR parentNode, Utf8CP name, DPoint2dCR point) const
    {
    BeXmlNodeP pointNode = parentNode.AddEmptyElement(name);
    AddElementDoubleValue(*pointNode, "X", point.x);
    AddElementDoubleValue(*pointNode, "Y", point.y);
    }

END_BENTLEY_DATACAPTURE_NAMESPACE
