/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnCustomAttributeHelper.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
//*****************************************************************
//DgnCustomAttributeHelper
//*****************************************************************

//---------------------------------------------------------------------------------------
//@bsimethod                                               Robert.Schili   09 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DgnCustomAttributeHelper::TryGetHandlerInfo(bool& requiredForModification, bool& allowDeleteIfMissing, ECClassCR ecClass, bool includeInherited)
    {
    IECInstancePtr customAttribute = includeInherited ? ecClass.GetCustomAttribute("HandlerInfo") : ecClass.GetCustomAttributeLocal("HandlerInfo");
    if (customAttribute == nullptr)
        return false;

    ECValue v;
    ECObjectsStatus stat = customAttribute->GetValue(v, "RequiredForModification");
    requiredForModification = (ECOBJECTS_STATUS_Success == stat && !v.IsNull() && v.GetBoolean());

    stat = customAttribute->GetValue(v, "AllowDeleteIfMissing");
    allowDeleteIfMissing = (ECOBJECTS_STATUS_Success == stat && !v.IsNull() && v.GetBoolean());

    return true;
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE
