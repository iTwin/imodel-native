/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeSecurity/BeSecurity.h>
#include <BeSecurity/SecureStore.h>
#include <BeJsonCpp/BeJsonUtilities.h>

BEGIN_BENTLEY_SECURITY_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                julius.cepukenas    01/2019
*
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ISecureLocalState> ISecureLocalStatePtr;
struct ISecureLocalState : ILocalState
    {
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                julius.cepukenas    01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct SecureLocalState : ISecureLocalState
    {
    private:
        ILocalState* m_localState;
        ICipherPtr m_cipher;

    public:
        void _SaveValue(Utf8CP nameSpace, Utf8CP key, Utf8StringCR value) override
            {
            auto encryptedValue = (value.empty() ? value : m_cipher->Encrypt(value.c_str()));
            m_localState->SaveValue(nameSpace, key, encryptedValue);
            };

        Utf8String _GetValue(Utf8CP nameSpace, Utf8CP key) const override
            {
            Utf8String value = m_localState->GetValue(nameSpace, key);
            return m_cipher->Decrypt(value.c_str());
            };

    public:
        //! Create new secure store object with default local state to store encrypted data.
        SecureLocalState(ILocalState* localState, ICipherPtr cipher = nullptr) 
            : m_localState(localState), m_cipher(cipher ? cipher : std::make_shared<SecureStore>(*m_localState)) {};
    };

END_BENTLEY_SECURITY_NAMESPACE
