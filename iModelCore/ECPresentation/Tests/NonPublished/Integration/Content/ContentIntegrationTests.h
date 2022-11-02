/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "../PresentationManagerIntegrationTests.h"
#include "../../Helpers/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

//=======================================================================================
// @bsiclass
//=======================================================================================
struct RulesDrivenECPresentationManagerContentTests : PresentationManagerIntegrationTests
{
#ifdef wip_related_content_without_instances
    ECClassCP m_widgetClass;
    ECClassCP m_gadgetClass;
    ECClassCP m_sprocketClass;
    ECSchemaCP m_schema;
#endif

    void SetUp() override
        {
        PresentationManagerIntegrationTests::SetUp();

#ifdef wip_related_content_without_instances
        m_schema = s_project->GetECDb().Schemas().GetSchema("RulesEngineTest");
        ASSERT_TRUE(nullptr != m_schema);

        m_widgetClass = m_schema->GetClassCP("Widget");
        m_gadgetClass = m_schema->GetClassCP("Gadget");
        m_sprocketClass = m_schema->GetClassCP("Sprocket");
#endif
        }

    void CloseConnection(IConnectionCR connection)
        {
        static_cast<TestConnectionManager&>(*m_connectionManager).NotifyConnectionClosed(connection);
        }

    ContentCPtr GetVerifiedContent(ContentDescriptorCR descriptor, bool verifyPagedResponses = true)
        {
        auto params = AsyncContentRequestParams::Create(s_project->GetECDb(), descriptor);
        size_t size = GetValidatedResponse(m_manager->GetContentSetSize(params));
        ContentCPtr fullContent = GetValidatedResponse(m_manager->GetContent(params));
        if (fullContent.IsNull())
            {
            EXPECT_EQ(0, size);
            return nullptr;
            }
        EXPECT_EQ(size, fullContent->GetContentSet().GetSize());
        if (verifyPagedResponses)
            {
            for (size_t i = 0; i < size; ++i)
                {
                ContentCPtr pagedContent = GetValidatedResponse(m_manager->GetContent(MakePaged(params, PageOptions(i, 1))));
                EXPECT_TRUE(pagedContent.IsValid());
                EXPECT_EQ(1, pagedContent->GetContentSet().GetSize());
                EXPECT_EQ(fullContent->GetContentSet().Get(i)->AsJson(), pagedContent->GetContentSet().Get(0)->AsJson())
                    << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(fullContent->GetContentSet().Get(i)->AsJson()) << "\r\n"
                    << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(pagedContent->GetContentSet().Get(0)->AsJson());
                }
            }
        return fullContent;
        }

    static void ValidateFieldCategoriesHierarchy(ContentDescriptor::Field const&, bvector<Utf8String> const& expectedCategoriesHierarchyLeafToRoot);
    static bvector<ECPropertyCP> CreateNProperties(ECEntityClassR ecClass, int numberOfProperties);
};
