#include <ptlic/PointoolsBentleyLicenseAPI.h>

PointoolsBentleyLicense	pointoolsBentleyLicense;

#define	BENTLEY_POINTOOLS_PRODUCT_ID												2175
#define BENTLEY_POINTOOLS_PRODUCT_NAME												L"Bentley Pointools"
static const PointoolsBentleyLicense::LicenseIsDesktopOrIware s_desktopOrIWare =	PointoolsBentleyLicense::License_Is_Deskop;

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

static void exitApplication(void)
{
	exit(0);
}


void _stdcall limitedModeTimeoutCallback(HWND hWnd, UINT a, UINT_PTR b, DWORD c)
{
															// Kill exit timer
	KillTimer(NULL, limitedModeTimerID);

// ToDo: Add Alertbox message

//	ptui::alertBox(STR_LIMITED_MODE_SHUTDOWN_TITLE, STR_LIMITED_MODE_SHUTDOWN);

	exitApplication();
}

void _stdcall limitedModeTimeoutWarningCallback(HWND hWnd, UINT a, UINT_PTR b, DWORD c)
{
															// Kill warning timer
	KillTimer(NULL, limitedModeWarningTimerID);
															// Start application exit time limiter
	if((limitedModeTimerID = SetTimer(NULL, 2, LIMITED_MODE_SHUTDOWN_TIMER, &limitedModeTimeoutCallback)) == 0)
	{
															// If timed callback couldn't be created, just shut down now for safety
		exitApplication();
	}
															// Give warning that application will soon be shut down
// ToDo: Add alertbox message
//	ptui::alertBox(STR_LIMITED_MODE_SHUTDOWN_TITLE, STR_LIMITED_MODE_SHUTDOWN_WARNING);
}

bool startLicenseBentley(void)
{
#ifdef _DEBUG
	//// sorry guys, faraz needs a license for debugging
	//{
	//	int mode = LICENSE_NORMAL_MODE;
	//	ptapp::env()->set(STR_MODE, mode);
	//}
	//return true;
#endif

	PointoolsBentleyLicense::Status			status;
	PointoolsBentleyLicense::LicenseStatus	licenseStatus;
	int										mode = 0;

	PointoolsBentleyLicense &pointoolsBentleyLicense = thePointoolsBentleyLicense();
															// Initialize licensing system for use with this application
	if((status = pointoolsBentleyLicense.initialize(pt::String(BENTLEY_POINTOOLS_PRODUCT_NAME), BENTLEY_POINTOOLS_PRODUCT_ID , pt::String("00.00.00.00"), pt::String(L""), pt::String(L".\\"), s_desktopOrIWare)) != PointoolsBentleyLicense::Status_OK)
	{
		return false;
	}
															// Attempt to start (obtain) a valid license
	licenseStatus = pointoolsBentleyLicense.startLicenseDesktop();
		
															// Set application mode based on the license status
	if (s_desktopOrIWare != PointoolsBentleyLicense::License_Is_IWare)
	{
		switch(licenseStatus)
		{
																// If activated and licensed, run normally
			case LICENSE_STATUS_Ok:
				mode = LICENSE_NORMAL_MODE;
				break;
																// If Offline (and not expired), run normally
			case LICENSE_STATUS_Offline:
				mode = LICENSE_NORMAL_MODE;
				break;
																// If Trial mode (and not expired), run normally
			case LICENSE_STATUS_Trial:
				mode = LICENSE_NORMAL_MODE;
				break;
																// If running in limited mode, run in limited mode
			case LICENSE_STATUS_Crippled:
				mode = LICENSE_FREE_MODE;
				break;
																// If a licensing error ocured, run in limited mode
			case LICENSE_STATUS_Error:
				mode = LICENSE_FREE_MODE;
				break;

			case LICENSE_STATUS_Expired:
			default:
																// Exit now. (Note: A call to exitApplication is not valid at this point in time.)
				exit(0);
		}

																// If License mode is Limited 'Free' mode
		if(mode == LICENSE_FREE_MODE)
		{

																	// Start application time limiter
			if((limitedModeWarningTimerID = SetTimer(NULL, 1, LIMITED_MODE_SHUTDOWN_WARNING_TIMER, &limitedModeTimeoutWarningCallback)) == 0)
			{
																		// If timed callback couldn't be created, just shut down now for safety
				exitApplication();	
			}
		}
	}
	else
	{
																// This is an iWare app (e.g. Bentley Pointools View) set the mode to be free to limit disallowed features, but do not set the timeout timer
		mode = LICENSE_FREE_MODE;
	}
															// Set application mode
// ToDo: Anything to port here?
//	ptapp::env()->set(STR_MODE, mode);

															// Return OK
	return true;
}
