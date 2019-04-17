/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "../Common.h"
#include <DgnPlatform/DgnCoreAPI.h>
#include <Bentley/BeTest.h>

USING_NAMESPACE_BENTLEY_DGN
#define SCHEMA_VERSION_TEST_SCHEMA_NAME "SchemaVersionTest"
#define TEST_ELEMENT_CLASS_NAME "TestElement"

//=======================================================================================
// @bsiclass                                               Algirdas.Mikoliunas   10/17
//=======================================================================================
struct SchemaVersionTestDomain : DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(SchemaVersionTestDomain, )
    private:
        BeFileName m_relativePath;

        WCharCP _GetSchemaRelativePath() const override { return m_relativePath.GetName(); }

        //---------------------------------------------------------------------------------------
        //@bsimethod                                   Algirdas.Mikoliunas             10/2017
        //---------------------------------------------------------------------------------------
        PhysicalModelPtr InsertPhysicalModel(DgnDbR db, Utf8CP partitionName) const;

        //---------------------------------------------------------------------------------------
        //@bsimethod                                   Algirdas.Mikoliunas             10/2017
        //---------------------------------------------------------------------------------------
        DgnCategoryId InsertSpatialCategory(DgnDbR db, Utf8CP categoryName) const;

        //---------------------------------------------------------------------------------------
        //@bsimethod                                   Algirdas.Mikoliunas             10/2017
        //---------------------------------------------------------------------------------------
        void _OnSchemaImported(DgnDbR db) const override;

    public:
        //---------------------------------------------------------------------------------------
        //@bsimethod                                   Algirdas.Mikoliunas             10/2017
        //---------------------------------------------------------------------------------------
        SchemaVersionTestDomain();

        //---------------------------------------------------------------------------------------
        //@bsimethod                                   Algirdas.Mikoliunas             10/2017
        //---------------------------------------------------------------------------------------
        void ClearHandlers();

        //---------------------------------------------------------------------------------------
        //@bsimethod                                   Algirdas.Mikoliunas             10/2017
        //---------------------------------------------------------------------------------------
        void SetVersion(Utf8CP version);

        //---------------------------------------------------------------------------------------
        //@bsimethod                                   Algirdas.Mikoliunas             10/2017
        //---------------------------------------------------------------------------------------
        static void Register(Utf8CP version, DgnDomain::Required isRequired, DgnDomain::Readonly isReadonly);
        //---------------------------------------------------------------------------------------
        //@bsimethod                                   Algirdas.Mikoliunas             10/2017
        //---------------------------------------------------------------------------------------
        static DgnCode CreateCode(DgnDbR dgndb, Utf8StringCR value);
    };

//=======================================================================================
// @bsiclass                                               Algirdas.Mikoliunas   10/17
//=======================================================================================
struct TestElement : PhysicalElement
    {
    friend struct TestElementHandler;

    DGNELEMENT_DECLARE_MEMBERS(TEST_ELEMENT_CLASS_NAME, PhysicalElement)
    public:
        TestElement(CreateParams const& params);

        static Dgn::DgnClassId QueryClassId(DgnDbCR dgndb);

        static RefCountedPtr<TestElement> Create(Dgn::PhysicalModelR model, Dgn::DgnCategoryId categoryId, Dgn::DgnCode code = Dgn::DgnCode());
    };

//=======================================================================================
// @bsiclass                                               Algirdas.Mikoliunas   10/17
//=======================================================================================
struct TestElementHandler : dgn_ElementHandler::Physical
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(TEST_ELEMENT_CLASS_NAME, TestElement, TestElementHandler, dgn_ElementHandler::Physical, );
    };
