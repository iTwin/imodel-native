/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/PolicyToken.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>

#include <Licensing/Utils/JWToken.h>

#include <json\json.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PolicyToken
{
private:
    Json::Value m_policy;

private:
    PolicyToken(JsonValueCR policy);

public:
    LICENSING_EXPORT static std::shared_ptr<PolicyToken> Create(std::shared_ptr<JWToken> jwToken);

    LICENSING_EXPORT JsonValueCR GetQualifier(Utf8StringCR qualifierName) const;
};

END_BENTLEY_LICENSING_NAMESPACE
