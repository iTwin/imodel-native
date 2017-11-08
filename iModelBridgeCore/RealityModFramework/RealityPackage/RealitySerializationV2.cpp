/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPackage/RealitySerializationV2.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
        GeoPoint2dP corners = NULL;
        BeXmlNodeP pCornerNode = pDataNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Corners);
        if (NULL != pCornerNode)
            {
            corners = new GeoPoint2d[4];
            RealityPackageStatus status2 = RealityPackageStatus::Success;
            if (RealityPackageStatus::Success != (status2 = RealityDataSerializer::ReadLongLat(corners[ImageryData::Corners::LowerLeft].longitude, corners[ImageryData::Corners::LowerLeft].latitude, *pCornerNode, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_LowerLeft)) ||
                RealityPackageStatus::Success != (status2 = RealityDataSerializer::ReadLongLat(corners[ImageryData::Corners::LowerRight].longitude, corners[ImageryData::Corners::LowerRight].latitude, *pCornerNode, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_LowerRight)) ||
                RealityPackageStatus::Success != (status2 = RealityDataSerializer::ReadLongLat(corners[ImageryData::Corners::UpperLeft].longitude, corners[ImageryData::Corners::UpperLeft].latitude, *pCornerNode, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_UpperLeft)) ||
                RealityPackageStatus::Success != (status2 = RealityDataSerializer::ReadLongLat(corners[ImageryData::Corners::UpperRight].longitude, corners[ImageryData::Corners::UpperRight].latitude, *pCornerNode, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_UpperRight)))
                {
                return status2;
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
        pModelData->SetDataId(id.c_str());
        pModelData->SetDataName(name.c_str());
        pModelData->SetDataset(dataset.c_str());
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
        PinnedDataPtr pPinnedData = PinnedData::Create(*pSources[0], location.longitude, location.latitude);
        pPinnedData->SetDataId(id.c_str());
        pPinnedData->SetDataName(name.c_str());
        pPinnedData->SetDataset(dataset.c_str());
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
        pTerrainData->SetDataId(id.c_str());
        pTerrainData->SetDataName(name.c_str());
        pTerrainData->SetDataset(dataset.c_str());
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

    RealityDataPackage::UndefinedGroup& undefinedGroup = package.GetUndefinedGroupR();
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
        
        // Create undefined data and add alternate sources.
        UndefinedDataPtr pUndefinedData = UndefinedData::Create(*pSources[0]);
        pUndefinedData->SetDataId(id.c_str());
        pUndefinedData->SetDataName(name.c_str());
        pUndefinedData->SetDataset(dataset.c_str());
        for (size_t i = 1; i < pSources.size(); ++i)
            {
            pUndefinedData->AddSource(*pSources[i]);
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
    pRootNode->AddAttributeStringValue(PACKAGE_ATTRIBUTE_Version, "2.1");
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
    RealityDataPackage::ImageryGroup imgGroup = package.GetImageryGroup();
    for(ImageryDataPtr pImgData : imgGroup)
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
                if (RealityPackageStatus::Success != WriteSource(*pSourcesNode, pImgData->GetSource(sourceIndex)))
                    {
                    pGroupNode->RemoveChildNode(pDataNode);
                    continue;
                    }
                }
            }

        // Write ImageryData specific.
        GeoPoint2dCP pCorners = pImgData->GetCornersCP();
        if (NULL == pCorners)
            continue;  // Corners are optional.

        BeXmlNodeP pCornerNode = pDataNode->AddEmptyElement(PACKAGE_ELEMENT_Corners);
        if (RealityPackageStatus::Success != (status = RealityDataSerializer::WriteLongLat(*pCornerNode, PACKAGE_ELEMENT_LowerLeft, pCorners[ImageryData::Corners::LowerLeft].longitude, pCorners[ImageryData::Corners::LowerLeft].latitude)) ||
            RealityPackageStatus::Success != (status = RealityDataSerializer::WriteLongLat(*pCornerNode, PACKAGE_ELEMENT_LowerRight, pCorners[ImageryData::Corners::LowerRight].longitude, pCorners[ImageryData::Corners::LowerRight].latitude)) ||
            RealityPackageStatus::Success != (status = RealityDataSerializer::WriteLongLat(*pCornerNode, PACKAGE_ELEMENT_UpperLeft, pCorners[ImageryData::Corners::UpperLeft].longitude, pCorners[ImageryData::Corners::UpperLeft].latitude)) ||
            RealityPackageStatus::Success != (status = RealityDataSerializer::WriteLongLat(*pCornerNode, PACKAGE_ELEMENT_UpperRight, pCorners[ImageryData::Corners::UpperRight].longitude, pCorners[ImageryData::Corners::UpperRight].latitude)))
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
    RealityDataPackage::ModelGroup modelGroup = package.GetModelGroup();
    for (ModelDataPtr pModelData : modelGroup)
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
                if (RealityPackageStatus::Success != WriteSource(*pSourcesNode, pModelData->GetSource(sourceIndex)))
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
    RealityDataPackage::PinnedGroup pinnedGroup = package.GetPinnedGroup();
    for (PinnedDataPtr pPinnedData : pinnedGroup)
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
                if (RealityPackageStatus::Success != WriteSource(*pSourcesNode, pPinnedData->GetSource(sourceIndex)))
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
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Area, pPinnedData->GetAreaCP()->ToString().c_str());

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
    RealityDataPackage::TerrainGroup terrainGroup = package.GetTerrainGroup();
    for (TerrainDataPtr pTerrainData : terrainGroup)
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
                if (RealityPackageStatus::Success != WriteSource(*pSourcesNode, pTerrainData->GetSource(sourceIndex)))
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
    RealityDataPackage::UndefinedGroup undefinedGroup = package.GetUndefinedGroup();
    for (UndefinedDataPtr pUndefinedData : undefinedGroup)
        {
        if (!pUndefinedData.IsValid())
            continue;

        // Write base first.
        BeXmlNodeP pDataNode = pGroupNode->AddEmptyElement(PACKAGE_ELEMENT_UndefinedData);
        
        // Id.
        if (!pUndefinedData->GetDataId().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Id, pUndefinedData->GetDataId().c_str());

        // Name.
        if (!pUndefinedData->GetDataName().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Name, pUndefinedData->GetDataName().c_str());

        // Dataset.
        if (!pUndefinedData->GetDataset().empty())
            pDataNode->AddElementStringValue(PACKAGE_ELEMENT_Dataset, pUndefinedData->GetDataset().c_str());

        // Sources.
        if (0 != pUndefinedData->GetNumSources())
            {
            BeXmlNodeP pSourcesNode = pDataNode->AddEmptyElement(PACKAGE_ELEMENT_Sources);

            for (size_t sourceIndex = 0; sourceIndex < pUndefinedData->GetNumSources(); ++sourceIndex)
                {
                if (RealityPackageStatus::Success != WriteSource(*pSourcesNode, pUndefinedData->GetSource(sourceIndex)))
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
RealityPackageStatus RealityDataSerializerV2::_WriteSource(BeXmlNodeR node, RealityDataSourceCR source) const
    {
    // Required fields.
    UriCR uri = source.GetUri();
    Utf8String type = source.GetType();
    if (uri.ToString().empty() || type.empty())
        return RealityPackageStatus::MissingSourceAttribute;

    BeXmlNodeP pSourceNode = node.AddEmptyElement(source.GetElementName());
    pSourceNode->AddAttributeStringValue(PACKAGE_SOURCE_ATTRIBUTE_Uri, uri.ToString().c_str());
    pSourceNode->AddAttributeStringValue(PACKAGE_SOURCE_ATTRIBUTE_Type, type.c_str());

    // Optional fields.
    if (source.IsStreamed())
        pSourceNode->AddAttributeBooleanValue(PACKAGE_SOURCE_ATTRIBUTE_Streamed, source.IsStreamed());

    if (!source.GetCopyright().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_Copyright, source.GetCopyright().c_str());

    if (!source.GetTermOfUse().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_TermOfUse, source.GetTermOfUse().c_str());

    if (!source.GetId().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_Id, source.GetId().c_str());

    if (!source.GetProvider().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_Provider, source.GetProvider().c_str());

    if (!source.GetServerLoginKey().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_ServerLoginKey, source.GetServerLoginKey().c_str());

    if (!source.GetServerLoginMethod().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_ServerLoginMethod, source.GetServerLoginMethod().c_str());

    if (!source.GetServerRegistrationPage().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_ServerRegPage, source.GetServerRegistrationPage().c_str());

    if (!source.GetServerOrganisationPage().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_ServerOrgPage, source.GetServerOrganisationPage().c_str());

    if (0 != source.GetSize())
        pSourceNode->AddElementUInt64Value(PACKAGE_ELEMENT_Size, source.GetSize());

    if (!source.GetMetadata().empty())
        {
        BeXmlNodeP pMetadataNode = pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_Metadata, source.GetMetadata().c_str());

        if(!source.GetMetadataType().empty())
            pMetadataNode->AddAttributeStringValue(PACKAGE_SOURCE_ATTRIBUTE_Type, source.GetMetadataType().c_str());
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

END_BENTLEY_REALITYPACKAGE_NAMESPACE