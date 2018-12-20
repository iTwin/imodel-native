/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealitySerialization.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if !defined(ANDROID)
#pragma once
#endif

#include <RealityPlatform/RealityPlatformAPI.h>
#include <RealityPlatform/RealityDataPackage.h>
#include <RealityPlatform/SpatialEntity.h>
#include <BeXml/BeXml.h>
#include "RealitySerialization.h"


BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE


//=======================================================================================
//                              RealityDataSerializer
//=======================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::Read(RealityDataPackageR package, BeXmlDomR xmlDom) { return _Read(package, xmlDom); }
RealityPackageStatus RealityDataSerializer::Write(BeXmlDomR xmlDom, RealityDataPackageCR package) const { return _Write(xmlDom, package); }

RealityPackageStatus RealityDataSerializer::ReadPackageInfo(RealityDataPackageR package, BeXmlDomR xmlDom) { return _ReadPackageInfo(package, xmlDom); }
RealityPackageStatus RealityDataSerializer::ReadImageryGroup(RealityDataPackageR package, BeXmlDomR xmlDom) { return _ReadImageryGroup(package, xmlDom); }
RealityPackageStatus RealityDataSerializer::ReadModelGroup(RealityDataPackageR package, BeXmlDomR xmlDom) { return _ReadModelGroup(package, xmlDom); }
RealityPackageStatus RealityDataSerializer::ReadPinnedGroup(RealityDataPackageR package, BeXmlDomR xmlDom) { return _ReadPinnedGroup(package, xmlDom); }
RealityPackageStatus RealityDataSerializer::ReadTerrainGroup(RealityDataPackageR package, BeXmlDomR xmlDom) { return _ReadTerrainGroup(package, xmlDom); }
RealityPackageStatus RealityDataSerializer::ReadUndefinedGroup(RealityDataPackageR package, BeXmlDomR xmlDom) { return _ReadUndefinedGroup(package, xmlDom); }
RealityPackageStatus RealityDataSerializer::ReadUnknownElements(RealityDataPackageR package, BeXmlNodeP pNode) { return _ReadUnknownElements(package, pNode); }
SpatialEntityDataSourcePtr RealityDataSerializer::ReadSource(RealityPackageStatus& status, BeXmlNodeP pNode) { return _ReadSource(status, pNode); }
MultiBandSourcePtr   RealityDataSerializer::ReadMultiBandSource(RealityPackageStatus& status, BeXmlNodeP pNode) { return _ReadMultiBandSource(status, pNode); }

RealityPackageStatus RealityDataSerializer::WritePackageInfo(BeXmlNodeR node, RealityDataPackageCR package) const { return _WritePackageInfo(node, package); }
RealityPackageStatus RealityDataSerializer::WriteImageryGroup(BeXmlNodeR node, RealityDataPackageCR package) const { return _WriteImageryGroup(node, package); }
RealityPackageStatus RealityDataSerializer::WriteModelGroup(BeXmlNodeR node, RealityDataPackageCR package) const { return _WriteModelGroup(node, package); }
RealityPackageStatus RealityDataSerializer::WritePinnedGroup(BeXmlNodeR node, RealityDataPackageCR package) const { return _WritePinnedGroup(node, package); }
RealityPackageStatus RealityDataSerializer::WriteTerrainGroup(BeXmlNodeR node, RealityDataPackageCR package) const { return _WriteTerrainGroup(node, package); }
RealityPackageStatus RealityDataSerializer::WriteUndefinedGroup(BeXmlNodeR node, RealityDataPackageCR package) const { return _WriteUndefinedGroup(node, package); }
RealityPackageStatus RealityDataSerializer::WriteSource(BeXmlNodeR node, SpatialEntityDataSourceCR source) const { return _WriteSource(node, source); }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
bool RealityDataSerializer::IsValidLongLat(double longitude, double latitude)
    {
    if (IN_RANGE(longitude, -180, 180) && IN_RANGE(latitude, -90, 90))
        return true;

    return false;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::ReadLongLat(double& longitude, double& latitude, BeXmlNodeR parent, Utf8CP childName)
    {
    GeoPoint2d longLat;
    RealityPackageStatus status = ReadGeoPoint2d(longLat, parent, childName);
    if(RealityPackageStatus::Success != status)
        return status;

    if(!IsValidLongLat(longLat.longitude, longLat.latitude))
        return RealityPackageStatus::InvalidLongitudeLatitude;

    longitude = longLat.longitude;
    latitude = longLat.latitude;

    return RealityPackageStatus::Success;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::WriteLongLat(BeXmlNodeR parent, Utf8CP childName, double longitude, double latitude)
    {
    if(!IsValidLongLat(longitude, latitude))
        return RealityPackageStatus::InvalidLongitudeLatitude;

    GeoPoint2d longLat = {longitude, latitude};

    return WriteGeoPoint2d(parent, childName, longLat);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::ReadGeoPoint2d(GeoPoint2dR point, BeXmlNodeR parent, Utf8CP childName)
    {
    // Use UTF8 since it is the native format.
    Utf8String pointStr;
    if(BEXML_Success != parent.GetContent(pointStr, childName))
        return RealityPackageStatus::UnknownError;  

    Utf8StringTokenizer tokenizer(pointStr, SPACE_DELIMITER);

    if(!tokenizer.Get(point.longitude) || !tokenizer.Get(point.latitude))
        return RealityPackageStatus::UnknownError;  

    return RealityPackageStatus::Success;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::WriteGeoPoint2d(BeXmlNodeR parent, Utf8CP childName, GeoPoint2dCR point)
    {
    WString pointString;
    pointString.Sprintf (LATLONG_PRINT_FORMAT, point.longitude, point.latitude);
    if(NULL == parent.AddElementStringValue(childName, pointString.c_str()))
        return RealityPackageStatus::UnknownError;

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::_ReadPackageInfo(RealityDataPackageR package, BeXmlDomR xmlDom)
    {
    // Get root node and context.
    BeXmlNodeP pRootNode = xmlDom.GetRootElement();
    if (NULL == pRootNode)
        return RealityPackageStatus::XmlReadError;

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(pRootNode);

    // Origin.
    Utf8String origin;
    xmlDom.SelectNodeContent(origin, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Origin, pContext, BeXmlDom::NODE_BIAS_First);
    package.SetOrigin(origin.c_str());

    // Requesting application.
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
    Utf8String polygonString; 
    BoundingPolygonPtr pPolygon = BoundingPolygon::Create();
    xmlDom.SelectNodeContent(polygonString, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_BoundingPolygon, pContext, BeXmlDom::NODE_BIAS_First);
    if (!polygonString.empty() && (pPolygon = BoundingPolygon::FromString(polygonString.c_str())).IsNull())
        return RealityPackageStatus::PolygonParsingError; // If present, format must be valid.
    package.SetBoundingPolygon(*pPolygon);

    // Context
    Utf8String context;
    xmlDom.SelectNodeContent(id, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Context, pContext, BeXmlDom::NODE_BIAS_First);
    package.SetContext(id.c_str());

    // Unknown elements.
    _ReadUnknownElements(package, pRootNode->GetFirstChild());

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::_ReadImageryGroup(RealityDataPackageR package, BeXmlDomR xmlDom)
    { 
    return RealityPackageStatus::UnknownError; 
    };

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::_ReadModelGroup(RealityDataPackageR package, BeXmlDomR xmlDom)
    {
    return RealityPackageStatus::UnknownError; 
    };

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::_ReadPinnedGroup(RealityDataPackageR package, BeXmlDomR xmlDom)
    {
    return RealityPackageStatus::UnknownError;
    };

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::_ReadTerrainGroup(RealityDataPackageR package, BeXmlDomR xmlDom)
    {
    return RealityPackageStatus::UnknownError;
    };

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                    04/2017
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::_ReadUndefinedGroup(RealityDataPackageR package, BeXmlDomR xmlDom)
    {
    return RealityPackageStatus::UnknownError;
    };

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    8/2016
// &&JFC TODO: Not efficient, find a better way to do this.
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::_ReadUnknownElements(RealityDataPackageR package, BeXmlNodeP pNode)
    {
    // At least one unknow element was found, return.
    if (package.HasUnknownElements())
        return RealityPackageStatus::Success;

    // Check if it is an unknown element.       
    if (0 == pNode->NameStricmp(PACKAGE_ELEMENT_Name) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_Description) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_CreationDate) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_Copyright) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_PackageId) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_BoundingPolygon) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_PackageOrigin) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_ImageryGroup) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_RequestingApplication) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_Dataset) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_RequestingApplication) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_Resolution) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_ImageryData) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_Corners) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_LowerLeft) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_LowerRight) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_UpperLeft) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_UpperRight) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_ModelGroup) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_ModelData) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_PinnedGroup) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_PinnedData) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_Position) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_Area) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_TerrainGroup) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_TerrainData) ||
        0 == pNode->NameStricmp(PACKAGE_ELEMENT_UndefinedGroup) || 0 == pNode->NameStricmp(PACKAGE_ELEMENT_UndefinedData) ||
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
            _ReadUnknownElements(package, pNode->GetFirstChild());

        // Look for siblings.
        if (NULL != pNode->GetNextSibling())
            _ReadUnknownElements(package, pNode->GetNextSibling());
        }
    else
        {
        package.SetUnknownElements(true);
        return RealityPackageStatus::Success;
        }

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
SpatialEntityDataSourcePtr RealityDataSerializer::_ReadSource(RealityPackageStatus& status, BeXmlNodeP pNode)
    {
    // Create data source from uri and type.
    Utf8String uri, type;
    if (BEXML_Success != pNode->GetAttributeStringValue(uri, PACKAGE_SOURCE_ATTRIBUTE_Uri))
        {
        status = RealityPackageStatus::MissingSourceAttribute;
        return NULL;
        }
    UriPtr pUri = Uri::Create(uri.c_str());
    pNode->GetAttributeStringValue(type, PACKAGE_SOURCE_ATTRIBUTE_Type);

    SpatialEntityDataSourcePtr pDataSource = SpatialEntityDataSource::Create(*pUri, type.c_str());
    if (pDataSource == NULL)
        {
        status = RealityPackageStatus::UnknownError;
        return NULL;
        }

    SpatialEntityServerPtr pServer = SpatialEntityServer::Create();

    // Streamed.
    bool isStreamed;
    pNode->GetAttributeBooleanValue(isStreamed, PACKAGE_SOURCE_ATTRIBUTE_Streamed);
    pServer->SetStreamed(isStreamed);

    // Id.
    Utf8String id;
    pNode->GetContent(id, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Id);
    pDataSource->SetId(id.c_str());

    SpatialEntityMetadataPtr pMetadata = SpatialEntityMetadata::Create();

    // Copyright.
    Utf8String copyright;
    pNode->GetContent(copyright, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Copyright);
    pMetadata->SetLegal(copyright.c_str());

    // TermOfUse.
    Utf8String termOfUse;
    pNode->GetContent(termOfUse, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_TermOfUse);
    pMetadata->SetTermsOfUse(termOfUse.c_str());

    // Provider.
    Utf8String provider;
    pNode->GetContent(provider, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Provider);
    pDataSource->SetProvider(provider.c_str());

    // Server login key.
    Utf8String serverLoginKey;
    pNode->GetContent(serverLoginKey, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_ServerLoginKey);
    pServer->SetLoginKey(serverLoginKey.c_str());

    // Server login method.
    Utf8String serverLoginMethod;
    pNode->GetContent(serverLoginMethod, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_ServerLoginMethod);
    pServer->SetLoginMethod(serverLoginMethod.c_str());

    // Server registration page.
    Utf8String serverRegPage;
    pNode->GetContent(serverRegPage, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_ServerRegPage);
    pServer->SetRegistrationPage(serverRegPage.c_str());

    // Server organisation page.
    Utf8String serverOrgPage;
    pNode->GetContent(serverOrgPage, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_ServerOrgPage);
    pServer->SetOrganisationPage(serverOrgPage.c_str());

    pDataSource->SetServer(pServer);

    // Visibility.
    Utf8String visibilityTag;
    pNode->GetContent(visibilityTag, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Visibility);
    pDataSource->SetVisibilityByTag(visibilityTag.c_str());

    // Size.
    uint64_t size;
    if (BeXmlStatus::BEXML_Success != pNode->GetContentUInt64Value(size, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Filesize))
        pNode->GetContentUInt64Value(size, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Size);
    pDataSource->SetSize(size);

    // Metadata.
    Utf8String metadata;
    pNode->GetContent(metadata, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Metadata);
    pMetadata->SetDescription(metadata.c_str());

    // Metadata type.
    Utf8String metadataType;
    BeXmlNodeP pMetadataNode = pNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Metadata);
    if (NULL != pMetadataNode)
        {
        pMetadataNode->GetAttributeStringValue(metadataType, PACKAGE_SOURCE_ATTRIBUTE_Type);
        pMetadata->SetMetadataType(metadataType.c_str());
        }

    pDataSource->SetMetadata(pMetadata);

    // GeoCS.
    Utf8String geocs;
    pNode->GetContent(geocs, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_GeoCS);
    pDataSource->SetGeoCS(geocs.c_str());

    // NoDataValue.
    Utf8String nodatavalue;
    pNode->GetContent(nodatavalue, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_NoDataValue);
    pDataSource->SetNoDataValue(nodatavalue.c_str());

    // Sister files.    
    BeXmlNodeP pSisterFilesNode = pNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_SisterFiles);
    if (NULL != pSisterFilesNode)
        {
        Utf8String file;
        bvector<UriPtr> sisterFiles;

        BeXmlDom::IterableNodeSet fileNodes;
        pSisterFilesNode->SelectChildNodes(fileNodes, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_File);
        for (BeXmlNodeP const& pFileNode : fileNodes)
            {
            pFileNode->GetContent(file);
            sisterFiles.push_back(Uri::Create(file.c_str()));
            }

        if (!sisterFiles.empty())
            pDataSource->SetSisterFiles(sisterFiles);
        }

    return pDataSource;
    };

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
MultiBandSourcePtr RealityDataSerializer::_ReadMultiBandSource(RealityPackageStatus& status, BeXmlNodeP pNode)
    {
    // *** Read base data source ***
    // Create data source from uri and type.
    Utf8String uri, type;
    if (BEXML_Success != pNode->GetAttributeStringValue(uri, PACKAGE_SOURCE_ATTRIBUTE_Uri))
        {
        status = RealityPackageStatus::MissingSourceAttribute;
        return NULL;
        }
    UriPtr pUri = Uri::Create(uri.c_str());
    pNode->GetAttributeStringValue(type, PACKAGE_SOURCE_ATTRIBUTE_Type);

    MultiBandSourcePtr pDataSource = MultiBandSource::Create(*pUri, type.c_str());
    if (pDataSource == NULL)
        {
        status = RealityPackageStatus::UnknownError;
        return NULL;
        }

    SpatialEntityServerPtr pServer = SpatialEntityServer::Create();

    // Streamed.
    bool isStreamed;
    pNode->GetAttributeBooleanValue(isStreamed, PACKAGE_SOURCE_ATTRIBUTE_Streamed);
    pServer->SetStreamed(isStreamed);

    // Id.
    Utf8String id;
    pNode->GetContent(id, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Id);
    pDataSource->SetId(id.c_str());

    SpatialEntityMetadataPtr pMetadata = SpatialEntityMetadata::Create();

    // Copyright.
    Utf8String copyright;
    pNode->GetContent(copyright, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Copyright);
    pMetadata->SetLegal(copyright.c_str());

    // TermOfUse.
    Utf8String termOfUse;
    pNode->GetContent(termOfUse, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_TermOfUse);
    pMetadata->SetTermsOfUse(termOfUse.c_str());

    // Provider.
    Utf8String provider;
    pNode->GetContent(provider, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Provider);
    pDataSource->SetProvider(provider.c_str());
    
    // Server login key.
    Utf8String serverLoginKey;
    pNode->GetContent(serverLoginKey, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_ServerLoginKey);
    pServer->SetLoginKey(serverLoginKey.c_str());

    // Server login method.
    Utf8String serverLoginMethod;
    pNode->GetContent(serverLoginMethod, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_ServerLoginMethod);
    pServer->SetLoginMethod(serverLoginMethod.c_str());

    // Server registration page.
    Utf8String serverRegPage;
    pNode->GetContent(serverRegPage, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_ServerRegPage);
    pServer->SetRegistrationPage(serverRegPage.c_str());

    // Server organisation page.
    Utf8String serverOrgPage;
    pNode->GetContent(serverOrgPage, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_ServerOrgPage);
    pServer->SetOrganisationPage(serverOrgPage.c_str());

    pDataSource->SetServer(pServer);

    // Visibility.
    Utf8String visibilityTag;
    pNode->GetContent(visibilityTag, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Visibility);
    pDataSource->SetVisibilityByTag(visibilityTag.c_str());

    // Size.
    uint64_t size;
    pNode->GetContentUInt64Value(size, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Size);
    pDataSource->SetSize(size);

    // Metadata.
    Utf8String metadata;
    pNode->GetContent(metadata, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Metadata);
    pMetadata->SetDescription(metadata.c_str());

    // Metadata type.
    Utf8String metadataType;
    BeXmlNodeP pMetadataNode = pNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_Metadata);
    if (NULL != pMetadataNode)
        {
        pMetadataNode->GetAttributeStringValue(metadataType, PACKAGE_SOURCE_ATTRIBUTE_Type);
        pMetadata->SetMetadataType(metadataType.c_str());
        }

    pDataSource->SetMetadata(pMetadata);

    // GeoCS.
    Utf8String geocs;
    pNode->GetContent(geocs, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_GeoCS);
    pDataSource->SetGeoCS(geocs.c_str());

    // NoDataValue.
    Utf8String nodatavalue;
    pNode->GetContent(nodatavalue, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_NoDataValue);
    pDataSource->SetNoDataValue(nodatavalue.c_str());

    // Sister files.    
    BeXmlNodeP pSisterFilesNode = pNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_SisterFiles);
    if (NULL != pSisterFilesNode)
        {
        Utf8String file;
        bvector<UriPtr> sisterFiles;

        BeXmlDom::IterableNodeSet fileNodes;
        pSisterFilesNode->SelectChildNodes(fileNodes, PACKAGE_PREFIX ":" PACKAGE_ELEMENT_File);
        for (BeXmlNodeP const& pFileNode : fileNodes)
            {
            pFileNode->GetContent(file);
            sisterFiles.push_back(Uri::Create(file.c_str()));
            }

        if (!sisterFiles.empty())
            pDataSource->SetSisterFiles(sisterFiles);
        }

    // *** Read MultiBand specific data ***
    // Red band.
    BeXmlNodeP pRedBandNode = pNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_RedBand);
    if (pRedBandNode != NULL)
        {
        SpatialEntityDataSourcePtr pRedBandSource = ReadSource(status, pRedBandNode->GetFirstChild());

        if (pRedBandSource != NULL)
            {
            // Force visibility to same as multiband
            pRedBandSource->SetVisibility(pDataSource->GetVisibility());
            pDataSource->SetRedBand(*pRedBandSource);
            }
        }

    // Green band.
    BeXmlNodeP pGreenBandNode = pNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_GreenBand);
    if (pGreenBandNode != NULL)
        {
        SpatialEntityDataSourcePtr pGreenBandSource = ReadSource(status, pGreenBandNode->GetFirstChild());

        if (pGreenBandSource != NULL)
            {
            // Force visibility to same as multiband
            pGreenBandSource->SetVisibility(pDataSource->GetVisibility());
            pDataSource->SetGreenBand(*pGreenBandSource);
            }
        }

    // Blue band.
    BeXmlNodeP pBlueBandNode = pNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_BlueBand);
    if (pBlueBandNode != NULL)
        {
        SpatialEntityDataSourcePtr pBlueBandSource = ReadSource(status, pBlueBandNode->GetFirstChild());
        if (pBlueBandSource != NULL)
            {
            // Force visibility to same as multiband
            pBlueBandSource->SetVisibility(pDataSource->GetVisibility());
            pDataSource->SetBlueBand(*pBlueBandSource);
            }
        }

    // Panchromatic band.
    BeXmlNodeP pPanchromaticNode = pNode->SelectSingleNode(PACKAGE_PREFIX ":" PACKAGE_ELEMENT_PanchromaticBand);
    if (pPanchromaticNode != NULL)
        {
        SpatialEntityDataSourcePtr pPanchromaticBandSource = ReadSource(status, pPanchromaticNode->GetFirstChild());
        if (pPanchromaticBandSource != NULL)
            {
            // Force visibility to same as multiband
            pPanchromaticBandSource->SetVisibility(pDataSource->GetVisibility());
            pDataSource->SetPanchromaticBand(*pPanchromaticBandSource);
            }
        }

    return pDataSource;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::_WritePackageInfo(BeXmlNodeR node, RealityDataPackageCR package) const 
    { 
    // Optional fields, if empty don't add them to the package.
    if (!package.GetOrigin().empty())
        node.AddElementStringValue(PACKAGE_ELEMENT_Origin, package.GetOrigin().c_str());

    if (!package.GetRequestingApplication().empty())
        node.AddElementStringValue(PACKAGE_ELEMENT_RequestingApplication, package.GetRequestingApplication().c_str());
    
    if (!package.GetName().empty())
        node.AddElementStringValue(PACKAGE_ELEMENT_Name, package.GetName().c_str());

    if (!package.GetDescription().empty())
        node.AddElementStringValue(PACKAGE_ELEMENT_Description, package.GetDescription().c_str());

    if (!package.GetCreationDate().ToString().empty())
        node.AddElementStringValue(PACKAGE_ELEMENT_CreationDate, package.GetCreationDate().ToString().c_str());

    if (!package.GetCopyright().empty())
        node.AddElementStringValue(PACKAGE_ELEMENT_Copyright, package.GetCopyright().c_str());

    if (!package.GetId().empty())
        node.AddElementStringValue(PACKAGE_ELEMENT_Id, package.GetId().c_str());

    if (!package.GetBoundingPolygon().ToString().empty())
        node.AddElementStringValue(PACKAGE_ELEMENT_BoundingPolygon, package.GetBoundingPolygon().ToString().c_str());

    return RealityPackageStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::_WriteImageryGroup(BeXmlNodeR node, RealityDataPackageCR package) const 
    { 
    return RealityPackageStatus::UnknownError;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::_WriteModelGroup(BeXmlNodeR node, RealityDataPackageCR package) const
    {
    return RealityPackageStatus::UnknownError;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::_WritePinnedGroup(BeXmlNodeR node, RealityDataPackageCR package) const
    {
    return RealityPackageStatus::UnknownError;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::_WriteTerrainGroup(BeXmlNodeR node, RealityDataPackageCR package) const
    {
    return RealityPackageStatus::UnknownError;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                    04/2017
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::_WriteUndefinedGroup(BeXmlNodeR node, RealityDataPackageCR package) const
    {
    return RealityPackageStatus::UnknownError;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::_WriteSource(BeXmlNodeR node, SpatialEntityDataSourceCR source) const
    {
    return RealityPackageStatus::UnknownError;
    }

//=======================================================================================
//                              RealityDataSerializer - Factory
//=======================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityDataSerializerPtr RealityDataSerializerFactory::CreateSerializer(BeXmlDomR xmlDom)
    {
    // Read version.
    uint32_t majorVersion, minorVersion;
    if (RealityPackageStatus::Success != RealityDataSerializerFactory::ReadVersion(majorVersion, minorVersion, xmlDom))
        return NULL;

    // Create proper serializer.
    return RealityDataSerializerFactory::CreateSerializer(majorVersion);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityDataSerializerPtr RealityDataSerializerFactory::CreateSerializer(uint32_t majorVersion)
    {
    // Create proper serializer.
    switch (majorVersion)
        {
        case 1:
            return RealityDataSerializerV1::Create();
        case 2:
            return RealityDataSerializerV2::Create();
        default:
            return NULL;
        }
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializerFactory::ReadVersion(uint32_t& majorVersion, uint32_t& minorVersion, BeXmlDomR xmlDom)
    {  
    BeXmlNodeP pRootNode = xmlDom.GetRootElement();
    if (NULL == pRootNode)
        return RealityPackageStatus::XmlReadError;

    // Get version.
    Utf8String version;
    if (BEXML_Success != pRootNode->GetAttributeStringValue(version, PACKAGE_ATTRIBUTE_Version))
        return RealityPackageStatus::XmlReadError;

    // Parse.
    if (2 != BE_STRING_UTILITIES_UTF8_SSCANF(version.c_str(), "%u.%u", &majorVersion, &minorVersion))
        return RealityPackageStatus::XmlReadError;

    return RealityPackageStatus::Success;
    }


END_BENTLEY_REALITYPLATFORM_NAMESPACE