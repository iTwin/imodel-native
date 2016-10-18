/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPackage/RealitySerializationV2.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if !defined(ANDROID)
#pragma once
#endif

#include <RealityPackage/RealityPackage.h>
#include <RealityPackage/RealityDataPackage.h>
#include <RealityPackage/RealityDataSource.h>
#include <BeXml/BeXml.h>
#include "RealitySerialization.h"


BEGIN_BENTLEY_REALITYPACKAGE_NAMESPACE


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityDataSerializerV2Ptr RealityDataSerializerV2::Create()
    {
    return new RealityDataSerializerV2();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityDataSerializerV2::RealityDataSerializerV2() {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV2::_Read(RealityDataPackageR package, BeXmlDomR xmlDom)
    {
    RealityPackageStatus status = RealityPackageStatus::UnknownError;

    // Package basic info.
    status = ReadPackageInfo(package, xmlDom);
    if (RealityPackageStatus::Success != status)
        return status;

    // Imagery group.
    status = ReadImageryGroup(package, xmlDom);
    if (RealityPackageStatus::Success != status)
        return status;

    // Model group.
    status = ReadModelGroup(package, xmlDom);
    if (RealityPackageStatus::Success != status)
        return status;

    // Pinned group.
    status = ReadPinnedGroup(package, xmlDom);
    if (RealityPackageStatus::Success != status)
        return status;

    // Terrain group.
    status = ReadTerrainGroup(package, xmlDom);
    if (RealityPackageStatus::Success != status)
        return status;

    return status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV2::ReadPackageInfo(RealityDataPackageR package, BeXmlDomR xmlDom)
    {
    // Namespace.
    xmlDom.RegisterNamespace(PACKAGE_PREFIX, PACKAGE_CURRENT_NAMESPACE);

    // Version.
    package.SetMajorVersion(2);
    package.SetMinorVersion(0);

    // Get root node and context.
    BeXmlNodeP pRootNode = xmlDom.GetRootElement();
    if (NULL == pRootNode)
        return RealityPackageStatus::XmlReadError;

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(pRootNode);

    // Origin.
    Utf8String origin;
    xmlDom.SelectNodeContent(origin, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Origin, pContext, BeXmlDom::NODE_BIAS_First);
    package.SetOrigin(origin.c_str());

    // Origin.
    Utf8String requestingApplication;
    xmlDom.SelectNodeContent(requestingApplication, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_RequestingApplication, pContext, BeXmlDom::NODE_BIAS_First);
    package.SetRequestingApplication(requestingApplication.c_str());

    // Name.
    Utf8String name;
    xmlDom.SelectNodeContent(name, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Name, pContext, BeXmlDom::NODE_BIAS_First);
    package.SetName(name.c_str());

    // Description.
    Utf8String description;
    xmlDom.SelectNodeContent(description, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Description, pContext, BeXmlDom::NODE_BIAS_First);
    package.SetDescription(description.c_str());

    // Creation date.
    Utf8String creationDateUTC; DateTime creationDate;
    xmlDom.SelectNodeContent(creationDateUTC, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_CreationDate, pContext, BeXmlDom::NODE_BIAS_First);
    if (!creationDateUTC.empty() && BentleyStatus::SUCCESS != DateTime::FromString(creationDate, creationDateUTC.c_str()))
    return RealityPackageStatus::InvalidDateFormat; // If present, format must be valid.
    package.SetCreationDate(creationDate);

    // Copyright.
    Utf8String copyright;
    xmlDom.SelectNodeContent(copyright, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Copyright, pContext, BeXmlDom::NODE_BIAS_First);
    package.SetCopyright(copyright.c_str());

    // Id.
    Utf8String id;
    xmlDom.SelectNodeContent(id, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Id, pContext, BeXmlDom::NODE_BIAS_First);
    package.SetId(id.c_str());

    // Bounding polygon.
    Utf8String polygonString; BoundingPolygonPtr pPolygon;
    xmlDom.SelectNodeContent(polygonString, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_BoundingPolygon, pContext, BeXmlDom::NODE_BIAS_First);
    if (!polygonString.empty() && (pPolygon = BoundingPolygon::FromString(polygonString.c_str())).IsNull())
    return RealityPackageStatus::PolygonParsingError; // If present, format must be valid.

    package.SetBoundingPolygon(*pPolygon);
    //BeAssert(package.GetBoundingPolygon().IsValid()); // An instance is required.

    // Unknown elements.
    ReadUnknownElements(package, pRootNode->GetFirstChild());

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV2::ReadImageryGroup(RealityDataPackageR package, BeXmlDomR xmlDom)
    {
    RealityPackageStatus status = RealityPackageStatus::Success;

    // Get root node and context.
    BeXmlNodeP pRootNode = xmlDom.GetRootElement();
    if (NULL == pRootNode)
        return RealityPackageStatus::XmlReadError;

    // Get imagery group node.
    BeXmlNodeP pGroupNode = pRootNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_ImageryGroup);
    if (NULL == pGroupNode)
        return RealityPackageStatus::Success; // Source groups are optional.

    RealityDataPackage::ImageryGroup& imgGroup = package.GetImageryGroupR();
    for (BeXmlNodeP pDataNode = pGroupNode->GetFirstChild(); NULL != pDataNode; pDataNode = pDataNode->GetNextSibling())
        {
        // Read base first.        
        // Id.
        Utf8String id;
        BeXmlNodeP pIdNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Id);
        if (NULL != pIdNode)
            pIdNode->GetContent(id);

        // Name.
        Utf8String name;
        BeXmlNodeP pNameNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Name);
        if (NULL != pNameNode)
            pNameNode->GetContent(name);

        // Dataset.
        Utf8String dataset;
        BeXmlNodeP pDatasetNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Dataset);
        if (NULL != pDatasetNode)
            pDatasetNode->GetContent(dataset);

        // Sources.
        BeXmlNodeP pSourcesNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Sources);
        if (NULL == pSourcesNode)
            continue; // Missing source node.           

        // Split sources by type (source, multibandsource, etc.).
        BeXmlDom::IterableNodeSet sourceNodes;
        BeXmlDom::IterableNodeSet multibandSourceNodes;
        pSourcesNode->SelectChildNodes(sourceNodes, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Source);        
        pSourcesNode->SelectChildNodes(multibandSourceNodes, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_MultiBandSource);
        if (0 == sourceNodes.size() && 0 == multibandSourceNodes.size())
            return RealityPackageStatus::MissingSourceAttribute;

        // Source.
        bvector<RealityDataSourcePtr> pSources;
        for (BeXmlNodeP const& pSourceNode : sourceNodes)
            {
            RealityDataSourcePtr pDataSource = ReadSource(status, pSourceNode);
            if (status != RealityPackageStatus::Success)
                return status;

            pSources.push_back(pDataSource);
            }

        // MultiBand source.
        bvector<MultiBandSourcePtr> pMultiBandSources;
        for (BeXmlNodeP const& pMultiBandSourceNode : multibandSourceNodes)
        {
            MultiBandSourcePtr pMultiBandSource = ReadMultiBandSource(status, pMultiBandSourceNode);
            if (pMultiBandSource == NULL)
                return status;

            pMultiBandSources.push_back(pMultiBandSource);
        }

        // Read ImageryData specific.
        DPoint2dP corners = NULL;
        BeXmlNodeP pCornerNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Corners);
        if (NULL != pCornerNode)
            {
            corners = new DPoint2d[4];
            RealityPackageStatus status = RealityPackageStatus::Success;
            if (RealityPackageStatus::Success != (status = RealityDataSerializer::ReadLongLat(corners[ImageryData::Corners::LowerLeft].x, corners[ImageryData::Corners::LowerLeft].y, *pCornerNode, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_LowerLeft)) ||
                RealityPackageStatus::Success != (status = RealityDataSerializer::ReadLongLat(corners[ImageryData::Corners::LowerRight].x, corners[ImageryData::Corners::LowerRight].y, *pCornerNode, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_LowerRight)) ||
                RealityPackageStatus::Success != (status = RealityDataSerializer::ReadLongLat(corners[ImageryData::Corners::UpperLeft].x, corners[ImageryData::Corners::UpperLeft].y, *pCornerNode, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_UpperLeft)) ||
                RealityPackageStatus::Success != (status = RealityDataSerializer::ReadLongLat(corners[ImageryData::Corners::UpperRight].x, corners[ImageryData::Corners::UpperRight].y, *pCornerNode, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_UpperRight)))
                {
                return status;
                }
            }

        // Create imagery data and add alternate sources.
        ImageryDataPtr pImgData;
        if (!pSources.empty())
            {
            pImgData = ImageryData::Create(*pSources[0], corners);
            pImgData->SetDataId(id.c_str());
            pImgData->SetDataName(name.c_str());
            pImgData->SetDataset(dataset.c_str());
            for (size_t i = 1; i < pSources.size(); ++i)
                {
                pImgData->AddSource(*pSources[i]);
                }
            }
        if(!pMultiBandSources.empty())
            {
            size_t start = 0;
            if (pImgData.IsNull())
                {
                pImgData = ImageryData::Create(*pMultiBandSources[0], corners);
                pImgData->SetDataId(id.c_str());
                pImgData->SetDataName(name.c_str());
                pImgData->SetDataset(dataset.c_str());
                start = 1;
                }
            for (size_t i = start; i < pMultiBandSources.size(); ++i)
                {
                pImgData->AddSource(*pMultiBandSources[i]);
                }
            }
        
        // Add data to group.
        imgGroup.push_back(pImgData);
        }

    // Unknown elements.
    ReadUnknownElements(package, pRootNode->GetFirstChild());

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV2::ReadModelGroup(RealityDataPackageR package, BeXmlDomR xmlDom)
    {
    RealityPackageStatus status = RealityPackageStatus::Success;

    // Get root node and context.
    BeXmlNodeP pRootNode = xmlDom.GetRootElement();
    if (NULL == pRootNode)
        return RealityPackageStatus::XmlReadError;

    // Get model group node.
    BeXmlNodeP pGroupNode = pRootNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_ModelGroup);
    if (NULL == pGroupNode)
        return RealityPackageStatus::Success; // Source groups are optional.

    RealityDataPackage::ModelGroup& modelGroup = package.GetModelGroupR();
    for (BeXmlNodeP pDataNode = pGroupNode->GetFirstChild(); NULL != pDataNode; pDataNode = pDataNode->GetNextSibling())
        {
        // Read base first.
        // Id.
        Utf8String id;
        BeXmlNodeP pIdNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Id);
        if (NULL != pIdNode)
            pIdNode->GetContent(id);

        // Name.
        Utf8String name;
        BeXmlNodeP pNameNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Name);
        if (NULL != pNameNode)
            pNameNode->GetContent(name);

        // Dataset.
        Utf8String dataset;
        BeXmlNodeP pDatasetNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Dataset);
        if (NULL != pDatasetNode)
            pDatasetNode->GetContent(dataset);


        // Sources.
        BeXmlNodeP pSourcesNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Sources);
        if (NULL == pSourcesNode)
            continue; // Missing source node.       

        BeXmlDom::IterableNodeSet sourceNodes;
        pSourcesNode->SelectChildNodes(sourceNodes, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Source);

        bvector<RealityDataSourcePtr> pSources;
        for (BeXmlNodeP const& pSourceNode : sourceNodes)
            {
            RealityDataSourcePtr pDataSource = ReadSource(status, pSourceNode);
            if (status != RealityPackageStatus::Success)
                return status;

            pSources.push_back(pDataSource);
            }

        // Create model data and add alternate sources.
        ModelDataPtr pModelData = ModelData::Create(*pSources[0]);
        for (size_t i = 1; i < pSources.size(); ++i)
            {
            pModelData->AddSource(*pSources[i]);
            }

        // Add data to group.
        modelGroup.push_back(pModelData);
        }

    // Unknown elements.
    ReadUnknownElements(package, pRootNode->GetFirstChild());

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV2::ReadPinnedGroup(RealityDataPackageR package, BeXmlDomR xmlDom)
    {
    RealityPackageStatus status = RealityPackageStatus::Success;

    // Get root node and context.
    BeXmlNodeP pRootNode = xmlDom.GetRootElement();
    if (NULL == pRootNode)
        return RealityPackageStatus::XmlReadError;

    // Get pinned group node.
    BeXmlNodeP pGroupNode = pRootNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_PinnedGroup);
    if (NULL == pGroupNode)
        return RealityPackageStatus::Success; // Source groups are optional.

    RealityDataPackage::PinnedGroup& pinnedGroup = package.GetPinnedGroupR();
    for (BeXmlNodeP pDataNode = pGroupNode->GetFirstChild(); NULL != pDataNode; pDataNode = pDataNode->GetNextSibling())
        {
        // Read base first.
        // Id.
        Utf8String id;
        BeXmlNodeP pIdNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Id);
        if (NULL != pIdNode)
            pIdNode->GetContent(id);

        // Name.
        Utf8String name;
        BeXmlNodeP pNameNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Name);
        if (NULL != pNameNode)
            pNameNode->GetContent(name);

        // Dataset.
        Utf8String dataset;
        BeXmlNodeP pDatasetNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Dataset);
        if (NULL != pDatasetNode)
            pDatasetNode->GetContent(dataset);

        // Sources.
        BeXmlNodeP pSourcesNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Sources);
        if (NULL == pSourcesNode)
            continue; // Missing source node.       

        BeXmlDom::IterableNodeSet sourceNodes;
        pSourcesNode->SelectChildNodes(sourceNodes, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Source);

        bvector<RealityDataSourcePtr> pSources;
        for (BeXmlNodeP const& pSourceNode : sourceNodes)
            {
            RealityDataSourcePtr pDataSource = ReadSource(status, pSourceNode);
            if (status != RealityPackageStatus::Success)
                return status;

            pSources.push_back(pDataSource);
            }

        // Read PinnedData specific.
        // Position.
        DPoint2d location; location.x = 0.0; location.y = 0.0;
        if (RealityPackageStatus::Success != (status = RealityDataSerializer::ReadLongLat(location.x, location.y, *pDataNode, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Position)))
            return status;  // Location is mandatory for pinned data.

        if (!RealityDataSerializer::IsValidLongLat(location.x, location.y))
            return RealityPackageStatus::InvalidLongitudeLatitude;

        // Area.
        Utf8String areaStr;
        BoundingPolygonPtr pPolygon = BoundingPolygon::Create();
        BeXmlNodeP pAreaNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Area);
        if (NULL != pAreaNode)
            {
            pAreaNode->GetContent(areaStr);

            if (!areaStr.empty() && (pPolygon = BoundingPolygon::FromString(areaStr.c_str())).IsNull())
                return RealityPackageStatus::PolygonParsingError; // If present, format must be valid.
            }

        // Create pinned data and add alternate sources.
        PinnedDataPtr pPinnedData = PinnedData::Create(*pSources[0], location.x, location.y);
        for (size_t i = 1; i < pSources.size(); ++i)
            {
            pPinnedData->AddSource(*pSources[i]);
            }
        pPinnedData->SetArea(*pPolygon);

        // Add data to group.
        pinnedGroup.push_back(pPinnedData);
        }

    // Unknown elements.
    ReadUnknownElements(package, pRootNode->GetFirstChild());

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV2::ReadTerrainGroup(RealityDataPackageR package, BeXmlDomR xmlDom)
    {
    RealityPackageStatus status = RealityPackageStatus::Success;

    // Get root node and context.
    BeXmlNodeP pRootNode = xmlDom.GetRootElement();
    if (NULL == pRootNode)
        return RealityPackageStatus::XmlReadError;

    // Get terrain group node.
    BeXmlNodeP pGroupNode = pRootNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_TerrainGroup);
    if (NULL == pGroupNode)
        return RealityPackageStatus::Success; // Source groups are optional.

    RealityDataPackage::TerrainGroup& terrainGroup = package.GetTerrainGroupR();
    for (BeXmlNodeP pDataNode = pGroupNode->GetFirstChild(); NULL != pDataNode; pDataNode = pDataNode->GetNextSibling())
        {
        // Read base first.
        // Id.
        Utf8String id;
        BeXmlNodeP pIdNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Id);
        if (NULL != pIdNode)
            pIdNode->GetContent(id);

        // Name.
        Utf8String name;
        BeXmlNodeP pNameNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Name);
        if (NULL != pNameNode)
            pNameNode->GetContent(name);

        // Dataset.
        Utf8String dataset;
        BeXmlNodeP pDatasetNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Dataset);
        if (NULL != pDatasetNode)
            pDatasetNode->GetContent(dataset);

        // Sources.
        BeXmlNodeP pSourcesNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Sources);
        if (NULL == pSourcesNode)
            continue; // Missing source node.       

        BeXmlDom::IterableNodeSet sourceNodes;
        pSourcesNode->SelectChildNodes(sourceNodes, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Source);

        bvector<RealityDataSourcePtr> pSources;
        for (BeXmlNodeP const& pSourceNode : sourceNodes)
            {
            RealityDataSourcePtr pDataSource = ReadSource(status, pSourceNode);
            if (status != RealityPackageStatus::Success)
                return status;

            pSources.push_back(pDataSource);
            }
        
        // Create terrain data and add alternate sources.
        TerrainDataPtr pTerrainData = TerrainData::Create(*pSources[0]);
        for (size_t i = 1; i < pSources.size(); ++i)
            {
            pTerrainData->AddSource(*pSources[i]);
            }

        // Add data to group.
        terrainGroup.push_back(pTerrainData);
        }

    // Unknown elements.
    ReadUnknownElements(package, pRootNode->GetFirstChild());

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    8/2016
// &&JFC TODO: Not efficient, find a better way to do this.
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV2::ReadUnknownElements(RealityDataPackageR package, BeXmlNodeP pNode)
    {
    // At least one unknow element was found, return.
    if (package.HasUnknownElements())
        return RealityPackageStatus::Success;

    // Check if it is an unknown element.       
    if (0 == pNode->NameStricmp(PACKAGE_ELEMENT_Name) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_Description) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_CreationDate) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_Copyright) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_PackageId) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_BoundingPolygon) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_PackageOrigin) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_ImageryGroup) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_RequestingApplication) ||  0 == pNode->NameStricmp(PACKAGE_ELEMENT_Dataset) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_ImageryData) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_Corners) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_LowerLeft) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_LowerRight) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_UpperLeft) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_UpperRight) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_ModelGroup) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_ModelData) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_PinnedGroup) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_PinnedData) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_Position) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_Area) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_TerrainGroup) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_TerrainData) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_Sources) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_Source) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_Copyright) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_Id) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_Provider) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_Size) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_Filesize) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_Metadata) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_GeoCS) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_NoDataValue) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_SisterFiles) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_File) || 
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_WmsSource) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_OsmSource))
        {
        // Look for childs.
        if (NULL != pNode->GetFirstChild())
            ReadUnknownElements(package, pNode->GetFirstChild());

        // Look for siblings.
        if (NULL != pNode->GetNextSibling())
            ReadUnknownElements(package, pNode->GetNextSibling());
        }
    else
        {
        package.SetUnknownElements(true);
        return RealityPackageStatus::Success;
        }

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityDataSourcePtr RealityDataSerializerV2::ReadSource(RealityPackageStatus& status, BeXmlNodeP pSourceNode)
    {
    // Create data source from uri and type.
    Utf8String uri, type;
    if (BEXML_Success != pSourceNode->GetAttributeStringValue(uri, PACKAGE_SOURCE_ATTRIBUTE_Uri))
        {
        status = RealityPackageStatus::MissingSourceAttribute;
        return NULL;
        }
    UriPtr pUri = Uri::Create(uri.c_str());
    pSourceNode->GetAttributeStringValue(type, PACKAGE_SOURCE_ATTRIBUTE_Type);

    RealityDataSourcePtr pDataSource = RealityDataSource::Create(*pUri, type.c_str());
    if (pDataSource == NULL)
        {
        status = RealityPackageStatus::UnknownError;
        return NULL;
        }        

    // Id.
    Utf8String id;
    pSourceNode->GetContent(id, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Id);
    pDataSource->SetId(id.c_str());

    // Copyright.
    Utf8String copyright;
    pSourceNode->GetContent(copyright, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Copyright);
    pDataSource->SetCopyright(copyright.c_str());    

    // TermOfUse.
    Utf8String termOfUse;
    pSourceNode->GetContent(termOfUse, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_TermOfUse);
    pDataSource->SetTermOfUse(termOfUse.c_str());   

    // Provider.
    Utf8String provider;
    pSourceNode->GetContent(provider, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Provider);
    pDataSource->SetProvider(provider.c_str());

    // Size.
    uint64_t size;
    pSourceNode->GetContentUInt64Value(size, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Size);
    pDataSource->SetSize(size);

    // Metadata.
    Utf8String metadata;
    pSourceNode->GetContent(metadata, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Metadata);
    pDataSource->SetMetadata(metadata.c_str());

    // Metadata type.
    Utf8String metadataType;
    BeXmlNodeP pMetadataNode = pSourceNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Metadata);
    if (NULL != pMetadataNode)
        {
        pMetadataNode->GetAttributeStringValue(metadataType, PACKAGE_SOURCE_ATTRIBUTE_Type);
        pDataSource->SetMetadataType(metadataType.c_str());
        }

    // GeoCS.
    Utf8String geocs;
    pSourceNode->GetContent(geocs, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_GeoCS);
    pDataSource->SetGeoCS(geocs.c_str());

    // NoDataValue.
    Utf8String nodatavalue;
    pSourceNode->GetContent(nodatavalue, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_NoDataValue);
    pDataSource->SetNoDataValue(nodatavalue.c_str());

    // &&JFC Sister files.
    bvector<UriPtr> sisterFiles;

 

    BeXmlNodeP pSisterFilesNode = pSourceNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_SisterFiles);
    if (NULL != pSisterFilesNode)
        {

        BeXmlDom::IterableNodeSet fileNodes;
        pSisterFilesNode->SelectChildNodes(fileNodes, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_File);
        for (BeXmlNodeP const& pFileNode : fileNodes)
            {
            Utf8String file;
            pFileNode->GetContent(file);
            sisterFiles.push_back(Uri::Create(file.c_str()));
            }
        if(!sisterFiles.empty())
            pDataSource->SetSisterFiles(sisterFiles);
        }   

    return pDataSource;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
MultiBandSourcePtr RealityDataSerializerV2::ReadMultiBandSource(RealityPackageStatus& status, BeXmlNodeP pSourceNode)
    {
    // *** Read base data source ***
    // Create data source from uri and type.
    Utf8String uri, type;
    if (BEXML_Success != pSourceNode->GetAttributeStringValue(uri, PACKAGE_SOURCE_ATTRIBUTE_Uri))
    {
        status = RealityPackageStatus::MissingSourceAttribute;
        return NULL;
    }
    UriPtr pUri = Uri::Create(uri.c_str());
    pSourceNode->GetAttributeStringValue(type, PACKAGE_SOURCE_ATTRIBUTE_Type);

    MultiBandSourcePtr pDataSource = MultiBandSource::Create(*pUri, type.c_str());
    if (pDataSource == NULL)
        {
        status = RealityPackageStatus::UnknownError;
        return NULL;
        }

    // Id.
    Utf8String id;
    pSourceNode->GetContent(id, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Id);
    pDataSource->SetId(id.c_str());

    // Copyright.
    Utf8String copyright;
    pSourceNode->GetContent(copyright, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Copyright);
    pDataSource->SetCopyright(copyright.c_str());

    // TermOfUse.
    Utf8String termOfUse;
    pSourceNode->GetContent(termOfUse, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_TermOfUse);
    pDataSource->SetTermOfUse(termOfUse.c_str()); 

    // Provider.
    Utf8String provider;
    pSourceNode->GetContent(provider, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Provider);
    pDataSource->SetProvider(provider.c_str());

    // Size.
    uint64_t size;
    pSourceNode->GetContentUInt64Value(size, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Size);
    pDataSource->SetSize(size);

    // Metadata.
    Utf8String metadata;
    pSourceNode->GetContent(metadata, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Metadata);
    pDataSource->SetMetadata(metadata.c_str());

    // Metadata type.
    Utf8String metadataType;
    BeXmlNodeP pMetadataNode = pSourceNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Metadata);
    if (NULL != pMetadataNode)
        {
        pMetadataNode->GetAttributeStringValue(metadataType, PACKAGE_SOURCE_ATTRIBUTE_Type);
        pDataSource->SetMetadataType(metadataType.c_str());
        }

    // GeoCS.
    Utf8String geocs;
    pSourceNode->GetContent(geocs, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_GeoCS);
    pDataSource->SetGeoCS(geocs.c_str());

    // NoDataValue.
    Utf8String nodatavalue;
    pSourceNode->GetContent(nodatavalue, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_NoDataValue);
    pDataSource->SetNoDataValue(nodatavalue.c_str());

    bvector<UriPtr> sisterFiles;
    BeXmlNodeP pSisterFilesNode = pSourceNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_SisterFiles);
    if (NULL != pSisterFilesNode)
        {
        BeXmlDom::IterableNodeSet fileNodes;
        pSisterFilesNode->SelectChildNodes(fileNodes, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_File);
        for (BeXmlNodeP const& pFileNode : fileNodes)
            {
            Utf8String file;
            pFileNode->GetContent(file);
            sisterFiles.push_back(Uri::Create(file.c_str()));
            }
        if (!sisterFiles.empty())
            pDataSource->SetSisterFiles(sisterFiles);
        }

    // *** Read MultiBand specific data ***
    // Red band.
    BeXmlNodeP pRedBandNode = pSourceNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_RedBand);
    if (pRedBandNode != NULL)
        {
        RealityDataSourcePtr pRedBandSource = ReadSource(status, pRedBandNode->GetFirstChild());
        if (pRedBandSource != NULL)
            pDataSource->SetRedBand(*pRedBandSource);
        }    

    // Green band.
    BeXmlNodeP pGreenBandNode = pSourceNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_GreenBand);
    if (pGreenBandNode != NULL)
        {
        RealityDataSourcePtr pGreenBandSource = ReadSource(status, pGreenBandNode->GetFirstChild());
        if (pGreenBandSource != NULL)
            pDataSource->SetGreenBand(*pGreenBandSource);
        }

    // Blue band.
    BeXmlNodeP pBlueBandNode = pSourceNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_BlueBand);
    if (pBlueBandNode != NULL)
        {
        RealityDataSourcePtr pBlueBandSource = ReadSource(status, pBlueBandNode->GetFirstChild());
        if (pBlueBandSource != NULL)
            pDataSource->SetBlueBand(*pBlueBandSource);
        }

    // Panchromatic band.
    BeXmlNodeP pPanchromaticNode = pSourceNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_PanchromaticBand);
    if (pPanchromaticNode != NULL)
        {
        RealityDataSourcePtr pPanchromaticBandSource = ReadSource(status, pPanchromaticNode->GetFirstChild());
        if (pPanchromaticBandSource != NULL)
            pDataSource->SetPanchromaticBand(*pPanchromaticBandSource);
        }
    
    return pDataSource;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV2::_Write(BeXmlDomR xmlDom, RealityDataPackageCR package) const
    {
    RealityPackageStatus status = RealityPackageStatus::UnknownError;

    // Create root node.
    BeXmlNodeP pRootNode = xmlDom.AddNewElement(PACKAGE_ELEMENT_Root, NULL, NULL);
    if (NULL == pRootNode)
        return RealityPackageStatus::UnknownError;

    // Package basic info.
    status = WritePackageInfo(*pRootNode, package);
    if (RealityPackageStatus::Success != status)
        return status;

    // Imagery group.
    status = WriteImageryGroup(*pRootNode, package);
    if (RealityPackageStatus::Success != status)
        return status;

    // Model group.
    status = WriteModelGroup(*pRootNode, package);
    if (RealityPackageStatus::Success != status)
        return status;

    // Pinned group.
    status = WritePinnedGroup(*pRootNode, package);
    if (RealityPackageStatus::Success != status)
        return status;

    // Terrain group.
    status = WriteTerrainGroup(*pRootNode, package);
    if (RealityPackageStatus::Success != status)
        return status;

    return status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV2::WritePackageInfo(BeXmlNodeR node, RealityDataPackageCR package) const
    {
    // Match the version we are persisting.
    node.AddAttributeStringValue(PACKAGE_ATTRIBUTE_Version, "2.0");

    // Namespaces
    node.AddNamespace(NULL, PACKAGE_CURRENT_NAMESPACE);       // Set as default namespace.
    node.AddNamespace(W3SCHEMA_PREFIX, W3SCHEMA_URI);

    // Optional fields, if empty don't add them to the package.
    if (!package.GetName().empty())
        node.AddElementStringValue(PACKAGE_ELEMENT_Name, package.GetName().c_str());

    if (!package.GetCreationDate().ToString().empty())
        node.AddElementStringValue(PACKAGE_ELEMENT_CreationDate, package.GetCreationDate().ToString().c_str());

    if (!package.GetOrigin().empty())
        node.AddElementStringValue(PACKAGE_ELEMENT_Origin, package.GetOrigin().c_str());

    if (!package.GetRequestingApplication().empty())
        node.AddElementStringValue(PACKAGE_ELEMENT_RequestingApplication, package.GetRequestingApplication().c_str());

    if (!package.GetDescription().empty())
        node.AddElementStringValue(PACKAGE_ELEMENT_Description, package.GetDescription().c_str());

    if (!package.GetCopyright().empty())
        node.AddElementStringValue(PACKAGE_ELEMENT_Copyright, package.GetCopyright().c_str());

    if (!package.GetId().empty())
        node.AddElementStringValue(PACKAGE_ELEMENT_Id, package.GetId().c_str());

    if (!package.GetBoundingPolygon().ToString().empty())
        node.AddElementStringValue(PACKAGE_ELEMENT_BoundingPolygon, package.GetBoundingPolygon().ToString().c_str());

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV2::WriteImageryGroup(BeXmlNodeR node, RealityDataPackageCR package) const
    {
    RealityPackageStatus status = RealityPackageStatus::UnknownError;

    if (package.GetImageryGroup().empty())
        return RealityPackageStatus::Success; // No imagery data.

    // Group node.
    BeXmlNodeP pGroupNode = node.AddEmptyElement(PACKAGE_ELEMENT_ImageryGroup);
    if (NULL == pGroupNode)
        return RealityPackageStatus::UnknownError;

    // Add data to group.
    RealityDataPackage::ImageryGroup imgGroup = package.GetImageryGroup();
#if defined(ANDROID)
    for(ImageryDataPtr pImgData : imgGroup)
#else
    for each (ImageryDataPtr pImgData in imgGroup)
#endif
        {
        if (!pImgData.IsValid())
            continue;

        // Write base first.
        BeXmlNodeP pDataNode = pGroupNode->AddEmptyElement(PACKAGE_ELEMENT_ImageryData);

        // Id.
        if (!pImgData->GetDataId().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Id, pImgData->GetDataId().c_str());

        // Name.
        if (!pImgData->GetDataName().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Name, pImgData->GetDataName().c_str());

        // Dataset.
        if (!pImgData->GetDataset().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Dataset, pImgData->GetDataset().c_str());      

        // Sources.
        if (0 != pImgData->GetNumSources())
            { 
            BeXmlNodeP pSourcesNode = pDataNode->AddEmptyElement(PACKAGE_ELEMENT_Sources);

            for (size_t sourceIndex = 0; sourceIndex < pImgData->GetNumSources(); ++sourceIndex)
                {
                // Split sources by type (source, multibandsource, etc.).
                RealityDataSourceCR source = pImgData->GetSource(sourceIndex);

                MultiBandSourceCP pMultiBandSource = dynamic_cast<MultiBandSourceCP>(&source);
                if (NULL != pMultiBandSource)
                    {
                    if (RealityPackageStatus::Success != WriteMultiBandSource(*pSourcesNode, *pMultiBandSource))
                        {
                        pGroupNode->RemoveChildNode(pDataNode);
                        continue;
                        }
                    }
                else
                    {
                    if (RealityPackageStatus::Success != WriteSource(*pSourcesNode, source))
                        {
                        pGroupNode->RemoveChildNode(pDataNode);
                        continue;
                        }
                    }
                //&&JFC TODO WmsSource
                //&&JFC TODO OsmSource
                }  
            }

        // Write ImageryData specific.
        DPoint2dCP pCorners = pImgData->GetCornersCP();
        if (NULL == pCorners)
            continue;  // Corners are optional.

        BeXmlNodeP pCornerNode = pDataNode->AddEmptyElement(PACKAGE_ELEMENT_Corners);
        if (RealityPackageStatus::Success != (status = RealityDataSerializer::WriteLongLat(*pCornerNode, PACKAGE_ELEMENT_LowerLeft, pCorners[ImageryData::Corners::LowerLeft].x, pCorners[ImageryData::Corners::LowerLeft].y)) ||
            RealityPackageStatus::Success != (status = RealityDataSerializer::WriteLongLat(*pCornerNode, PACKAGE_ELEMENT_LowerRight, pCorners[ImageryData::Corners::LowerRight].x, pCorners[ImageryData::Corners::LowerRight].y)) ||
            RealityPackageStatus::Success != (status = RealityDataSerializer::WriteLongLat(*pCornerNode, PACKAGE_ELEMENT_UpperLeft, pCorners[ImageryData::Corners::UpperLeft].x, pCorners[ImageryData::Corners::UpperLeft].y)) ||
            RealityPackageStatus::Success != (status = RealityDataSerializer::WriteLongLat(*pCornerNode, PACKAGE_ELEMENT_UpperRight, pCorners[ImageryData::Corners::UpperRight].x, pCorners[ImageryData::Corners::UpperRight].y)))
            {
            pDataNode->RemoveChildNode(pCornerNode);
            continue;
            }
        }

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV2::WriteModelGroup(BeXmlNodeR node, RealityDataPackageCR package) const
    {
    if (package.GetModelGroup().empty())
        return RealityPackageStatus::Success; // No model data.

    // Group node.
    BeXmlNodeP pGroupNode = node.AddEmptyElement(PACKAGE_ELEMENT_ModelGroup);
    if (NULL == pGroupNode)
        return RealityPackageStatus::UnknownError;

    // Add data to group.
    RealityDataPackage::ModelGroup modelGroup = package.GetModelGroup();
#if defined(ANDROID)
    for (ModelDataPtr pModelData : modelGroup)
#else
    for each (ModelDataPtr pModelData in modelGroup)
#endif
        {
        if (!pModelData.IsValid())
            continue;

        // Write base first.
        BeXmlNodeP pDataNode = pGroupNode->AddEmptyElement(PACKAGE_ELEMENT_ModelData);

        // Id.
        if (!pModelData->GetDataId().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Id, pModelData->GetDataId().c_str());

        // Name.
        if (!pModelData->GetDataName().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Name, pModelData->GetDataName().c_str());

        // Dataset.
        if (!pModelData->GetDataset().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Dataset, pModelData->GetDataset().c_str());

        // Sources.
        if (0 != pModelData->GetNumSources())
            {
            BeXmlNodeP pSourcesNode = pDataNode->AddEmptyElement(PACKAGE_ELEMENT_Sources);

            for (size_t sourceIndex = 0; sourceIndex < pModelData->GetNumSources(); ++sourceIndex)
                {
                // Split sources by type (source, multibandsource, etc.).
                RealityDataSourceCR source = pModelData->GetSource(sourceIndex);

                MultiBandSourceCP pMultiBandSource = dynamic_cast<MultiBandSourceCP>(&source);
                if (NULL != pMultiBandSource)
                    {
                    if (RealityPackageStatus::Success != WriteMultiBandSource(*pSourcesNode, *pMultiBandSource))
                        {
                        pGroupNode->RemoveChildNode(pDataNode);
                        continue;
                        }
                    }
                else
                    {
                    if (RealityPackageStatus::Success != WriteSource(*pSourcesNode, source))
                        {
                        pGroupNode->RemoveChildNode(pDataNode);
                        continue;
                        }
                    }
                //&&JFC TODO WmsSource
                //&&JFC TODO OsmSource
                }
            }
        }

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV2::WritePinnedGroup(BeXmlNodeR node, RealityDataPackageCR package) const
    {
    if (package.GetPinnedGroup().empty())
        return RealityPackageStatus::Success; // No pinned data.

    // Group node.
    BeXmlNodeP pGroupNode = node.AddEmptyElement(PACKAGE_ELEMENT_PinnedGroup);
    if (NULL == pGroupNode)
        return RealityPackageStatus::UnknownError;

    // Add data to group.
    RealityDataPackage::PinnedGroup pinnedGroup = package.GetPinnedGroup();
#if defined(ANDROID)
    for (PinnedDataPtr pPinnedData : pinnedGroup)
#else
    for each (PinnedDataPtr pPinnedData in pinnedGroup)
#endif
        {
        if (!pPinnedData.IsValid())
            continue;

        // Write base first.
        BeXmlNodeP pDataNode = pGroupNode->AddEmptyElement(PACKAGE_ELEMENT_PinnedData);
        
        // Id.
        if (!pPinnedData->GetDataId().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Id, pPinnedData->GetDataId().c_str());

        // Name.
        if (!pPinnedData->GetDataName().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Name, pPinnedData->GetDataName().c_str());

        // Dataset.
        if (!pPinnedData->GetDataset().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Dataset, pPinnedData->GetDataset().c_str());

        // Sources.
        if (0 != pPinnedData->GetNumSources())
            {
            BeXmlNodeP pSourcesNode = pDataNode->AddEmptyElement(PACKAGE_ELEMENT_Sources);

            for (size_t sourceIndex = 0; sourceIndex < pPinnedData->GetNumSources(); ++sourceIndex)
                {
                // Split sources by type (source, multibandsource, etc.).
                RealityDataSourceCR source = pPinnedData->GetSource(sourceIndex);

                MultiBandSourceCP pMultiBandSource = dynamic_cast<MultiBandSourceCP>(&source);
                if (NULL != pMultiBandSource)
                    {
                    if (RealityPackageStatus::Success != WriteMultiBandSource(*pSourcesNode, *pMultiBandSource))
                        {
                        pGroupNode->RemoveChildNode(pDataNode);
                        continue;
                        }
                    }
                else
                    {
                    if (RealityPackageStatus::Success != WriteSource(*pSourcesNode, source))
                        {
                        pGroupNode->RemoveChildNode(pDataNode);
                        continue;
                        }
                    }
                //&&JFC TODO WmsSource
                //&&JFC TODO OsmSource
                }
            }

        // Write PinnedData specific.
        DPoint2dCR location = pPinnedData->GetLocation();
        BeAssert(RealityDataSerializer::IsValidLongLat(location.x, location.y));
        RealityDataSerializer::WriteLongLat(*pDataNode, PACKAGE_ELEMENT_Position, location.x, location.y);
        }

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV2::WriteTerrainGroup(BeXmlNodeR node, RealityDataPackageCR package) const
    {
    if (package.GetTerrainGroup().empty())
        return RealityPackageStatus::Success; // No terrain data.

    // Group node.
    BeXmlNodeP pGroupNode = node.AddEmptyElement(PACKAGE_ELEMENT_TerrainGroup);
    if (NULL == pGroupNode)
        return RealityPackageStatus::UnknownError;

    // Add data to group.
    RealityDataPackage::TerrainGroup terrainGroup = package.GetTerrainGroup();
#if defined(ANDROID)
    for (TerrainDataPtr pTerrainData : terrainGroup)
#else
    for each (TerrainDataPtr pTerrainData in terrainGroup)
#endif
        {
        if (!pTerrainData.IsValid())
            continue;

        // Write base first.
        BeXmlNodeP pDataNode = pGroupNode->AddEmptyElement(PACKAGE_ELEMENT_TerrainData);
        
        // Id.
        if (!pTerrainData->GetDataId().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Id, pTerrainData->GetDataId().c_str());

        // Name.
        if (!pTerrainData->GetDataName().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Name, pTerrainData->GetDataName().c_str());

        // Dataset.
        if (!pTerrainData->GetDataset().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Dataset, pTerrainData->GetDataset().c_str());

        // Sources.
        if (0 != pTerrainData->GetNumSources())
            {
            BeXmlNodeP pSourcesNode = pDataNode->AddEmptyElement(PACKAGE_ELEMENT_Sources);

            for (size_t sourceIndex = 0; sourceIndex < pTerrainData->GetNumSources(); ++sourceIndex)
                {
                // Split sources by type (source, multibandsource, etc.).
                RealityDataSourceCR source = pTerrainData->GetSource(sourceIndex);

                MultiBandSourceCP pMultiBandSource = dynamic_cast<MultiBandSourceCP>(&source);
                if (NULL != pMultiBandSource)
                    {
                    if (RealityPackageStatus::Success != WriteMultiBandSource(*pSourcesNode, *pMultiBandSource))
                        {
                        pGroupNode->RemoveChildNode(pDataNode);
                        continue;
                        }
                    }
                else
                    {
                    if (RealityPackageStatus::Success != WriteSource(*pSourcesNode, source))
                        {
                        pGroupNode->RemoveChildNode(pDataNode);
                        continue;
                        }
                    }
                //&&JFC TODO WmsSource
                //&&JFC TODO OsmSource
                }
            }
        }

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV2::WriteSource(BeXmlNodeR node, RealityDataSourceCR source) const
    {
    // Required fields.
    UriCR uri = source.GetUri();
    Utf8String type = source.GetType();
    if (uri.ToString().empty() || type.empty())
        return RealityPackageStatus::MissingSourceAttribute;

    BeXmlNodeP pSourceNode = node.AddEmptyElement(PACKAGE_ELEMENT_Source);
    pSourceNode->AddAttributeStringValue(PACKAGE_SOURCE_ATTRIBUTE_Uri, uri.ToString().c_str());
    pSourceNode->AddAttributeStringValue(PACKAGE_SOURCE_ATTRIBUTE_Type, type.c_str());

    // Optional fields.
    if (!source.GetCopyright().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_Copyright, source.GetCopyright().c_str());

    if (!source.GetTermOfUse().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_TermOfUse, source.GetTermOfUse().c_str());

    if (!source.GetId().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_Id, source.GetId().c_str());

    if (!source.GetProvider().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_Provider, source.GetProvider().c_str());

    if (0 != source.GetSize())
        pSourceNode->AddElementUInt64Value(PACKAGE_ELEMENT_Size, source.GetSize());

    if (!source.GetMetadata().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_Metadata, source.GetMetadata().c_str());

    //&&JFC TODO MetadataType

    if (!source.GetGeoCS().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_GeoCS, source.GetGeoCS().c_str());

    if (!source.GetNoDataValue().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_NoDataValue, source.GetNoDataValue().c_str());

    //&&JFC TODO SisterFiles

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV2::WriteMultiBandSource(BeXmlNodeR node, MultiBandSourceCR source) const
    {
    // Required fields.
    UriCR uri = source.GetUri();
    Utf8String type = source.GetType();
    if (uri.ToString().empty() || type.empty())
        return RealityPackageStatus::MissingSourceAttribute;

    BeXmlNodeP pSourceNode = node.AddEmptyElement(PACKAGE_ELEMENT_MultiBandSource);
    pSourceNode->AddAttributeStringValue(PACKAGE_SOURCE_ATTRIBUTE_Uri, uri.ToString().c_str());
    pSourceNode->AddAttributeStringValue(PACKAGE_SOURCE_ATTRIBUTE_Type, type.c_str());

    // Write basic source.
    if (!source.GetCopyright().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_Copyright, source.GetCopyright().c_str());

    if (!source.GetTermOfUse().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_TermOfUse, source.GetTermOfUse().c_str());

    if (!source.GetId().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_Id, source.GetId().c_str());

    if (!source.GetProvider().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_Provider, source.GetProvider().c_str());

    if (0 != source.GetSize())
        pSourceNode->AddElementUInt64Value(PACKAGE_ELEMENT_Size, source.GetSize());

    if (!source.GetMetadata().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_Metadata, source.GetMetadata().c_str());

    //&&JFC TODO MetadataType

    if (!source.GetGeoCS().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_GeoCS, source.GetGeoCS().c_str());

    if (!source.GetNoDataValue().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_NoDataValue, source.GetNoDataValue().c_str());

    //&&JFC TODO SisterFiles

    // Write specific multiband source data.
    if (source.GetRedBand() != NULL)
        {
        BeXmlNodeP pRedBandNode = pSourceNode->AddEmptyElement(PACKAGE_ELEMENT_RedBand);
        WriteSource(*pRedBandNode, *source.GetRedBand());
        }

    if (source.GetGreenBand() != NULL)
        {
        BeXmlNodeP pGreenBandNode = pSourceNode->AddEmptyElement(PACKAGE_ELEMENT_GreenBand);
        WriteSource(*pGreenBandNode, *source.GetGreenBand());
        }

    if (source.GetBlueBand() != NULL)
        {
        BeXmlNodeP pBlueBandNode = pSourceNode->AddEmptyElement(PACKAGE_ELEMENT_BlueBand);
        WriteSource(*pBlueBandNode, *source.GetBlueBand());
        }

    if (source.GetPanchromaticBand() != NULL)
        {
        BeXmlNodeP pPanchromaticBandNode = pSourceNode->AddEmptyElement(PACKAGE_ELEMENT_PanchromaticBand);
        WriteSource(*pPanchromaticBandNode, *source.GetPanchromaticBand());
        }

    return RealityPackageStatus::Success;
    }


END_BENTLEY_REALITYPACKAGE_NAMESPACE