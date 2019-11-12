/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ConverterTestsBaseFixture.h"
#include <VersionedDgnV8Api/ECObjects/ECObjectsAPI.h>
#include <VersionedDgnV8Api/DgnPlatform/CustomItemType.h>

//---------------------------------------------------------------------------------------
// @bsiclass                                    Carole.MacDonald            07/2018
//---------------+---------------+---------------+---------------+---------------+-------
struct ItemTypesTests : public ConverterTestBaseFixture
    {
    DEFINE_T_SUPER(ConverterTestBaseFixture);

    protected:
        void CreateItemTypes();
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2018
//---------------+---------------+---------------+---------------+---------------+-------
void ItemTypesTests::CreateItemTypes()
    {
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    auto lib = DgnV8Api::ItemTypeLibrary::Create(L"Converter", *(v8editor.m_file));
    DgnV8Api::ItemType* itemSet = lib->AddItemType(L"Foo");

    auto prop = itemSet->AddProperty(L"Ichi");
    lib->Write();
    v8editor.Save();

    Bentley::DgnECInstanceEnablerP enabler = DgnV8Api::DgnECManager::GetManager().ObtainInstanceEnablerByName(lib->GetInternalName(), L"Foo", *(v8editor.m_file));
    ECObjectsV8::StandaloneECInstanceR wipInstance = enabler->GetSharedWipInstance();
    wipInstance.SetValue(L"Ichi", Bentley::ECN::ECValue(L"hana"));

    DgnV8Api::DgnElementECInstancePtr instance;

    DgnV8Api::ElementId eidWithInst;
    v8editor.AddLine(&eidWithInst);
    DgnV8Api::ElementHandle eh(eidWithInst, v8editor.m_defaultModel);
    DgnV8Api::DgnECInstanceStatus ecInstanceStatus = enabler->CreateInstanceOnElement(&instance, wipInstance, eh);

    v8editor.Save();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ItemTypesTests, Create)
    {
    LineUpFiles(L"ItemTypes.bim", L"Test3d.dgn", false);
    CreateItemTypes();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    BentleyApi::ECN::ECSchemaCP testSchema = db->Schemas().GetSchema("DgnCustomItemTypes_Converter");
    ASSERT_TRUE(NULL != testSchema);
    ASSERT_TRUE(testSchema->IsDynamicSchema());
    }