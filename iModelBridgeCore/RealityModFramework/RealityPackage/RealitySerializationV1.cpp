/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPackage/RealitySerializationV1.cpp $
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
RealityDataSerializerV1Ptr RealityDataSerializerV1::Create()
    {
    return new RealityDataSerializerV1();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityDataSerializerV1::RealityDataSerializerV1()
    {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV1::_Read(RealityDataPackageR package, BeXmlDomR xmlDom)
    {
    RealityPackageStatus status = RealityPackageStatus::UnknownError;

    // Set namespace and version.
    xmlDom.RegisterNamespace(PACKAGE_PREFIX, PACKAGE_V1_NAMESPACE);
    package.SetMajorVersion(1);
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

    // We do not read undefined group as it is not supported by version 1

    return status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV1::_ReadImageryGroup(RealityDataPackageR package, BeXmlDomR xmlDom)
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
        BeXmlNodeP pSourceNode = pDataNode->GetFirstChild();
        if (NULL == pSourceNode)
            continue; // Missing source node.

        RealityDataSourcePtr pDataSource = ReadSource(status, pSourceNode);
        if (status != RealityPackageStatus::Success)
            return status;

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

        // Create imagery data and add it to the group.
        ImageryDataPtr pImgData = ImageryData::Create(*pDataSource, corners);
        imgGroup.push_back(pImgData);
        }

    // Unknown elements.
    ReadUnknownElements(package, pRootNode->GetFirstChild());

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV1::_ReadModelGroup(RealityDataPackageR package, BeXmlDomR xmlDom)
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
        BeXmlNodeP pSourceNode = pDataNode->GetFirstChild();
        if (NULL == pSourceNode)
            continue; // Missing source node.

        RealityDataSourcePtr pDataSource = ReadSource(status, pSourceNode);
        if (status != RealityPackageStatus::Success)
            return status;

        // Create model data and add it to the group.
        ModelDataPtr pModelData = ModelData::Create(*pDataSource);
        modelGroup.push_back(pModelData);
        }

    // Unknown elements.
    ReadUnknownElements(package, pRootNode->GetFirstChild());

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV1::_ReadPinnedGroup(RealityDataPackageR package, BeXmlDomR xmlDom)
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
        BeXmlNodeP pSourceNode = pDataNode->GetFirstChild();
        if (NULL == pSourceNode)
            continue; // Missing source node.

        RealityDataSourcePtr pDataSource = ReadSource(status, pSourceNode);
        if (status != RealityPackageStatus::Success)
            return status;

        // Read PinnedData specific.
        GeoPoint2d location; location.longitude = 0.0; location.latitude = 0.0;
        if (RealityPackageStatus::Success != (status = RealityDataSerializer::ReadLongLat(location.longitude, location.latitude, *pDataNode, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Position)))
            return status;  // Location is mandatory for pinned data.

        if (!RealityDataSerializer::IsValidLongLat(location.longitude, location.latitude))
            return RealityPackageStatus::InvalidLongitudeLatitude;

        // Create pinned data and add it to the group.
        PinnedDataPtr pPinnedData = PinnedData::Create(*pDataSource, location.longitude, location.latitude);
        pinnedGroup.push_back(pPinnedData);
        }

    // Unknown elements.
    ReadUnknownElements(package, pRootNode->GetFirstChild());

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV1::_ReadTerrainGroup(RealityDataPackageR package, BeXmlDomR xmlDom)
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
        BeXmlNodeP pSourceNode = pDataNode->GetFirstChild();
        if (NULL == pSourceNode)
            continue; // Missing source node.

        RealityDataSourcePtr pDataSource = ReadSource(status, pSourceNode);
        if (status != RealityPackageStatus::Success)
            return status;

        // Create terrain data and add it to the group.
        TerrainDataPtr pTerrainData = TerrainData::Create(*pDataSource);
        terrainGroup.push_back(pTerrainData);
        }

    // Unknown elements.
    ReadUnknownElements(package, pRootNode->GetFirstChild());

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV1::_Write(BeXmlDomR xmlDom, RealityDataPackageCR package) const
    {
    RealityPackageStatus status = RealityPackageStatus::UnknownError;

    // Create root node.
    BeXmlNodeP pRootNode = xmlDom.AddNewElement(PACKAGE_ELEMENT_Root, NULL, NULL);
    if (NULL == pRootNode)
        return RealityPackageStatus::UnknownError;

    // Add version and namespace.
    pRootNode->AddAttributeStringValue(PACKAGE_ATTRIBUTE_Version, "1.0");
    pRootNode->AddNamespace(NULL, PACKAGE_V1_NAMESPACE);
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

    // We do not write undefined group as it is not supported by version 1.0
    return status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV1::_WriteImageryGroup(BeXmlNodeR node, RealityDataPackageCR package) const
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
    for (ImageryDataPtr pImgData : imgGroup)
        {
        if (!pImgData.IsValid())
            continue;

        // Write base first.
        BeXmlNodeP pDataNode = pGroupNode->AddEmptyElement(PACKAGE_ELEMENT_ImageryData);
        if (RealityPackageStatus::Success != WriteSource(*pDataNode, pImgData->GetSource(0)))
            {
            pGroupNode->RemoveChildNode(pDataNode);
            continue;
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
RealityPackageStatus RealityDataSerializerV1::_WriteModelGroup(BeXmlNodeR node, RealityDataPackageCR package) const
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
        if (RealityPackageStatus::Success != WriteSource(*pDataNode, pModelData->GetSource(0)))
            {
            pGroupNode->RemoveChildNode(pDataNode);
            continue;
            }
        }

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV1::_WritePinnedGroup(BeXmlNodeR node, RealityDataPackageCR package) const
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
        if (RealityPackageStatus::Success != WriteSource(*pDataNode, pPinnedData->GetSource(0)))
            {
            pGroupNode->RemoveChildNode(pDataNode);
            continue;
            }

        // Write PinnedData specific.
        GeoPoint2dCR location = pPinnedData->GetLocation();
        BeAssert(RealityDataSerializer::IsValidLongLat(location.longitude, location.latitude));
        RealityDataSerializer::WriteLongLat(*pDataNode, PACKAGE_ELEMENT_Position, location.longitude, location.latitude);
        }

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV1::_WriteTerrainGroup(BeXmlNodeR node, RealityDataPackageCR package) const
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
        if (RealityPackageStatus::Success != WriteSource(*pDataNode, pTerrainData->GetSource(0)))
            {
            pGroupNode->RemoveChildNode(pDataNode);
            continue;
            }
        }

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV1::_WriteSource(BeXmlNodeR node, RealityDataSourceCR source) const
    {
    // Required fields.
    UriCR uri = source.GetUri();
    Utf8String type = source.GetType();
    if (uri.ToString().empty() || type.empty())
        return RealityPackageStatus::MissingSourceAttribute;

    BeXmlNodeP pSourceNode = node.AddEmptyElement(PACKAGE_ELEMENT_Source);
    pSourceNode->AddAttributeStringValue(PACKAGE_SOURCE_ATTRIBUTE_Uri, uri.GetSource().c_str());
    pSourceNode->AddAttributeStringValue(PACKAGE_SOURCE_ATTRIBUTE_Type, type.c_str());

    // Optional fields.
    if (!source.GetCopyright().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_Copyright, source.GetCopyright().c_str());

    if (!source.GetId().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_Id, source.GetId().c_str());

    if (!source.GetProvider().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_Provider, source.GetProvider().c_str());

    if (0 != source.GetSize())
        pSourceNode->AddElementUInt64Value(PACKAGE_ELEMENT_Filesize, source.GetSize());

    if (!source.GetMetadata().empty())
        pSourceNode->AddElementStringValue(PACKAGE_ELEMENT_Metadata, source.GetMetadata().c_str());

    return RealityPackageStatus::Success;
    }

END_BENTLEY_REALITYPACKAGE_NAMESPACE
