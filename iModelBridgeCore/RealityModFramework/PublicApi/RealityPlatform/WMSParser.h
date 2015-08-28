/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/WMSParser.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>

#if defined (__WMSPARSER_BUILD__)
#   define WMSPARSER_EXPORT EXPORT_ATTRIBUTE
#else
#   define WMSPARSER_EXPORT IMPORT_ATTRIBUTE
#endif

#define BEGIN_BENTLEY_WMSPARSER_NAMESPACE         BEGIN_BENTLEY_NAMESPACE namespace WMSParser {
#define END_BENTLEY_WMSPARSER_NAMESPACE           }}
#define USING_NAMESPACE_BENTLEY_WMSPARSER         using namespace BentleyApi::WMSParser;


#define WMSPARSER_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_WMSPARSER_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_WMSPARSER_NAMESPACE

#define WMSPARSER_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_WMSPARSER_NAMESPACE struct _sname_; DEFINE_REF_COUNTED_PTR(_sname_) END_BENTLEY_WMSPARSER_NAMESPACE

// Capabilities
WMSPARSER_TYPEDEFS(WMSCapabilities)
WMSPARSER_REF_COUNTED_PTR(WMSCapabilities)

WMSPARSER_TYPEDEFS(WMSSingleLevelList)
WMSPARSER_REF_COUNTED_PTR(WMSSingleLevelList)

WMSPARSER_TYPEDEFS(WMSMultiLevelList)
WMSPARSER_REF_COUNTED_PTR(WMSMultiLevelList)

// Service
WMSPARSER_TYPEDEFS(WMSService)
WMSPARSER_REF_COUNTED_PTR(WMSService)

WMSPARSER_TYPEDEFS(WMSOnlineResource)
WMSPARSER_REF_COUNTED_PTR(WMSOnlineResource)

WMSPARSER_TYPEDEFS(WMSContactInformation)
WMSPARSER_REF_COUNTED_PTR(WMSContactInformation)

WMSPARSER_TYPEDEFS(WMSContactPerson)
WMSPARSER_REF_COUNTED_PTR(WMSContactPerson)

WMSPARSER_TYPEDEFS(WMSContactAddress)
WMSPARSER_REF_COUNTED_PTR(WMSContactAddress)

// Capability
WMSPARSER_TYPEDEFS(WMSCapability)
WMSPARSER_REF_COUNTED_PTR(WMSCapability)

WMSPARSER_TYPEDEFS(WMSRequest)
WMSPARSER_REF_COUNTED_PTR(WMSRequest)

WMSPARSER_TYPEDEFS(WMSOperationType)
WMSPARSER_REF_COUNTED_PTR(WMSOperationType)

WMSPARSER_TYPEDEFS(WMSDCPType)
WMSPARSER_REF_COUNTED_PTR(WMSDCPType)

WMSPARSER_TYPEDEFS(WMSLayer)
WMSPARSER_REF_COUNTED_PTR(WMSLayer)

WMSPARSER_TYPEDEFS(WMSGeoBoundingBox)
WMSPARSER_REF_COUNTED_PTR(WMSGeoBoundingBox)

WMSPARSER_TYPEDEFS(WMSLatLonBoundingBox)
WMSPARSER_REF_COUNTED_PTR(WMSLatLonBoundingBox)

WMSPARSER_REF_COUNTED_PTR(WMSBoundingBox)

WMSPARSER_TYPEDEFS(WMSDimension)
WMSPARSER_REF_COUNTED_PTR(WMSDimension)

WMSPARSER_TYPEDEFS(WMSAttribution)
WMSPARSER_REF_COUNTED_PTR(WMSAttribution)

WMSPARSER_TYPEDEFS(WMSUrl)
WMSPARSER_REF_COUNTED_PTR(WMSUrl)

WMSPARSER_TYPEDEFS(WMSIdentifier)
WMSPARSER_REF_COUNTED_PTR(WMSIdentifier)

WMSPARSER_TYPEDEFS(WMSStyle)
WMSPARSER_REF_COUNTED_PTR(WMSStyle)


BEGIN_BENTLEY_WMSPARSER_NAMESPACE

//! Status codes for WMSParser operations.
enum class WMSParserStatus
    {
    Success                 = SUCCESS,  //!< The operation was successful.
    UnknownVersion,                     //!< Version is either undefined or we can't handle it.
    UnknownNode,                        //!< Node is not known and will not be processed.
    XmlReadError,                       //!< BeXML operation failed.
    MissingMandatoryNode,               //!< Mandatory node is missing.
    StringParsingError,                 //!< Error while parsing a string (name, title, etc.).
    // *** Add new here.
    UnknownError            = ERROR,    //!< The operation failed with an unspecified error.
    };

END_BENTLEY_WMSPARSER_NAMESPACE