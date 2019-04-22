/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "Tests.h"
#include <DgnPlatform/UnitTests/DgnDbTestUtils.h>

//----------------------------------------------------------------------------------------
// @bsiclass                                    Sam.Wilson                      04/15
//----------------------------------------------------------------------------------------
struct ConverterTestBaseFixture : public testing::Test
    {
    protected:
        DgnElementCPtr FindV8ElementInDgnDb(DgnDbR db, DgnV8Api::ElementId eV8Id, RepositoryLinkId rlinkId = RepositoryLinkId());

        void InitializeTheConverter();

        bool m_wantCleanUp = true;

        ISChemaImportVerifier *m_verifier = nullptr;
        virtual bool _ShouldImportSchema(BentleyApi::Utf8StringCR fullSchemaName, DgnV8Api::DgnModel& v8Model) { return true; }

    public:
    struct ConvertOptions
        {
        ConvertOptions() : m_useTiledConverter(false), m_expectModelToBeMappedIn(true) {;}
        bool m_useTiledConverter;
        bool m_expectModelToBeMappedIn;
        BentleyApi::bvector<BentleyApi::BeFileName> m_tiles;
        };

    static ConverterTestsHost m_host;
    static void SetUpTestCase();
    static void TearDownTestCase();

    RootModelConverter::RootModelSpatialParams m_params;
    uint32_t m_count;
    BentleyApi::BeFileName m_v8FileName;
    BentleyApi::BeFileName m_dgnDbFileName;
    BentleyApi::BeFileName m_seedDgnDbFileName;
    ConvertOptions m_opts;
    bool m_noGcs = false;

    virtual void SetUp();
    virtual void TearDown();

    void SetGcsDef();

    RepositoryLinkId FindRepositoryLinkIdByFilename(DgnDbR db, BentleyApi::BeFileNameCR filename);
    RepositoryLinkCPtr FindRepositoryLinkByFilename(DgnDbR db, BentleyApi::BeFileNameCR filename) {auto id = FindRepositoryLinkIdByFilename(db, filename); return db.Elements().Get<RepositoryLink>(id);} 

    static BentleyApi::BeFileName GetOutputDir();
    static BentleyApi::BeFileName GetTempDir();
    BentleyApi::BeFileName GetInputFileName(BentleyApi::WCharCP filename);
    BentleyApi::BeFileName GetOutputFileName(BentleyApi::WCharCP filename);
    void MakeWritableCopyOf(BentleyApi::BeFileName& fnoutFile, BentleyApi::WCharCP filename);
    void MakeWritableCopyOf(BentleyApi::BeFileName& fnoutFile, BentleyApi::BeFileNameCR inputFileName, BentleyApi::WCharCP filename);
    void DeleteExistingDgnDb(BentleyApi::BeFileNameCR);
    void SetUp_CreateNewDgnDb(); // look for name of bim to create in m_seedDgnDbFileName
    DgnDbPtr OpenExistingDgnDb(BentleyApi::BeFileNameCR projectName, DgnDb::OpenMode mode = DgnDb::OpenMode::ReadWrite);
    int CountGeometricModels(DgnDbR);

    void LineUpFiles(BentleyApi::WCharCP outputDgnDbFileName, BentleyApi::WCharCP inputV8FileName, bool doConvert);
    void MakeCopyOfFile(BentleyApi::BeFileNameR refV8File, BentleyApi::WCharCP suffix);
    void CreateAndAddV8Attachment(BentleyApi::BeFileNameR refV8File, int32_t num = 1 , bool useOffsetForElement = false);
    void AddV8Level(BentleyApi::Utf8CP levelname, BentleyApi::BeFileNameCR v8FileName = BentleyApi::BeFileName());
    void SetV8LevelColor(BentleyApi::Utf8CP levelname, uint32_t v8ColorId, BentleyApi::BeFileNameCR v8FileNameIn = BentleyApi::BeFileName());

    void DoConvert(BentleyApi::BeFileNameCR output, BentleyApi::BeFileNameCR input);
    void DoUpdate(BentleyApi::BeFileNameCR output, BentleyApi::BeFileNameCR input, bool expectFailure = false, bool expectUpdate = true);

    void TestElementChanges(BentleyApi::BeFileNameCR rootV8FileName, BentleyApi::BeFileNameCR editV8FileName, size_t nModelsExpected);
    DgnClassId getBisClassId(DgnDbR db, Utf8CP className);
    DgnElementId findFirstElementByClass(DgnDbR db, DgnClassId classId);
    void countElements(DgnModel& model, int expected);
    void countElementsInModelByClass(DgnModel& model, DgnClassId classId, int expected);
    void countModels(DgnDbR db, int ndrawings_expected, int nspatial_expected);
    SubjectCPtr GetFirstJobSubjectUnderParent(SubjectCR);
    SubjectCPtr GetFirstJobSubject(DgnDbR db);
    SubjectCPtr GetJobHierarchySubject(DgnDbR db);
    SubjectCPtr GetReferencesChildSubjectOf(SubjectCR);
    SubjectCPtr GetFirstSourceMasterModelSubject(DgnDbR);
    size_t CountJobSubjects(DgnDbR);
    size_t CountSourceMasterModelSubjects(DgnDbR);

    DefinitionModelPtr GetJobDefinitionModel(DgnDbR db);
    bool ShouldImportSchema(BentleyApi::Utf8StringCR fullSchemaName, DgnV8Api::DgnModel& v8Model) { return _ShouldImportSchema(fullSchemaName, v8Model); }

    //! Use ECSql to SELECT COUNT(*) from the specified ECClass name
    static int SelectCountFromECClass(DgnDbR, Utf8CP className);
    };

struct TestRootModelCreator : RootModelConverter
    {
    DEFINE_T_SUPER(RootModelConverter)
    private:
        ConverterTestBaseFixture* m_testFixture;

    public:
        explicit TestRootModelCreator(RootModelConverter::RootModelSpatialParams& params, ConverterTestBaseFixture* fixture) : T_Super(params), m_testFixture(fixture) {}

        bool _ShouldImportSchema(BentleyApi::Utf8StringCR fullSchemaName, DgnV8Api::DgnModel& v8Model) override { return m_testFixture->ShouldImportSchema(fullSchemaName, v8Model); }

    };

