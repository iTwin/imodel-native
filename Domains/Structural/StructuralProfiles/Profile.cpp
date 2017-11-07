#include <StructuralDomain/StructuralProfiles/Profile.h>

HANDLER_DEFINE_MEMBERS(ProfileHandler)

Profile::Profile(CreateParams const& params) : T_Super(params)
    {
    }

ECN::StandaloneECEnablerPtr Profile::GetCustomCardinalPointsEnabler()
    {
    ECN::ECClassCP structClass = GetElementClass()->GetSchema().GetClassCP(ecclass_name_CustomCardinalPointStruct());
    BeAssert(nullptr != structClass);

    ECN::StandaloneECEnablerPtr customCardinalPointsEnabler = structClass->GetDefaultStandaloneEnabler();
    BeAssert(customCardinalPointsEnabler.IsValid());

    return customCardinalPointsEnabler;
    }

bool Profile::RemoveAllCustomCardinalPoints()
    {
    uint32_t iCustomCardinalPointsIndex(0);

    Dgn::DgnDbStatus status = GetPropertyIndex(iCustomCardinalPointsIndex, prop_CustomCardinalPoints());
    BeAssert(status == Dgn::DgnDbStatus::Success);

    status = ClearPropertyArray(iCustomCardinalPointsIndex);

    return Dgn::DgnDbStatus::Success == status;
    }

bool Profile::SetCustomCardinalPoint(Utf8CP name, double x, double y)
    {
    DPoint2d coordinates;
    coordinates.Init(x, y);

    return SetCustomCardinalPoint(name, coordinates);
    }

bool Profile::SetCustomCardinalPoint(Utf8CP name, DPoint2dCR coordinates)
    {
    uint32_t index(0);
    bool bRet = FindCustomCardinalPointIndexByName(index, name);

    if (false != bRet)
        {
        uint32_t iCustomCardinalPointsIndex(0);

        Dgn::DgnDbStatus status = GetPropertyIndex(iCustomCardinalPointsIndex, prop_CustomCardinalPoints());
        BeAssert(status == Dgn::DgnDbStatus::Success);

        ECN::IECInstancePtr structInstance = GetCustomCardinalPointsEnabler()->CreateInstance().get();
        BeAssert(structInstance.IsValid());

        ECN::ECObjectsStatus objectStatus = structInstance->SetValue("Name", ECN::ECValue(name));
        BeAssert(ECN::ECObjectsStatus::Success == objectStatus);

        objectStatus = structInstance->SetValue("Coordinates", ECN::ECValue(coordinates));
        BeAssert(ECN::ECObjectsStatus::Success == objectStatus);

        ECN::ECValue structValue;
        BentleyStatus bentleyStatus = structValue.SetStruct(structInstance.get());
        BeAssert(BentleyStatus::SUCCESS == bentleyStatus);

        status = SetPropertyValue(prop_CustomCardinalPoints(), structValue, Dgn::PropertyArrayIndex(index));
        BeAssert(Dgn::DgnDbStatus::Success == status);

        bRet = Dgn::DgnDbStatus::Success == status;
        }

    return bRet;
    }

bool Profile::LookupCustomCardinalPointByName(Utf8CP name)
    {
    uint32_t index(0);
    return FindCustomCardinalPointIndexByName(index, name);
    }

bool Profile::FindCustomCardinalPointIndexByName(uint32_t& index, Utf8CP name)
    {
    bool bRet(false);
    Dgn::DgnDbStatus status;
    ECN::ECValue testValue;

    for (uint32_t uiIdx = 0; uiIdx < CustomCardinalPointsCount(); ++uiIdx)
        {
        ECN::IECInstancePtr checkInstance;
        ECN::ECValue checkValue;
        status = GetPropertyValue(checkValue, prop_CustomCardinalPoints(), Dgn::PropertyArrayIndex(uiIdx));
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

bool Profile::RemoveCustomCardinalPoint(Utf8CP name)
    {
    uint32_t index(0);
    bool bRet = FindCustomCardinalPointIndexByName(index, name);

    if (false != bRet)
        {
        uint32_t iCustomCardinalPointsIndex(0);

        Dgn::DgnDbStatus status = GetPropertyIndex(iCustomCardinalPointsIndex, prop_CustomCardinalPoints());
        BeAssert(status == Dgn::DgnDbStatus::Success);

        status = RemovePropertyArrayItem(iCustomCardinalPointsIndex, index);
        bRet = (status == Dgn::DgnDbStatus::Success);
        }

    return bRet;
    }

bool Profile::AddCustomCardinalPoint(Utf8CP name, double x, double y)
    {
    DPoint2d coordinates;
    coordinates.Init(x, y);

    return AddCustomCardinalPoint(name, coordinates);
    }

uint32_t Profile::CustomCardinalPointsCount()
    {
    ECN::ECValue testValue;
    Dgn::DgnDbStatus status = GetPropertyValue(testValue, prop_CustomCardinalPoints());
    BeAssert(Dgn::DgnDbStatus::Success == status);

    ECN::ArrayInfo arrayInfo = testValue.GetArrayInfo();

    return arrayInfo.GetCount();
    }

bool Profile::AddCustomCardinalPoint(Utf8CP name, DPoint2dCR coordinates)
    {
    if (LookupCustomCardinalPointByName(name))
        {
        return false;
        }

    uint32_t iCustomCardinalPointsIndex(0);

    Dgn::DgnDbStatus status = GetPropertyIndex(iCustomCardinalPointsIndex, prop_CustomCardinalPoints());
    BeAssert(status == Dgn::DgnDbStatus::Success);

    ECN::ECObjectsStatus objectStatus(ECN::ECObjectsStatus::Success);
    ECN::IECInstancePtr structInstance = GetCustomCardinalPointsEnabler()->CreateInstance().get();
    BeAssert(structInstance.IsValid());

    uint32_t size = CustomCardinalPointsCount();

    status = InsertPropertyArrayItems(iCustomCardinalPointsIndex, size, 1);
    BeAssert(status == Dgn::DgnDbStatus::Success);

    objectStatus = structInstance->SetValue("Name", ECN::ECValue(name));
    BeAssert(ECN::ECObjectsStatus::Success == objectStatus);

    objectStatus = structInstance->SetValue("Coordinates", ECN::ECValue(coordinates));
    BeAssert(ECN::ECObjectsStatus::Success == objectStatus);

    ECN::ECValue structValue;
    BentleyStatus bentleyStatus = structValue.SetStruct(structInstance.get());
    BeAssert(BentleyStatus::SUCCESS == bentleyStatus);

    status = SetPropertyValue(prop_CustomCardinalPoints(), structValue, Dgn::PropertyArrayIndex(size));
    BeAssert(Dgn::DgnDbStatus::Success == status);

    return Dgn::DgnDbStatus::Success == status;
    }
