/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/UsgsSource.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <RealityPlatform/UsgsSource.h>
#include <BeXml/BeXml.h>

// Xml Fragment Tags
#define USGSSOURCE_PREFIX               "usgs"

#define USGSSOURCE_ELEMENT_Root         "Usgs"
#define USGSSOURCE_ELEMENT_Url          "Url"
#define USGSSOURCE_ELEMENT_Data         "Data"
#define USGSSOURCE_ELEMENT_SisterFiles  "SisterFiles"
#define USGSSOURCE_ELEMENT_File         "File"
#define USGSSOURCE_ELEMENT_Metadata     "Metadata"

#define USGSSOURCE_ATTRIBUTE_DataType   "type"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
UsgsSourcePtr UsgsSource::Create(Utf8CP url, Utf8CP dataType, Utf8CP metadata)
    {
    return new UsgsSource(url, dataType, metadata);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
Utf8StringCR    UsgsSource::GetUrl() const { return m_url; }
void            UsgsSource::SetUrl(Utf8CP url) { m_url = url; }

Utf8StringCR    UsgsSource::GetDataType() const { return m_dataType; }
void            UsgsSource::SetDataType(Utf8CP type) { m_dataType = type; }

Utf8StringCR    UsgsSource::GetDataLocation() const { return m_dataLocation; }
void            UsgsSource::SetDataLocation(Utf8CP locationInCompound) { m_dataLocation = locationInCompound; }

const bvector<Utf8String>&  UsgsSource::GetSisterFiles() const { return m_sisterFiles; }
void                        UsgsSource::SetSisterFiles(const bvector<Utf8String>& sisterFiles) { m_sisterFiles = sisterFiles; }

Utf8StringCR    UsgsSource::GetMetadata() const { return m_metadata; }
void            UsgsSource::SetMetadata(Utf8CP metadata) { m_metadata = metadata; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
void UsgsSource::ToXml(Utf8StringR xmlFragment) const
    {
    // Create new dom.
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateEmpty();

    // Add root node.
    BeXmlNodeP pRootNode = pXmlDom->AddNewElement(USGSSOURCE_ELEMENT_Root, NULL, NULL);

    // &&JFC WIP: Add namespace
    //pRootNode->SetNamespace(USGSSOURCE_PREFIX, NULL);

    WString temp;

    // [Required] Url  
    pRootNode->AddElementStringValue(USGSSOURCE_ELEMENT_Url, m_url.c_str());

    // [Required] DataType & [Optional] DataLocation
    BeXmlNodeP pDataNode = pRootNode->AddElementStringValue(USGSSOURCE_ELEMENT_Data, m_dataLocation.c_str());
    pDataNode->AddAttributeStringValue(USGSSOURCE_ATTRIBUTE_DataType, m_dataType.c_str());

    // [Optional] Sister files
    if (!m_sisterFiles.empty())
        {
        BeXmlNodeP pSisterFilesNode = pRootNode->AddEmptyElement(USGSSOURCE_ELEMENT_SisterFiles);
        for (Utf8StringCR filename : m_sisterFiles)
            {
            pSisterFilesNode->AddElementStringValue(USGSSOURCE_ELEMENT_File, filename.c_str());
            }
        }

    // [Required] Metadata
    pRootNode->AddElementStringValue(USGSSOURCE_ELEMENT_Metadata, m_metadata.c_str());

    // Convert to string.
    pRootNode->GetXmlString(xmlFragment);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
void UsgsSource::FromXml(Utf8CP xmlFragment)
    {
    if (NULL == xmlFragment)
        return;

    // Create XmlDom from XmlFragment
    BeXmlStatus xmlStatus = BEXML_ReadError;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlFragment);
    if (BEXML_Success != xmlStatus)
        return;

    BeXmlNodeP pRootNode = pXmlDom->GetRootElement()->SelectSingleNode(USGSSOURCE_ELEMENT_Root);
    if (NULL == pRootNode)
        return;

    BeXmlNodeP pChildNode = NULL;

    // Url
    pChildNode = pRootNode->SelectSingleNode(USGSSOURCE_ELEMENT_Url);
    if (NULL != pChildNode)
        pChildNode->GetContent(m_url);

    // Data
    pChildNode = pRootNode->SelectSingleNode(USGSSOURCE_ELEMENT_Data);
    if (NULL != pChildNode)
        {
        pChildNode->GetContent(m_dataLocation);
        pChildNode->GetAttributeStringValue(m_dataType, USGSSOURCE_ATTRIBUTE_DataType);
        }

    // Sister files
    pChildNode = pRootNode->SelectSingleNode(USGSSOURCE_ELEMENT_SisterFiles);
    if (NULL != pChildNode)
        {
        BeXmlDom::IterableNodeSet fileNodes;
        pChildNode->SelectChildNodes(fileNodes, USGSSOURCE_ELEMENT_File);

        Utf8String filename;
        for (BeXmlNodeP const& pFileNode : fileNodes)
            {         
            pFileNode->GetContent(filename);
            m_sisterFiles.push_back(filename.c_str());
            }
        }

    // Metadata
    pChildNode = pRootNode->SelectSingleNode(USGSSOURCE_ELEMENT_Metadata);
    if (NULL != pChildNode)
        pChildNode->GetContent(m_metadata);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
UsgsSource::UsgsSource(Utf8CP url, Utf8CP dataType, Utf8CP metadata)
    : m_url(url),
      m_dataType(dataType),
      m_dataLocation("Location unknown"),       // Default.
      m_metadata(metadata)
    {
    m_sisterFiles = bvector<Utf8String>();      // Default.
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
UsgsSource::~UsgsSource() {}