#include "PointoolsVortexAPIInternal.h"

#define POINTOOLS_API_BUILD_DLL

#ifdef POINTOOLS_BENTLEY_LICENSING
#include <ptlic/PointoolsBentleyLicenseAPI.h>
#endif

#include <ptapi/PointoolsVortexAPI.h>

#include <ptapi/PointoolsAPI_handle.h>

#include <ptengine/PointsScene.h>
#include <ptengine/PointsPager.h>
#include <ptengine/RenderEngine.h>
#include <ptengine/engine.h>

#include <ptappdll/ptapp.h>
#include <ptcloud2/pod.h>

#include <ptl/branch.h>
#include <ptl/project.h>
#include <pt/project.h>
#include <pt/registry.h>
#include <pt/timestamp.h>
 
#include <encryption/tea.h>

#include <PTRMI/TestPTRMI.h>
#include <PTRMI/Manager.h>
#include <PTRMI/PipeManagerExt.h>
#include <ptds/DataSourceManager.h>
#include <ptds/DataSourceServer.h>
#include <ptds/DataSourceCache.h>

#include <ptengine/StreamHost.h>

#if NEEDS_WORK_VORTEX_DGNDB 
#include <PTRMI/ClientInterfaceExtDataBentley.h>
#endif

#include <pt/trace.h>

#define xstr(s) str(s)
#define str(s) #s


PTbool				initializeScenesProject					(void);
PTbool				initializeSceneGraphProject				(void);

PThandle			ptOpenPODFile							(const PTstr filepath);
PThandle			ptOpenPODServerFile						(const PTstr filepath);
PThandle			ptOpenPODFakeServerFile					(const PTstr filepath);
PThandle			ptReOpenPODStructuredStorageStream		(const PTstr filepath);

PTbool PTAPI		ptCreateFakePOD							(PTstr originalServerFile, PTvoid *bentleyData, PTuint bentleyDataSize, PTstr fakeFile);
PTbool PTAPI		ptSetReleaseClientServerBufferCallBack	(PTuint (*function)(PTvoid *buffer));
PTuint PTAPI		ptSetServerCallBack						(PTuint (*function)(PTvoid *dataSend, PTuint dataSendSize, PTvoid *extDataSend, PTuint extDataSendSize, PTvoid **dataReceive, PTuint *dataReceiveSize));
PTuint PTAPI		ptProcessServerRequest					(PTvoid *dataReceive, PTuint dataReceiveSize, PTvoid **dataSend, PTuint *dataSendSize);
PTuint PTAPI		ptProcessServerRequestClientID			(PTvoid *dataReceive, PTuint dataReceiveSize, PTvoid **dataSend, PTuint *dataSendSize, PTuint64 *clientIDFirst64, PTuint64 *clientIDSecond64);
PTuint PTAPI		ptProcessServerRequestClientID2			(PTvoid *dataReceive, PTuint dataReceiveSize, PTuint64 *clientIDFirst64, PTuint64 *clientIDSecond64, PTuint (*function)(PTvoid *dataSend, PTuint dataSendSize, PTuint64 clientIDFirst64, PTuint64 clientIDSecond64));

PTbool PTAPI		ptSetClientServerLogFile				(PTstr logFile);
PTbool PTAPI		ptSetClientServerSendRetries			(PTuint numRetries, PTuint milliseconds, PTuint millisecondsIncrement);
void   PTAPI		ptGetClientServerSendRetries			(PTuint *numRetries, PTuint *delayMilliseconds, PTuint *incrementMilliseconds);
PTbool PTAPI		ptSetClientStreaming					(PTuint min, PTuint max, PTuint refresh, PTdouble scalar);


PTbool PTAPI		ptSetClientCacheFolder					(PTstr path);
const  PTstr PTAPI	ptGetClientCacheFolder					(void);

PTbool PTAPI		ptEnableClientServerCaching				(PTbool enable);
PTbool PTAPI		ptGetClientServerCachingEnabled			(void);

PTbool PTAPI		ptSetClientServerCacheDataSize			(PTuint size);
PTuint PTAPI		ptGetClientServerCacheDataSize			(void);

PTbool PTAPI		ptSetClientCacheCompletionThreshold		(PTuint size);
PTuint PTAPI		ptGetClientCacheCompletionThreshold		(void);

PTbool PTAPI		ptSetClientServerCacheNameMode			(PTenum mode);
PTenum PTAPI		ptGetClientServerCacheNameMode			(void);

PTuint				testServerCallBack						(PTvoid *dataSend, PTuint dataSendSize, PTvoid *extDataSend, PTuint extDataSendSize, PTvoid **dataReceive, PTuint *dataReceiveSize);

bool				ptSetClientServerURLOverride			(const wchar_t *url);
const wchar_t	*	ptGetClientServerURLOverride			(void);

#ifdef NEEDS_WORK_VORTEX_DGNDB
//PTRMI_INSTANCE_CLIENT_INTERFACE(ptds::DataSourceServerClientInterface, ptds::DataSourceServer)
//PTRMI_INSTANCE_SERVER_INTERFACE(ptds::DataSourceServerServerInterface<ptds::DataSourceServer>, ptds::DataSourceServer)

PTRMI_INSTANCE_SERVER_INTERFACE(ptds::DataSourceServerServerInterface<ptds::DataSource>, ptds::DataSource)

//PTRMI_INHERIT_SERVER_INTERFACE(MyServerInterface<MyObjClass>, MyDerivedObjClass)
#endif


#define			DEFAULT_DEMO_TIMEOUT 300
#define			HAS_EXPIRED ( systime.wYear > expire_year \
					|| (systime.wYear == expire_year &&	(systime.wMonth > expire_month ||  \
					(systime.wMonth == expire_month && systime.wDay > expire_day))))

using namespace pt;
using namespace pointsengine;
using namespace PTRMI;

int				expire_year = 2100;
int				expire_month = 0;
int				expire_day = 0;

int				g_timeOut = -1;
bool			_failed = false;
TimeStamp		g_startTime;

SYSTEMTIME		systime;
FILETIME		ftime;
ULARGE_INTEGER	ftime_cmp, expire_cmp;

bool			_initialized = false;
unsigned char	_nonDemoCode =0;

PTenum			g_autoBaseMethod = PT_AUTO_BASE_DISABLED;

wchar_t			g_lastError[1024];
PTres			g_lastErrorCode = PTV_SUCCESS;

PTenum PTAPI ptGetAutoBaseMethod()
{
	return g_autoBaseMethod;
}
PTvoid PTAPI ptSetAutoBaseMethod( PTenum method )
{
	g_autoBaseMethod = method;
}

//
//

static char teakey [] = { -43, 20, -56, -12, 2, -103, -87, 95, 2, 39, 74, 8, 82, 17, -61, 97 };
static const char* txt_any = "any";
static const PTstr txt_licModuleFailure = L"Not licensed for use with this module";
static const PTstr txt_expired = L"License has expired";
static const PTstr txt_setback = L"Clock setback detected";
static const char* txt_restricted = "restricted";
static const char* txt_demo = "demo";
static const PTstr txt_licDemoTimeFailure = L"Not yet available for demo re-use, please try again";
static std::wstring lastrunKey = L"Software\\Pointools\\Vortex\\LastRun";
static std::wstring lastrunKeySetback = L"Software\\Pointools\\Vortex\\lr";
static bool _hasPSTimeOut = true;

#if NOTUSED
//-----------------------------------------------------------------------------
bool checkSetback()
{
	SYSTEMTIME st;
	GetSystemTime(&st); 
	int day = (st.wYear-1995) * 10000 + (st.wMonth + 8) * 100 + st.wDay+34;

	CRegStdWORD reg(lastrunKeySetback, 0, FALSE, HKEY_CURRENT_USER);
	if (reg.exists())
	{
		int lastday = reg;
		if (lastday && day < (lastday - 1)) return true;
	}
	reg = day;
	reg.write();
	return false;
} 
#endif

//-----------------------------------------------------------------------------
bool checkPreSessionTimeOut()
{
	return true;
	if (g_timeOut < 0 || !_hasPSTimeOut) return false;

	/* check wait period */ 
	CRegStdWORD reg(lastrunKey, 0, FALSE, HKEY_LOCAL_MACHINE);
	
	if (reg.exists())
	{ 
		int slr = reg;
		SYSTEMTIME st;
		GetSystemTime(&st); 
		int s = st.wHour * 3600 + st.wMinute * 60 + st.wSecond;
		int s_remaining = g_timeOut - (s-slr); 
		
		if (s_remaining > 0)
		{
			if (s_remaining < 60)
				swprintf(g_lastError, L"%s, <1min remaining", txt_licDemoTimeFailure);
			else if (s_remaining < 60*60)
				swprintf(g_lastError, L"%s, %imins remaining", txt_licDemoTimeFailure, (s_remaining/60+1));
			else 
			{
				/* 24 hrboundary */ 
				reg = s;
				reg.write();
				g_startTime.tick();
				_hasPSTimeOut = false;
				return true;
			}
			_hasPSTimeOut = true;
			return false;
		}
		
	}
	/* reset timers*/ 
	g_startTime.tick();
	_hasPSTimeOut = false;
	return true;
}

//-------------------------------------------------------------------------------
// Get Last Error
// returns the last error if there was one with a human readable message
//-------------------------------------------------------------------------------
PTstr PTAPI ptGetLastErrorString(void)
{
    switch (g_lastErrorCode)
        {
        case PTV_SUCCESS: return L"No error";

            // File Errors
        case PTV_FILE_NOT_EXIST: return L"File does not exist";
        case PTV_FILE_NOT_ACCESSIBLE: return L"File is not accessible";
        case PTV_FILE_WRONG_TYPE:	return L"Wrong file type";
        case PTV_FILE_COM_ERROR:	return L"COM error";
        case PTV_FILE_USER_CANCELLED:	return L"User cancelled file operation";
        case PTV_FILE_ALREADY_OPENED:	return L"File is already opened";
        case PTV_FILE_NOTHING_TO_WRITE: return L"Nothing to write";
        case PTV_FILE_WRITE_FAILURE:	return L"General write failure";
        case PTV_FILE_READ_FAILURE:	return L"General read failure";
        case PTV_FILE_FAILED_TO_CREATE: return L"Failed to create file";
        case PTV_FILE_INVALID_POD:	return L"POD file invalid or corrupt";
        case PTV_FILE_VERSION_NOT_HANDLED: return L"POD version not handled";

            // Generic Errors
        case PTV_UNKNOWN_ERROR:	return L"Unknown error";
        case PTV_INVALID_PARAMETER:	return L"Invalid parameter";
        case PTV_VALUE_OUT_OF_RANGE:	return L"Value out of range";
        case PTV_INVALID_OPTION:	return L"Invalid option";
        case PTV_INVALID_VALUE_FOR_PARAMETER:	return L"Invalid value for parameter";
        case PTV_VOID_POINTER:		return L"Null pointer used";
        case PTV_NOT_INITIALIZED:	return L"Library not initialised, did you forgot to call ptInitialise ?";
        case PTV_NOT_IMPLEMENTED_IN_VERSION: return L"Function called has not been implemented in this version";
        case PTV_OUT_OF_MEMORY:	return L"Out of memory";

            // Licensing
        case PTV_LICENSE_EXPIRY:	return L"Licence has expired";
        case PTV_LICENSE_MODULE_ERROR: return L"Not licensed for this client module";
        case PTV_LICENSE_CORRUPT: return L"License is corrupted";
        case PTV_NO_LICENSE_FOR_FEATURE: return L"No license available for this feature";
        case PTV_PRODUCT_LICENSE_NA: return L"Pointools product license not available for license share";

            // Viewports
        case PTV_MAXIMUM_VIEWPORTS_USED: return L"Max number of viewports used";
        case PTV_MIN_OPENGL_VERSION_NA: return L"Minimum OpenGL version (1.4) is not available on this machine";

            // IO Block errors
        case PTV_INVALID_BLOCK_VERSION: return L"Invalid block version for data type";

            // Metadata
        case PTV_METATAG_NOT_FOUND: return L"Metatag not found";
        case PTV_METATAG_EMPTY: return L"Metatag is empty";

        default:
            return L"Unrecognised error code";
        }
}
//-----------------------------------------------------------------------------
PTres PTAPI ptGetLastErrorCode()
{
	return g_lastErrorCode; 
}
//-----------------------------------------------------------------------------
void clearLastError()
{
	g_lastErrorCode = PTV_SUCCESS;
}
//-----------------------------------------------------------------------------
int setLastErrorCode( int code )
{
	g_lastErrorCode = code;

	if (code < 0)
	{
		PTTRACEOUT << "ERROR SET CODE " << code;
	}
	return code;
}
//-------------------------------------------------------------------------------
// Version info
//-------------------------------------------------------------------------------
	 
void PTAPI ptGetVersionNum(PTubyte *version)
{
	sscanf(str(VERSION), "%u.%u.%u.%u", (uint*)&(version[0]), (uint*)&(version[1]), (uint*)&(version[2]), (uint*)&(version[3]));
}

//-------------------------------------------------------------------------------
// (not a published API, for internal use only)
//-------------------------------------------------------------------------------

std::wstring version;

const PTstr getShortVersionString()
{
	// this string is used with SELECT and must be of the form xx.xx.xx.xx
	// Note that versions of the for xx.xx.xx.xxx are not accepted by the 
	// Bentley licensing lib.
	if(version.length() == 0)
	{
		PTubyte	v[4];
		
		ptGetVersionNum(v);
		
		wchar_t versionString[12];
		
		swprintf(L"%02d.%02d.%02d.%02d", 12, versionString, v[0], v[1], v[2], v[3]);

		version = versionString;
	}

	return version.c_str();
}

const PTstr PTAPI ptGetVersionString()
{
	return getShortVersionString();
}

//-------------------------------------------------------------------------------
// demo license code and some global stuff
//-------------------------------------------------------------------------------
namespace pod 
{
	extern unsigned char g_demoCode;
	static ptl::BranchHandler *s_ptlHandler = 0;
	PThandle s_lastHandle = 0;
}

//-----------------------------------------------------------------------------
// Extract POD filenames from PTL project file
//-----------------------------------------------------------------------------
bool _extractPODfromPTL(const pt::datatree::Branch *b)
{
	if (!checkPreSessionTimeOut()) return false;

	std::vector<ptds::FilePath *> paths;
	
	int size;
	b->getNode("number_of_PODs", size);

	ptds::FilePath project_path = *ptl::Project::project()->filepath();
	ptds::FilePath::setProjectDirectory(project_path.path());
	ptds::FilePath::makeProjectDirectoryCurrent();

	int i= 0;
	char pod[16];

	pt::String path;

	for (i=0; i<size; i++)
	{
		sprintf(pod, "cloud%d", i);
		datatree::Branch *cbr = b->getBranch(pod);
		if (!cbr) continue;

		cbr->getNode("absolute_path", path);	
		paths.push_back(new ptds::FilePath (path.c_str()));
		
		cbr->getNode("relative_path", path);	
		ptds::FilePath *fp = new ptds::FilePath(path.c_wstr());
		
		fp->setParent(ptl::Project::project()->filepath());
		fp->setRelative();

		paths.push_back(fp);
	}
	/*load files in*/ 
	for (i=0; i<size*2; i+=2)
	{
		ptds::FilePath *afp = paths[i];
		ptds::FilePath *rfp = paths[i+1];

		/*relative path*/ 
		PThandle h = ptOpenPOD(rfp->filename());

		if (!h)
			h= ptOpenPOD(afp->filename());

		delete afp;
		delete rfp;

		pod::s_lastHandle = h;
		//if (h)
		//{
		//	sprintf(pod, "cloud%d", i/2);
		//	datatree::Branch *cbr = b->getBranch(pod);
		//	
		//	if (cbr)
		//	{
		//		for (int c =0; c < sc->numObjects(); c++)
		//		{
		//			pcloud::PointCloud *pc = sc->cloud(c);
		//			
		//			sprintf(pod, "pc_%d", c);
		//			datatree::Branch *pcbr = cbr->getBranch(pod);

		//			if (pcbr)
		//			{
		//				pt::vector3d translation, rotation, scale;

		//				if (pcbr->getNode("translation", translation))
		//					pc->transform().setTranslation(translation);

		//				if (pcbr->getNode("rotation", rotation))
		//					pc->transform().setEulers(rotation);

		//				if (pcbr->getNode("scale", scale))
		//					pc->transform().setScale(scale);

		//				pc->transform().updateMatrix();
		//			}
		//		}
		//	}
		//}
	}
	return true;
}
//-------------------------------------------------------------------------------
// ptInitializeEnvVariables
// intialises settings based on env variables
//-------------------------------------------------------------------------------
PTbool ptInitializeEnvVariables(void)
{
	wchar_t envValue[512];

	if(::GetEnvironmentVariableW(L"PTVORTEX_LOG", envValue, 512) > 0)
	{
		if(wcscmp(envValue, L"None") != 0 && wcscmp(envValue, L"none") != 0)
		{
			PTRMI::Status::setLogFile(envValue);
		}
	}

	if(::GetEnvironmentVariableW(L"PTVORTEX_STREAM_MAX_READS", envValue, 512) > 0)
	{
		if(wcscmp(envValue, L"") != 0)
		{
			unsigned int maxReads = 0;
			swscanf(envValue, L"%d", &maxReads);
			if(maxReads > 0)
			{
				StreamHost::setStreamMaxReadsDefault(maxReads);
			}
		}
	}

	if(::GetEnvironmentVariableW(L"PTVORTEX_STREAM_MIN_VOXEL_BUDGET", envValue, 512) > 0)
	{
		if(wcscmp(envValue, L"") != 0)
		{
			unsigned int minVoxelBudget = 0;
			swscanf(envValue, L"%d", &minVoxelBudget);
			if(minVoxelBudget > 0)
			{
				StreamHost::setStreamMinVoxelBudget(minVoxelBudget);
			}
		}
	}

	if(::GetEnvironmentVariableW(L"PTVORTEX_STREAM_MIN_BUDGET", envValue, 512) > 0)
	{
		if(wcscmp(envValue, L"") != 0)
		{
			unsigned int minBudget = 0;
			swscanf(envValue, L"%d", &minBudget);
			if(minBudget > 0)
			{
				StreamHost::setStreamMinDefault(minBudget);
				StreamHost::setStreamMinDefaultEnforce(true);
			}
		}
	}

	if(::GetEnvironmentVariableW(L"PTVORTEX_STREAM_MAX_BUDGET", envValue, 512) > 0)
	{
		if(wcscmp(envValue, L"") != 0)
		{
			unsigned int maxBudget = 0;
			swscanf(envValue, L"%d", &maxBudget);
			if(maxBudget > 0)
			{
				StreamHost::setStreamMaxDefault(maxBudget);
			}
		}
	}

	if(::GetEnvironmentVariableW(L"PTVORTEX_STREAM_SCALAR", envValue, 512) > 0)
	{
		if(wcscmp(envValue, L"") != 0)
		{
			float streamScalar = 0;
			swscanf(envValue, L"%f", &streamScalar);
			if(streamScalar > 1.0)
			{
				StreamHost::setStreamScalarDefault(streamScalar);
			}
		}
	}


	return true;
}

//-------------------------------------------------------------------------------
// Check the key passed to ptInitialize to see if SELECT licensing should be
// used. For now the only company that does not use SELECT licensing is Siemens.
//-------------------------------------------------------------------------------

const unsigned int COMPANY_NON_SELECT_LICENSES_NUM = 2;
const unsigned int COMPANY_NON_SELECT_LICENSES_MAX_NAME_LENGTH = 64;

auto scrambler = [](std::string toScramble) -> std::string 
    {
    int i = 0;
    std::string out;
    for (char& value : toScramble)
        {
        value += 100 + i*7;
        }
    out[toScramble.size()] = 100 + toScramble.size() * 7;
    return out;
    };

std::string company1 = scrambler("Pointools Ltd");
std::string company2 = scrambler("Siemens");

std::string companyNonSELECTLicensesScrambled[COMPANY_NON_SELECT_LICENSES_NUM] = {{ company1 }, { company2 }};

bool useSELECTLicense(std::string& company, std::string& module, std::string& type, const char* ex, std::string& expires)
{
	unsigned int c;

    auto unscrambler = [](std::string &str, std::string &out)
        {
        char			c;
        unsigned int	t = 0;
        while ((c = str[t] - 100 - (t * 7)) != 0x0 && t < COMPANY_NON_SELECT_LICENSES_MAX_NAME_LENGTH)
            {
            out += c;
            t++;
            }
        };
	for(c = 0; c < COMPANY_NON_SELECT_LICENSES_NUM; c++)
	{
		std::string ptCompanyPlain;

		std::string ptCompany(companyNonSELECTLicensesScrambled[c]);
        unscrambler(ptCompany, ptCompanyPlain);

		if(ptCompanyPlain == company)
			return false;
	}

	return true;

/*
	// Hide "Siemens" so it's not visible in plain text in the DLL
	PTubyte start[] = { 183, 198, 187, 188, 173, 175, 173, 51 };
	int a = 100;
	char name[64] = {0};
	for (int i = 0; i < 8; i++)
		name[i] = start[i]-100+(i*7);

	if (strcmp(company.c_str(), name) == 0)
		return false; // no SELECT license needed

	std::string ptCompanyPlain;
	std::string ptCompany(pointoolsName);
	unScrambleString(ptCompany, ptCompanyPlain);

	if(company == ptCompanyPlain)
	{
		return false;
	}

	return true; // SELECT license needed
*/
}

//-------------------------------------------------------------------------------
// Inititalize API
// intialises engine and loads ramps
//-------------------------------------------------------------------------------
PTbool PTAPI ptInitialize(const PTubyte* licenseData)
{
	PTTRACE_FUNC

	::GetSystemTime(&systime);
	SystemTimeToFileTime(&systime, &ftime);
	ftime_cmp.LowPart = ftime.dwLowDateTime;
	ftime_cmp.HighPart = ftime.dwHighDateTime;

	g_startTime.tick();

	_failed = false;

	if(initializeScenesProject() == PT_FALSE)
		return PT_FALSE;

	if(initializeSceneGraphProject() == PT_FALSE)
		return PT_FALSE;

	ptInitializeEnvVariables();


	std::string company;
	std::string module;
	std::string type;
	const char *ex;
	std::string expires("000000");

	if (!_initialized)
	{
		if(licenseData)
		{
			char plaintxt[255];
			char cyphertxt[255];

			{
				CTEA tea;
				tea.Initialize(teakey, 16);
				for (int i=0; i<128; i++) cyphertxt[i] = licenseData[i] - 128;
				memset(plaintxt, 0, sizeof(plaintxt));
				tea.Decrypt( cyphertxt, plaintxt, 128);
			}
			/* company + "#" + module + "#" + type + "#" + expires; */ 
			company = strtok( plaintxt, "#");
			module = strtok( 0, "#");
			type = strtok( 0, "#");
			ex = strtok( 0, "#");

			if (ex)
			{
				expires = ex;
				const char* secs = strtok(0, "#");
				if (secs) 
				{
					int s = atoi(secs);
					if (s > DEFAULT_DEMO_TIMEOUT) 
						g_timeOut = s;
				}
			}
		
			//std::cout << "company = " << company << std::endl;
			//std::cout << "module = " << module << std::endl;
			//std::cout << "type = " << type << std::endl;
			//std::cout << "expires = " << expires << std::endl;
			bool hasExpiry = true;

			if (strcmp(expires.c_str(), "000000"))
			{
				char year[] = { expires[0], expires[1] };
				char month[] = { expires[2], expires[3] };
				expire_day = atoi(&expires[4]); 
				expire_month =  atoi(month);
				expire_year = 2000 +  atoi(year);
			}
			else hasExpiry = false;

			// hard coded *special expiry* for no expiry
			if (expire_year == 2090 && expire_month == 1 && expire_day == 1)
			{
				hasExpiry = false;
			}

			SYSTEMTIME t;
			ZeroMemory(&t, sizeof(t));
			t.wDay = expire_day; 
			t.wMonth =  expire_month;
			t.wYear = expire_year;

			FILETIME f;
			SystemTimeToFileTime( &t, &f );
			expire_cmp.LowPart = f.dwLowDateTime;
			expire_cmp.HighPart = f.dwHighDateTime;

			/* check module name if NOT set to 'any' */ 
			if (strcmp(module.c_str(), txt_any))
			{
				char mod[64];
				::GetModuleFileNameA(NULL, mod, 64);
				::PathStripPathA(mod);

				if (strnicmp(mod, module.c_str(), 64))
				{
					wcscpy(g_lastError, txt_licModuleFailure);			
					_failed = true;
					setLastErrorCode( PTV_LICENSE_MODULE_ERROR );
				}
			}
			if (hasExpiry && (HAS_EXPIRED && !_failed)) 
			{
                wcscpy(g_lastError, txt_expired);
				_failed = true;
				setLastErrorCode( PTV_LICENSE_EXPIRY );
			}

	// Pip Option
			//if ( hasExpiry && checkSetback() && t.wYear != 2100 && t.wYear != 2050 && !_failed) 
			//{
			//	wcscpy(g_lastError, txt_setback); 
			//	_failed = true;
			//}

			if (!_failed && strcmp(type.c_str(), txt_restricted)) /* only demo datasets can be used */ 
			{
				pod::g_demoCode = _nonDemoCode;		
			}

			if (strcmp(type.c_str(), txt_demo)==0)
			{
				if (g_timeOut < 0) 
				{
					g_timeOut = DEFAULT_DEMO_TIMEOUT; /* default if 5 mins */ 
				}
				_failed = !checkPreSessionTimeOut();
			}
			else g_timeOut = -1;
		}

#ifdef POINTOOLS_BENTLEY_LICENSING
		// Check for special users that do not require a SELECT license and allow these to skip the SELECT license check in startLicenseBentley().
		// Note that any ditribution of Vortex that uses SELECT licensing now requires the bentley licensing DLLs to be distributed with it
		// or startLicenseBentley() will fail when unable to load Bentley.liblib.DLL. 
		if (licenseData == NULL || useSELECTLicense(company, module, type, ex, expires))
		{					
															// Don't support demo mode files with SELECT licensing
			pod::g_demoCode = _nonDemoCode;		

			const PointoolsBentleyLicense::ProductVersion productVersion(getShortVersionString());			
			if(startLicenseBentley(productVersion) == false) 
			{
				setLastErrorCode( PTV_NO_LICENSE_FOR_FEATURE );	
				return PT_FALSE;		
			}
		}
#endif
		PTRMI::initialize();

#ifdef NEEDS_WORK_VORTEX_DGNDB								
        // Initialize server data sources with PTRMI
		PTRMI::getManager().newMetaInterface<ptds::DataSourceServer>(L"DataSourceServer");
#endif

		pointsengine::initializeEngine();
		extern PTvoid _ptInitialiseShaders();
		_ptInitialiseShaders();

		clearLastError();

		pod::s_ptlHandler = new ptl::BranchHandler("Point Clouds", _extractPODfromPTL, ptl::BranchHandler::write_cb());
	}
	_initialized = true;

	if (_failed) 
	{
		return PT_FALSE;
	}


	return PT_TRUE;
}

//-------------------------------------------------------------------------------
// Release
// Releases resources
//-------------------------------------------------------------------------------
PTvoid PTAPI ptRelease()
{
	PTTRACE_FUNC

//	if (_initialized && !_hasPSTimeOut)
	if (_initialized)
	{
		pointsengine::destroy();

		SYSTEMTIME st;
		GetSystemTime(&st);
		int s = st.wHour * 3600 + st.wMinute * 60 + st.wSecond;
		s = static_cast<int>(s - (g_timeOut * 0.5)); /* add on half the timeout period remaining */

		CRegStdWORD reg(lastrunKey, s, FALSE, HKEY_LOCAL_MACHINE);
		reg = (DWORD)s;
		reg.write();

															// Delete all items from PTRMI system
		PTRMI::getManager().deleteAll();
	}
}
//-------------------------------------------------------------------------------
// Is Inititalized
// indicates initialisation state
//-------------------------------------------------------------------------------
PTbool	PTAPI ptIsInitialized()
{
	return _initialized ? PT_TRUE : PT_FALSE;
}
//-------------------------------------------------------------------------------
// Set the working folder for the host application
// Useful for plug-in and resource loading from other than the default folder
//-------------------------------------------------------------------------------
PTvoid PTAPI ptSetWorkingFolder(const PTstr folder)
{
	ptapp::setAppPath(folder);
}
//-------------------------------------------------------------------------------
// Set the working folder for the host application
// Useful for plug-in and resource loading from other than the default folder
//-------------------------------------------------------------------------------
const PTstr PTAPI ptGetWorkingFolder()
{
	return ptapp::apppath();
}
//-------------------------------------------------------------------------------
// Check if file is already opened
//-------------------------------------------------------------------------------
PThandle PTAPI ptIsOpen(const PTstr filepath)
{	
	int numScenes = thePointsScene().size();

	for (int i=0; i<numScenes; i++)
	{
		const pcloud::Scene *sc = thePointsScene()[i];
		if (wcscmp(sc->filepath().path(), filepath) == 0)
			return sc->key();
	}
	return PT_NULL;
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PThandle PTAPI ptOpenPTL(const PTstr filepath)
{
	if (!checkPreSessionTimeOut())
		return 0;

	ptl::Project::project()->setFilepath(filepath);
	ptl::Project::project()->open();

	return pod::s_lastHandle;
}
//-------------------------------------------------------------------------------
// Browse and Open a POD file
//-------------------------------------------------------------------------------
PThandle PTAPI ptBrowseAndOpenPOD()
{
	PTTRACE_FUNC

	if (!checkPreSessionTimeOut())
		return 0;

	if (!ptIsInitialized())
	{
		setLastErrorCode(PTV_NOT_INITIALIZED);
		return 0;
	}
	// get file name first
	wchar_t filename[4096];

	OPENFILENAMEW ofn;       

	/* Initialize OPENFILENAME*/ 
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = ::GetActiveWindow();
	ofn.lpstrFile = filename;
	
	//
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	//
	ofn.lpstrFile[0] = '\0'; 
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrFilter = L"Pointools POD file(*.pod)\0*.pod\0Pointools PTL files(*.ptl)\0*.ptl\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT;

	if (HAS_EXPIRED)
	{
		setLastErrorCode( PTV_LICENSE_EXPIRY );
		return 0;
	}

	if (!GetOpenFileNameW(&ofn))
	{
		DWORD err = CommDlgExtendedError();

		if (err) setLastErrorCode( PTV_FILE_COM_ERROR );
		else setLastErrorCode( PTV_FILE_USER_CANCELLED );
		return PT_NULL;
	}
	
	int len = wcslen(filename);
	if ( wcsicmp(&filename[len-3], L"ptl") == 0 )
	{
		return ptOpenPTL(filename);
	}
	else
	{
		//return ptOpenPOD(filename);

		const wchar_t *strptr = filename;
        wchar_t fpath[260];
        wchar_t fname[260];

        wcscpy(fpath, strptr);
		int pathlen = wcslen(fpath);

		strptr += pathlen + 1;
		int numscenes = 0;

		PThandle lastHandle = 0;

		while (strptr[0] != 0)
		{	
			numscenes++;
			swprintf(fname, L"%s\\%s", fpath, strptr);
			strptr += wcslen(fname) - pathlen;

			PThandle h = ptOpenPOD(fname);
			if (h) lastHandle = h;
		}
		/* single file picked */ 
		if (!numscenes) return ptOpenPOD(filename);
		return lastHandle;
	}
}
bool blockDuplicates = true;

unsigned int sceneErrorToVortexError( int error )
{
	using namespace pcloud;

	switch (error)
	{
	case Scene::Success:				return PTV_SUCCESS;
	case Scene::ReadPntStreamFailure:	return PTV_FILE_READ_FAILURE;
	case Scene::OutOfMemory:			return PTV_OUT_OF_MEMORY;
	case Scene::FileWriteError:			return PTV_FILE_WRITE_FAILURE;
	case Scene::NoPointsInStream:		return PTV_FILE_NOTHING_TO_WRITE;
	case Scene::ReadStreamError:		return PTV_FILE_READ_FAILURE;
	case Scene::CantOpenPODFile:		return PTV_FILE_NOT_ACCESSIBLE;
	case Scene::InvalidPODFile:			return PTV_FILE_INVALID_POD;
	case Scene::PODVersionNotHandled:	return PTV_FILE_VERSION_NOT_HANDLED;
	}
	return PTV_UNKNOWN_ERROR;
}
//-------------------------------------------------------------------------------
// Open a POD file. 
// Todo: Returns a handle to the scene
//-------------------------------------------------------------------------------

bool firstCall = true;

// Pip Option

/*
#define TEST_SS

#ifdef TEST_SS
#ifdef _DEBUG

if(firstCall)
{
	firstCall = false;

	IStorage *	storage = NULL; 
	IStream	 *	stream = NULL;

	HRESULT		error;

	if(FAILED(error = StgOpenStorageEx(L"C:\\Users\\Pip\\Documents\\SS_Test.dgn", STGM_READ | STGM_SHARE_DENY_WRITE, STGFMT_STORAGE, 0, 0, NULL, IID_IStorage, (void **) &storage)))
		return 0;

	//	ptds::listStorage(storage);

	if(FAILED(error = storage->OpenStream(L"result_8825Gatehouse.pod", 0, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &stream)))
		return 0;

	PThandle result = ptOpenPODStructuredStorageStream(filepath, stream);

	firstCall = true;

	return result;
}
#endif
#endif
*/


PThandle ptOpenPODFromDataSource(ptds::DataSourcePtr dataSource, const PTstr filepath) 
{
	PTTRACE_FUNC

	if(dataSource == NULL || dataSource->validHandle() == false)
		return PT_NULL;

	if (!checkPreSessionTimeOut())
		return PT_NULL;

	if (!ptIsInitialized())
	{
		setLastErrorCode(PTV_NOT_INITIALIZED);
		return PT_NULL;
	}

	PThandle sceneHandle = ptIsOpen(filepath);

	if (sceneHandle && blockDuplicates) 
	{
		setLastErrorCode( PTV_FILE_ALREADY_OPENED );	
		return PT_NULL;
	}

	if (HAS_EXPIRED)
	{
		setLastErrorCode( PTV_LICENSE_EXPIRY );	
		return PT_NULL;
	}

	unsigned char	version[4];
	int				error = 0;

	thePointsPager().pause();

	dataSource->readBytes(version,4);
	ptds::dataSourceManager.close(dataSource);

	if (pcloud::PodIO::handlesVersion(version))
	{

		pcloud::Scene *sc = thePointsScene().openScene(filepath, error);

		if (sc) 
		{
			sceneHandle = sc->key();
			pt::Project3D::project().addScene(sc);

			pt::vector3d wcen;
			pt::vector3d cen( pt::Project3D::project().projectBounds().bounds().center() );
			pt::Project3D::project().project2WorldSpace(cen, wcen);

			//	g_autoBaseMethod = PT_AUTO_BASE_CENTER | PT_AUTO_BASE_FIRST_ONLY;

			if ( g_autoBaseMethod )
			{
				if ( !(g_autoBaseMethod & PT_AUTO_BASE_FIRST_ONLY) 
					|| pt::Project3D::project().numObjects() == 1 )
				{
					if ( g_autoBaseMethod & PT_AUTO_BASE_CENTER )
						pt::Project3D::project().setProjectSpaceOrigin(wcen);
					else if ( g_autoBaseMethod & PT_AUTO_BASE_REDUCE )
						pt::Project3D::project().optimizeCoordinateSpace();
				}
				else if (g_autoBaseMethod & PT_AUTO_BASE_FIRST_ONLY)
				{
					PTdouble cb[3];
					ptGetCoordinateBase(cb);
					pt::Project3D::project().setProjectSpaceOrigin( cb );
				}
			}
			clearLastError();
		}
		else
		{
			sceneHandle = 0;
			setLastErrorCode( sceneErrorToVortexError(error) );
		}
	}
	else setLastErrorCode( PTV_FILE_WRONG_TYPE );

	thePointsPager().unpause();


	return sceneHandle;
}

#ifdef NEEDS_WORK_VORTEX_DGNDB
PThandle PTAPI test_ptOpenPODStructuredStorageStream(const PTstr filepath) 
{
	//  Pip Test
	IStorage *	storage = NULL; 
	IStream	 *	stream = NULL;

	HRESULT		error;

	if(FAILED(error = StgOpenStorageEx(L"C:\\Users\\Pip\\Documents\\SS_Storage.imodel", STGM_READ | STGM_SHARE_DENY_WRITE, STGFMT_STORAGE, 0, 0, NULL, IID_IStorage, (void **) &storage)))
		return 0;

	if(FAILED(error = storage->OpenStream(L"result_8825Gatehouse.pod", 0, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &stream)))
		return 0;

	std::wstring url = std::wstring(L"PTSS://result_8825Gatehouse.pod");

	PThandle result = ptOpenPODStructuredStorageStream(url.c_str(), stream);

/*
Sleep(2000);

static bool r = true;

if(r)
{
	ptRemoveAll();

	test_ptOpenPODStructuredStorageStream(filepath);

	r = false;
}
*/

	return result;
}



PThandle ptOpenPODServer(const PTstr clientFilePath, const PTstr serverFilePath, PTRMI::GUID *fileGUID);


PTbool PTAPI ptCreateFakePOD(PTstr originalServerFile, PTvoid *bentleyData, PTuint bentleyDataSize, PTstr fakeFile)
{
	ClientInterfaceExtDataBentley	file;
	PTRMI::Status					status;
															// Set original file path on server
	if((status = file.setOriginalServerFile(URL(originalServerFile))).isFailed())
		return false;
															// Set bentley data
	if((status = file.setBentleyData(reinterpret_cast<DataBuffer::Data *>(bentleyData), bentleyDataSize)).isFailed())
		return false;

	if((status = file.writeFile(fakeFile)).isFailed())
		return false;
															// Return OK
	return true;
}


PTbool PTAPI ptSetClientServerLogFile(PTstr logFile)
{
	PTRMI::Status::setLogFile(logFile);

	return PTRMI::Status::getLogEnabled();
}


PThandle ptOpenPODServer(const PTstr filepath);

void testCalls(ptds::FilePath &path);

PTbool PTAPI ptSetClientStreaming(PTuint min, PTuint max, PTuint refresh, PTdouble scalar)
{

	bool useOverride = false;

	wchar_t envValue[512];
	if(::GetEnvironmentVariableW(L"PTVORTEX_STREAM_OVERRIDE", envValue, 512) > 0)
	{
		if(wcscmp(envValue, L"TRUE") == 0 || wcscmp(envValue, L"true") == 0)
		{
			useOverride = true;
		}
	}

	if(useOverride == false)
	{
															// Return false if the values are not recommended
		if(min < (1024 * 32) || scalar < 1.01)
		{
			return false;
		}

		pointsengine::StreamHost::setStreamMinDefault(min);
		pointsengine::StreamHost::setStreamMaxDefault(max);
		pointsengine::StreamHost::setStreamRefreshDefault(refresh);
        pointsengine::StreamHost::setStreamScalarDefault(static_cast<float>(scalar));
	}
															// Return true
	return true;
}
#endif

void f(void);

namespace PTRMI
{
	void testVariants(void);
}

namespace ptds
{
	void testGraph(void);
}

PThandle PTAPI ptOpenPOD(const PTstr filepath)
{
	PTTRACE_FUNC_P1( filepath )
															// Initialize DataSourceAnalyzer test graphs if required
	ptds::DataSourceAnalyzer::initializeGraphs();

// Pip Option
#ifdef _TEST
//	ptSetClientServerURLOverride(L"PTRI://localhost:8091/");	// Test - Force to TCP/IP
//	ptSetClientServerURLOverride(L"PTCI://localhost:8091/");	// Test - Force to TCP/IP with Caching
#endif

	PThandle h;

#ifdef NEEDS_WORK_VORTEX_DGNDB
															// Try to re-open an existing structured storage stream (needed for ptCreateSceneInstance with structured storage)
	if(h = ptReOpenPODStructuredStorageStream(filepath))
	{
		return h;
	}
															// Try to open POD using fake file server reference
	if(h = ptOpenPODFakeServerFile(filepath))
	{
		return h;
	}
															// Try to open a server file
	if(h = ptOpenPODServerFile(filepath))
	{
		return h;
	}
#endif
															// Try to open POD local file
	if(h = ptOpenPODFile(filepath))
	{
		PTTRACEOUT << " Handle = " << h; 
		return h;
	}
															// Return failed to open
	return 0;
}


PThandle ptOpenPODFile(const PTstr filepath)
{
	PTTRACE_FUNC_P1( filepath )

	ptds::DataSourcePtr dataSource;
																// Open POD normally
	ptds::FilePath fp(filepath);

	if(ptds::DataSourceFile::isValidPath(&fp) == false)
		return PT_NULL;
															// Create File specific data source
	if((dataSource = new ptds::DataSourceFile()) == NULL)
		return PT_NULL;
															// Open file data source for read
	if(dataSource->openForRead(&fp) == false)
		return PT_NULL;
															// Open File DataSource
	return ptOpenPODFromDataSource(dataSource, filepath);
}

#ifdef NEEDS_WORK_VORTEX_DGNDB

PThandle ptReOpenPODStructuredStorageStream(const PTstr filepath)
{
	URL						url(filepath);
	ptds::DataSourcePtr		dataSource;
															// If file path starts with PTSS
	if(url.isProtocol(URL::PT_PTSS))
	{
															// Attempt to re-open currently open structured storage stream
		dataSource = ptds::dataSourceManager.createDataSource(&(ptds::FilePath(url.getString().c_str())));
		if(dataSource == NULL)
			return 0;
															// Attempt to open structured storage
		return ptOpenPODFromDataSource(dataSource, filepath);
	}

															// Return failed
	return 0;
}


PThandle ptOpenPODServerFile(const PTstr filepath)
{
	PThandle	h;
	PTRMI::URL	url(filepath);
															// If URL is a non cached server pipe protocol, it is valid
	if(url.isProtocol(URL::PT_PTRI) || url.isProtocol(URL::PT_PTRE))
	{
		if(h = ptOpenPODServer(NULL, filepath, NULL))
			return h;
	}
															// Not a server pipe protocol, so return NULL
	return 0;
}

//-------------------------------------------------------------------------------
// Open a POD file on a remote server using Ext or TCP based protocols.
// Note: The filepath provided should include the URI specifier if not a standard file.
// PTRE: (External)
// PTRI: (TCP/IP)
// Note: Stream can be READ, WRITE or READWRITE as required.
//-------------------------------------------------------------------------------

std::wstring clientServerURLOverride;

bool ptSetClientServerURLOverride(const wchar_t *url)
{
	if(url == NULL)
		return false;

	clientServerURLOverride = url;

	unsigned int urlLength = clientServerURLOverride.length();

	if(urlLength > 0 && clientServerURLOverride[urlLength - 1] != L'/')
	{
		clientServerURLOverride.append(L"/");
	}

	return true;
}

const wchar_t *ptGetClientServerURLOverride(void)
{
	if(clientServerURLOverride.length() == 0)
		return NULL;

	return clientServerURLOverride.c_str();
}

PThandle ptOpenPODFakeServerFile(const PTstr filepath)
{
	PTRMI::URL							serverFilepath;
	PTRMI::URL							serverFileURL;
	ClientInterfaceExtDataBentley		extData;
															// Attempt to read POD as an Ext fake POD file													
	if(extData.readFile(filepath).isFailed())
	{
		return NULL;
	}
															// Get path of file on the server
	extData.getOriginalServerFile(serverFilepath);
	if(serverFilepath.getLength() == 0)
		return NULL;

															// Create a PTRE:// URL to the file (not real but needed)
	if(ptGetClientServerURLOverride() == NULL)
	{
		if(ptds::DataSourceCache::getCachingEnabled() == false)
		{
															// Use Pointools Remote External data source
			serverFileURL = URL(URL::PT_PTRE) + URL(L"://ExtServer/") + serverFilepath;
		}
		else
		{
															// Use Pointools Remote External Cached data source
			serverFileURL = URL(URL::PT_PTCE) + URL(L"://ExtServer/") + serverFilepath;
		}
	}
	else
	{
															// Use Client Server URL over ride if defined (e.g. for TCP/IP)
		serverFileURL = URL(ptGetClientServerURLOverride())  + serverFilepath;
	}
															// Attempt to open server on the given URL
	return ptOpenPODServer(filepath, serverFileURL.getString().c_str(), extData.getFileGUID());

}

PTuint PTAPI ptSetServerCallBack(PTuint (*function)(PTvoid *dataSend, PTuint dataSendSize, PTvoid *extDataSend, PTuint extDataSendSize, PTvoid **dataReceive, PTuint *dataReceiveSize))
{
	PTRMI::Status status;

	status = PTRMI::getManager().setSendExternalCallFunction(function);

	return static_cast<PTuint>(status.get());
}


PTuint PTAPI ptSetReleaseClientServerBufferCallBack(PTbool (*function)(PTvoid *buffer))
{
	PTRMI::Status status;

	status = PTRMI::getManager().setReleaseExternalBufferFunction(function);

	return static_cast<PTuint>(status.get());
}


void PTAPI ptGetSessionID(PTuint64 &idFirst64, PTuint64 &idSecond64)
{
	const PTRMI::GUID managerGUID = PTRMI::getManager().getManagerGUID();

	idFirst64	= managerGUID.getRawFirst64();
	idSecond64	= managerGUID.getRawSecond64();
}


bool PTAPI ptServerClientLost(PTuint64 clientIDFirst64, PTuint64 clientIDSecond64)
{
	PTRMI::GUID		hostGUID;
	PTRMI::Status	status;
															// Get GUID representing ID (ID can be a GUID)
	hostGUID.setRaw(clientIDFirst64, clientIDSecond64);
															// Return OK if NULL GUID is passed
	if(hostGUID.isValidGUID() == false)
		return true;

#ifdef PTRMI_LOGGING
PTRMI::Status::log(L"ptServerClientLost() called with valid GUID", L"");
#endif

															// Delete all resources associated with the remote client
	PTRMI::getManager().deleteHost(hostGUID);
															// Always return true because deleteHost may fail if already gracefully deleted.
	return true;
}


PTuint PTAPI ptProcessServerRequest(PTvoid *dataReceive, PTuint dataReceiveSize, PTvoid **dataSend, PTuint *dataSendSize)
{
	PTRMI::Status	status;

#ifdef PTRMI_LOGGING
PTRMI::Status::log(L"ptProcessServerRequest", L"");
#endif

	status = PTRMI::getManager().receiveExternalCall(dataReceive, dataReceiveSize, dataSend, dataSendSize);

	return static_cast<PTuint>(status.get());
}


PTuint PTAPI ptProcessServerRequestClientID(PTvoid *dataReceive, PTuint dataReceiveSize, PTvoid **dataSend, PTuint *dataSendSize, PTuint64 *clientIDFirst64, PTuint64 *clientIDSecond64)
{
	PTRMI::Status	status;
	PTRMI::GUID		clientManager;

#ifdef PTRMI_LOGGING
	PTRMI::Status::log(L"ptProcessServerRequestClientID", L"");
#endif

	status = PTRMI::getManager().receiveExternalCall(dataReceive, dataReceiveSize, dataSend, dataSendSize, &clientManager);

	if(clientIDFirst64 != NULL && clientIDSecond64 != NULL)
	{
		*clientIDFirst64	= clientManager.getRawFirst64();
		*clientIDSecond64	= clientManager.getRawSecond64();
	}

	return static_cast<PTuint>(status.get());
}


PTuint PTAPI ptProcessServerRequestClientID2(PTvoid *dataReceive, PTuint dataReceiveSize, PTuint64 *clientIDFirst64, PTuint64 *clientIDSecond64, PTuint (*function)(PTvoid *dataSend, PTuint dataSendSize, PTuint64 clientIDFirst64, PTuint64 clientIDSecond64))
{
	PTRMI::Status	status;
	PTRMI::GUID		clientManager;

#ifdef PTRMI_LOGGING
	PTRMI::Status::log(L"ptProcessServerRequestClientID2", L"");
#endif

	status = PTRMI::getManager().receiveExternalCallCB(dataReceive, dataReceiveSize, &clientManager, function);

	if(clientIDFirst64 != NULL && clientIDSecond64 != NULL)
	{
		*clientIDFirst64	= clientManager.getRawFirst64();
		*clientIDSecond64	= clientManager.getRawSecond64();
	}

	return static_cast<PTuint>(status.get());
}


PTuint testServerCallBack(PTvoid *dataSend, PTuint dataSendSize, PTvoid *extDataSend, PTuint extDataSendSize, PTvoid **dataReceive, PTuint *dataReceiveSize)
{
															// Cross Over Send & Receive
	return ptProcessServerRequest(dataSend, dataSendSize, dataReceive, dataReceiveSize);
}


//PThandle PTAPI ptOpenPODServer(const PTstr url)

void testCalls(ptds::FilePath &path)
{
	bool						opened;
	bool						validHandle;
	bool						moved;
	unsigned char				buffer[1024];
	ptds::DataSource::DataSize	size;
	ptds::DataSource::DataSize	fileSize;


	PTRMI::RemotePtr<ptds::DataSourceServer> remoteObj = PTRMI::getManager().newRemoteObject<ptds::DataSourceServer>(Name(L"DataSourceServer"), Name(L"PTSI://localhost/MyDataSourceServer"));

	//	const	FilePath	*	getFilePath				(void);

					remoteObj->setFilePath(&path);

	opened		=	remoteObj->openForRead(&path, false);
	fileSize	=	remoteObj->getFileSize();
	size		=	remoteObj->readBytes(buffer, 16);
	size		=	remoteObj->readBytes(buffer, 32);
	validHandle	=	remoteObj->validHandle();
	remoteObj->close();

	opened		=	remoteObj->openForRead(&path, false);
	moved		= 	remoteObj->movePointerTo(32);
	moved		= 	remoteObj->movePointerBy(32);
	validHandle	=	remoteObj->validHandle();
	remoteObj->close();


/*
	bool			openForRead				(const FilePath *path, bool create = false) = 0;
	bool			validHandle				(void) = 0;
	bool			closeAndDelete			(void) = 0;
	Size			readBytes				(Data *buffer, Size number_bytes) = 0;
	Size			writeBytes				(const Data *buffer, Size number_bytes) = 0;
	DataSize		getFileSize				(void) = 0;
	bool			movePointerBy			(DataPointer numBytes) = 0;
	bool			movePointerTo			(DataPointer position) = 0;
	DataSourceType	getPathDataSourceType	(const ptds::FilePath *path);
*/

// Will these delete the file ?
/*
	opened = remoteObj->openForReadWrite(&path, false);
	remoteObj->close();


	opened = remoteObj->openForReadWrite(&path, false);
	remoteObj->close();
*/

}

// Set callback
//	ptSetServerCallBack(testServerCallBack);

//ptds::FilePath fp = url;
//testCalls(remoteObj, fp);


PThandle ptOpenPODServer(const PTstr serverFilePath)
{
	PTRMI::Name								className(L"DataSourceServer");
	PTRMI::Name								dataSourceServerName;
	ptds::FilePath							fp;
	ptds::DataSourceServerClientInterface *	clientInterface;

															// Get GUID based name
	dataSourceServerName.generateGUIDName(serverFilePath);
															// Create a new data source on the remote server
	PTRMI::RemotePtr<ptds::DataSourceServer> remoteDataSource = PTRMI::getManager().newRemoteObject<ptds::DataSourceServer>(className, dataSourceServerName);
															// If not created, return NULL
	if((clientInterface = dynamic_cast<ptds::DataSourceServerClientInterface *>(remoteDataSource.getObjectClientInterface())) == NULL)
		return NULL;

	fp = serverFilePath;
															// Open file data source for read
	if(remoteDataSource->openForRead(&fp) == false)
	{
		return PT_NULL;
	}

															// Open File DataSource
	return ptOpenPODFromDataSource(remoteDataSource.getObjectClientInterface(), serverFilePath);
}


PTbool PTAPI ptEnableClientServerCaching(PTbool enable)
{

// Ensure caching is turned off for this release
ptds::DataSourceCache::setCachingEnabled(false);

// Pip Option
// ptds::DataSourceCache::setCachingEnabled(enable);

return true;

// If false was requested, return OK, otherwise return error is caching was requested
if(enable == false)
	return true;

return false;

//  Pip Test - NOTE: This is required for initial Bentley without Caching
//	return ptds::DataSourceCache::setCachingEnabled(enable);
}


PTbool PTAPI ptGetClientServerCachingEnabled(void)
{
	return ptds::DataSourceCache::getCachingEnabled();
}


PTbool PTAPI ptSetClientCacheFolder(PTstr path)
{
	return ptds::DataSourceCache::setCacheFolderPath(path);
}


const PTstr PTAPI ptGetClientCacheFolder(void)
{
	return ptds::DataSourceCache::getCacheFolderPath();
}


PTbool PTAPI ptSetClientServerCacheDataSize(PTuint size)
{
	return ptds::DataSourceCache::setDefaultCachePageSize(size);
}


PTuint PTAPI ptGetClientServerCacheDataSize(void)
{
	return static_cast<PTuint>(ptds::DataSourceCache::getDefaultCachePageSize());
}


PTbool PTAPI ptSetClientCacheCompletionThreshold(PTuint size)
{
	return ptds::DataSourceCache::setDefaultCacheCompletionThreshold(size);
}


PTuint PTAPI ptGetClientCacheCompletionThreshold(void)
{
	return static_cast<PTuint>(ptds::DataSourceCache::getDefaultCacheCompletionThreshold());
}


PTbool PTAPI ptSetClientServerCacheNameMode(PTenum mode)
{
	return ptds::DataSourceCache::setCacheFileNameMode(static_cast<ptds::DataSourceCache::CacheFileNameMode>(mode));
}


PTenum PTAPI ptGetClientServerCacheNameMode(void)
{
	return static_cast<PTenum>(ptds::DataSourceCache::getCacheFileNameMode());
}


PTbool PTAPI ptSetClientServerSendRetries(PTuint numRetries, PTuint delayMilliseconds, PTuint incrementMilliseconds)
{
	bool	ret = true;

	ret &= PTRMI::Stream::setSendMessageNumRetries(numRetries);
	ret &= PTRMI::Stream::setSendMessageDelay(delayMilliseconds);
	ret &= PTRMI::Stream::setSendMessageDelayIncrement(incrementMilliseconds);

	return ret;
}


void PTAPI ptGetClientServerSendRetries(PTuint *numRetries, PTuint *delayMilliseconds, PTuint *incrementMilliseconds)
{
	if(numRetries)
		*numRetries = PTRMI::Stream::getSendMessageNumRetries();

	if(delayMilliseconds)
		*delayMilliseconds = PTRMI::Stream::getSendMessageDelay();

	if(incrementMilliseconds)
		*incrementMilliseconds = PTRMI::Stream::getSendMessageDelayIncrement();
}



PThandle ptOpenPODServer(const PTstr clientFilePath, const PTstr serverFilePath, PTRMI::GUID *fileGUID)
{
	ptds::FilePath		fp;
	ptds::DataSource *	dataSource;

															// If caching is disabled or local fake file or GUID not provided to do caching
	if(ptds::DataSourceCache::getCachingEnabled() == false || clientFilePath == NULL || fileGUID == NULL)
	{
															// Create a standard server connection
		if((dataSource = ptds::dataSourceManager.createRemoteDataSourceServer(clientFilePath, serverFilePath)) == NULL)
			return NULL;
	}
	else
	{
															// Otherwise, set up a cached server connection
		if((dataSource = ptds::dataSourceManager.createDataSourceCache(clientFilePath, serverFilePath, fileGUID)) == NULL)
			return NULL;
	}

	fp = serverFilePath;
															// Open file data source for read
	if(dataSource->openForRead(&fp) == false)
	{
		return PT_NULL;
	}

															// Open File DataSource
	return ptOpenPODFromDataSource(dataSource, serverFilePath);
}



//-------------------------------------------------------------------------------
// Open a POD file using the given COM structured storage stream.
// Note: The filepath provided should include the URI specifier if not a standard file.
// Note: Stream can be READ, WRITE or READWRITE as required.
//-------------------------------------------------------------------------------
PThandle PTAPI ptOpenPODStructuredStorageStream(const PTstr filepath, PTvoid *initStream)
{
#ifdef NEEDS_WORK_VORTEX_DGNDB
	ptds::DataSourcePtr dataSource;

	ptds::FilePath fp(filepath);

	if(ptds::DataSourceStructuredStorage::isValidPath(&fp) == false)
		return PT_NULL;

	if((dataSource = new ptds::DataSourceStructuredStorage(&fp, reinterpret_cast<IStream *>(initStream))) == NULL)
		return PT_NULL;
															// Open File DataSource
	return ptOpenPODFromDataSource(dataSource, filepath);
#else
    return PT_NULL;
#endif
}
#endif

//-----------------------------------------------------------------------------
// scene instancing - loads file in again into different pcloud
//-----------------------------------------------------------------------------
PThandle PTAPI ptCreateSceneInstance( PThandle h )
{
	extern pcloud::Scene*sceneFromHandle( PThandle );

	pcloud::Scene* scene = sceneFromHandle( h );
	if (!scene)
	{
		setLastErrorCode( PTV_INVALID_HANDLE );	
		return PT_NULL;
	}

	blockDuplicates = false;

	ptds::DataSource *dataSource;
															// Create a data source of the required type
	if((dataSource = ptds::dataSourceManager.openForRead(&(scene->filepath()))) == NULL)
	{
		setLastErrorCode( PTV_FILE_READ_FAILURE );	
		return 0;
	}
															// Open using the data source
	PThandle hInstance = ptOpenPODFromDataSource(dataSource, scene->filepath().path());

	if(hInstance == 0)
	{
		setLastErrorCode( PTV_FILE_READ_FAILURE );	
	}


	blockDuplicates = true;

	return hInstance;
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PTvoid PTAPI ptGetCoordinateBase(PTdouble *coordinateBase)
{
	pt::vector3d origin(0,0,0), worigin;
	pt::Project3D::project().project2WorldSpace(origin, worigin);
	worigin.get(coordinateBase);
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PTvoid PTAPI ptSetCoordinateBase(PTdouble *coordinateBase)
{
	if (coordinateBase)
	{
		PTTRACE_FUNC_P3( coordinateBase[0], coordinateBase[1], coordinateBase[2] )

		pt::vector3d base(coordinateBase[0], coordinateBase[1], coordinateBase[2] );
		pt::Project3D::project().setProjectSpaceOrigin( base );
	}
	else setLastErrorCode( PTV_VOID_POINTER );	
}
//-------------------------------------------------------------------------------
// Clear all scenes
//-------------------------------------------------------------------------------
PTvoid PTAPI ptRemoveAll()
{
	PTTRACE_FUNC

	pauseEngine(); 

	pt::Project3D::project().removeAllScenes();
	thePointsScene().clear(); 

	unpauseEngine();

	if (g_autoBaseMethod & PT_AUTO_BASE_FIRST_ONLY)
	{
		PTdouble origin[] = {0,0,0};
		ptSetCoordinateBase( origin );
	}

	PointsPager::completeRequests();
}

