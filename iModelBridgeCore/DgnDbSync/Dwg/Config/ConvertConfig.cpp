/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "DwgImportInternal.h"
#include <libxml/xpathInternals.h>

struct XPathDeleter {void operator()(xmlXPathObject* x) { xmlXPathFreeObject(x); }};
typedef std::unique_ptr<xmlXPathObject,XPathDeleter> XPathObjectPtr;

BEGIN_DWG_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bern.McCarty    12/2012
//---------------------------------------------------------------------------------------
DwgImporter::Config::~Config()
    {
    if (nullptr != m_xpathContextRoot)
        xmlXPathFreeContext(m_xpathContextRoot);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bern.McCarty    12/2012
//---------------------------------------------------------------------------------------
void DwgImporter::Config::CacheOptions()
    {
    BeXmlNodeP rootElementNodeP = m_instanceDom->GetRootElement();
    BeXmlNodeP optionsNodeP = rootElementNodeP->SelectSingleNode ("Options");
    if (nullptr == optionsNodeP)
        return;

    BeXmlNodeP optionNodeP = optionsNodeP->GetFirstChild();
    while (nullptr != optionNodeP)
        {
        WString name;
        optionNodeP->GetAttributeStringValue(name, "name");
        Utf8String nameUtf8(name);
        if (optionNodeP->IsName("OptionString"))
            {
            WString value;
            optionNodeP->GetAttributeStringValue(value, "value");
            m_utf8LUT[nameUtf8] = Utf8String(value);
            }
        else if (optionNodeP->IsName("OptionBool"))
            {
            bool value;
            optionNodeP->GetAttributeBooleanValue(value, "value");
            m_boolLUT[nameUtf8] = value;
            }
        else if (optionNodeP->IsName("OptionDouble"))
            {
            double value;
            optionNodeP->GetAttributeDoubleValue(value, "value");
            m_doubleLUT[nameUtf8] = value;
            }
        else if (optionNodeP->IsName("OptionInt64"))
            {
            int64_t value;
            uint64_t* valueP = (uint64_t*) &value;
            optionNodeP->GetAttributeUInt64Value(*valueP, "value");
            m_int64LUT[nameUtf8] = value;
            }

        optionNodeP = optionNodeP->GetNextSibling();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bern.McCarty    12/2012
//---------------------------------------------------------------------------------------
void DwgImporter::Config::ReadFromXmlFile ()
    {
    if (m_instanceDom.IsValid())
        return;

    LIBXML_TEST_VERSION;

    BeXmlStatus beXmlInstanceStatus;
    m_instanceDom = BeXmlDom::CreateAndReadFromFile (beXmlInstanceStatus, m_instanceFilename.c_str());
    if (BEXML_Success != beXmlInstanceStatus)
        {
        int lineNumber, column;
        m_instanceDom->GetErrorLocation (lineNumber, column);

        WString errorMsg;
        m_instanceDom->GetErrorMessage (errorMsg);
        m_dwgImporter.ReportIssueV(DwgImporter::IssueSeverity::Error, IssueCategory::ConfigXml(), Issue::ConfigFileError(), nullptr, Utf8String(GetInstanceFilename()).c_str(), lineNumber, column, errorMsg.c_str());
        m_dwgImporter.ReportIssue(DwgImporter::IssueSeverity::Warning, IssueCategory::ConfigXml(), Issue::ConfigUsingDefault(), "");
        m_instanceDom = nullptr;
        return;
        }

    m_dwgImporter._OnConfigurationRead(*m_instanceDom);

    m_instanceDom->AcquireXPathContext(nullptr);  // acqure an XPath context in our BeXmlDom for the root node.
    m_xpathContextRoot = xmlXPathNewContext (&m_instanceDom->GetDocument());

    CacheOptions();
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                                   Bern.McCarty    12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static bool hasGoodString(xmlXPathObjectPtr xpo)
    {
    return nullptr != xpo && XPATH_STRING == xpo->type && nullptr != xpo->stringval;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                                   Bern.McCarty    12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static bool hasGoodNonEmptyString(xmlXPathObjectPtr xpo)
    {
    return hasGoodString(xpo) && '\0' != xpo->stringval[0];
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                                   Bern.McCarty    12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static bool boolValueFromString (xmlChar const* vtStrP)
    {
    bool boolValue = 0 == xmlStrcmp(vtStrP, (const xmlChar *) "1") || 0 == xmlStrcmp(vtStrP, (const xmlChar *) "true");

    return boolValue;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                                   Bern.McCarty    12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static bool hasGoodBooleanString(xmlXPathObjectPtr xpo)
    {
    if (!hasGoodNonEmptyString(xpo))
        return false;

    xmlChar const* vtStrP = xpo->stringval;
    bool boolValue =
        0 == xmlStrcmp(vtStrP, (const xmlChar *) "1") || 0 == xmlStrcmp(vtStrP, (const xmlChar *) "true") ||
        0 == xmlStrcmp(vtStrP, (const xmlChar *) "0") || 0 == xmlStrcmp(vtStrP, (const xmlChar *) "false");

    return boolValue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bern.McCarty    12/2012
//---------------------------------------------------------------------------------------
bool  DwgImporter::Config::OptionExists(Utf8CP optionName) const
    {
    Utf8String optionNameUtf8(optionName);

    bmap<Utf8String,bool>::const_iterator posBool = m_boolLUT.find(optionNameUtf8);
    if (m_boolLUT.end() != posBool)
        return true;

    bmap<Utf8String,Utf8String>::const_iterator posUtf8String = m_utf8LUT.find(optionNameUtf8);
    if (m_utf8LUT.end() != posUtf8String)
        return true;

    bmap<Utf8String,double>::const_iterator posDouble = m_doubleLUT.find(optionNameUtf8);
    if (m_doubleLUT.end() != posDouble)
        return true;

    bmap<Utf8String,int64_t>::const_iterator posInt64 = m_int64LUT.find(optionNameUtf8);
    if (m_int64LUT.end() != posInt64)
        return true;

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bern.McCarty    12/2012
//---------------------------------------------------------------------------------------
Utf8String DwgImporter::Config::GetOptionValueString(Utf8CP optionName, Utf8CP defaultVal) const
    {
    bmap<Utf8String,Utf8String>::const_iterator pos = m_utf8LUT.find(Utf8String(optionName));
    return (m_utf8LUT.end() != pos) ? pos->second : Utf8String(defaultVal);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bern.McCarty    12/2012
//---------------------------------------------------------------------------------------
bool DwgImporter::Config::GetOptionValueBool(Utf8CP optionName, bool defaultVal) const
    {
    bmap<Utf8String,bool>::const_iterator pos = m_boolLUT.find(Utf8String(optionName));
    return (m_boolLUT.end() != pos) ? pos->second : defaultVal;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bern.McCarty    12/2012
//---------------------------------------------------------------------------------------
double DwgImporter::Config::GetOptionValueDouble(Utf8CP optionName, double defaultVal) const
    {
    bmap<Utf8String,double>::const_iterator pos = m_doubleLUT.find(Utf8String(optionName));
    return (m_doubleLUT.end() != pos) ? pos->second : defaultVal;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int64_t DwgImporter::Config::GetOptionValueInt64(Utf8CP optionName, int64_t defaultVal) const
    {
    bmap<Utf8String,int64_t>::const_iterator pos = m_int64LUT.find(Utf8String(optionName));
    return (m_int64LUT.end() != pos) ? pos->second : defaultVal;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bern.McCarty    12/2012
//---------------------------------------------------------------------------------------
Utf8String DwgImporter::Config::GetXPathString(Utf8CP xpathExpression, Utf8CP defaultVal) const
    {
    if (m_instanceDom.IsNull())
        return Utf8String(defaultVal);

    XPathObjectPtr xpo(xmlXPathEvalExpression ((xmlChar const*)xpathExpression, m_xpathContextRoot));

    if (nullptr == xpo)
        return defaultVal;

    if (XPATH_STRING != xpo->type)
        xpo = XPathObjectPtr(xmlXPathConvertString(xpo.release())); // xmlXPathConvertString frees the xpo and returns a new, different one.

    return hasGoodNonEmptyString(xpo.get()) ? (Utf8CP) xpo->stringval : defaultVal;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bern.McCarty    12/2012
//---------------------------------------------------------------------------------------
bool DwgImporter::Config::GetXPathBool(Utf8CP xpathExpression, bool defaultVal) const
    {
    if (m_instanceDom.IsNull())
        return defaultVal;

    XPathObjectPtr xpo (xmlXPathEvalExpression ((xmlChar const*)xpathExpression, m_xpathContextRoot));

    if (nullptr == xpo)
        return defaultVal;

    if (XPATH_STRING != xpo->type)
        xpo = XPathObjectPtr(xmlXPathConvertString(xpo.release())); // xmlXPathConvertString frees the xpo and returns a new, different one.

    return hasGoodBooleanString(xpo.get()) ? boolValueFromString(xpo->stringval) : defaultVal;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bern.McCarty    12/2012
//---------------------------------------------------------------------------------------
double DwgImporter::Config::GetXPathDouble(Utf8CP xpathExpression, double defaultVal) const
    {
    if (m_instanceDom.IsNull())
        return defaultVal;

    XPathObjectPtr xpo (xmlXPathEvalExpression ((xmlChar const*)xpathExpression, m_xpathContextRoot));

    if (nullptr == xpo)
        return defaultVal;

    if (XPATH_NUMBER != xpo->type)
        xpo = XPathObjectPtr(xmlXPathConvertNumber(xpo.release())); // xmlXPathConvertNumber frees the xpo and returns a new, different one.

    // If xmlXPathConvertNumber cannot parse the value, it sets the value to xmlXPathNAN.
    if (xmlXPathNAN == xpo->floatval)
        return defaultVal;

    return (nullptr == xpo) ? defaultVal : xpo->floatval;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bern.McCarty    12/2012
//---------------------------------------------------------------------------------------
int64_t DwgImporter::Config::GetXPathInt64(Utf8CP xpathExpression, int64_t defaultVal) const
    {
    if (m_instanceDom.IsNull())
        return defaultVal;

    XPathObjectPtr xpo (xmlXPathEvalExpression ((xmlChar const*)xpathExpression, m_xpathContextRoot));

    if (nullptr == xpo)
        return defaultVal;

    if (XPATH_NUMBER != xpo->type)
        xpo = XPathObjectPtr(xmlXPathConvertNumber(xpo.release())); // xmlXPathConvertNumber frees the xpo and returns a new, different one.

    // If xmlXPathConvertNumber cannot parse the value, it sets the value to xmlXPathNAN.
    if (xmlXPathNAN == xpo->floatval)
        return defaultVal;

    return (nullptr == xpo) ? defaultVal : static_cast<int64_t>(xpo->floatval);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                                   Bern.McCarty    12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DwgImporter::Config::EvaluateXPath(Utf8StringR value, Utf8CP xpathExpression) const
    {
    if (m_instanceDom.IsNull())
        return BSIERROR;

    XPathObjectPtr xpo (xmlXPathEvalExpression ((xmlChar const*)xpathExpression, m_xpathContextRoot));

    if (nullptr == xpo)
        return BSIERROR;

    if (XPATH_STRING != xpo->type)
        xpo = XPathObjectPtr(xmlXPathConvertString(xpo.release())); // xmlXPathConvertString frees the xpo and returns a new, different one.

    if (nullptr == xpo)
        return BSIERROR;

    value.assign((Utf8CP) xpo->stringval);
    return SUCCESS;
    }

BeXmlDom* DwgImporter::Config::GetDom() const {return m_instanceDom.get();}

END_DWG_NAMESPACE
