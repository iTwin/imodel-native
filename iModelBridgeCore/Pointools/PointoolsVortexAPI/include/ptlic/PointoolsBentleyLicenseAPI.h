#include <ptlic/PointoolsBentleyLicense.h>
/*
#ifdef PT_POINTOOLS_BENTLEY_LICENSE_API_EXPORTS
	#define PT_POINTOOLS_BENTLEY_LICENSE_API EXPORT_ATTRIBUTE
#else
	#define PT_POINTOOLS_BENTLEY_LICENSE_API IMPORT_ATTRIBUTE
#endif

PT_POINTOOLS_BENTLEY_LICENSE_API*/ 


bool startLicenseBentley			(const PointoolsBentleyLicense::ProductVersion& productVersion);
bool getLicenseStatusDescription	(PointoolsBentleyLicense::LicenseDescriptionSet &description);
