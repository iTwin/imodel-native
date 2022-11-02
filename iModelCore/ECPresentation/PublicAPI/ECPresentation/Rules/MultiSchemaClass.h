/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ECPresentation/Rules/PresentationRulesTypes.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct MultiSchemaClass : NoXmlSupport<PresentationKey>
    {
    DEFINE_T_SUPER(NoXmlSupport<PresentationKey>)

    private:
        Utf8String m_schemaName;
        bool m_arePolymorphic;
        bvector<Utf8String> m_classNames;

    protected:
        ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
        ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

        Utf8CP _GetJsonElementTypeAttributeName() const override { return nullptr; }
        ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
        ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
        ECPRESENTATION_EXPORT void _WriteJson(JsonValueR) const override;

    public:
        MultiSchemaClass() : m_arePolymorphic(false) {}
        MultiSchemaClass(Utf8String schemaName, bool arePolymorphic, bvector<Utf8String> classNames)
            {
            m_schemaName = schemaName;
            m_arePolymorphic = arePolymorphic;
            m_classNames = classNames;
            }

        //copy constructor
        ECPRESENTATION_EXPORT MultiSchemaClass(MultiSchemaClass const&);
        // move constructor
        ECPRESENTATION_EXPORT MultiSchemaClass(MultiSchemaClass&&);

        ECPRESENTATION_EXPORT static MultiSchemaClass* LoadFromJson(JsonValueCR json, bool defaultPolymorphism);

        Utf8StringCR GetSchemaName() const { return m_schemaName; }
        void SetSchemaName(Utf8String value) { m_schemaName = value; InvalidateHash(); }

        bool GetArePolymorphic() const { return m_arePolymorphic; }
        void SetArePolymorphic(bool value) { m_arePolymorphic = value; InvalidateHash(); }

        bvector<Utf8String> const& GetClassNames() const { return m_classNames; }
        void SetClassNames(bvector<Utf8String> value) { m_classNames = value; InvalidateHash(); }
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
