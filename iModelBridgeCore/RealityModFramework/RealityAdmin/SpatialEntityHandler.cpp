/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityAdmin/SpatialEntityHandler.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeDirectoryIterator.h>
#include <BeXml/BeXml.h>

#include <RealityAdmin/SpatialEntityHandler.h>
#include <RealityPlatformTools/RealityDataDownload.h>


#define THUMBNAIL_WIDTH     512
#define THUMBNAIL_HEIGHT    512

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
SpatialEntityHandlerStatus SpatialEntityHandler::UnzipFiles(Utf8CP inputDirPath, Utf8CP outputDirPath)
    {
    // Get a list of zip files to process.
    bvector<BeFileName> fileFoundList;
    BeFileName rootDir(inputDirPath);
    BeDirectoryIterator::WalkDirsAndMatch(fileFoundList, rootDir, L"*.zip", true);

    // Unzip files.    
    for (size_t i = 0; i < fileFoundList.size(); ++i)
        {
        WString outputDirPathW(outputDirPath, BentleyCharEncoding::Utf8);
        AString outputDirPathA(outputDirPath);

        // Construct output path.
        WString outputFolderName;
        RealityDataDownload::ExtractFileName(outputFolderName, fileFoundList[i].GetNameUtf8());
        outputFolderName.erase(outputFolderName.find_last_of('.'));
        outputDirPathW.append(outputFolderName);
        BeFileName::CreateNewDirectory(outputDirPathW.c_str());

        WString filenameW(fileFoundList[i].GetName());
        RealityDataDownload::UnZipFile(filenameW, outputDirPathW);
        }

    return SpatialEntityHandlerStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
BeFileName SpatialEntityHandler::BuildMetadataFilename(Utf8CP dirPath)
    {
    bvector<BeFileName> fileFoundList;
    BeFileName rootDir(dirPath);
    BeDirectoryIterator::WalkDirsAndMatch(fileFoundList, rootDir, L"*.xml", false);

    if (fileFoundList.empty())
        return BeFileName();

    // Find the xml file corresponding to the metadata.
    for (BeFileNameCR file : fileFoundList)
        {
        // Create xmlDom from file.
        BeXmlStatus xmlStatus = BEXML_Success;
        BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, file.GetNameUtf8().c_str());
        if (BEXML_Success != xmlStatus)
            {
            return BeFileName();
            }

        // Make sure the root node is <gmd:MD_Metadata>.
        BeXmlNodeP pRootNode = pXmlDom->GetRootElement();
        if (NULL == pRootNode)
            return BeFileName();

        if (pRootNode->IsIName("MD_Metadata"))
            return file;
        }

    // No metadata file found.
    return BeFileName();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
Utf8String SpatialEntityHandler::RetrieveGeocodingFromMetadata(BeFileNameCR filename)
    {
    Utf8String geocoding;

    // Create xmlDom from metadata file.
    BeXmlStatus xmlStatus = BEXML_Success;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, filename.GetNameUtf8().c_str());
    if (BEXML_Success != xmlStatus)
        {
        return NULL;
        }

    pXmlDom->RegisterNamespace("gmd", "http://www.isotc211.org/2005/gmd");

    // Get root node.
    BeXmlNodeP pRootNode = pXmlDom->GetRootElement();
    if (NULL == pRootNode)
        return NULL;

    // Get reference system info node.
    BeXmlNodeP pRefSysNode = pRootNode->SelectSingleNode("gmd:referenceSystemInfo");
    if (NULL == pRefSysNode)
        return NULL;

    // Get md reference system node.
    BeXmlNodeP pMdRefNode = pRefSysNode->SelectSingleNode("gmd:MD_ReferenceSystem");
    if (NULL == pMdRefNode)
        return NULL;

    // Get reference system identifier node.
    BeXmlNodeP pRefSysIdNode = pMdRefNode->SelectSingleNode("gmd:referenceSystemIdentifier");
    if (NULL == pRefSysIdNode)
        return NULL;

    // Get rs identifier node.
    BeXmlNodeP pRsIdNode = pRefSysIdNode->SelectSingleNode("gmd:RS_Identifier");
    if (NULL == pRsIdNode)
        return NULL;

    // Get code.
    BeXmlNodeP pCodeNode = pRsIdNode->SelectSingleNode("gmd:code");
    if (NULL == pCodeNode)
        return NULL;

    xmlStatus = pCodeNode->GetContent(geocoding);
    if (BEXML_Success != xmlStatus)
        return NULL;

    return geocoding.Trim();
    }

