/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../RealityPlatformToolsCurl/WSGServices.cpp"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM
PUSH_DISABLE_DEPRECATION_WARNINGS
//-------------------------------------------------------------------------------------
// @bsimethod                                   Vishal.Shingare                04/2020
//-------------------------------------------------------------------------------------
void ConnectTokenManager::RefreshToken() const
    {
    if(m_tokenCallback)
        return m_tokenCallback(m_token, m_tokenRefreshTimer);

    if (nullptr == m_tokenProvider)
        {
        m_token = Utf8String();
        return;
        }

    m_token = Utf8String("Authorization: ");
    if (m_isFromTokenProvider)
        m_token.append(" Token ");
    else
        m_token.append(" Bearer ");
    Utf8String tokenStr = Utf8String();
    WebServices::ISecurityTokenPtr tokenPtr = nullptr;

    tokenPtr = m_tokenProvider->GetToken();
    if (tokenPtr == nullptr)
        tokenPtr = m_tokenProvider->UpdateToken()->GetResult();

    if (tokenPtr != nullptr)
        {
        tokenStr = tokenPtr->ToAuthorizationString();
        }
    else
        {
        m_token = Utf8String();
        }

    m_token.append(tokenStr);
    }
POP_DISABLE_DEPRECATION_WARNINGS
