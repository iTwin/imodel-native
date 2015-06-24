#include <ptlic/PointoolsBentleyLicense.h>

std::wstring		PointoolsBentleyLicense::activationWizardExe			= L"ActivationWizard.exe";
const static int	HINSTANCE_ERROR_THRESHOLD								= 32;

const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_PRODUCT[]				= L"Product";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_VERSION[]				= L"Version";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_STATUS[]					= L"License Status";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_TYPE[]					= L"License Type";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_ACTIVATION_KEY[]			= L"Activation Key";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_DAYS_UNTIL_DISABLED[]	= L"Days Remaining";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_COMPUTER_ARCHITECTURE[]	= L"Computer Architecture";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_COMPUTER_OS_VERSION[]	= L"OS Version";

const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_USER_NAME[]				= L"User Name";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_ORGANIZATION_NAME[]		= L"Organization Name";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_ACTIVATED_COUNTRY[]		= L"Activated Country";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_SITE_ID[]				= L"Site ID";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_SERVER_NAME[]			= L"Server Name";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_SERVER_SERIAL_NUMBER[]	= L"Server Serial Number";

const wchar_t POINTOOLS_BENTLEY_LICENSE_STATUS_DESC_UNKNOWN[]				= L"Unknown";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_STATUS_ACTIVATED[]				= L"Activated";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_STATUS_TRIAL[]					= L"Trial";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_STATUS_OFFLINE[]				= L"Offline";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_STATUS_LIMITED[]				= L"Limited";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_STATUS_ERROR[]					= L"Licensing Error";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_STATUS_EXPIRED[]				= L"Expired";

const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_TYPE_UNKNOWN[]					= L"Unknown";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_TYPE_COMMERCIAL[]				= L"Commercial";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_TYPE_ACADEMIC[]				= L"Licensed For Academic Use Only";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_TYPE_HOME_USE[]				= L"Licensed For Home Use";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_TYPE_NON_COMMERCIAL[]			= L"Not For Commercial Use";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_TYPE_FUTURE_TYPE_1[]			= L"Future Type 1";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_TYPE_FUTURE_TYPE_2[]			= L"Future Type 2";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_TYPE_FUTURE_TYPE_ERROR[]		= L"Error";

const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_ARCHITECTURE_X86[]				= L"X86";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_ARCHITECTURE_X64[]				= L"X64";
const wchar_t POINTOOLS_BENTLEY_LICENSE_DESC_ARCHITEXTURE_UNKNOWN[]			= L"Unknown";


PointoolsBentleyLicense::PointoolsBentleyLicense(void)
{
	clear();
}


PointoolsBentleyLicense::~PointoolsBentleyLicense(void)
{

}


// ************************************************************************************************************
//	PointoolsBentleyLicense::startLicenseDesktop()
// 
//	Return values and actions to be taken by caller:
//
//	LICENSE_STATUS_Ok		:	Run application
//	LICENSE_STATUS_Trial	:	Run application
//	LICENSE_STATUS_Offline	:	Run application
//	LICENSE_STATUS_Crippled	:	Run application in limited mode for limited time
//	LICENSE_STATUS_Error	:	Run application in limited mode for limited time
//	LICENSE_STATUS_Expired	:	Run application in limited mode for limited time or quit depending on policy
//	
// ************************************************************************************************************

PointoolsBentleyLicense::LicenseStatus PointoolsBentleyLicense::startLicenseDesktop(void)
{
	UINT32			licenseConfigured;
	LICENSE_STATUS	licenseStatus;

	if(getInitalized() == false)
		return LICENSE_STATUS_Error;
	
																	// Check if product is activated
	licenseConfigured	= getLicenseConfigured();

	__try 
	{
																	// Attempt to start license
		licenseStatus	= LicenseClient_StartDesktopLicense3(getProductID(), getProductVersionStr(), LICCLIENT_DURATION_Desktop, getProductFeaturesStr(), NULL);
	}	
	__except (EXCEPTION_EXECUTE_HANDLER) 
	{
																	// the Bentley licensing DLLs were not able to be loaded therefore licensing cannot be checked 
		return LICENSE_STATUS_Error;
	}
																	// Is this is an iWare license don't check the status returned from LicenseClient_StartDesktopLicense3()
																	// (See http://inside/bsw/Community/selectlicensingsdk/SELECT%20Licensing%20SDK/Recording%20Usage%20For%20Free%20Products.aspx)	
// Don't show any messages when licensing the Vortex DLL																	
/*	if(licenseConfigured == 0 && licenseStatus != LICENSE_STATUS_Trial)
	{
																	// If not, run activation wizard
		if(runActivationWizard() != Status_OK)
		{
																	// If an error occurred, return error
			return LICENSE_STATUS_Error;
		}

																	// Check if product is activated
		licenseConfigured	= getLicenseConfigured();
																	// Attempt to start license
		licenseStatus		= LicenseClient_StartDesktopLicense3(getProductID(), getProductVersionStr(), LICCLIENT_DURATION_Desktop, getProductFeaturesStr(), NULL);
																	// If not in trial mode, exit
		if(licenseConfigured == 0 && licenseStatus != LICENSE_STATUS_Trial)
		{
			exit(0);
		}
	}


	switch(licenseStatus)
	{
																	// If in 'Trial' mode (not activated)
	case LICENSE_STATUS_Trial:
																	//  OR 'Disconnected' (activated but disconnected from license server)
	case LICENSE_STATUS_Offline:
	{
		UINT32		daysLeft = 0;
		wchar_t		message[POINTOOLSBENTLEYLICENSE_MAX_MESSAGE_LENGTH];

																	// Get days until disabled in either trial or offline status
		int status = LicenseClient_GetDaysUntilDisabled3(&daysLeft, getProductID(), getProductFeaturesStr());

		swprintf_s(message, POINTOOLSBENTLEYLICENSE_MAX_MESSAGE_LENGTH, L"Days Left: %d", daysLeft);
																	// Show daily message. Note that a message can only display once per day
		displayDailyMessage(message);
																	// If in Trial mode and Trial has expired, return Expired. This is just a safety net for the internal XM logic
		if(licenseStatus == LICENSE_STATUS_Trial && daysLeft == 0)
		{
			return LICENSE_STATUS_Expired;
		}

		break;
	}      
																	// If activated and license has been obtained, return OK
	case LICENSE_STATUS_Ok:
																	// If product has fully expired, return expired
	case LICENSE_STATUS_Expired:
	{
		wchar_t		message[POINTOOLSBENTLEYLICENSE_MAX_MESSAGE_LENGTH];

		swprintf_s(message, POINTOOLSBENTLEYLICENSE_MAX_MESSAGE_LENGTH, L"License Expired.");
																	// Show daily message. Note that a message can only display once per day
		displayDailyMessage(message);

		break;
	}
																	// If beyond 30 day expiry, run in limited mode
	case LICENSE_STATUS_Crippled:
	case LICENSE_STATUS_Error:
	default:
																	// Show daily message. Note that a message can only display once per day
		displayDailyMessage(L"Running Limited Mode");

		break;
	}
*/																	// Return the license status
	return licenseStatus;	
}


PointoolsBentleyLicense::Status PointoolsBentleyLicense::initialize(const ProductName &name, ProductID id, const ProductVersion &version, const ProductFeatures &features, const ProductPath &toolsPath)
{
	if(getInitalized())
	{
		return Status_Already_Initialized;
	}

	switch(CoInitialize(NULL))
	{
	case S_OK:
		break;

	case S_FALSE:
	case RPC_E_CHANGED_MODE:
	default:
		return Status_Failed;
	}
																	// Validate parameters. Version must be xx.xx.xx.xx with eleven characters.
	if(name.length() == 0 || version.length() != 11)
	{
		return Status_Bad_Parameter;
	}
																	// Enable develop time debug logs
#ifdef _DEBUG
	LicenseClient_SetErrorLogPath(L"C:\\BentleyLicenseLog.txt");
	LicenseClient_SetLoggerOutputLevel(-5);
#endif
																	// Set product name string (can be arbitrary)
	setProductName(name);
																	// Set path to executable licensing tools such as the Activation Wizard
	setToolsPath(toolsPath);
																	// Set product ID
	setProductID(id);
																	// Set product version string in xx.xx.xx.xx format
	setProductVersion(version);
																	// Set feature string
	setProductFeatures(features);					
																	// Set initialized
	setInitialized(true);
																	// Return OK
	return Status_OK;
}


PointoolsBentleyLicense::Status PointoolsBentleyLicense::shutDown(void)
{
	if(getInitalized() == false)
		return Status_Not_Initialized;

	CoUninitialize();

	return Status_OK;
}


void PointoolsBentleyLicense::clear(void)
{
	setInitialized(false);

	setProductName(pt::String(DEFAULT_PRODUCT_NAME));

	setProductID(DEFAULT_PRODUCT_ID);

	setProductVersion(pt::String(DEFAULT_PRODUCT_VERSION));

	setProductFeatures(pt::String(DEFAULT_PRODUCT_FEATURES));	
}


void PointoolsBentleyLicense::setProductID(ProductID id)
{
	productID = id;
}


PointoolsBentleyLicense::ProductID PointoolsBentleyLicense::getProductID(void)
{
	return productID;
}


void PointoolsBentleyLicense::setProductVersion(const ProductVersion &version)
{
	productVersion = version;
}


const PointoolsBentleyLicense::ProductVersion &PointoolsBentleyLicense::getProductVersion(void)
{
	return productVersion;
}


const wchar_t *PointoolsBentleyLicense::getProductVersionStr(void)
{
	return productVersion.c_wstr();
}


void PointoolsBentleyLicense::setProductFeatures(const ProductFeatures &features)
{
	productFeatures = features;
}


const PointoolsBentleyLicense::ProductFeatures &PointoolsBentleyLicense::getProductFeatures(void)
{
	return productFeatures;
}

const wchar_t *PointoolsBentleyLicense::getProductFeaturesStr(void)
{
	if(getProductFeatures().length() > 0)
	{
		return getProductFeatures().c_wstr();
	}

	return NULL;
}


UINT32 PointoolsBentleyLicense::getLicenseConfigured(void)
{
	__try 
	{
		return LicenseClient_IsConfigured(getProductID(), getProductVersionStr(), getProductFeaturesStr());
	}	
	__except (EXCEPTION_EXECUTE_HANDLER) 
	{
		return 0;
	}
}


PointoolsBentleyLicense::LicenseStatus PointoolsBentleyLicense::getLicenseStatus(void)
{
	__try 
	{
		return LicenseClient_GetStatus3(getProductID(), getProductFeaturesStr());
	}
	__except (EXCEPTION_EXECUTE_HANDLER) 
	{
		return LICENSE_STATUS_Error;
	}
}


PointoolsBentleyLicense::LicenseType PointoolsBentleyLicense::getLicenseType(void)
{
	__try 
	{
		return LicenseClient_GetLicenseType(getProductID(), getProductVersionStr(), getProductFeaturesStr());
	}
	__except (EXCEPTION_EXECUTE_HANDLER) 
	{
		return LICENSE_TYPE_Error;
	}
}


int PointoolsBentleyLicense::getDaysUntilDisabled(void)
{
	UINT32	days;

	__try 
	{
		if(LicenseClient_GetDaysUntilDisabled3(&days, getProductID(), getProductFeaturesStr()) != BENTLEY_LICENSE_SUCCESS)
		{
			days = -1;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) 
	{
		days = -1;
	}

	return days;
}


bool PointoolsBentleyLicense::getUserName(pt::String &value)
{
	wchar_t	buffer[POINTOOLS_BENTLEY_DEFAULT_BUFFER_SIZE];

	StatusInt result;

	__try 
	{
		if((result = LicenseClient_GetUserName(buffer, POINTOOLS_BENTLEY_DEFAULT_BUFFER_SIZE, getProductID(), getProductVersionStr(), getProductFeaturesStr())) == BENTLEY_LICENSE_SUCCESS)
		{
			value = buffer;

			return true;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) { }

	value = L"";

	return false;
}


bool PointoolsBentleyLicense::getOrganizationName(pt::String &value)
{
	wchar_t	buffer[POINTOOLS_BENTLEY_DEFAULT_BUFFER_SIZE];

	StatusInt result;

	__try 
	{
		if((result = LicenseClient_GetOrganizationName(buffer, POINTOOLS_BENTLEY_DEFAULT_BUFFER_SIZE, getProductID(), getProductVersionStr(), getProductFeaturesStr())) == BENTLEY_LICENSE_SUCCESS)
		{
			value = buffer;

			return true;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) { }

	value = L"";

	return false;}


bool PointoolsBentleyLicense::getActivatedCountry(pt::String &value)
{
	wchar_t	buffer[POINTOOLS_BENTLEY_DEFAULT_BUFFER_SIZE];

	StatusInt result;

	__try 
	{
		if((result = LicenseClient_GetActivatedCountry(buffer, POINTOOLS_BENTLEY_DEFAULT_BUFFER_SIZE, getProductID())) == BENTLEY_LICENSE_SUCCESS)
		{
			value = buffer;

			return true;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) { }

	value = L"";

	return false;
}


bool PointoolsBentleyLicense::getSiteID(pt::String &value)
{
	wchar_t	buffer[POINTOOLS_BENTLEY_DEFAULT_BUFFER_SIZE];

	StatusInt result;

	__try 
	{
		if((result = LicenseClient_GetSiteID(buffer, POINTOOLS_BENTLEY_DEFAULT_BUFFER_SIZE, getProductID(), getProductVersionStr(), getProductFeaturesStr())) == BENTLEY_LICENSE_SUCCESS)
		{
			value = buffer;

			return true;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) { }

	value = L"";

	return false;
}


bool PointoolsBentleyLicense::getServerName(pt::String &value)
{
	wchar_t	buffer[POINTOOLS_BENTLEY_DEFAULT_BUFFER_SIZE];

	StatusInt result;

	__try 
	{
		if((result = LicenseClient_GetServerName(buffer, POINTOOLS_BENTLEY_DEFAULT_BUFFER_SIZE, getProductID(), getProductVersionStr(), getProductFeaturesStr())) == BENTLEY_LICENSE_SUCCESS)
		{
			value = buffer;

			return true;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) { }

	value = L"";

	return false;
}


bool PointoolsBentleyLicense::getServerSerialNumber(pt::String &value)
{
	wchar_t	buffer[POINTOOLS_BENTLEY_DEFAULT_BUFFER_SIZE];

	StatusInt result;
	
	__try 
	{
		if((result = LicenseClient_GetServerSerialNumber(buffer, POINTOOLS_BENTLEY_DEFAULT_BUFFER_SIZE, getProductID())) == BENTLEY_LICENSE_SUCCESS)
		{
			value = buffer;

			return true;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) { }

	value = L"";

	return false;
}


PointoolsBentleyLicense::Architecture PointoolsBentleyLicense::getComputerArchitecture(void)
{
	__try 
	{
		switch(LicenseClient_GetComputerArchitecture())
		{
		case ARCH_X86:
			return Architecture_X86;

		case ARCH_X64:
			return Architecture_X64;

		default:

			return Architecture_Unknown;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) 
	{
		return Architecture_Unknown;
	}
}


bool PointoolsBentleyLicense::getOSVersion(pt::String &value)
{
	wchar_t	buffer[POINTOOLS_BENTLEY_DEFAULT_BUFFER_SIZE];

	__try 
	{
		if(LicenseClient_GetOSVersion(buffer, POINTOOLS_BENTLEY_DEFAULT_BUFFER_SIZE) == BENTLEY_LICENSE_SUCCESS)
		{
			value = buffer;
			return true;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) { }

	return false;
}


void PointoolsBentleyLicense::setToolsPath(const ProductPath &path)
{
	toolsPath = path;
}


const PointoolsBentleyLicense::ProductPath &PointoolsBentleyLicense::getToolsPath(void)
{
	return toolsPath;
}


PointoolsBentleyLicense::Status PointoolsBentleyLicense::runExternalExe(const pt::String &pathEXE, bool wait)
{
	SHELLEXECUTEINFOW executeInfo;

	if(getInitalized() == false)
		return Status_Not_Initialized;

	ZeroMemory(&executeInfo, sizeof(executeInfo));

	executeInfo.cbSize	= sizeof(executeInfo);
	executeInfo.fMask	= SEE_MASK_NOCLOSEPROCESS;
	executeInfo.lpFile	= pathEXE.c_wstr();
	executeInfo.nShow	= SW_SHOWNORMAL;
															// Execute external activation wizard
	BOOL res = ::ShellExecuteExW(&executeInfo);	
															// If failed, return failure
	if(res == false || executeInfo.hProcess == INVALID_HANDLE_VALUE)
	{
		return Status_External_Execution_Failed;
	}
															// Wait for dialog to complete
	if(wait)
	{
		if(WaitForSingleObject(executeInfo.hProcess, 1000 * 60 * 60 * 48) != WAIT_OBJECT_0)
		{
			return Status_External_Execution_Failed;
		}
	}
															// Return EXE ran
	return Status_OK;
}


PointoolsBentleyLicense::Status PointoolsBentleyLicense::runActivationWizard(void)
{
	std::wstring	wizardPath;

	if(getInitalized() == false)
		return Status_Not_Initialized;

	wizardPath = getToolsPath().c_wstr();
	wizardPath += getActivationWizardExe();
															// Construct path to activation wizard executable
	pt::String pathEXE(wizardPath.c_str());
															// Return Wizard executed (but not necessarily activated)
	return runExternalExe(pathEXE, true);
}


const std::wstring &PointoolsBentleyLicense::getActivationWizardExe(void)
{
	return activationWizardExe;
}


void PointoolsBentleyLicense::setProductName(const ProductPath &name)
{
	productName = name;
}


const PointoolsBentleyLicense::ProductName &PointoolsBentleyLicense::getProductName(void)
{
	return productName;
}


const wchar_t *PointoolsBentleyLicense::getProductNameStr(void)
{
	return productName.c_wstr();
}

/*
void PointoolsBentleyLicense::displayDailyMessage(const wchar_t *message)
{
	StatusInt s;

	if(getInitalized() == false)
		return;

	if(message)
	{
															// Dispay message. This will only display a maximum of once per day
		s = LicenseClient_DisplayStatusMessage(NULL, getProductID(), getProductFeaturesStr(), getProductNameStr(), message, MSG_LANGUAGE_English);
	}
}
*/
unsigned int PointoolsBentleyLicense::getLicenseDescription(LicenseDescriptionSet &description)
{
	pt::String				value;

	if(getInitalized() == false)
		return 0;

	description.push_back(LicenseDescriptionItem(pt::String(POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_PRODUCT), getProductName()));

	description.push_back(LicenseDescriptionItem(pt::String(POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_VERSION), getProductVersion()));

	getLicenseDescStatus(value);
	description.push_back(LicenseDescriptionItem(pt::String(POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_STATUS), value));

// 	getLicenseDescActivationKey(value);
// 	description.push_back(LicenseDescriptionItem(POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_ACTIVATION_KEY, value));

	if(getLicenseDescDaysUntilDisabled(value))
	{
		description.push_back(LicenseDescriptionItem(pt::String(POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_DAYS_UNTIL_DISABLED), value));
	}

	getLicenseDescType(value);
	description.push_back(LicenseDescriptionItem(pt::String(POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_TYPE), value));

	getUserName(value);
	description.push_back(LicenseDescriptionItem(pt::String(POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_USER_NAME), value));

	getOrganizationName(value);
	description.push_back(LicenseDescriptionItem(pt::String(POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_ORGANIZATION_NAME), value));

	getActivatedCountry(value);
	description.push_back(LicenseDescriptionItem(pt::String(POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_ACTIVATED_COUNTRY), value));

	getSiteID(value);
	description.push_back(LicenseDescriptionItem(pt::String(POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_SITE_ID), value));

	getServerName(value);
	description.push_back(LicenseDescriptionItem(pt::String(POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_SERVER_NAME), value));

	getServerSerialNumber(value);
	description.push_back(LicenseDescriptionItem(pt::String(POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_SERVER_SERIAL_NUMBER), value));
	
	getLicenseDescComputerArchitecture(value);
	description.push_back(LicenseDescriptionItem(pt::String(POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_COMPUTER_ARCHITECTURE), value));

	getOSVersion(value);
	description.push_back(LicenseDescriptionItem(pt::String(POINTOOLS_BENTLEY_LICENSE_DESC_FIELD_COMPUTER_OS_VERSION), value));
															// Return the number of items in the description
	return static_cast<unsigned int>(description.size());
}


bool PointoolsBentleyLicense::getLicenseDescStatus(pt::String &value)
{
	if(getInitalized() == false)
		return false;

	LicenseStatus licenseStatus = getLicenseStatus();

	value = L"";

	switch(licenseStatus)
	{
	case LICENSE_STATUS_Ok:
		value = POINTOOLS_BENTLEY_LICENSE_DESC_STATUS_ACTIVATED;
		break;
															// If Offline (and not expired), run normally
	case LICENSE_STATUS_Offline:
		value = POINTOOLS_BENTLEY_LICENSE_DESC_STATUS_OFFLINE;
		break;
															// If Trial mode (and not expired), run normally
	case LICENSE_STATUS_Trial:
		value = POINTOOLS_BENTLEY_LICENSE_DESC_STATUS_TRIAL;
		break;
															// If running in limited mode, run in limited mode
	case LICENSE_STATUS_Crippled:
		value = POINTOOLS_BENTLEY_LICENSE_DESC_STATUS_LIMITED;
		break;
															// If a licensing error ocurred, run in limited mode
	case LICENSE_STATUS_Error:
		value = POINTOOLS_BENTLEY_LICENSE_DESC_STATUS_ERROR;
		break;
															// License has expired
	case LICENSE_STATUS_Expired:
		value = POINTOOLS_BENTLEY_LICENSE_DESC_STATUS_EXPIRED;
		break;

	default: ;
	}

	return (value.length() > 0);
}


bool PointoolsBentleyLicense::getLicenseDescType(pt::String &value)
{
	LicenseType	licenseType;

	if(getInitalized() == false)
		return false;

	licenseType = getLicenseType();

	value = L"";

	switch(licenseType)
	{
	case LICENSE_TYPE_Unknown:
		value = POINTOOLS_BENTLEY_LICENSE_DESC_TYPE_UNKNOWN;
		break;

	case LICENSE_TYPE_Comercial:
		value = POINTOOLS_BENTLEY_LICENSE_DESC_TYPE_COMMERCIAL;
		break;

	case LICENSE_TYPE_Academic:
		value = POINTOOLS_BENTLEY_LICENSE_DESC_TYPE_ACADEMIC;
		break;

	case LICENSE_TYPE_HomeUse:
		value = POINTOOLS_BENTLEY_LICENSE_DESC_TYPE_HOME_USE;
		break;

	case LICENSE_TYPE_NonCommercial:
		value = POINTOOLS_BENTLEY_LICENSE_DESC_TYPE_NON_COMMERCIAL;
		break;

	case LICENSE_TYPE_FutureType1:
		value = POINTOOLS_BENTLEY_LICENSE_DESC_TYPE_FUTURE_TYPE_1;
		break;

	case LICENSE_TYPE_FutureType2:
		value = POINTOOLS_BENTLEY_LICENSE_DESC_TYPE_FUTURE_TYPE_2;
		break;

	case LICENSE_TYPE_Error:
	default:
		value = POINTOOLS_BENTLEY_LICENSE_DESC_TYPE_FUTURE_TYPE_ERROR;
	}

	return (value.length() > 0);
}


bool PointoolsBentleyLicense::getLicenseDescActivationKey(pt::String &value)
{
	if(getInitalized() == false)
		return false;

	wchar_t	buffer[POINTOOLS_BENTLEY_DEFAULT_BUFFER_SIZE];

	value = L"";

	__try
	{
		if(LicenseClient_GetDefaultActivationKey(buffer, POINTOOLS_BENTLEY_DEFAULT_BUFFER_SIZE) == BENTLEY_LICENSE_SUCCESS)
		{
			value = buffer;
			return true;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) { }

	return false;
}

bool PointoolsBentleyLicense::getLicenseDescDaysUntilDisabled(pt::String &value)
{
	if(getInitalized() == false)
		return false;

	if(getLicenseStatus() != LICENSE_STATUS_Trial && getLicenseStatus() != LICENSE_STATUS_Offline)
	{
		return false;
	}

	value = L"";

	int days = getDaysUntilDisabled();

	if(days >= 0)
	{
		wchar_t	buffer[POINTOOLS_BENTLEY_DEFAULT_BUFFER_SIZE];

		swprintf_s(&buffer[0], POINTOOLS_BENTLEY_DEFAULT_BUFFER_SIZE, L"%d", days);

		value = buffer;

		return true;
	}

	return false;
}


bool PointoolsBentleyLicense::getLicenseDescComputerArchitecture(pt::String &value)
{
	if(getInitalized() == false)
		return false;

	switch(getComputerArchitecture())
	{
	case Architecture_X86:
		value = POINTOOLS_BENTLEY_LICENSE_DESC_ARCHITECTURE_X86;
		break;

	case Architecture_X64:
		value = POINTOOLS_BENTLEY_LICENSE_DESC_ARCHITECTURE_X64;
		break;

	case Architecture_Unknown:
	default:
		value = POINTOOLS_BENTLEY_LICENSE_DESC_ARCHITEXTURE_UNKNOWN;
	}

	return true;
}

void PointoolsBentleyLicense::setInitialized(bool init)
{
	initialized = init;
}


bool PointoolsBentleyLicense::getInitalized(void)
{
	return initialized;
}


