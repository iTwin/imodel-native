/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnCustomAttributeHelper.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define DGN_CUSTOMATTRIBUTE_HANDLERINFO_NAME                            "HandlerInfo"
#define DGN_CUSTOMATTRIBUTE_HANDLERINFO_REQUIREDFORMODIFICATION_NAME    "RequiredForModification"
#define DGN_CUSTOMATTRIBUTE_HANDLERINFO_ALLOWDELETEIFMISSING_NAME       "AllowDeleteIfMissing"

//---------------------------------------------------------------------------------------
//@bsimethod                                               Robert.Schili   09 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool DgnCustomAttributeHelper::TryGetHandlerInfo(bool& requiredForModification, bool& allowDeleteIfMissing, ECClassCR ecClass, bool includeInherited)
    {
    IECInstancePtr customAttribute = includeInherited ? ecClass.GetCustomAttribute(DGN_CUSTOMATTRIBUTE_HANDLERINFO_NAME) : ecClass.GetCustomAttributeLocal(DGN_CUSTOMATTRIBUTE_HANDLERINFO_NAME);
    if (customAttribute == nullptr)
        return false;

    ECValue v;
    ECObjectsStatus stat = customAttribute->GetValue(v, DGN_CUSTOMATTRIBUTE_HANDLERINFO_REQUIREDFORMODIFICATION_NAME);
    requiredForModification = (ECOBJECTS_STATUS_Success == stat && !v.IsNull() && v.GetBoolean());

    stat = customAttribute->GetValue(v, DGN_CUSTOMATTRIBUTE_HANDLERINFO_ALLOWDELETEIFMISSING_NAME);
    allowDeleteIfMissing = (ECOBJECTS_STATUS_Success == stat && !v.IsNull() && v.GetBoolean());

    return true;
    }
