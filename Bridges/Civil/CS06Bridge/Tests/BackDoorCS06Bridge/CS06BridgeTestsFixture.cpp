#include "../BackDoorCS06Bridge/PublicApi/BackDoor/CS06Bridge/BackDoor.h"

//=======================================================================================
// @bsiclass
//=======================================================================================
struct TestKnownLocationsAdmin : DgnPlatformLib::Host::IKnownLocationsAdmin
{
	BeFileName m_tempDir;
	BeFileName m_assetsDir;

	virtual BeFileNameCR _GetLocalTempDirectoryBaseName() override { return m_tempDir; }
	virtual BeFileNameCR _GetDgnPlatformAssetsDirectory() override { return m_assetsDir; }

	TestKnownLocationsAdmin()
	{
		BeTest::GetHost().GetTempDir(m_tempDir);
		BeTest::GetHost().GetDgnPlatformAssetsDirectory(m_assetsDir);
	}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct CS06BridgeTestsHostImpl : DgnPlatformLib::Host
{
	bool m_isInitialized;

	CS06BridgeTestsHostImpl();
	~CS06BridgeTestsHostImpl();

	virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new TestKnownLocationsAdmin(); }
	//virtual RepositoryAdmin & _SupplyRepositoryAdmin() override { return *new TestRepositoryAdmin(); }

	virtual void _SupplyProductName(Utf8StringR name) override { name.assign("CS06BridgeTestsHostImpl"); }
	virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() { return BeSQLite::L10N::SqlangFiles(BeFileName()); } // no translatable strings
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName CS06BridgeTestsHost::GetTestAppProductDirectory()
{
	char* outPath = getenv("OutRoot");

	BeFileName testAppPath(outPath);
	testAppPath.AppendA("Winx64\\Product\\CS06BridgeTestApp\\");

	return testAppPath;
}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
BeFileName CS06BridgeTestsHost::GetOutputDirectory()
{
	BeFileName outputDir;
	BeTest::GetHost().GetOutputRoot(outputDir);
	outputDir.AppendToPath(L"CiviliModelBridgesCS06Bridge\\");
	return outputDir;
}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
BeFileName CS06BridgeTestsHost::GetDgnPlatformAssetsDirectory()
{
	BeFileName assetsRootDirectory;
	BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsRootDirectory);
	return assetsRootDirectory;
}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
BeFileName CS06BridgeTestsHost::BuildProjectFileName(WCharCP baseName)
{
	BeFileName projectFileName = GetOutputDirectory();
	projectFileName.AppendToPath(baseName);
	return projectFileName;
}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
CS06BridgeTestsHost::CS06BridgeTestsHost()
{
	m_pimpl = new CS06BridgeTestsHostImpl;
	CleanOutputDirectory();
}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
CS06BridgeTestsHost::~CS06BridgeTestsHost()
{
	delete m_pimpl;
}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
void CS06BridgeTestsHost::CleanOutputDirectory()
{
	static bool wasOutputDirectoryCleaned = false;
	if (!wasOutputDirectoryCleaned)
	{
		// clean up files from last run
		BeFileName outputDir = GetOutputDirectory();
		BeFileName::EmptyAndRemoveDirectory(outputDir.GetName());
		BeFileName::CreateNewDirectory(outputDir.GetName());
		wasOutputDirectoryCleaned = true;
	}
}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
CS06BridgeTestsHostImpl::CS06BridgeTestsHostImpl() : m_isInitialized(false)
{
	BeAssert((DgnPlatformLib::QueryHost() == NULL) && L"This means an old host is still registered. You should have terminated it first before creating a new host.");

	DgnPlatformLib::Initialize(*this, false);
	//DgnDomains::RegisterDomain(LinearReferencingDomain::GetDomain(), DgnDomain::Required::Yes);
	//DgnDomains::RegisterDomain(RoadRailAlignmentDomain::GetDomain(), DgnDomain::Required::Yes);
	//DgnDomains::RegisterDomain(RoadRailPhysical::RoadRailPhysicalDomain::GetDomain(), DgnDomain::Required::Yes);
	m_isInitialized = true;
}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
CS06BridgeTestsHostImpl::~CS06BridgeTestsHostImpl()
{
	if (m_isInitialized)
	{
		Terminate(false);
	}
}

CS06BridgeTestsHost* CiviliModelBridgesCS06BridgeTestsFixture::m_host = nullptr;

//---------------------------------------------------------------------------------------
// Automatically called by gTest framework before running every test
//---------------------------------------------------------------------------------------
void CiviliModelBridgesCS06BridgeTestsFixture::SetUpTestCase()
{
	m_host = new CS06BridgeTestsHost();
}
//---------------------------------------------------------------------------------------
// Automatically called by gTest framework after running every test
//---------------------------------------------------------------------------------------
void CiviliModelBridgesCS06BridgeTestsFixture::TearDownTestCase()
{
	delete m_host;
	m_host = nullptr;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool CiviliModelBridgesCS06BridgeTestsFixture::RunTestApp(Utf8CP input, Utf8CP bimFileName)
{
	char* outPath = getenv("OutRoot");
	BeFileName testAppPath = m_host->GetTestAppProductDirectory();
	testAppPath.AppendA("CS06BridgeTestApp.exe");

	BeFileName inputPath(outPath);
	inputPath.AppendA("Winx64\\Product\\CS06-CiviliModelBridges-Tests\\Assets\\TestFiles\\CS06\\");
	inputPath.AppendA(input);

	BeFileName outputPath = m_host->GetOutputDirectory();
	outputPath.AppendA(bimFileName);

	Utf8PrintfString cmd("%s -i=\"%s\" -o=\"%s\"",
		Utf8String(testAppPath.c_str()).c_str(),
		Utf8String(inputPath.c_str()).c_str(),
		Utf8String(outputPath.c_str()).c_str());

	int errcode = system(cmd.c_str());
	bool retVal = (0 == errcode);

	if (retVal)
	{
		if (!outputPath.DoesPathExist())
			BeAssert(false);

		BeFileName syncInfoPath(outputPath);
		syncInfoPath.AppendA(".imodelbridge_syncinfo");
		if (!syncInfoPath.DoesPathExist())
			BeAssert(false);
	}

	return retVal;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr CiviliModelBridgesCS06BridgeTestsFixture::VerifyConvertedElements(Utf8CP bimFileName, size_t alignmentCount, size_t roadwayCount)
{
	BeFileName outputPath = m_host->GetOutputDirectory();
	outputPath.AppendA(bimFileName);

	DbResult result;
	DgnDbPtr dgnDbPtr = DgnDb::OpenDgnDb(&result, outputPath, DgnDb::OpenParams(Db::OpenMode::Readonly));

	//ECSqlStatement stmt;
	//stmt.Prepare(*dgnDbPtr, "SELECT COUNT(*) FROM "
	//	BRRA_SCHEMA(BRRA_CLASS_Alignment) " a," BIS_SCHEMA(BIS_CLASS_Model) " m, " BIS_SCHEMA(BIS_CLASS_SpatialLocationPartition) " p "
	//	"WHERE p.CodeValue=? AND p.ECInstanceId = m.ModeledElement.Id AND a.Model.Id = m.ECInstanceId "
	//	"GROUP BY a.Model.Id");
	//BeAssert(stmt.IsPrepared());

	//stmt.BindText(1, ORDBRIDGE_AlignmentModelName, IECSqlBinder::MakeCopy::Yes);
	//BeAssert(DbResult::BE_SQLITE_ROW == stmt.Step());
	//BeAssert(alignmentCount == stmt.GetValueInt(0));

	//stmt.Finalize();

	//stmt.Prepare(*dgnDbPtr, "SELECT COUNT(*) FROM "
	//	BRRP_SCHEMA(BRRP_CLASS_Roadway) " r," BIS_SCHEMA(BIS_CLASS_Model) " m, " BIS_SCHEMA(BIS_CLASS_PhysicalPartition) " p "
	//	"WHERE p.CodeValue=? AND p.ECInstanceId = m.ModeledElement.Id AND r.Model.Id = m.ECInstanceId "
	//	"GROUP BY r.Model.Id");
	//BeAssert(stmt.IsPrepared());

	//stmt.BindText(1, ORDBRIDGE_PhysicalModelName, IECSqlBinder::MakeCopy::Yes);

	//if (roadwayCount == 0)
	//	BeAssert(DbResult::BE_SQLITE_DONE == stmt.Step());
	//else
	//{
	//	BeAssert(DbResult::BE_SQLITE_ROW == stmt.Step());
	//	BeAssert(roadwayCount == stmt.GetValueInt(0));
	//}

	return dgnDbPtr;
}