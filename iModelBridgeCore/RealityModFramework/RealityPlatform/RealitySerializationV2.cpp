/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <RealityPlatform/RealityPlatformAPI.h>
#include <RealityPlatform/RealityDataPackage.h>
#include <RealityPlatform/SpatialEntity.h>
#include <BeXml/BeXml.h>
#include "RealitySerialization.h"


BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE


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

    // Set namespace and version.
    xmlDom.RegisterNamespace(PACKAGE_PREFIX, PACKAGE_CURRENT_NAMESPACE);
    package.SetMajorVersion(2);
    package.SetMinorVersion(0);

    // Read basic info.
    status = ReadPackageInfo(package, xmlDom);
    if (RealityPackageStatus::Success != status)
        return status;

    // Read imagery data.
    status = ReadImageryGroup(package, xmlDom);
    if (RealityPackageStatus::Success != status)
        return status;

    // Read model data.
    status = ReadModelGroup(package, xmlDom);
    if (RealityPackageStatus::Success != status)
        return status;

    // Read pinned data.
    status = ReadPinnedGroup(package, xmlDom);
    if (RealityPackageStatus::Success != status)
        return status;

    // Read terrain data.
    status = ReadTerrainGroup(package, xmlDom);
    if (RealityPackageStatus::Success != status)
        return status;

    // Read unefined data. (version 2.1 and up)
    status = ReadUndefinedGroup(package, xmlDom);
    if (RealityPackageStatus::Success != status)
        return status;

    return status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV2::_ReadImageryGroup(RealityDataPackageR package, BeXmlDomR xmlDom)
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

    bvector<PackageRealityDataPtr>& imgGroup = package.GetImageryGroupR();
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

        // Resolution.
        Utf8String resolution;
        BeXmlNodeP pResolutionNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Resolution);
        if (NULL != pResolutionNode)
            pResolutionNode->GetContent(resolution);

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
        bvector<SpatialEntityDataSourcePtr> pSources;
        for (BeXmlNodeP const& pSourceNode : sourceNodes)
            {
            SpatialEntityDataSourcePtr pDataSource = ReadSource(status, pSourceNode);
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
        bvector<GeoPoint2d> corners;
        BeXmlNodeP pCornerNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Corners);
        if (NULL != pCornerNode)
            {
            corners = bvector<GeoPoint2d>(4);
            RealityPackageStatus status2 = RealityPackageStatus::Success;
            if (RealityPackageStatus::Success != (status2 = RealityDataSerializer::ReadLongLat(corners[PackageRealityData::Corners::LowerLeft].longitude, corners[PackageRealityData::Corners::LowerLeft].latitude, *pCornerNode, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_LowerLeft)) ||
                RealityPackageStatus::Success != (status2 = RealityDataSerializer::ReadLongLat(corners[PackageRealityData::Corners::LowerRight].longitude, corners[PackageRealityData::Corners::LowerRight].latitude, *pCornerNode, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_LowerRight)) ||
                RealityPackageStatus::Success != (status2 = RealityDataSerializer::ReadLongLat(corners[PackageRealityData::Corners::UpperLeft].longitude, corners[PackageRealityData::Corners::UpperLeft].latitude, *pCornerNode, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_UpperLeft)) ||
                RealityPackageStatus::Success != (status2 = RealityDataSerializer::ReadLongLat(corners[PackageRealityData::Corners::UpperRight].longitude, corners[PackageRealityData::Corners::UpperRight].latitude, *pCornerNode, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_UpperRight)))
                {
                return status2;
                }
            }
        else
            corners = bvector<GeoPoint2d>();

        // Create imagery data and add alternate sources.
        PackageRealityDataPtr pImgData;
        if (!pSources.empty())
            {
            pImgData = PackageRealityData::CreateImagery(*pSources[0], corners);
            pImgData->SetIdentifier(id.c_str());
            pImgData->SetName(name.c_str());
            pImgData->SetDataset(dataset.c_str());
            pImgData->SetResolution(resolution.c_str());

            for (size_t i = 1; i < pSources.size(); ++i)
                {
                pImgData->AddDataSource(*pSources[i]);
                }
            }
        if(!pMultiBandSources.empty())
            {
            size_t start = 0;
            if (pImgData.IsNull())
                {
                pImgData = PackageRealityData::CreateImagery(*pMultiBandSources[0], corners);
                pImgData->SetIdentifier(id.c_str());
                pImgData->SetName(name.c_str());
                pImgData->SetDataset(dataset.c_str());
                pImgData->SetResolution(resolution.c_str());
                start = 1;
                }
            for (size_t i = start; i < pMultiBandSources.size(); ++i)
                {
                pImgData->AddDataSource(*pMultiBandSources[i]);
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
RealityPackageStatus RealityDataSerializerV2::_ReadModelGroup(RealityDataPackageR package, BeXmlDomR xmlDom)
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

    bvector<PackageRealityDataPtr>& modelGroup = package.GetModelGroupR();
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

        // Resolution.
        Utf8String resolution;
        BeXmlNodeP pResolutionNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Resolution);
        if (NULL != pResolutionNode)
            pResolutionNode->GetContent(resolution);

        // Sources.
        BeXmlNodeP pSourcesNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Sources);
        if (NULL == pSourcesNode)
            continue; // Missing source node.       

        BeXmlDom::IterableNodeSet sourceNodes;
        pSourcesNode->SelectChildNodes(sourceNodes, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Source);

        bvector<SpatialEntityDataSourcePtr> pSources;
        for (BeXmlNodeP const& pSourceNode : sourceNodes)
            {
            SpatialEntityDataSourcePtr pDataSource = ReadSource(status, pSourceNode);
            if (status != RealityPackageStatus::Success)
                return status;

            pSources.push_back(pDataSource);
            }

        // Create model data and add alternate sources.
        PackageRealityDataPtr pModelData = PackageRealityData::CreateModel(*pSources[0]);
        pModelData->SetIdentifier(id.c_str());
        pModelData->SetName(name.c_str());
        pModelData->SetDataset(dataset.c_str());
        pModelData->SetResolution(resolution.c_str());
        for (size_t i = 1; i < pSources.size(); ++i)
            {
            pModelData->AddDataSource(*pSources[i]);
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
RealityPackageStatus RealityDataSerializerV2::_ReadPinnedGroup(RealityDataPackageR package, BeXmlDomR xmlDom)
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

    bvector<PackageRealityDataPtr>& pinnedGroup = package.GetPinnedGroupR();
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

        // Resolution.
        Utf8String resolution;
        BeXmlNodeP pResolutionNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Resolution);
        if (NULL != pResolutionNode)
            pResolutionNode->GetContent(resolution);

        // Sources.
        BeXmlNodeP pSourcesNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Sources);
        if (NULL == pSourcesNode)
            continue; // Missing source node.       

        BeXmlDom::IterableNodeSet sourceNodes;
        pSourcesNode->SelectChildNodes(sourceNodes, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Source);

        bvector<SpatialEntityDataSourcePtr> pSources;
        for (BeXmlNodeP const& pSourceNode : sourceNodes)
            {
            SpatialEntityDataSourcePtr pDataSource = ReadSource(status, pSourceNode);
            if (status != RealityPackageStatus::Success)
                return status;

            pSources.push_back(pDataSource);
            }

        // Read PinnedData specific.
        // Position.
        GeoPoint2d location; location.longitude = 0.0; location.latitude = 0.0;
        if (RealityPackageStatus::Success != (status = RealityDataSerializer::ReadLongLat(location.longitude, location.latitude, *pDataNode, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Position)))
            return status;  // Location is mandatory for pinned data.

        if (!RealityDataSerializer::IsValidLongLat(location.longitude, location.latitude))
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
        PackageRealityDataPtr pPinnedData = PackageRealityData::CreatePinned(*pSources[0], location.longitude, location.latitude);
        pPinnedData->SetIdentifier(id.c_str());
        pPinnedData->SetName(name.c_str());
        pPinnedData->SetDataset(dataset.c_str());
        pPinnedData->SetResolution(resolution.c_str());
        for (size_t i = 1; i < pSources.size(); ++i)
            {
            pPinnedData->AddDataSource(*pSources[i]);
            }
        pPinnedData->SetFootprint(pPolygon->GetFootprint());

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
RealityPackageStatus RealityDataSerializerV2::_ReadTerrainGroup(RealityDataPackageR package, BeXmlDomR xmlDom)
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

    bvector<PackageRealityDataPtr>& terrainGroup = package.GetTerrainGroupR();
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

        // Resolution.
        Utf8String resolution;
        BeXmlNodeP pResolutionNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Resolution);
        if (NULL != pResolutionNode)
            pResolutionNode->GetContent(resolution);

        // Sources.
        BeXmlNodeP pSourcesNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Sources);
        if (NULL == pSourcesNode)
            continue; // Missing source node.       

        BeXmlDom::IterableNodeSet sourceNodes;
        pSourcesNode->SelectChildNodes(sourceNodes, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Source);

        bvector<SpatialEntityDataSourcePtr> pSources;
        for (BeXmlNodeP const& pSourceNode : sourceNodes)
            {
            SpatialEntityDataSourcePtr pDataSource = ReadSource(status, pSourceNode);
            if (status != RealityPackageStatus::Success)
                return status;

            pSources.push_back(pDataSource);
            }
        
        // Create terrain data and add alternate sources.
        PackageRealityDataPtr pTerrainData = PackageRealityData::CreateTerrain(*pSources[0]);
        pTerrainData->SetIdentifier(id.c_str());
        pTerrainData->SetName(name.c_str());
        pTerrainData->SetDataset(dataset.c_str());
        pTerrainData->SetResolution(resolution.c_str());
        for (size_t i = 1; i < pSources.size(); ++i)
            {
            pTerrainData->AddDataSource(*pSources[i]);
            }

        // Add data to group.
        terrainGroup.push_back(pTerrainData);
        }

    // Unknown elements.
    ReadUnknownElements(package, pRootNode->GetFirstChild());

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                 04/2017
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV2::_ReadUndefinedGroup(RealityDataPackageR package, BeXmlDomR xmlDom)
    {
    RealityPackageStatus status = RealityPackageStatus::Success;

    // Get root node and context.
    BeXmlNodeP pRootNode = xmlDom.GetRootElement();
    if (NULL == pRootNode)
        return RealityPackageStatus::XmlReadError;

    // Get terrain group node.
    BeXmlNodeP pGroupNode = pRootNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_UndefinedGroup);
    if (NULL == pGroupNode)
        return RealityPackageStatus::Success; // Source groups are optional.

    bvector<PackageRealityDataPtr>& undefinedGroup = package.GetUndefinedGroupR();
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

        // Resolution.
        Utf8String resolution;
        BeXmlNodeP pResolutionNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Resolution);
        if (NULL != pResolutionNode)
            pResolutionNode->GetContent(resolution);

        // Sources.
        BeXmlNodeP pSourcesNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Sources);
        if (NULL == pSourcesNode)
            continue; // Missing source node.       

        BeXmlDom::IterableNodeSet sourceNodes;
        pSourcesNode->SelectChildNodes(sourceNodes, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Source);

        bvector<SpatialEntityDataSourcePtr> pSources;
        for (BeXmlNodeP const& pSourceNode : sourceNodes)
            {
            SpatialEntityDataSourcePtr pDataSource = ReadSource(status, pSourceNode);
            if (status != RealityPackageStatus::Success)
                return status;

            pSources.push_back(pDataSource);
            }
        
        // Create undefined data and add alternate sources.
        PackageRealityDataPtr pUndefinedData = PackageRealityData::CreateUndefined(*pSources[0]);
        pUndefinedData->SetIdentifier(id.c_str());
        pUndefinedData->SetName(name.c_str());
        pUndefinedData->SetDataset(dataset.c_str());
        pUndefinedData->SetResolution(resolution.c_str());
        for (size_t i = 1; i < pSources.size(); ++i)
            {
            pUndefinedData->AddDataSource(*pSources[i]);
            }

        // Add data to group.
        undefinedGroup.push_back(pUndefinedData);
        }

    // Unknown elements.
    ReadUnknownElements(package, pRootNode->GetFirstChild());

    return RealityPackageStatus::Success;
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

    // Add version and namespace.
    pRootNode->AddAttributeStringValue(PACKAGE_ATTRIBUTE_Version, "2.2");
    pRootNode->AddNamespace(NULL, PACKAGE_CURRENT_NAMESPACE);
    pRootNode->AddNamespace(W3SCHEMA_PREFIX, W3SCHEMA_URI);

    // Add basic info.
    status = WritePackageInfo(*pRootNode, package);
    if (RealityPackageStatus::Success != status)
        return status;

    // Add imagery data.
    status = WriteImageryGroup(*pRootNode, package);
    if (RealityPackageStatus::Success != status)
        return status;

    // Add model data.
    status = WriteModelGroup(*pRootNode, package);
    if (RealityPackageStatus::Success != status)
        return status;

    // Add pinned data.
    status = WritePinnedGroup(*pRootNode, package);
    if (RealityPackageStatus::Success != status)
        return status;

    // Add terrain data.
    status = WriteTerrainGroup(*pRootNode, package);
    if (RealityPackageStatus::Success != status)
        return status;

    // Add undefined data.
    status = WriteUndefinedGroup(*pRootNode, package);
    if (RealityPackageStatus::Success != status)
        return status;

    return status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV2::_WriteImageryGroup(BeXmlNodeR node, RealityDataPackageCR package) const
    {
    RealityPackageStatus status = RealityPackageStatus::UnknownError;

    if (package.GetImageryGroup().empty())
        return RealityPackageStatus::Success; // No imagery data.

    // Group node.
    BeXmlNodeP pGroupNode = node.AddEmptyElement(PACKAGE_ELEMENT_ImageryGroup);
    if (NULL == pGroupNode)
        return RealityPackageStatus::UnknownError;

    // Add data to group.
    bvector<PackageRealityDataPtr> imgGroup = package.GetImageryGroup();
    for(PackageRealityDataPtr pImgData : imgGroup)
        {
        if (!pImgData.IsValid())
            continue;

        // Write base first.
        BeXmlNodeP pDataNode = pGroupNode->AddEmptyElement(PACKAGE_ELEMENT_ImageryData);

        // Id.
        if (!pImgData->GetIdentifier().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Id, pImgData->GetIdentifier().c_str());

        // Name.
        if (!pImgData->GetName().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Name, pImgData->GetName().c_str());

        // Dataset.
        if (!pImgData->GetDataset().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Dataset, pImgData->GetDataset().c_str());      

        // Resolution.
        if (!pImgData->GetResolution().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Resolution, pImgData->GetResolution().c_str());  

        // Sources.
        if (0 != pImgData->GetDataSourceCount())
            { 
            BeXmlNodeP pSourcesNode = pDataNode->AddEmptyElement(PACKAGE_ELEMENT_Sources);

            for (size_t sourceIndex = 0; sourceIndex < pImgData->GetDataSourceCount(); ++sourceIndex)
                {
                if (RealityPackageStatus::Success != WriteSource(*pSourcesNode, pImgData->GetDataSource(sourceIndex)))
                    {
                    pGroupNode->RemoveChildNode(pDataNode);
                    continue;
                    }
                }
            }

        // Write ImageryData specific.
        const bvector<GeoPoint2d>& pCorners = pImgData->GetFootprint();
        if (pCorners.empty())
            continue;  // Corners are optional.

        BeXmlNodeP pCornerNode = pDataNode->AddEmptyElement(PACKAGE_ELEMENT_Corners);
        if (RealityPackageStatus::Success != (status = RealityDataSerializer::WriteLongLat(*pCornerNode, PACKAGE_ELEMENT_LowerLeft, pCorners[PackageRealityData::Corners::LowerLeft].longitude, pCorners[PackageRealityData::Corners::LowerLeft].latitude)) ||
            RealityPackageStatus::Success != (status = RealityDataSerializer::WriteLongLat(*pCornerNode, PACKAGE_ELEMENT_LowerRight, pCorners[PackageRealityData::Corners::LowerRight].longitude, pCorners[PackageRealityData::Corners::LowerRight].latitude)) ||
            RealityPackageStatus::Success != (status = RealityDataSerializer::WriteLongLat(*pCornerNode, PACKAGE_ELEMENT_UpperLeft, pCorners[PackageRealityData::Corners::UpperLeft].longitude, pCorners[PackageRealityData::Corners::UpperLeft].latitude)) ||
            RealityPackageStatus::Success != (status = RealityDataSerializer::WriteLongLat(*pCornerNode, PACKAGE_ELEMENT_UpperRight, pCorners[PackageRealityData::Corners::UpperRight].longitude, pCorners[PackageRealityData::Corners::UpperRight].latitude)))
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
RealityPackageStatus RealityDataSerializerV2::_WriteModelGroup(BeXmlNodeR node, RealityDataPackageCR package) const
    {
    if (package.GetModelGroup().empty())
        return RealityPackageStatus::Success; // No model data.

    // Group node.
    BeXmlNodeP pGroupNode = node.AddEmptyElement(PACKAGE_ELEMENT_ModelGroup);
    if (NULL == pGroupNode)
        return RealityPackageStatus::UnknownError;

    // Add data to group.
    bvector<PackageRealityDataPtr> modelGroup = package.GetModelGroup();
    for (PackageRealityDataPtr pModelData : modelGroup)
        {
        if (!pModelData.IsValid())
            continue;

        // Write base first.
        BeXmlNodeP pDataNode = pGroupNode->AddEmptyElement(PACKAGE_ELEMENT_ModelData);

        // Id.
        if (!pModelData->GetIdentifier().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Id, pModelData->GetIdentifier().c_str());

        // Name.
        if (!pModelData->GetName().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Name, pModelData->GetName().c_str());

        // Dataset.
        if (!pModelData->GetDataset().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Dataset, pModelData->GetDataset().c_str());

        // Resolution.
        if (!pModelData->GetResolution().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Resolution, pModelData->GetResolution().c_str());

        // Sources.
        if (0 != pModelData->GetDataSourceCount())
            {
            BeXmlNodeP pSourcesNode = pDataNode->AddEmptyElement(PACKAGE_ELEMENT_Sources);

            for (size_t sourceIndex = 0; sourceIndex < pModelData->GetDataSourceCount(); ++sourceIndex)
                {
                if (RealityPackageStatus::Success != WriteSource(*pSourcesNode, pModelData->GetDataSource(sourceIndex)))
                    {
                    pGroupNode->RemoveChildNode(pDataNode);
                    continue;
                    }                    
                }
            }
        }

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV2::_WritePinnedGroup(BeXmlNodeR node, RealityDataPackageCR package) const
    {
    if (package.GetPinnedGroup().empty())
        return RealityPackageStatus::Success; // No pinned data.

    // Group node.
    BeXmlNodeP pGroupNode = node.AddEmptyElement(PACKAGE_ELEMENT_PinnedGroup);
    if (NULL == pGroupNode)
        return RealityPackageStatus::UnknownError;

    // Add data to group.
    bvector<PackageRealityDataPtr> pinnedGroup = package.GetPinnedGroup();
    for (PackageRealityDataPtr pPinnedData : pinnedGroup)
        {
        if (!pPinnedData.IsValid())
            continue;

        // Write base first.
        BeXmlNodeP pDataNode = pGroupNode->AddEmptyElement(PACKAGE_ELEMENT_PinnedData);
        
        // Id.
        if (!pPinnedData->GetIdentifier().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Id, pPinnedData->GetIdentifier().c_str());

        // Name.
        if (!pPinnedData->GetName().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Name, pPinnedData->GetName().c_str());

        // Dataset.
        if (!pPinnedData->GetDataset().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Dataset, pPinnedData->GetDataset().c_str());

        // Resolution.
        if (!pPinnedData->GetResolution().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Resolution, pPinnedData->GetResolution().c_str());

        // Sources.
        if (0 != pPinnedData->GetDataSourceCount())
            {
            BeXmlNodeP pSourcesNode = pDataNode->AddEmptyElement(PACKAGE_ELEMENT_Sources);

            for (size_t sourceIndex = 0; sourceIndex < pPinnedData->GetDataSourceCount(); ++sourceIndex)
                {
                if (RealityPackageStatus::Success != WriteSource(*pSourcesNode, pPinnedData->GetDataSource(sourceIndex)))
                    {
                    pGroupNode->RemoveChildNode(pDataNode);
                    continue;
                    }
                }
            }

        // Write PinnedData specific.
        GeoPoint2dCR location = pPinnedData->GetLocation();
        BeAssert(RealityDataSerializer::IsValidLongLat(location.longitude, location.latitude));
        RealityDataSerializer::WriteLongLat(*pDataNode, PACKAGE_ELEMENT_Position, location.longitude, location.latitude);

        // Area
        if (pPinnedData->HasArea())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Area, pPinnedData->GetFootprintString().c_str());

        }

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV2::_WriteTerrainGroup(BeXmlNodeR node, RealityDataPackageCR package) const
    {
    if (package.GetTerrainGroup().empty())
        return RealityPackageStatus::Success; // No terrain data.

    // Group node.
    BeXmlNodeP pGroupNode = node.AddEmptyElement(PACKAGE_ELEMENT_TerrainGroup);
    if (NULL == pGroupNode)
        return RealityPackageStatus::UnknownError;

    // Add data to group.
    bvector<PackageRealityDataPtr> terrainGroup = package.GetTerrainGroup();
    for (PackageRealityDataPtr pTerrainData : terrainGroup)
        {
        if (!pTerrainData.IsValid())
            continue;

        // Write base first.
        BeXmlNodeP pDataNode = pGroupNode->AddEmptyElement(PACKAGE_ELEMENT_TerrainData);
        
        // Id.
        if (!pTerrainData->GetIdentifier().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Id, pTerrainData->GetIdentifier().c_str());

        // Name.
        if (!pTerrainData->GetName().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Name, pTerrainData->GetName().c_str());

        // Dataset.
        if (!pTerrainData->GetDataset().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Dataset, pTerrainData->GetDataset().c_str());

        // Resolution.
        if (!pTerrainData->GetResolution().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Resolution, pTerrainData->GetResolution().c_str());

        // Sources.
        if (0 != pTerrainData->GetDataSourceCount())
            {
            BeXmlNodeP pSourcesNode = pDataNode->AddEmptyElement(PACKAGE_ELEMENT_Sources);

            for (size_t sourceIndex = 0; sourceIndex < pTerrainData->GetDataSourceCount(); ++sourceIndex)
                {
                if (RealityPackageStatus::Success != WriteSource(*pSourcesNode, pTerrainData->GetDataSource(sourceIndex)))
                    {
                    pGroupNode->RemoveChildNode(pDataNode);
                    continue;
                    }                    
                }
            }
        }

    return RealityPackageStatus::Success;
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV2::_WriteUndefinedGroup(BeXmlNodeR node, RealityDataPackageCR package) const
    {
    if (package.GetUndefinedGroup().empty())
        return RealityPackageStatus::Success; // No terrain data.

    // Group node.
    BeXmlNodeP pGroupNode = node.AddEmptyElement(PACKAGE_ELEMENT_UndefinedGroup);
    if (NULL == pGroupNode)
        return RealityPackageStatus::UnknownError;

    // Add data to group.
    bvector<PackageRealityDataPtr> undefinedGroup = package.GetUndefinedGroup();
    for (PackageRealityDataPtr pUndefinedData : undefinedGroup)
        {
        if (!pUndefinedData.IsValid())
            continue;

        // Write base first.
        BeXmlNodeP pDataNode = pGroupNode->AddEmptyElement(PACKAGE_ELEMENT_UndefinedData);
        
        // Id.
        if (!pUndefinedData->GetIdentifier().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Id, pUndefinedData->GetIdentifier().c_str());

        // Name.
        if (!pUndefinedData->GetName().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Name, pUndefinedData->GetName().c_str());

        // Dataset.
        if (!pUndefinedData->GetDataset().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Dataset, pUndefinedData->GetDataset().c_str());

        // Resolution.
        if (!pUndefinedData->GetResolution().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Resolution, pUndefinedData->GetResolution().c_str());

        // Sources.
        if (0 != pUndefinedData->GetDataSourceCount())
            {
            BeXmlNodeP pSourcesNode = pDataNode->AddEmptyElement(PACKAGE_ELEMENT_Sources);

            for (size_t sourceIndex = 0; sourceIndex < pUndefinedData->GetDataSourceCount(); ++sourceIndex)
                {
                if (RealityPackageStatus::Success != WriteSource(*pSourcesNode, pUndefinedData->GetDataSource(sourceIndex)))
                    {
                    pGroupNode->RemoveChildNode(pDataNode);
                    continue;
                    }                    
                }
            }
        }

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV2::_WriteSource(BeXmlNodeR node, SpatialEntityDataSourceCR source) const
    {
    // Required fields.
    UriCR uri = source.GetUri();
    Utf8String type = source.GetDataType();
    if (uri.ToString().empty() || type.empty())
        return RealityPackageStatus::MissingSourceAttribute;

    BeXmlNodeP pSourceNode = node.AddEmptyElement(source.GetElementName());
    pSourceNode->AddAttributeStringValue(PACKAGE_SOURCE_ATTRIBUTE_Uri, uri.ToString().c_str());
    pSourceNode->AddAttributeStringValue(PACKAGE_SOURCE_ATTRIBUTE_Type, type.c_str());

    // Optional fields.
    if (source.GetServerCP() != nullptr && source.GetServerCP()->IsStreamed())
        pSourceNode->AddAttributeBooleanValue(PACKAGE_SOURCE_ATTRIBUTE_Streamed, source.GetServerCP()->IsStreamed());

    if (source.GetMetadataCP() != nullptr && !source.GetMetadataCP()->GetLegal().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_Copyright, source.GetMetadataCP()->GetLegal().c_str());

    if (source.GetMetadataCP() != nullptr && !source.GetMetadataCP()->GetTermsOfUse().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_TermOfUse, source.GetMetadataCP()->GetTermsOfUse().c_str());

    if (!source.GetId().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_Id, source.GetId().c_str());

    if (!source.GetProvider().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_Provider, source.GetProvider().c_str());

    if (source.GetServerCP() != nullptr && !source.GetServerCP()->GetLoginKey().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_ServerLoginKey, source.GetServerCP()->GetLoginKey().c_str());

    if (source.GetServerCP() != nullptr && !source.GetServerCP()->GetLoginMethod().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_ServerLoginMethod, source.GetServerCP()->GetLoginMethod().c_str());

    if (source.GetServerCP() != nullptr && !source.GetServerCP()->GetRegistrationPage().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_ServerRegPage, source.GetServerCP()->GetRegistrationPage().c_str());

    if (source.GetServerCP() != nullptr && !source.GetServerCP()->GetOrganisationPage().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_ServerOrgPage, source.GetServerCP()->GetOrganisationPage().c_str());

    if (source.GetVisibility() != RealityDataBase::Visibility::UNDEFINED_VISIBILITY)
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_Visibility, source.GetVisibilityTag().c_str());

    if (0 != source.GetSize())
        pSourceNode->AddElementUInt64Value(PACKAGE_ELEMENT_Size, source.GetSize());

    if (source.GetMetadataCP() != nullptr && !source.GetMetadataCP()->GetDescription().empty())
        {
        BeXmlNodeP pMetadataNode = pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_Metadata, source.GetMetadataCP()->GetDescription().c_str());

        if(source.GetMetadataCP() != nullptr && !source.GetMetadataCP()->GetMetadataType().empty())
            pMetadataNode->AddAttributeStringValue(PACKAGE_SOURCE_ATTRIBUTE_Type, source.GetMetadataCP()->GetMetadataType().c_str());
        }

    if (!source.GetGeoCS().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_GeoCS, source.GetGeoCS().c_str());

    if (!source.GetNoDataValue().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_NoDataValue, source.GetNoDataValue().c_str());

    if (!source.GetSisterFiles().empty())
        {
        BeXmlNodeP pSisterFilesNode = pSourceNode->AddEmptyElement(PACKAGE_ELEMENT_SisterFiles);

        bvector<UriPtr> sisterFiles = source.GetSisterFiles();
        for (UriPtr const& uri2 : sisterFiles)
            {
            pSisterFilesNode->AddElementStringValue(PACKAGE_ELEMENT_File, uri2->ToString().c_str());
            }
        }

    // Write specific source data.
    Utf8String sourceType = source.GetElementName();
    if (PACKAGE_ELEMENT_WmsSource == sourceType)
        {
        // Wms source.
        WmsDataSourceCP pWmsSource = dynamic_cast<WmsDataSourceCP>(&source);

        if (!pWmsSource->GetMapSettings().empty())
            {
            // Create XmlDom from string.
            BeXmlStatus xmlStatus = BEXML_Success;
            BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, pWmsSource->GetMapSettings().c_str());
            if (BEXML_Success != xmlStatus)
                return RealityPackageStatus::XmlReadError;

            // Add node.
            pSourceNode->ImportNode(pXmlDom->GetRootElement());
            }
        }
    else if (PACKAGE_ELEMENT_OsmSource == sourceType)
        {
        // Osm source.
        OsmDataSourceCP pOsmSource = dynamic_cast<OsmDataSourceCP>(&source);

        if (!pOsmSource->GetOsmResource().empty())
            {
            // Create XmlDom from string.
            BeXmlStatus xmlStatus = BEXML_Success;
            BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, pOsmSource->GetOsmResource().c_str());
            if (BEXML_Success != xmlStatus)
                return RealityPackageStatus::XmlReadError;

            // Add node.
            pSourceNode->ImportNode(pXmlDom->GetRootElement());
            }
        }
    else if (PACKAGE_ELEMENT_MultiBandSource == sourceType)
        {
        // MultiBand source.
        MultiBandSourceCP pMultiBandSource = dynamic_cast<MultiBandSourceCP>(&source);

        if (pMultiBandSource->GetRedBand() != NULL)
            {
            BeXmlNodeP pRedBandNode = pSourceNode->AddEmptyElement(PACKAGE_ELEMENT_RedBand);
            WriteSource(*pRedBandNode, *pMultiBandSource->GetRedBand());
            }

        if (pMultiBandSource->GetGreenBand() != NULL)
            {
            BeXmlNodeP pGreenBandNode = pSourceNode->AddEmptyElement(PACKAGE_ELEMENT_GreenBand);
            WriteSource(*pGreenBandNode, *pMultiBandSource->GetGreenBand());
            }

        if (pMultiBandSource->GetBlueBand() != NULL)
            {
            BeXmlNodeP pBlueBandNode = pSourceNode->AddEmptyElement(PACKAGE_ELEMENT_BlueBand);
            WriteSource(*pBlueBandNode, *pMultiBandSource->GetBlueBand());
            }

        if (pMultiBandSource->GetPanchromaticBand() != NULL)
            {
            BeXmlNodeP pPanchromaticBandNode = pSourceNode->AddEmptyElement(PACKAGE_ELEMENT_PanchromaticBand);
            WriteSource(*pPanchromaticBandNode, *pMultiBandSource->GetPanchromaticBand());
            }
        }

    return RealityPackageStatus::Success;
    }

END_BENTLEY_REALITYPLATFORM_NAMESPACE
