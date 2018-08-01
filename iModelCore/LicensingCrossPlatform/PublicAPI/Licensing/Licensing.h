/*--------------------------------------------------------------------------------------+
 |
 |     $Source: LicensingCrossPlatform/PublicAPI/Licensing/Licensing.h $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>

//__PUBLISH_SECTION_END__
#undef LICENSING_EXPORT
#ifdef __BELICENSING_BUILD__
    #define LICENSING_EXPORT EXPORT_ATTRIBUTE
#else
//__PUBLISH_SECTION_START__
    #define LICENSING_EXPORT IMPORT_ATTRIBUTE
//__PUBLISH_SECTION_END__ 
#endif
//__PUBLISH_SECTION_START__

enum LicenseStatus
    {
    /*! An error occurred trying to determine a license status. */
    LICENSE_STATUS_Error = (-1),
    /*! Okay to run - product has been in touch with a Select Server recently, or has a checked-out license. */
    LICENSE_STATUS_Ok = (101),
    /*! We are in the Configured Grace Period. Product runs as a full product, but a message is displayed to the user. */
    LICENSE_STATUS_Offline = (102),
    /*! We have never activated and are within the 7-day default PreActivation Period. */
    LICENSE_STATUS_PreActivation = (104),
    /*! We are beyond the full-product PreActivation mode and have never been activated. Product should not run. */
    LICENSE_STATUS_Expired = (105),
    /*! The policy explicitly denies access */
    LICENSE_STATUS_AccessDenied = (106),
    /*! Offline is disabled because usage logs have not been sent (at all or recently enough) */
    LICENSE_STATUS_DisabledByLogSend = (107),
    /*! DisabledByLogSend mode is the status of a product when the policy is invalid */
    LICENSE_STATUS_DisabledByPolicy = (108),
    /*! Trial mode is the status of a product when a product is used in evaluation usage  */
    LICENSE_STATUS_Trial = (109),
    /*NotEntitled is the status used when there is no policy file and we need to insert a record for first date used*/
    LICENSE_STATUS_NotEntitled = (110)
    };

#define BENTLEY_LICENSING_NAMESPACE_NAME   Licensing
#define BEGIN_BENTLEY_LICENSING_NAMESPACE  BEGIN_BENTLEY_NAMESPACE namespace BENTLEY_LICENSING_NAMESPACE_NAME {
#define END_BENTLEY_LICENSING_NAMESPACE    END_BENTLEY_NAMESPACE }
#define USING_NAMESPACE_BENTLEY_LICENSING  using namespace BentleyApi::BENTLEY_LICENSING_NAMESPACE_NAME;

#define LOGGER_NAMESPACE_BENTLEY_LICENSING  "Bentley.LICENSING"

