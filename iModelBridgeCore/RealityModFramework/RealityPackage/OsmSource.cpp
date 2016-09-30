/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPackage/OsmSource.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <RealityPackage/OsmSource.h>
#include <BeXml/BeXml.h>

// Xml Fragment Tags
#define OSMSOURCE_PREFIX                        "osm"
        
#define OSMSOURCE_ELEMENT_Root                  "OsmResource"
#define OSMSOURCE_ELEMENT_AlternateUrl          "AlternateUrl"
#define OSMSOURCE_ELEMENT_Url                   "Url"

// lat/long precision of 0.1 millimeter
// g == The precision specifies the maximum number of significant digits printed. Any trailing zeros are truncated. 
// So to get get the precision we want 9 decimal AFTER the point we need 12 digits. xxx.123456789
#define LATLONG_PRINT_FORMAT                    "%.12g,%.12g"    //  lat/long precision of 0.1 millimeter.

USING_NAMESPACE_BENTLEY_REALITYPACKAGE

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
OsmResourcePtr OsmResource::Create(DRange2dCR bbox)
    {
    return new OsmResource(bbox);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
OsmResourcePtr OsmResource::CreateFromXml(Utf8CP xmlFragment)
    {
    if (NULL == xmlFragment)
        return 0;

    // Create XmlDom from XmlFragment
    BeXmlStatus xmlStatus = BEXML_ReadError;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlFragment);
    if (BEXML_Success != xmlStatus)
        return 0;

    BeXmlNodeP pRootNode = pXmlDom->GetRootElement()->SelectSingleNode(OSMSOURCE_ELEMENT_Root);
    if (NULL == pRootNode)
        return 0;

    OsmResourcePtr pOsmResource = OsmResource::Create(DRange2d::NullRange());

    // Alternate url list.
     BeXmlNodeP pChildNode = pRootNode->SelectSingleNode(OSMSOURCE_ELEMENT_AlternateUrl);
    if (NULL != pChildNode)
        {
        BeXmlDom::IterableNodeSet urlNodes;
        pChildNode->SelectChildNodes(urlNodes, OSMSOURCE_ELEMENT_Url);

        Utf8String url;
        bvector<Utf8String> urlList = bvector<Utf8String>();
        for (BeXmlNodeP const& pUrlNode : urlNodes)
            {
            pUrlNode->GetContent(url);
            urlList.push_back(url.c_str());
            }

        pOsmResource->SetAlternateUrlList(urlList);
        }

    return pOsmResource;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
const bvector<Utf8String>&  OsmResource::GetAlternateUrlList() const { return m_alternateUrlList; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
void OsmResource::SetAlternateUrlList(const bvector<Utf8String>& alternateUrlList) 
    { 
    // Convert bbox to a comma separated string.
    Utf8String result;
    Utf8PrintfString lowPtStr(LATLONG_PRINT_FORMAT ",", m_bbox.low.x, m_bbox.low.y);
    result.append(lowPtStr);
    Utf8PrintfString highPtStr(LATLONG_PRINT_FORMAT ",", m_bbox.high.x, m_bbox.high.y);
    result.append(highPtStr);

    // Remove extra comma
    if (result.size() > 1)
        result.resize(result.size() - 1);

    // Append bbox to urls.
    for (Utf8String url : alternateUrlList)
        {
        m_alternateUrlList.push_back(url + "*[bbox=" + result + "]");
        }
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
void OsmResource::ToXml(Utf8StringR xmlFragment) const
    {
    // Create new dom.
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateEmpty();

    // Add root node.
    BeXmlNodeP pRootNode = pXmlDom->AddNewElement(OSMSOURCE_ELEMENT_Root, NULL, NULL);

    // &&JFC WIP: Add namespace.
    //pRootNode->SetNamespace(OSMSOURCE_PREFIX, NULL);

    // [Optional] Alternate url list.
    if (!m_alternateUrlList.empty())
        {
        BeXmlNodeP pAlternateUrlListNode = pRootNode->AddEmptyElement(OSMSOURCE_ELEMENT_AlternateUrl);
        for (Utf8StringCR url : m_alternateUrlList)
            {
            pAlternateUrlListNode->AddElementStringValue(OSMSOURCE_ELEMENT_Url, url.c_str());
            }
        }

    // Convert to string.
    pRootNode->GetXmlString(xmlFragment);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
OsmResource::OsmResource(DRange2dCR bbox)
    : m_bbox(bbox)
    {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
OsmResource::~OsmResource() {}
