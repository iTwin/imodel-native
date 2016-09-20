/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPackage/RealitySerialization.cpp $
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


//=======================================================================================
//                              RealityDataSerializer
//=======================================================================================

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::Read(RealityDataPackageR package, BeXmlDomR xmlDom)
    {
    return _Read(package, xmlDom);
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    6/2016
//-------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::Write(BeXmlDomR xmlDom, RealityDataPackageCR package) const
    {
    return _Write(xmlDom, package);
    }

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
    DPoint2d longLat;
    RealityPackageStatus status = ReadDPoint2d(longLat, parent, childName);
    if(RealityPackageStatus::Success != status)
        return status;

    if(!IsValidLongLat(longLat.x, longLat.y))
        return RealityPackageStatus::InvalidLongitudeLatitude;

    longitude = longLat.x;
    latitude = longLat.y;

    return RealityPackageStatus::Success;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::WriteLongLat(BeXmlNodeR parent, Utf8CP childName, double longitude, double latitude)
    {
    if(!IsValidLongLat(longitude, latitude))
        return RealityPackageStatus::InvalidLongitudeLatitude;

    DPoint2d longLat = {longitude, latitude};

    return WriteDPoint2d(parent, childName, longLat);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::ReadDPoint2d(DPoint2dR point, BeXmlNodeR parent, Utf8CP childName)
    {
    // Use UTF8 since it is the native format.
    Utf8String pointStr;
    if(BEXML_Success != parent.GetContent(pointStr, childName))
        return RealityPackageStatus::UnknownError;  

    Utf8StringTokenizer tokenizer(pointStr, SPACE_DELIMITER);

    if(!tokenizer.Get(point.x) || !tokenizer.Get(point.y))
        return RealityPackageStatus::UnknownError;  

    return RealityPackageStatus::Success;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
RealityPackageStatus RealityDataSerializer::WriteDPoint2d(BeXmlNodeR parent, Utf8CP childName, DPoint2dCR point)
    {
    WString pointString;
    pointString.Sprintf (LATLONG_PRINT_FORMAT, point.x, point.y);
    if(NULL == parent.AddElementStringValue(childName, pointString.c_str()))
        return RealityPackageStatus::UnknownError;

    return RealityPackageStatus::Success;
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


END_BENTLEY_REALITYPACKAGE_NAMESPACE