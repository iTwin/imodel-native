#include "ProfilesInternal.h"
#include <ProfilesInternal\ProfilesCardinalPoints.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Static array of standard CardinalPoint names. This structs items must be aligned with
* StandardCardinalPoint enumerations.
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8CP s_standardCardinalPointNames[] =
    {
    "BottomLeft",
    "BottomCenter",
    "BottomRight",
    "MidDepthLeft",
    "MidDepthCenter",
    "MidDepthRight",
    "TopLeft",
    "TopCenter",
    "TopRight",
    "GeometricCentroid",
    "BottomInLineWithGeometricCentroid",
    "LeftInLineWithGeometricCentroid",
    "RightInLineWithGeometricCentroid",
    "TopInLineWithGeometricCentroid",
    "ShearCentre",
    "BottomInLineWithShearCentre",
    "LeftInLineWithShearCentre",
    "RightInLineWithShearCentre",
    "TopInLineWithShearCentre"
    };
constexpr uint32_t s_standardCardinalPointCount = _countof (s_standardCardinalPointNames);

/*---------------------------------------------------------------------------------**//**
* Creates a CardinalPoint corresponding ECInstance and sets it to a previously allocated
* array index for the Profile.CardinalPoints property.
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus setCardinalPoint (Profile& profile, CardinalPoint const& cardinalPoint, uint32_t arrayIndex)
    {
    static StandaloneECEnablerPtr s_enablerPtr = nullptr;
    if (s_enablerPtr.IsNull())
        {
        ECClassCP pClass = profile.QueryClass (profile.GetDgnDb(), PRF_CLASS_CardinalPoint);
        if (pClass == nullptr)
            return DgnDbStatus::WrongClass;

        ClassLayoutPtr classLayoutPtr = ClassLayout::BuildFromClass (*pClass);
        if (classLayoutPtr.IsNull())
            return DgnDbStatus::WrongClass;

        s_enablerPtr = StandaloneECEnabler::CreateEnabler (*pClass, *classLayoutPtr, nullptr);
        if (s_enablerPtr.IsNull())
            return DgnDbStatus::WrongClass;
        }

    StandaloneECInstancePtr instancePtr = s_enablerPtr->CreateInstance();

    BeAssert (instancePtr->SetValue (PRF_PROP_CardinalPoint_Name, ECValue (cardinalPoint.name.c_str())) == ECObjectsStatus::Success);
    BeAssert (instancePtr->SetValue (PRF_PROP_CardinalPoint_Location, ECValue (cardinalPoint.location)) == ECObjectsStatus::Success);

    ECValue arrayValue;
    arrayValue.SetStruct (instancePtr.get());

    uint32_t propertyIndex = 0;
    profile.GetPropertyIndex (propertyIndex, PRF_PROP_Profile_CardinalPoints);

    return profile.SetPropertyValue (PRF_PROP_Profile_CardinalPoints, arrayValue, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* Returns item count of Profile.CardinalPoints array.
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static uint32_t getCardinalPointsArraySize (Profile const& profile)
    {
    ECValue arrayValue;
    DgnDbStatus status = profile.GetPropertyValue (arrayValue, PRF_PROP_Profile_CardinalPoints);
    if (status != DgnDbStatus::Success)
        return 0;

    return arrayValue.GetArrayInfo().GetCount();
    }

/*---------------------------------------------------------------------------------**//**
* Searches for a CardinalPoint by name in Profile.CardinalPoints array.
* @returns If CardinalPoint with given name exists - Success is returned with filled
* in \p structValue, error code otherwise.
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus findCardinalPointECValue (Profile const& profile, Utf8String const& name, ECValue& structValue)
    {
    uint32_t arraySize = getCardinalPointsArraySize (profile);
    for (uint32_t i = 0; i < arraySize; ++i)
        {
        DgnDbStatus status = profile.GetPropertyValue (structValue, PRF_PROP_Profile_CardinalPoints, i);
        if (status != DgnDbStatus::Success)
            return status;

        ECValue propertyValue;
        BeAssert (structValue.GetStruct()->GetValue (propertyValue, PRF_PROP_CardinalPoint_Name) == ECObjectsStatus::Success);

        if (name.Equals (propertyValue.GetUtf8CP()))
            return DgnDbStatus::Success;
        }

    return DgnDbStatus::NotFound;
    }

/*---------------------------------------------------------------------------------**//**
* Construct CardinalPoint struct from an IECInstance representing it.
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
CardinalPoint cardinalPointFromECValue (ECValue const& ecValue)
    {
    BeAssert (ecValue.IsStruct());

    CardinalPoint cardinalPoint;
    ECValue propertyValue;

    BeAssert (ecValue.GetStruct()->GetValue (propertyValue, PRF_PROP_CardinalPoint_Name) == ECObjectsStatus::Success);
    cardinalPoint.name = propertyValue.GetUtf8CP();

    BeAssert (ecValue.GetStruct()->GetValue (propertyValue, PRF_PROP_CardinalPoint_Location) == ECObjectsStatus::Success);
    cardinalPoint.location = propertyValue.GetPoint2d();

    return cardinalPoint;
    }

/*---------------------------------------------------------------------------------**//**
* Construct standard CardinalPoint struct by enumeration.
* NOTE: ShearPoint and GeometricCentroid cardinal point locations are currently not supported.
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
CardinalPoint createStandardCardinalPoint (StandardCardinalPoint type, Profile const& profile)
    {
    IGeometryPtr shapePtr = profile.GetShape();
    BeAssert (shapePtr.IsValid());

    DRange3d shapeRange = {};
    BeAssert (shapePtr->TryGetRange (shapeRange));

    double const halfWidth = shapeRange.XLength() / 2.0;
    double const halfDepth = shapeRange.YLength() / 2.0;
    Utf8CP pName = s_standardCardinalPointNames[static_cast<int> (type)];

    switch (type)
        {
        case StandardCardinalPoint::BottomLeft:     return CardinalPoint (pName, DPoint2d::From (-halfWidth, -halfDepth));
        case StandardCardinalPoint::BottomCenter:   return CardinalPoint (pName, DPoint2d::From (0.0, -halfDepth));
        case StandardCardinalPoint::BottomRight:    return CardinalPoint (pName, DPoint2d::From (halfWidth, -halfDepth));
        case StandardCardinalPoint::MidDepthLeft:   return CardinalPoint (pName, DPoint2d::From (-halfWidth, 0.0));
        case StandardCardinalPoint::MidDepthCenter: return CardinalPoint (pName, DPoint2d::From (0.0, 0.0));
        case StandardCardinalPoint::MidDepthRight:  return CardinalPoint (pName, DPoint2d::From (halfWidth, 0.0));
        case StandardCardinalPoint::TopLeft:        return CardinalPoint (pName, DPoint2d::From (-halfWidth, halfDepth));
        case StandardCardinalPoint::TopCenter:      return CardinalPoint (pName, DPoint2d::From (0.0, halfDepth));
        case StandardCardinalPoint::TopRight:       return CardinalPoint (pName, DPoint2d::From (halfWidth, halfDepth));
        // ShearPoint and GeometricCentroid cardinal point locations are currently not supported.
        default:
            return CardinalPoint (pName, DPoint2d::From (0.0, 0.0));
        }
    }

/*---------------------------------------------------------------------------------**//**
* Insert all standard CardinalPoints for the Profile.CardinalPoints array. Cardinal
* points are inserted at the begging of the array.
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ProfilesCardinalPoints::AddStandardCardinalPoints (Profile& profile)
    {
    uint32_t propertyIndex = 0;
    profile.GetPropertyIndex (propertyIndex, PRF_PROP_Profile_CardinalPoints);

    // Ensure standard cardinal points are the start of the array
    DgnDbStatus status = profile.InsertPropertyArrayItems (propertyIndex, 0, s_standardCardinalPointCount);
    if (status != DgnDbStatus::Success)
        return status;

    for (int i = 0; i < s_standardCardinalPointCount; ++i)
        {
        CardinalPoint cardinalPoint = createStandardCardinalPoint (static_cast<StandardCardinalPoint> (i), profile);

        status = setCardinalPoint (profile, cardinalPoint, i);
        if (status != DgnDbStatus::Success)
            return status;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* Appends a user-defined CardinalPoint to the end of Profile.CardinalPoints array.
* CardinalPoint name must be unique for the profile.
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ProfilesCardinalPoints::AddCustomCardinalPoint (Profile& profile, CardinalPoint const& customCardinalPoint)
    {
    ECValue structValue;
    if (findCardinalPointECValue (profile, customCardinalPoint.name, structValue) == DgnDbStatus::Success)
        return DgnDbStatus::DuplicateName;

    uint32_t propertyIndex = 0;
    profile.GetPropertyIndex (propertyIndex, PRF_PROP_Profile_CardinalPoints);

    uint32_t arraySize = getCardinalPointsArraySize (profile);

    DgnDbStatus status = profile.AddPropertyArrayItems (propertyIndex, 1);
    if (status != DgnDbStatus::Success)
        return status;

    return setCardinalPoint (profile, customCardinalPoint, arraySize);
    }

/*---------------------------------------------------------------------------------**//**
* Get all cardinal points that the profile has. Returns an empty vector in case of an error.
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<CardinalPoint> ProfilesCardinalPoints::GetCardinalPoints (Profile const& profile)
    {
    uint32_t arraySize = getCardinalPointsArraySize (profile);
    bvector<CardinalPoint> cardinalPoints (static_cast<size_t> (arraySize));

    for (uint32_t i = 0; i < arraySize; ++i)
        {
        ECValue structValue;

        if (profile.GetPropertyValue (structValue, PRF_PROP_Profile_CardinalPoints, i) != DgnDbStatus::Success)
            return bvector<CardinalPoint>();

        CardinalPoint cardinalPoint = cardinalPointFromECValue (structValue);
        cardinalPoints.push_back (cardinalPoint);
        }

    return cardinalPoints;
    }

/*---------------------------------------------------------------------------------**//**
* Get standard CardinalPoint by enumeration.
* @param cardinalPoint [out] CardinalPoint returned by the method on succes.
* @returns Success if CardinalPoint was found, error code otherwise.
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ProfilesCardinalPoints::GetCardinalPoint (Profile const& profile, StandardCardinalPoint standardType, CardinalPoint& cardinalPoint)
    {
    ECValue structValue;
    uint32_t standardPointIndex = static_cast<uint32_t> (standardType);

    DgnDbStatus status = profile.GetPropertyValue (structValue, PRF_PROP_Profile_CardinalPoints, standardPointIndex);
    if (status != DgnDbStatus::Success)
        return status;

    cardinalPoint = cardinalPointFromECValue (structValue);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* Get any CardinalPoint by name.
* @param cardinalPoint [out] CardinalPoint returned by the method on succes.
* @returns Success if CardinalPoint was found, error code otherwise.
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ProfilesCardinalPoints::GetCardinalPoint (Profile const& profile, Utf8String const& name, CardinalPoint& cardinalPoint)
    {
    ECValue structValue;
    DgnDbStatus status = findCardinalPointECValue (profile, name, structValue);
    if (status != DgnDbStatus::Success)
        return status;

    cardinalPoint = cardinalPointFromECValue (structValue);
    return DgnDbStatus::Success;
    }

END_BENTLEY_PROFILES_NAMESPACE
