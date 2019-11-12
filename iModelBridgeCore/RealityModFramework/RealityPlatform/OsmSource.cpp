/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <RealityPlatform/OsmSource.h>
#include <BeXml/BeXml.h>
#include <regex>

// Xml Fragment Tags
#define OSMSOURCE_PREFIX                        "osm"
        
#define OSMSOURCE_ELEMENT_Root                  "OsmResource"
#define OSMSOURCE_ELEMENT_AlternateUrl          "AlternateUrl"
#define OSMSOURCE_ELEMENT_Url                   "Url"

// lat/long precision of 0.1 millimeter
// g == The precision specifies the maximum number of significant digits printed. Any trailing zeros are truncated. 
// So to get get the precision we want 9 decimal AFTER the point we need 12 digits. xxx.123456789
#define LATLONG_PRINT_FORMAT                    "%.12g,%.12g"    //  lat/long precision of 0.1 millimeter.

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

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

    

    Utf8String bboxString = "";
    bvector<Utf8String> urlList = bvector<Utf8String>();

    // Alternate url list.
     BeXmlNodeP pChildNode = pRootNode->SelectSingleNode(OSMSOURCE_ELEMENT_AlternateUrl);
    if (NULL != pChildNode)
        {
        BeXmlDom::IterableNodeSet urlNodes;
        pChildNode->SelectChildNodes(urlNodes, OSMSOURCE_ELEMENT_Url);

        std::regex starRegEx("([^\\*]+?)(?:\\*)([\\S]*)");
        // [0] = https://secondurl.com*[bbox=0,0,100,100]
        // [1] = https://secondurl.com
        // [2] = [bbox=0,0,100,100]
        std::cmatch matches;

        Utf8String url;        
        for (BeXmlNodeP const& pUrlNode : urlNodes)
            {
            pUrlNode->GetContent(url);

            std::regex_match(url.c_str(), matches, starRegEx);
            BeAssert(3 == matches.size());

            if(bboxString.empty())
                {
                bboxString = matches[2].str().c_str();
                }
            else
                {
                // All bounding box string should be exactly the same;
                BeAssert(bboxString.Equals(matches[2].str().c_str()));
                }
            
            
            urlList.push_back(matches[1].str().c_str());
            }

        

        }

    OsmResourcePtr pOsmResource = nullptr;
    if(!urlList.empty())
        {
        size_t pos = bboxString.find("=");
        size_t posComma = bboxString.find(",");
        size_t posNextComma = bboxString.find(",", posComma + 1);

        double minX = atof(bboxString.substr(pos + 1, posComma - pos).c_str());
        double minY = atof(bboxString.substr(posComma + 1, posNextComma - posComma).c_str());

        posComma = posNextComma;
        posNextComma = bboxString.find(",", posComma + 1);
        double maxX = atof(bboxString.substr(posComma + 1, posNextComma - posComma).c_str());

        posComma = posNextComma;
        posNextComma = bboxString.find(",", posComma + 1);
        double maxY = atof(bboxString.substr(posComma + 1, posNextComma - posComma).c_str());


        pOsmResource = OsmResource::Create(DRange2d::From(minX, minY, maxX, maxY));
        pOsmResource->SetAlternateUrlList(urlList);
        }
    else
        {
        pOsmResource = OsmResource::Create(DRange2d::NullRange());
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
