#include <ptlic/PointoolsBentleyLicense.h>
/*
#ifdef PT_POINTOOLS_BENTLEY_LICENSE_API_EXPORTS
	#define PT_POINTOOLS_BENTLEY_LICENSE_API __declspec(dllexport)
#else
	#define PT_POINTOOLS_BENTLEY_LICENSE_API __declspec(dllimport)
#endif

PT_POINTOOLS_BENTLEY_LICENSE_API*/ 


bool startLicenseBentley			(void);
bool getLicenseStatusDescription	(PointoolsBentleyLicense::LicenseDescriptionSet &description);
