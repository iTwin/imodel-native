/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct RequiredSchemaSpecification : NoXmlSupport<PresentationKey>
{
    DEFINE_T_SUPER(NoXmlSupport<PresentationKey>)

private:
    Utf8String m_name;
    Nullable<Version> m_minVersion;
    Nullable<Version> m_maxVersion;

protected:
    Utf8CP _GetJsonElementTypeAttributeName() const override {return nullptr;}
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR) const override;

public:
    RequiredSchemaSpecification() {}
    RequiredSchemaSpecification(Utf8String name, Nullable<Version> minVersion = nullptr, Nullable<Version> maxVersion = nullptr)
        : m_name(name), m_minVersion(std::move(minVersion)), m_maxVersion(std::move(maxVersion))
        {}

    Utf8StringCR GetName() const {return m_name;}

    //! Minimum required schema version (inclusive)
    Nullable<Version> const& GetMinVersion() const {return m_minVersion;}
    void SetMinVersion(Nullable<Version> value) {m_minVersion = value; InvalidateHash();}

    //! MAximum allowed schema version (exclusive)
    Nullable<Version> const& GetMaxVersion() const {return m_maxVersion;}
    void SetMaxVersion(Nullable<Version> value) {m_maxVersion = value; InvalidateHash();}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
