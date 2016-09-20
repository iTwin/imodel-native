/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPackage/RealitySerializationV1.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

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
RealityPackageStatus RealityDataSerializerV1::ReadPackageInfo(RealityDataPackageR package, BeXmlDomR xmlDom)
    {
    // Namespace.
    xmlDom.RegisterNamespace(PACKAGE_PREFIX, PACKAGE_V1_NAMESPACE);

    // Version.
    package.SetMajorVersion(1);
    package.SetMinorVersion(0);

    // Get root node and context.
    BeXmlNodeP pRootNode = xmlDom.GetRootElement();
    if (NULL == pRootNode)
        return RealityPackageStatus::XmlReadError;

    xmlXPathContextPtr pRootContext = xmlDom.AcquireXPathContext(pRootNode);

    // Origin.
    Utf8String origin;
    xmlDom.SelectNodeContent(origin, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_PackageOrigin, pRootContext, BeXmlDom::NODE_BIAS_First);
    package.SetOrigin(origin.c_str());

    // Name.
    Utf8String name;
    xmlDom.SelectNodeContent(name, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Name, pRootContext, BeXmlDom::NODE_BIAS_First);
    package.SetName(name.c_str());

    // Description.
    Utf8String description;
    xmlDom.SelectNodeContent(description, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Description, pRootContext, BeXmlDom::NODE_BIAS_First);
    package.SetDescription(description.c_str());

    // Copyright.
    Utf8String copyright;
    xmlDom.SelectNodeContent(copyright, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Copyright, pRootContext, BeXmlDom::NODE_BIAS_First);
    package.SetCopyright(copyright.c_str());

    // Id.
    Utf8String id;
    xmlDom.SelectNodeContent(id, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Id, pRootContext, BeXmlDom::NODE_BIAS_First);
    package.SetId(id.c_str());

    // Creation date.
    Utf8String creationDateUTC; DateTime creationDate;
    xmlDom.SelectNodeContent(creationDateUTC, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_CreationDate, pRootContext, BeXmlDom::NODE_BIAS_First);
    if (!creationDateUTC.empty() && BentleyStatus::SUCCESS != DateTime::FromString(creationDate, creationDateUTC.c_str()))
        return RealityPackageStatus::InvalidDateFormat; // If present, format must be valid.

    package.SetCreationDate(creationDate);

    // Bounding polygon.
    Utf8String polygonString; BoundingPolygonPtr pPolygon;
    xmlDom.SelectNodeContent(polygonString, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_BoundingPolygon, pRootContext, BeXmlDom::NODE_BIAS_First);
    if (polygonString.empty() || (pPolygon = BoundingPolygon::FromString(polygonString.c_str())).IsNull())
        package.SetBoundingPolygon(*BoundingPolygon::Create());    
    else
        package.SetBoundingPolygon(*pPolygon);

    // Unknown elements.
    ReadUnknownElements(package, pRootNode->GetFirstChild());

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV1::ReadImageryGroup(RealityDataPackageR package, BeXmlDomR xmlDom)
    {
    RealityPackageStatus status = RealityPackageStatus::Success;

    // Get root node and context.
    BeXmlNodeP pRootNode = xmlDom.GetRootElement();
    if (NULL == pRootNode)
        return RealityPackageStatus::XmlReadError;

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(pRootNode); 
    pContext;

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
RealityPackageStatus RealityDataSerializerV1::ReadModelGroup(RealityDataPackageR package, BeXmlDomR xmlDom)
    {
    RealityPackageStatus status = RealityPackageStatus::Success;

    // Get root node and context.
    BeXmlNodeP pRootNode = xmlDom.GetRootElement();
    if (NULL == pRootNode)
        return RealityPackageStatus::XmlReadError;

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(pRootNode);
    pContext;

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
RealityPackageStatus RealityDataSerializerV1::ReadPinnedGroup(RealityDataPackageR package, BeXmlDomR xmlDom)
    {
    RealityPackageStatus status = RealityPackageStatus::Success;

    // Get root node and context.
    BeXmlNodeP pRootNode = xmlDom.GetRootElement();
    if (NULL == pRootNode)
        return RealityPackageStatus::XmlReadError;

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(pRootNode);
    pContext;

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
        DPoint2d location; location.x = 0.0; location.y = 0.0;
        if (RealityPackageStatus::Success != (status = RealityDataSerializer::ReadLongLat(location.x, location.y, *pDataNode, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Position)))
            return status;  // Location is mandatory for pinned data.

        if (!RealityDataSerializer::IsValidLongLat(location.x, location.y))
            return RealityPackageStatus::InvalidLongitudeLatitude;

        // Create pinned data and add it to the group.
        PinnedDataPtr pPinnedData = PinnedData::Create(*pDataSource, location.x, location.y);
        pinnedGroup.push_back(pPinnedData);
        }

    // Unknown elements.
    ReadUnknownElements(package, pRootNode->GetFirstChild());

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV1::ReadTerrainGroup(RealityDataPackageR package, BeXmlDomR xmlDom)
    {
    RealityPackageStatus status = RealityPackageStatus::Success;

    // Get root node and context.
    BeXmlNodeP pRootNode = xmlDom.GetRootElement();
    if (NULL == pRootNode)
        return RealityPackageStatus::XmlReadError;

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(pRootNode);
    pContext;

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
// @bsimethod                                   Jean-Francois.Cote         	    8/2016
// &&JFC TODO: Not efficient, find a better way to do this.
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV1::ReadUnknownElements(RealityDataPackageR package, BeXmlNodeP pNode)
    {
    // At least one unknow element was found, return.
    if (package.HasUnknownElements())
        return RealityPackageStatus::Success;

    // Check if it is an unknown element.       
    if (0 == pNode->NameStricmp(PACKAGE_ELEMENT_Name)           || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_Description)     ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_CreationDate)   || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_Copyright)       ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_PackageId)      || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_BoundingPolygon) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_PackageOrigin)  || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_ImageryGroup)    ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_ImageryData)    || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_Corners)         ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_LowerLeft)      || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_LowerRight)      ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_UpperLeft)      || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_UpperRight)      ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_ModelGroup)     || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_ModelData)       ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_PinnedGroup)    || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_PinnedData)      ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_Position)       || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_Area)            ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_TerrainGroup)   || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_TerrainData)     ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_Sources)        || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_Source)          ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_Copyright)      || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_Id)              ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_Provider)       || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_Size)            ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_Filesize)       || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_Metadata)        ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_GeoCS)          || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_SisterFiles)     ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_File)           || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_WmsSource)       ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_OsmSource))
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
RealityDataSourcePtr RealityDataSerializerV1::ReadSource(RealityPackageStatus& status, BeXmlNodeP pSourceNode)
    {
    // Create data source from uri and type.
    Utf8String uri, type;
    if (BEXML_Success != pSourceNode->GetAttributeStringValue(uri, PACKAGE_SOURCE_ATTRIBUTE_Uri) ||
        BEXML_Success != pSourceNode->GetAttributeStringValue(type, PACKAGE_SOURCE_ATTRIBUTE_Type))
        {
        status = RealityPackageStatus::MissingSourceAttribute;
        return NULL;
        }
        

    UriPtr pUri = Uri::Create(uri.c_str());
    RealityDataSourcePtr pDataSource = RealityDataSource::Create(*pUri, type.c_str());
    if (pDataSource == NULL)
        {
        status = RealityPackageStatus::UnknownError;
        return NULL;
        }
        
    // Copyright.
    Utf8String copyright;
    pSourceNode->GetContent(copyright, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Copyright);
    pDataSource->SetCopyright(copyright.c_str());

    // Id.
    Utf8String id;
    pSourceNode->GetContent(id, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Id);
    pDataSource->SetId(id.c_str());

    // Provider.
    Utf8String provider;
    pSourceNode->GetContent(provider, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Provider);
    pDataSource->SetProvider(provider.c_str());

    // Filesize.
    uint64_t filesize;
    pSourceNode->GetContentUInt64Value(filesize, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Filesize);
    pDataSource->SetSize(filesize);

    // Metadata.
    Utf8String metadata;
    pSourceNode->GetContent(metadata, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Metadata);
    pDataSource->SetMetadata(metadata.c_str());

    return pDataSource;
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
RealityPackageStatus RealityDataSerializerV1::WritePackageInfo(BeXmlNodeR node, RealityDataPackageCR package) const
    {
    // Match the version we are persisting.
    node.AddAttributeStringValue(PACKAGE_ATTRIBUTE_Version, "1.0");

    // Namespaces
    node.AddNamespace(NULL, PACKAGE_V1_NAMESPACE);       // Set as default namespace.
    node.AddNamespace(W3SCHEMA_PREFIX, W3SCHEMA_URI);

    // Root children
    node.AddElementStringValue(PACKAGE_ELEMENT_Name, package.GetName().c_str());
    node.AddElementStringValue(PACKAGE_ELEMENT_CreationDate, package.GetCreationDate().ToString().c_str());

    // Optional fields, if empty don't add them to the package.
    if (!package.GetOrigin().empty())
        node.AddElementStringValue(PACKAGE_ELEMENT_PackageOrigin, package.GetOrigin().c_str());

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
RealityPackageStatus RealityDataSerializerV1::WriteImageryGroup(BeXmlNodeR node, RealityDataPackageCR package) const
    {
    RealityPackageStatus status = RealityPackageStatus::UnknownError;

    if (package.GetImageryGroup()[0]->GetNumSources() <= 0)
        return RealityPackageStatus::Success; // No imagery data.
    
    // Group node.
    BeXmlNodeP pGroupNode = node.AddEmptyElement(PACKAGE_ELEMENT_ImageryGroup);
    if (NULL == pGroupNode)
        return RealityPackageStatus::UnknownError;

    // Add data to group.
    RealityDataPackage::ImageryGroup imgGroup = package.GetImageryGroup();
    for each (ImageryDataPtr pImgData in imgGroup)
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
RealityPackageStatus RealityDataSerializerV1::WriteModelGroup(BeXmlNodeR node, RealityDataPackageCR package) const
    {
    if (package.GetModelGroup()[0]->GetNumSources() <= 0)
        return RealityPackageStatus::Success; // No model data.

    // Group node.
    BeXmlNodeP pGroupNode = node.AddEmptyElement(PACKAGE_ELEMENT_ModelGroup);
    if (NULL == pGroupNode)
        return RealityPackageStatus::UnknownError;

    // Add data to group.
    RealityDataPackage::ModelGroup modelGroup = package.GetModelGroup();
    for each (ModelDataPtr pModelData in modelGroup)
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
RealityPackageStatus RealityDataSerializerV1::WritePinnedGroup(BeXmlNodeR node, RealityDataPackageCR package) const
    {
    if (package.GetPinnedGroup()[0]->GetNumSources() <= 0)
        return RealityPackageStatus::Success; // No pinned data.

    // Group node.
    BeXmlNodeP pGroupNode = node.AddEmptyElement(PACKAGE_ELEMENT_PinnedGroup);
    if (NULL == pGroupNode)
        return RealityPackageStatus::UnknownError;

    // Add data to group.
    RealityDataPackage::PinnedGroup pinnedGroup = package.GetPinnedGroup();
    for each (PinnedDataPtr pPinnedData in pinnedGroup)
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
        DPoint2dCR location = pPinnedData->GetLocation();
        BeAssert(RealityDataSerializer::IsValidLongLat(location.x, location.y));
        RealityDataSerializer::WriteLongLat(*pDataNode, PACKAGE_ELEMENT_Position, location.x, location.y);
        }

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerV1::WriteTerrainGroup(BeXmlNodeR node, RealityDataPackageCR package) const
    {
    if (package.GetTerrainGroup()[0]->GetNumSources() <= 0)
        return RealityPackageStatus::Success; // No terrain data.

    // Group node.
    BeXmlNodeP pGroupNode = node.AddEmptyElement(PACKAGE_ELEMENT_TerrainGroup);
    if (NULL == pGroupNode)
        return RealityPackageStatus::UnknownError;

    // Add data to group.
    RealityDataPackage::TerrainGroup terrainGroup = package.GetTerrainGroup();
    for each (TerrainDataPtr pTerrainData in terrainGroup)
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
RealityPackageStatus RealityDataSerializerV1::WriteSource(BeXmlNodeR node, RealityDataSourceCR source) const
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
