#include <ptlic/PointoolsBentleyLicenseAPI.h>

PointoolsBentleyLicense	pointoolsBentleyLicense;

#define	BENTLEY_POINTOOLS_PRODUCT_ID												2175
#define BENTLEY_POINTOOLS_PRODUCT_NAME												L"Bentley Pointools"

UINT_PTR	limitedModeWarningTimerID;
UINT_PTR	limitedModeTimerID;

#define LIMITED_MODE_SHUTDOWN_WARNING_TIMER	1000 * 60 * 13		// Time in milliseconds before a Limited mode shutdown warning is given (13 mins)
#define LIMITED_MODE_SHUTDOWN_TIMER			1000 * 60 * 2		// Time in milliseconds before a limited mode shutdown after warning was given (2 mins)


PointoolsBentleyLicense &thePointoolsBentleyLicense(void)
{
	return pointoolsBentleyLicense;
}

bool getLicenseStatusDescription(PointoolsBentleyLicense::LicenseDescriptionSet &description)
{
	PointoolsBentleyLicense &pointoolsBentleyLicense = thePointoolsBentleyLicense();
	return pointoolsBentleyLicense.getLicenseDescription(description);
}

bool startLicenseBentley(const PointoolsBentleyLicense::ProductVersion& productVersion)
{
	PointoolsBentleyLicense::Status			status;
	PointoolsBentleyLicense::LicenseStatus	licenseStatus;
	bool success = false;
							

	PointoolsBentleyLicense &pointoolsBentleyLicense = thePointoolsBentleyLicense();

															// Initialize licensing system, use the Bentley Pointools product ID even though we are licensing Vortex
															// Note that sending a "feature" param here returns a "Trial" license which is not what we want, also
															// a version number not in the form xx.xx.xx.xx is not accepted (e.g. 02.00.00.205 will not work).
	if((status = pointoolsBentleyLicense.initialize(pt::String(BENTLEY_POINTOOLS_PRODUCT_NAME), BENTLEY_POINTOOLS_PRODUCT_ID , productVersion, pt::String(L""), pt::String(L".\\"))) != PointoolsBentleyLicense::Status_OK)
	{
		return false;
	}
															// Attempt to start (obtain) a valid license
	licenseStatus = pointoolsBentleyLicense.startLicenseDesktop();
			
															// Respond to license status as the Vortex DLL, N.B. no messages can be shown and a license
															// can only be used if "Ok" or "Offline"
	switch(licenseStatus)
	{
															// If activated and licensed, run normally
		case LICENSE_STATUS_Ok:
			success = true;
			break;
															// If Offline (and not expired), run normally
		case LICENSE_STATUS_Offline:
			success = true;
			break;
															// If Trial mode do not run
		case LICENSE_STATUS_Trial:
			success = false;
			break;
															// If limited mode do not run
		case LICENSE_STATUS_Crippled:
			success = false;
			break;
															// If a licensing error ocured do not run
		case LICENSE_STATUS_Error:
			success = false;
			break;
															// If a license expired do not run
		case LICENSE_STATUS_Expired:
			success = false;
			break;

		default:															
			success = false;
			break;
	}
																
															// Return OK
	return success;
}
