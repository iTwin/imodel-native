#include <ptlic/PointoolsBentleyLicense.h>
/*
#ifdef PT_POINTOOLS_BENTLEY_LICENSE_API_EXPORTS
	#define PT_POINTOOLS_BENTLEY_LICENSE_API __declspec(dllexport)
#else
	#define PT_POINTOOLS_BENTLEY_LICENSE_API __declspec(dllimport)
#endif

PT_POINTOOLS_BENTLEY_LICENSE_API*/ 

#define LICENSE_TRIAL_EXPIRED_MODE		-1

#define LICENSE_NORMAL_MODE				0

#define LICENSE_NFR_MODE				1
#define LICENSE_FREE_MODE				2
#define LICENSE_EVALUATION_MODE			3
#define LICENSE_FAILURE_MODE			4

#define JAPANESE_LANG_FEATURE_ID		5

#define LICENSE_SYSTEM_REGISTRY_KEY		"Software\\Bentley\\BentleyPointoolsV8i\\LicenseSystem"

PointoolsBentleyLicense &thePointoolsBentleyLicense(void);

bool startLicenseBentley			(void);
bool getLicenseStatusDescription	(PointoolsBentleyLicense::LicenseDescriptionSet &description);
