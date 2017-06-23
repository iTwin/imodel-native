/*--------------------------------------------------------------------------------------+
|
|     $Source: Units/UnitConverter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "PublicApi/UnitConverter.h"
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform\DgnModel.h>
#include <cmath> 
#include <cstring>



USING_NAMESPACE_BENTLEY_DGN

BEGIN_BUILDING_NAMESPACE

static const double FPU = 3.28084;
static const double MPU = 1.0;
static const double M_PI = 3.14159265358979323846;

#define MAXLEN_ANGLE_STRING 100

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Wouter.Rombouts                 09/10
+---------------+---------------+---------------+---------------+---------------+------*/
double roundDouble(double x, int precision = 2)
    {
    x *= (10 * precision);
    if (x >= 0.0)
        {
        x += 0.5;
        return floor(x) / (10 * precision);;
        }

    x -= 0.5;

    double rounded = ceil(x);

    if (rounded == -0.0)
        {
        rounded = 0.0; //avoid  -0.0
        }

    rounded /= (10 * precision);;
    return rounded;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::ToMeters
(
double value
)
    {
    return roundDouble(value*MPU);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::ToSquareMeters
(
double value
)
    {
    return roundDouble (value*MPU*MPU);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::ToCubicMeters
(
double value
)
    {
    return roundDouble (value*MPU*MPU*MPU);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::FromMeters
(
double value
)
    {
    return roundDouble (value/MPU);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::FromSquareMeters
(
double value
)
    {
    return roundDouble ((value / MPU) / MPU);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::FromCubicMeters
(
double value
)
    {
    return roundDouble (((value / MPU) / MPU) / MPU);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::ToFeet
(
double value
)
    {
    return roundDouble (value*FPU);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::ToSquareFeet
(
double value
)
    {
    return roundDouble (value*FPU*FPU);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::ToCubicFeet
(
double value
)
    {
    return roundDouble (value*FPU*FPU*FPU);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::FromFeet
(
double value
)
    {
    return value/FPU;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::FromSquareFeet
(
double value
)
    {
    return ((value/FPU)/FPU);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          UnitConverter::FromCubicFeet
(
double value
)
    {
    return (((value/FPU)/FPU)/FPU);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String          UnitConverter::PrepareNumberForDisplay
(
double value
)
    {
    Utf8String resultString = Utf8PrintfString("%.2f", value);
    resultString = resultString.substr (resultString.size() - 3);
    int number = floor (value);
    int result = 0;
    bool skipped1st = false;
    while ((result = number % 1000) != 0 || number >= 1000)
        {
        if (skipped1st)
            resultString = Utf8PrintfString (",%s", resultString);
        else
            skipped1st = true;

        Utf8String tempResult = "";
        if (result != 0)
            tempResult = result > 99 || result == number ? Utf8PrintfString("%d", result) : result > 9 ? Utf8PrintfString("0%d", result) : Utf8PrintfString("00%d", result);
        else if (number >= 1000)
            tempResult = "000";

        resultString = Utf8PrintfString ("%s%s", tempResult, resultString);
        number = number / 1000;
        }

    if (value < 1)
        resultString = Utf8PrintfString ("0%s", resultString);
    return resultString;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Wouter.Rombouts                 08/2016
//---------------------------------------------------------------------------------------
void BuildingElement_notifyFail(Utf8CP pOperation, Dgn::DgnElement& elm, Dgn::DgnDbStatus* stat)
    {
    if (stat)
        {
        if (*stat == Dgn::DgnDbStatus::LockNotHeld)
            {
            auto eid = elm.GetElementId();
            int64_t eidVal = 0;
            if (eid.IsValid())
                {
                eidVal = eid.GetValue();
                }
            Utf8String notify = Utf8PrintfString("Error> Operation %s Failed on Element:%I64u, (Label:\"%s\"), due to LockNotHeld!", pOperation, eidVal, elm.GetUserLabel());
            Dgn::NotifyMessageDetails nmd(Dgn::OutputMessagePriority::Error, notify.c_str());
            Dgn::NotificationManager::OutputMessage(nmd);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonas.Valiunas   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DisplayLockFailedMessage
(
Dgn::DgnElementCR el, 
BeSQLite::DbOpcode op, 
Dgn::IBriefcaseManager::Response* pResponse
)
    {
    Dgn::RepositoryStatus status = pResponse ? pResponse->Result () : Dgn::RepositoryStatus::Success;
    Utf8String statusString;
    Utf8String additionalInfo = "";

    switch (status)
        {
    case RepositoryStatus::ServerUnavailable:
        statusString = "ServerUnavailable";
        break;
    case RepositoryStatus::LockAlreadyHeld:
        statusString = "LockAlreadyHeld";
        //extract who's using the element
        {
        Dgn::DgnLockInfoSet const& lockStates = pResponse->LockStates ();
        if (lockStates.size () > 0)
            {
            Dgn::DgnLockInfo lockInfo = *lockStates.begin ();
            Dgn::DgnLockOwnershipCR lockOwner = lockInfo.GetOwnership ();

            lockOwner.GetLockLevel ();
            }
        else
            additionalInfo += " Owner is not defined";

        break;
        }

        break;
    case RepositoryStatus::SyncError:
        statusString = "SyncError";
        break;
    case RepositoryStatus::InvalidResponse:
        statusString = "InvalidResponse";
        break;
    case RepositoryStatus::PendingTransactions:
        statusString = "PendingTransactions";
        break;
    case RepositoryStatus::LockUsed:
        statusString = "LockUsed";
        break;
    case RepositoryStatus::CannotCreateRevision:
        statusString = "CannotCreateRevision";
        break;
    case RepositoryStatus::InvalidRequest:
        statusString = "InvalidRequest";
        break;
    case RepositoryStatus::RevisionRequired:
        statusString = "RevisionRequired";
        break;
    case RepositoryStatus::CodeUnavailable:
        statusString = "CodeUnavailable";
        break;
    case RepositoryStatus::CodeNotReserved:
        statusString = "CodeNotReserved";
        break;
    case RepositoryStatus::CodeUsed:
        statusString = "CodeUsed";
        break;
    case RepositoryStatus::LockNotHeld:
        statusString = "LockNotHeld";
        break;
    case RepositoryStatus::RepositoryIsLocked:
        statusString = "RepositoryIsLocked";
        break;
        }

    Utf8String notify = Utf8PrintfString ("Error> acquire lock Failed on Element:%I64u, (Label:\"%s\"), due to \"%s\"", el.GetElementId (), el.GetUserLabel (), statusString).append(additionalInfo);
    Dgn::NotifyMessageDetails nmd (Dgn::OutputMessagePriority::Error, notify.c_str ());
    Dgn::NotificationManager::OutputMessage (nmd);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Jonas.Valiunas   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::RepositoryStatus BuildingLocks_LockElementForOperation 
(
Dgn::DgnElementCR el, 
BeSQLite::DbOpcode op, 
Utf8CP pOperation
)
    {
    IBriefcaseManager::Request request;
    auto stat = el.PopulateRequest (request, op);
    if (RepositoryStatus::Success == stat)
        {
        request.SetOptions (IBriefcaseManager::ResponseOptions::All);
        Dgn::IBriefcaseManager::Response response = el.GetDgnDb ().BriefcaseManager ().Acquire (request);
        //T_HOST.GetRepositoryAdmin ()._OnResponse (response, GetLocalizedToolName ().c_str ());

        if (RepositoryStatus::Success != response.Result())
            DisplayLockFailedMessage (el, op, &response);

        }
    else
        DisplayLockFailedMessage (el, op, NULL);

    return stat;
    }

StatusInt UnitConverter::MeetsAndBoundsStringToDouble(double& angle, Utf8CP string)
    {
    char bearing[MAXLEN_ANGLE_STRING];
    std::strncpy(bearing, string, std::strlen(string) + 1); //copying chars of string starting with string[1] ending with string[strlen(string)-2]

    //trim whitespaces
    while (0 != std::isspace(bearing[0]))
        for (int i = 0; i < strlen(bearing) - 2; ++i)
            bearing[i] = bearing[i + 1]; //trim leading whitespaces
    if (0 != bearing) //not all whitespaces
        {
        int firstTrailingWhitespacaIndex = 0;
        for (int i = strlen(bearing) - 1; i > 0; --i)
            if (0 == std::isspace(bearing[i]))
                {
                firstTrailingWhitespacaIndex = i + 1;
                break;
                }
        if (std::isspace(bearing[firstTrailingWhitespacaIndex]))
            bearing[firstTrailingWhitespacaIndex] = '\0'; //trim ending whitespaces
        }

    char first = bearing[0];
    char last = bearing[std::strlen(bearing) - 1];

    if (('n' != first && 's' != first) || ('e' != last && 'w' != last) || strlen(bearing)<3)
        return BentleyStatus::ERROR;

    char angleString[MAXLEN_ANGLE_STRING];
    std::strncpy(angleString, bearing + 1, std::strlen(bearing) - 2); //copying chars of string starting with string[1] ending with string[strlen(string)-2]
    angleString[std::strlen(bearing) - 2] = '\0';

    char* converted;
    strtod(angleString, &converted);
    if ('\0' != *converted || converted == angleString)
        return BentleyStatus::ERROR;
    

	angle = std::stod(angleString); //convert bearing to double value

	//bearing
	double degrees = std::floor(angle);
	double minutes = std::floor((angle - degrees) * 100);
	double seconds = (angle - degrees - minutes / 100) * 10000;

	angle = degrees + minutes / 60 + seconds / 3600;
	angle = angle * M_PI / 180;

	if (angle > M_PI / 2 || angle < 0)
		return BentleyStatus::ERROR;

	if (std::isnan(angle))
		return BentleyStatus::ERROR;

    if ('n' == first && 'e' == last) 
        {
        angle = M_PI / 2 - angle;
        return BentleyStatus::SUCCESS;
        }
    else if ('n' == first && 'w' == last)
        {
        angle = M_PI / 2 + angle;
        return BentleyStatus::SUCCESS;
        }
    else if ('s' == first && 'e' == last)
        {
        angle = 3 * M_PI / 2 + angle;
        return BentleyStatus::SUCCESS;
        }
    else if ('s' == first && 'w' == last)
        {
        angle = 3 * M_PI / 2 - angle;
        return BentleyStatus::SUCCESS;
        }

	return BentleyStatus::ERROR;
}
END_BUILDING_NAMESPACE

