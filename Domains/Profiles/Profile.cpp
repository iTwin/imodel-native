#include "PublicAPI\ProfilesDomain\Profile.h"
#include <Bentley/bvector.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE
HANDLER_DEFINE_MEMBERS(ProfileHandler)

static bvector<Utf8String> CardinalPointsStandardNames =
    {
    "LeftBottom",
    "MiddleBottom",
    "RightBottom",
    "LeftMiddle",
    "MiddleMiddle",
    "RightMiddle",
    "LeftTop",
    "MiddleTop",
    "RightTop",
    "CentroidCentroid",
    "CentroidBottom",
    "LeftCentroid",
    "RightCentroid",
    "CentroidTop",
    "ShearShear",
    "ShearBottom",
    "LeftShear",
    "RightShear",
    "ShearTop"
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Arturas.Mizaras                   11/2017
//---------------------------------------------------------------------------------------
Profile::Profile(CreateParams const& params) : T_Super(params)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Arturas.Mizaras                   11/2017
//---------------------------------------------------------------------------------------
ECN::StandaloneECEnablerPtr Profile::GetECEnabler(Utf8CP className)
    {
    ECN::ECClassCP structClass = GetElementClass()->GetSchema().GetClassCP(className);
    BeAssert(nullptr != structClass);

    ECN::StandaloneECEnablerPtr enabler = structClass->GetDefaultStandaloneEnabler();
    BeAssert(enabler.IsValid());

    return enabler;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Arturas.Mizaras                   11/2017
//---------------------------------------------------------------------------------------
bool Profile::RemoveAllCardinalPoints()
    {
    uint32_t iCardinalPointsIndex(0);

    Dgn::DgnDbStatus status = GetPropertyIndex(iCardinalPointsIndex, prop_CardinalPoints());
    BeAssert(status == Dgn::DgnDbStatus::Success);

    status = ClearPropertyArray(iCardinalPointsIndex);

    return Dgn::DgnDbStatus::Success == status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Arturas.Mizaras                   11/2017
//---------------------------------------------------------------------------------------
bool Profile::IsStandardCardinalPointName(Utf8CP name)
    {
    return CardinalPointsStandardNames.end() != std::find(CardinalPointsStandardNames.begin(), CardinalPointsStandardNames.end(), Utf8String(name));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Arturas.Mizaras                   11/2017
//---------------------------------------------------------------------------------------
bool Profile::SetCardinalPoint(Utf8CP name, double x, double y)
    {
    DPoint2d coordinates;
    coordinates.Init(x, y);

    return SetCardinalPoint(name, coordinates);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Arturas.Mizaras                   11/2017
//---------------------------------------------------------------------------------------
bool Profile::SetCardinalPoint(Utf8CP name, DPoint2dCR coordinates)
    {
    uint32_t index(0);
    bool bRet = FindCardinalPointIndexByName(index, name);

    if (false != bRet)
        {
        uint32_t iCardinalPointsIndex(0);

        Dgn::DgnDbStatus status = GetPropertyIndex(iCardinalPointsIndex, prop_CardinalPoints());
        BeAssert(status == Dgn::DgnDbStatus::Success);

        ECN::IECInstancePtr structInstance = GetECEnabler(ecclass_name_CardinalPointStruct())->CreateInstance().get();
        BeAssert(structInstance.IsValid());

        ECN::ECObjectsStatus objectStatus = structInstance->SetValue("Name", ECN::ECValue(name));
        BeAssert(ECN::ECObjectsStatus::Success == objectStatus);

        objectStatus = structInstance->SetValue("Coordinates", ECN::ECValue(coordinates));
        BeAssert(ECN::ECObjectsStatus::Success == objectStatus);

        ECN::ECValue structValue;
        BentleyStatus bentleyStatus = structValue.SetStruct(structInstance.get());
        BeAssert(BentleyStatus::SUCCESS == bentleyStatus);

        status = SetPropertyValue(prop_CardinalPoints(), structValue, Dgn::PropertyArrayIndex(index));
        BeAssert(Dgn::DgnDbStatus::Success == status);

        bRet = Dgn::DgnDbStatus::Success == status;
        }

    return bRet;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Arturas.Mizaras                   11/2017
//---------------------------------------------------------------------------------------
bool Profile::LookupCardinalPointByName(Utf8CP name)
    {
    uint32_t index(0);
    return FindCardinalPointIndexByName(index, name);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Arturas.Mizaras                   11/2017
//---------------------------------------------------------------------------------------
bool Profile::FindCardinalPointIndexByName(uint32_t& index, Utf8CP name)
    {
    bool bRet(false);
    Dgn::DgnDbStatus status;
    ECN::ECValue testValue;

    for (uint32_t uiIdx = 0; uiIdx < GetECArrayCount(prop_CardinalPoints()); ++uiIdx)
        {
        ECN::IECInstancePtr checkInstance;
        ECN::ECValue checkValue;
        status = GetPropertyValue(checkValue, prop_CardinalPoints(), Dgn::PropertyArrayIndex(uiIdx));
        BeAssert(Dgn::DgnDbStatus::Success == status);

        if (!checkValue.IsNull())
            {
            ECN::ECValue value;
            checkInstance = checkValue.GetStruct();
            BeAssert(checkInstance.IsValid());

            ECN::ECObjectsStatus objectStatus = checkInstance->GetValue(value, "Name");
            BeAssert(ECN::ECObjectsStatus::Success == objectStatus);

            Utf8CP instanceName = value.GetUtf8CP();

            if (0 == strcmp(name, instanceName))
                {
                bRet = true;
                index = uiIdx;
                break;
                }
            }
        }

    return bRet;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Arturas.Mizaras                   11/2017
//---------------------------------------------------------------------------------------
bool Profile::RemoveCardinalPoint(Utf8CP name)
    {
    uint32_t index(0);
    bool bRet = FindCardinalPointIndexByName(index, name);

    if (false != bRet)
        {
        uint32_t iCardinalPointsIndex(0);

        Dgn::DgnDbStatus status = GetPropertyIndex(iCardinalPointsIndex, prop_CardinalPoints());
        BeAssert(status == Dgn::DgnDbStatus::Success);

        status = RemovePropertyArrayItem(iCardinalPointsIndex, index);
        bRet = (status == Dgn::DgnDbStatus::Success);
        }

    return bRet;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Arturas.Mizaras                   11/2017
//---------------------------------------------------------------------------------------
bool Profile::AddCardinalPoint(Utf8CP name, double x, double y)
    {
    DPoint2d coordinates;
    coordinates.Init(x, y);

    return AddCardinalPoint(name, coordinates);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Arturas.Mizaras                   11/2017
//---------------------------------------------------------------------------------------
uint32_t Profile::GetECArrayCount(Utf8CP arrayPropertyName)
    {
    ECN::ECValue testValue;
    Dgn::DgnDbStatus status = GetPropertyValue(testValue, arrayPropertyName);
    BeAssert(Dgn::DgnDbStatus::Success == status);

    ECN::ArrayInfo arrayInfo = testValue.GetArrayInfo();

    return arrayInfo.GetCount();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Arturas.Mizaras                   11/2017
//---------------------------------------------------------------------------------------
bool Profile::AddCardinalPoint(Utf8CP name, DPoint2dCR coordinates)
    {
    if (LookupCardinalPointByName(name))
        {
        return false;
        }

    uint32_t iCardinalPointsIndex(0);

    Dgn::DgnDbStatus status = GetPropertyIndex(iCardinalPointsIndex, prop_CardinalPoints());
    BeAssert(status == Dgn::DgnDbStatus::Success);

    ECN::ECObjectsStatus objectStatus(ECN::ECObjectsStatus::Success);
    ECN::IECInstancePtr structInstance = GetECEnabler(ecclass_name_CardinalPointStruct())->CreateInstance().get();
    BeAssert(structInstance.IsValid());

    uint32_t size = GetECArrayCount(prop_CardinalPoints());

    status = InsertPropertyArrayItems(iCardinalPointsIndex, size, 1);
    BeAssert(status == Dgn::DgnDbStatus::Success);

    objectStatus = structInstance->SetValue("Name", ECN::ECValue(name));
    BeAssert(ECN::ECObjectsStatus::Success == objectStatus);

    objectStatus = structInstance->SetValue("Coordinates", ECN::ECValue(coordinates));
    BeAssert(ECN::ECObjectsStatus::Success == objectStatus);

    ECN::ECValue structValue;
    BentleyStatus bentleyStatus = structValue.SetStruct(structInstance.get());
    BeAssert(BentleyStatus::SUCCESS == bentleyStatus);

    status = SetPropertyValue(prop_CardinalPoints(), structValue, Dgn::PropertyArrayIndex(size));
    BeAssert(Dgn::DgnDbStatus::Success == status);

    return Dgn::DgnDbStatus::Success == status;
    }

END_BENTLEY_PROFILES_NAMESPACE