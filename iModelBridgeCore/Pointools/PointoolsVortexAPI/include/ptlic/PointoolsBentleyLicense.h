
#include <LicenseClient/LicClient.h>
#include <string.h>
#include <pt/ptstring.h>

#define DEFAULT_PRODUCT_NAME						L"Bentley"
#define DEFAULT_PRODUCT_ID							1
#define DEFAULT_PRODUCT_VERSION						L"00.00.00.00"
#define DEFAULT_PRODUCT_FEATURES					L""

#define POINTOOLS_BENTLEY_LICENSE_PATH_WIZARD_EXE	"ActivationWizard.exe";

#define POINTOOLSBENTLEYLICENSE_MAX_MESSAGE_LENGTH	64
#define POINTOOLS_BENTLEY_DEFAULT_BUFFER_SIZE		256

#define BENTLEY_LICENSE_SUCCESS						0


class PointoolsBentleyLicense
{

public:

	typedef LICENSE_STATUS							LicenseStatus;
	typedef LICENSE_TYPE							LicenseType;

	typedef pt::String								ProductPath;

	typedef INT32									ProductID;
	typedef pt::String								ProductName;
	typedef	pt::String								ProductVersion;
	typedef	pt::String								ProductFeatures;

	typedef std::pair<pt::String, pt::String>		LicenseDescriptionItem;
	typedef std::vector<LicenseDescriptionItem>		LicenseDescriptionSet;

public:

	enum Status
	{
		Status_OK,
		Status_Failed,

		Status_Bad_Parameter,
		Status_External_Execution_Failed,

		Status_Already_Initialized,
		Status_Not_Initialized,

		Status_NULL,
	};

	enum Architecture
	{
		Architecture_X86,
		Architecture_X64,

		Architecture_Unknown
	};

	enum LicenseIsDesktopOrIware
	{
		License_Is_Deskop,
		License_Is_IWare
	};

protected:

	static std::wstring		activationWizardExe;

protected:

	bool					initialized;

	ProductPath				toolsPath;

	ProductName				productName;
	ProductID				productID;
	ProductVersion			productVersion;
	ProductFeatures			productFeatures;

	LicenseStatus			licenseStatus;

	LicenseIsDesktopOrIware	licenseIsDesktopOrIware;

protected:

	void					clear									(void);

	void					setProductName							(const ProductName &name);
	void					setProductID							(ProductID id);
	void					setProductVersion						(const ProductVersion &version);
	void					setProductFeatures						(const ProductFeatures &features);

	void					setToolsPath							(const ProductPath &path);

	void					setLicenseIsDesktopOrIWare				(LicenseIsDesktopOrIware desktopOrIWare);

	Status					runExternalExe							(const pt::String &pathEXE, bool wait = true);
	Status					runActivationWizard						(void);

	const std::wstring &	getActivationWizardExe					(void);

	void					displayDailyMessage						(const wchar_t *message);

	void					setInitialized							(bool init);

	void					setLicenseStatus						(LicenseStatus status);

public:
							PointoolsBentleyLicense					(void);
						   ~PointoolsBentleyLicense					(void);

	Status					initialize								(const ProductName &name, ProductID product, const ProductVersion &version, const ProductFeatures &features, const ProductPath &toolPath, LicenseIsDesktopOrIware desktopOrIWare);
	Status					shutDown								(void);

	bool					getInitalized							(void);

	LicenseStatus			startLicenseDesktop						(void);

	UINT32					getLicenseConfigured					(void);
	LicenseStatus			getLicenseStatus						(void);
	LicenseType				getLicenseType							(void);
	int						getDaysUntilDisabled					(void);
	bool					getUserName								(pt::String &value);
	bool					getOrganizationName						(pt::String &value);
	bool					getActivatedCountry						(pt::String &value);
	bool					getSiteID								(pt::String &value);
	bool					getServerName							(pt::String &value);
	bool					getServerSerialNumber					(pt::String &value);
	Architecture			getComputerArchitecture					(void);
	bool					getOSVersion							(pt::String &value);

	const ProductName	  &	getProductName							(void);
	const wchar_t		  *	getProductNameStr						(void);
	ProductID				getProductID							(void);
	const ProductVersion  & getProductVersion						(void);
	const wchar_t		  *	getProductVersionStr					(void);
	const ProductFeatures & getProductFeatures						(void);
	const wchar_t		  *	getProductFeaturesStr					(void);

	const ProductPath	  &	getToolsPath							(void);

	LicenseIsDesktopOrIware getLicenseIsDesktopOrIWare				(void);

	unsigned int			getLicenseDescription					(LicenseDescriptionSet &description);

	bool					getLicenseDescStatus					(pt::String &value);
	bool					getLicenseDescType						(pt::String &value);
	bool					getLicenseDescActivationKey				(pt::String &value);
	bool					getLicenseDescDaysUntilDisabled			(pt::String &value);
	bool					getLicenseDescComputerArchitecture		(pt::String &value);
};


