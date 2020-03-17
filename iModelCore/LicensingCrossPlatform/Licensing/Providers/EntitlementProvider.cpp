/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "EntitlementProvider.h"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_WEBSERVICES

EntitlementProvider::EntitlementProvider
(
    IBuddiProviderPtr buddiProvider,
    IHttpHandlerPtr   httpHandler
) :
    m_buddiProvider(buddiProvider),
    m_httpHandler(httpHandler)
    {}

enum WebV4LicenseStatus
    {
    /// <summary>
    /// Represents a non-existent or invalid license.
    /// </summary>
    NoLicense = 0,

    /// <summary>
    /// Represents a status entitling the user to use a given product.
    /// </summary>
    Allowed = 1,

    /// <summary>
    /// Represents a status entitling the user to use a given product in the trial mode.
    /// </summary>
    AllowedTrial = 2,

    /// <summary>
    /// Represents a status entitling the user to use a given product in the evaluation mode.
    /// </summary>
    AllowedEvaluation = 3,

    /// <summary>
    /// Represents a status denying the use of a given product.
    /// </summary>
    Denied = -1,

    /// <summary>
    /// Represents a status denying the use of a given product in the trial mode, most
    /// likely when the trial period is expired.
    /// </summary>
    DeniedTrial = -2
    };

folly::Future<WebEntitlementResult> EntitlementProvider::FetchWebEntitlementV4(const std::vector<int>& productIds, BeVersionCR version, Utf8StringCR deviceId, Utf8StringCR projectId, Utf8StringCR accessToken, AuthType authType)
    {
    auto url = m_buddiProvider->EntitlementPolicyBaseUrl();// +"/v4.0/api/WebEntitlement";
    url.ReplaceAll("/api", ""); //extra api in buddi returned endpoint
    url = url + "/v4.0/api/WebEntitlement";
    HttpClient client(nullptr, m_httpHandler);

    auto request = client.CreateRequest(url, "POST");
    switch (authType)
        {
            case AuthType::SAML:
                request.GetHeaders().SetValue("authorization", "SAML " + accessToken);
                break;
            case AuthType::OIDC: //Fallthrough as OIDC is default case
            default:
                request.GetHeaders().SetValue("authorization", "Bearer " + accessToken);
        }
    request.GetHeaders().SetValue("content-type", "application/json; charset=utf-8");
    Json::Value requestJson(Json::objectValue);

    Json::Value requestedSecurables(Json::arrayValue);

    for (auto id : productIds)
        {
        Json::Value requestedSecurable(Json::objectValue);
        requestedSecurable["ProductId"] = id;
        requestedSecurable["FeatureString"] = "";
        //requestedSecurable["Version"] = version.ToString();  //since we passing multiple productIds I think we will have to pass multiple versions but this might not matter
        //it says on swagger version not required ??? so this might be unneed and above comment might be nonissue
        requestedSecurables.append(requestedSecurable);
        }


    requestJson["RequestedSecurables"] = requestedSecurables;

    requestJson["MachineName"] = deviceId;
    requestJson["ProjectId"] = projectId;
    requestJson["ClientDateTime"] = DateTime::GetCurrentTimeUtc().ToString();
    requestJson["Locale"] = "en-US"; //m_applicationInfo->GetLanguage(); //no ui in use, no message to consumer 
    requestJson["AppliesTo"] = ENTITLEMENT_POLICYSERVICE_RD_AppliesTo_Url;
    requestJson["CreateTrial"] = "true";
    request.SetRequestBody(HttpStringBody::Create(Json::FastWriter().write(requestJson)));

    //request.GetHeaders().SetContentType(REQUESTHEADER_ContentType_ApplicationJson);

    return request.Perform().then(
        [=] (Response response)
        {
        if (!response.IsSuccess())
            {
            LOG.errorv("FetchWebEntitlementV4 ERROR: Unable to perform web v4 request.");
            LOG.errorv("FetchWebEntitlementV4 ERROR: Response body: %s", response.GetBody().AsString().c_str());
            LOG.errorv("FetchWebEntitlementV4 ERROR: Response status: %s", response.ToStatusString(response.GetConnectionStatus(), response.GetHttpStatus()).c_str());
            LOG.errorv("FetchWebEntitlementV4 ERROR: Response URL: %s", response.GetEffectiveUrl().c_str());

            //WebEntitlementResult httpError{ -1, LicenseStatus::NotEntitled, nullptr };
            //return httpError;
            throw HttpError(response);
            }

        Utf8String resultstring = response.GetBody().AsString();

        Json::Value resultJson;
        Json::Reader::Parse(resultstring, resultJson);
        auto ents = Json::Value(resultJson)["result"]["Entitlements"];

        for (auto& ent : ents)
            {
            Utf8String ls = ent["LicenseStatus"].asString();

            if (ls == "Allowed")
                {
                Utf8String principalId(ent["PrincipalId"].asCString());
                WebEntitlementResult webEntitlement {ent["ProductId"].asInt(), LicenseStatus::Ok, principalId};
                return webEntitlement;
                }
            }

        for (auto& ent : ents)
            {
            Utf8String ls = ent["LicenseStatus"].asString();

            if (ls == "AllowedTrial")
                {
                Utf8String principalId(ent["PrincipalId"].asCString());
                WebEntitlementResult webEntitlement {ent["ProductId"].asInt(), LicenseStatus::Trial, principalId};
                return webEntitlement;
                }
            }

        for (auto& ent : ents)
            {
            Utf8String ls = ent["LicenseStatus"].asString();

            if (ls == "AllowedEvaluation")
                {
                Utf8String principalId(ent["PrincipalId"].asCString());
                WebEntitlementResult webEntitlement {ent["ProductId"].asInt(), LicenseStatus::Trial, principalId};
                return webEntitlement;
                }
            }

        WebEntitlementResult webEntitlement {-1, LicenseStatus::NotEntitled, nullptr};
        return webEntitlement;
        });
    }
