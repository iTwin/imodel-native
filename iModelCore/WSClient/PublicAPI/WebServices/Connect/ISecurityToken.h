/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <Bentley/DateTime.h>
#include <memory>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                julius.cepukenas    03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct ISecurityToken
    {
    public:
        virtual ~ISecurityToken() {};

        //! Check whenver given token is valid format
        virtual bool IsSupported() const = 0;

        //! Create string for authorization header
        virtual Utf8String ToAuthorizationString() const = 0;

        //! Return original token representation
        virtual Utf8StringCR AsString() const = 0;

        //! Returns expiration date if available and is known
        virtual DateTime GetExpirationDate() const { return DateTime(); };

        //! Compare contents of two tokens for equality
        virtual bool operator==(const ISecurityToken& other) const = 0;
    };
typedef std::shared_ptr<ISecurityToken> ISecurityTokenPtr;

END_BENTLEY_WEBSERVICES_NAMESPACE
