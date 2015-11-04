/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/UnitDefinition.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include    <DgnPlatform/ExportMacros.h>
#include    <DgnPlatform/DgnPlatform.r.h>
#include    <Bentley/WString.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! A unit can be categorized according to its system.  The system has no mathematical
//! significance.
//=======================================================================================
enum class UnitSystem
    {
    Undefined   = 0,
    English     = 1,
    Metric      = 2,
    USSurvey    = 3,
    };

END_BENTLEY_DGN_NAMESPACE

DGNPLATFORM_TYPEDEFS (UnitDefinition)
DGNPLATFORM_TYPEDEFS (UnitIteratorOptions)

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! A UnitDefinition describes a unit which is a context in which users and
//! applications express distances.  To correctly express the value of any distance
//! requires both a numerical value and a UnitDefinition.  In practice, for most
//! code the UnitDefinition for every distance is implied by some context.
//!
//! A unit is represented by a scale factor that describes its size relative to its
//! #UnitBase.  For example standard linear distance units should be based on
//! the SI unit 'Meter' represented by the value UnitBase::Meter.  The SI unit
//! 'Centimeter' is represented by the scale factor 100.0 since 1 Meter = 100.0
//! Centimeters.  Only units with identical bases can be compared.
//!
//! The UnitBase concept should not be confused with the concept of #UnitSystem.
//! UnitSystem is useful only for categorization and has no mathematical
//! significance.  For example, the units 'Kilometer' and 'Mile' are properly
//! categorized as UnitSystem::Metric and UnitSystem::English respectively, yet both
//! use UnitBase::Meter so they are comparable to one another.
//=======================================================================================
struct UnitDefinition
{
private:
    UnitBase        m_base;
    UnitSystem      m_system;
    double          m_numerator;
    double          m_denominator;
    WString         m_label;

protected:

public:
//__PUBLISH_SECTION_END__
    //! Make this unit larger by a scale factor.
    void Scale (double by);
    void Square ();
    void Cube ();
//__PUBLISH_SECTION_START__

    //! Construct an invalid UnitDefinition.
    DGNPLATFORM_EXPORT UnitDefinition ();

    //! Construct a fully initialized UnitDefinition.
    DGNPLATFORM_EXPORT UnitDefinition (UnitBase base, UnitSystem sys, double num, double den, WCharCP label);

    //! Initialize a UnitDefinition from data.
    DGNPLATFORM_EXPORT void Init (UnitBase base, UnitSystem sys, double num, double den, WCharCP label);

    //! Initialize a UnitDefinition from another UnitDefinition
    DGNPLATFORM_EXPORT void Init (UnitDefinitionCR source);

    DGNPLATFORM_EXPORT static UnitBase BaseFromInt (uint32_t base);
    DGNPLATFORM_EXPORT static UnitSystem SystemFromInt (uint32_t system);
    DGNPLATFORM_EXPORT static int CompareRatios (double num1, double den1, double num2, double den2);

    //! Test if the unit contains valid data.
    DGNPLATFORM_EXPORT bool IsValid () const;

    //! Make this unit invalid
    DGNPLATFORM_EXPORT void SetInvalid ();

//__PUBLISH_SECTION_END__
    //! Test if two units are identical.  This equality check can fail even if the units are the same size.
    //! See #CompareByScale for a less strict equality test.
//__PUBLISH_SECTION_START__
    DGNPLATFORM_EXPORT bool IsEqual (UnitDefinitionCR other) const;

    //! Test if two units are identical.  This equality check can fail even if the units are the same size.
    //! See #CompareByScale for a less strict equality test.
    DGNPLATFORM_EXPORT bool operator==(UnitDefinitionCR) const;

    //! Test if two units are not identical.
    DGNPLATFORM_EXPORT bool operator!=(UnitDefinitionCR other) const {return !(*this == other);}

    //! Test if the sizes of two units can be compared.  Only units with a common base can be compared.
    DGNPLATFORM_EXPORT bool AreComparable (UnitDefinitionCR other) const;

    //! Test if two units are the same size.  See #IsEqual for a more strict equality test.
    //! @return an integer i, such that:
    //!  - i == ERROR if the units could not be compared.  See #AreComparable.
    //!  - i < 0 if this unit is larger than other.
    //!  - i > 0 if other unit is larger than this unit.
    //!  - i == 0 if the two units are the same size.
    DGNPLATFORM_EXPORT int CompareByScale (UnitDefinitionCR other) const;

    //! Compute the scale factor used to convert distance from the input unit to this unit.
    //! The factor is defined by the equation: distInA = distInB * factorFromBtoA.  The
    //! method #ConvertDistanceFrom is preferable since that calculation is more direct.
    //! @return conversion factor, if units are not comparable, return 1.0.
    DGNPLATFORM_EXPORT double GetConversionFactorFrom(UnitDefinitionCR from) const;

    //! Convert a distance expressed in the input unit to this unit.
    //! @return the distance in this units. If units are not compareable, return inputVal.
    DGNPLATFORM_EXPORT double ConvertDistanceFrom (double inputVal, UnitDefinitionCR fromUnit) const;

    bool IsLinear() const {return m_base == UnitBase::Meter;}
    UnitBase GetBase() const { return m_base; };
    UnitSystem GetSystem() const { return m_system; };
    double GetNumerator() const { return m_numerator; };
    double GetDenominator() const { return m_denominator; };
    WString GetLabel() const { return m_label; };
    WCharCP GetLabelCP() const { return m_label.c_str(); };
    double ToMeters(double val=1.0) const {return val / (GetNumerator()/GetDenominator());}
    double ToMillimeters(double val=1.0) const {return 1000.0 * ToMeters(val);}

    DGNPLATFORM_EXPORT void SetLabel (WCharCP val);
    void SetLabel (Utf8String val) {SetLabel(WString(val.c_str(), true).c_str());}

    //! Get the definition for a standard unit.
    DGNPLATFORM_EXPORT static UnitDefinition GetStandardUnit (StandardUnit iUnit);

    //! Find the definition of a standard unit from its name in the current language.
    DGNPLATFORM_EXPORT static UnitDefinition GetStandardUnitByName (WCharCP name);

    //! Determine if a unit definition is the same size as a standard unit.
    //! @return StandardUnit::None if the input does not match a standard unit.
    DGNPLATFORM_EXPORT StandardUnit IsStandardUnit () const;

    //! Get the label of a standard unit in the current language.
    DGNPLATFORM_EXPORT static WString GetStandardLabel (StandardUnit iUnit);

    //! Get the name of a standard unit in the current language.
    DGNPLATFORM_EXPORT static WString GetStandardName (StandardUnit iUnit, bool singular=false);

    //! Get the name of a unit from the user's workspace.
    DGNPLATFORM_EXPORT WString GetName (bool singular=false, bool alsoStandard=false) const;

    //! Get the number of a unit from the user's workspace.
    DGNPLATFORM_EXPORT int GetNumber (bool alsoStandard=false) const;

    //! Get the next larger unit from the user's workspace.  The returned unit will have the same base and system as the search unit.
    //! If a larger unit cannot be found, the search unit is returned.
    DGNPLATFORM_EXPORT UnitDefinition GetNextLarger () const;

    //! Get the next smaller unit from the user's workspace.  The returned unit will have the same base and system as the search unit.
    //! If a smaller unit cannot be found, the search unit is returned.
    DGNPLATFORM_EXPORT UnitDefinition GetNextSmaller () const;

    //! Find the definition of a unit from the user's workspace by matching its name in the current language.
    DGNPLATFORM_EXPORT static UnitDefinition GetByName (WCharCP unitName, bool alsoStandard=false);

    //! Find the definition of a unit from the user's workspace by matching the number assigned to it in the current session.
    DGNPLATFORM_EXPORT static UnitDefinition GetByNumber (int unitNumber, bool alsoStandard=false);

    //! Find the definition of a unit from the user's workspace by matching one of its labels.
    DGNPLATFORM_EXPORT static UnitDefinition GetByLabel (WCharCP label, bool alsoStandard=false);

//__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT static void    BackDoor_ResetUserUnitsForTest();

    DGNPLATFORM_EXPORT void FromJson(Json::Value const& inValue);
    DGNPLATFORM_EXPORT void ToJson (Json::Value& outValue) const;

//__PUBLISH_SECTION_START__
};

/** @cond BENTLEY_SDK_Internal */

//=======================================================================================
//! Used to specify the behavior of a UnitIterator.
//=======================================================================================
struct UnitIteratorOptions
{
//! Used by #SetSizeCriteria to filter units based on size.
enum UnitCompareMethod
    {
    SIZECOMPARE_AllowSmaller         = 0,
    SIZECOMPARE_AllowSmallerOrEqual  = 1,
    SIZECOMPARE_AllowLarger          = 2,
    SIZECOMPARE_AllowLargerOrEqual   = 3,
    };

private:
    bool                m_orderAscending;

    bool                m_allowBaseNone;
    bool                m_allowBaseMeter;
    bool                m_allowBaseDegree;

    bool                m_allowSystemNone;
    bool                m_allowSystemMetric;
    bool                m_allowSystemEnglish;
    bool                m_allowSystemUSSurvey;

    UnitDefinition      m_compareUnit;
    bool                m_allowLarger;
    bool                m_allowSmaller;
    bool                m_allowEqual;

    void    DisallowAllBases    ();
    void    DisallowAllSystems  ();

public:
//__PUBLISH_SECTION_END__
    bool IsOrderAscending () const { return m_orderAscending; }
    DGNPLATFORM_EXPORT bool IsBaseAllowed (UnitBase base) const;
    DGNPLATFORM_EXPORT bool IsSystemAllowed (UnitSystem system) const;
    UnitDefinitionCR GetCompareUnit () const { return m_compareUnit; };
    bool GetCompareAllowLarger () const { return m_allowLarger; }
    bool GetCompareAllowSmaller () const { return m_allowSmaller; }
    bool GetCompareAllowEqual () const { return m_allowEqual; }

//__PUBLISH_SECTION_START__
    //! Construct a default set of options that will iterate from smallest
    //! to largest and include all units in the list.
    DGNPLATFORM_EXPORT UnitIteratorOptions ();

    //! Set the iteration order from smallest to largest.
    DGNPLATFORM_EXPORT void SetOrderAscending ();

    //! Set the iteration order from largest to smallest.
    DGNPLATFORM_EXPORT void SetOrderDescending ();

    //! Set the iteration to include only the specified base.
    DGNPLATFORM_EXPORT void SetAllowSingleBase (UnitBase base);

    //! Set the iteration to include the specified base in addition to the ones already allowed.
    DGNPLATFORM_EXPORT void SetAllowAdditionalBase (UnitBase base);

    //! Set the iteration to include only the specified system.
    DGNPLATFORM_EXPORT void SetAllowSingleSystem (UnitSystem system);

    //! Set the iteration to include the specified system in addition to the ones already allowed.
    DGNPLATFORM_EXPORT void SetAllowAdditionalSystem (UnitSystem system);

    //! Set the iteration to filter units according to size.  Since size comparison
    //! is only valid within a single #UnitBase, this method will restrict the iteration
    //! to only those units with the same base as the specified unit.
    //! @param [in] unitDef  The unit on which to base the filtering.
    //! @param [in] method   The method used to compare candidate units to the specified unit.
    DGNPLATFORM_EXPORT void SetSizeCriteria (UnitDefinitionCR unitDef, UnitCompareMethod method);

}; // UnitIteratorOptions

struct StandardUnitTableEntry;
//=======================================================================================
//! Collection of standard units.  The standard units are hard coded in DgnPlatform.
//=======================================================================================
struct StandardUnitCollection
{
private:
    UnitIteratorOptionsCR   m_options;

public:
    struct const_iterator;

    //!   Intialize a collection with options
    //! @param options The options that control the iteration of this collection
    DGNPLATFORM_EXPORT StandardUnitCollection (UnitIteratorOptionsCR options);

    //=======================================================================================
    //! Entry in the StandardUnitCollection
    //=======================================================================================
    struct Entry
    {
    private:
        friend struct  StandardUnitCollection;
        StandardUnitTableEntry const *  m_tableP;
        friend struct  const_iterator;

    public:
        //! Get information about the iterator's current unit.
        DGNPLATFORM_EXPORT UnitDefinition GetUnitDef () const;

        //! Get information about the iterator's current unit.
        DGNPLATFORM_EXPORT StandardUnit GetNumber () const;

        //! Get information about the iterator's current unit.
        DGNPLATFORM_EXPORT WString GetName (bool singular=false) const;
    };

    //=======================================================================================
    //! Iterator over the StandardUnitCollection
    //=======================================================================================
    struct const_iterator : std::iterator<std::forward_iterator_tag, Entry const>
    {
    private:
        friend struct StandardUnitCollection;

        UnitIteratorOptionsCP           m_options;
        Entry                           m_entry;

        bool UnitValidForIterator (StandardUnitTableEntry const* tableEntry);

        const_iterator (UnitIteratorOptionsCR options);
        const_iterator ();

    public:
        //! Advances the iterator to the next in collection.
        DGNPLATFORM_EXPORT const_iterator&  operator ++();

        //! Returns the unit value.
        DGNPLATFORM_EXPORT Entry const& operator *() const;

        //! Tests if the iterator object on the left side of the operator is not equal to the iterator object on the right side.
        DGNPLATFORM_EXPORT bool operator != (const_iterator const &) const;
        //! Tests if the iterator object on the left side of the operator is equal to the iterator object on the right side.
        DGNPLATFORM_EXPORT bool operator == (const_iterator const & rhs) const {return !(*this != rhs);}

    }; // UnitIterator

    typedef const_iterator iterator;    //!< only const iteration is possible

    //! Returns an iterator addressing the first element in the collection
    DGNPLATFORM_EXPORT const_iterator begin () const;

    //! Returns an iterator that addresses the location succeeding the last element in collection
    DGNPLATFORM_EXPORT const_iterator end () const;
};

struct UserUnitTableEntry;
//=======================================================================================
//! Collection of user units.  The user units are available to the user interface.
//=======================================================================================
struct UserUnitCollection
{
private:
    UnitIteratorOptionsCR   m_options;

    static bool IndexIsValid (size_t tableIndex);

public:
    struct const_iterator;

    //! Intialize a collection with options
    //! @param options The options that control the iteration of this collection
    DGNPLATFORM_EXPORT UserUnitCollection (UnitIteratorOptionsCR options);

    //======================================================================================
    //! Entry in the UserUnitCollection
    //======================================================================================
    struct Entry
    {
    private:
        friend struct  UserUnitCollection;
        size_t  m_index;
        friend struct  const_iterator;

    public:
        //! Get information about the iterator's current unit.
        DGNPLATFORM_EXPORT UnitDefinition GetUnitDef() const;

        //! Get information about the iterator's current unit.
        DGNPLATFORM_EXPORT StandardUnit GetNumber() const;

        //! Get information about the iterator's current unit.
        DGNPLATFORM_EXPORT WString GetName(bool singular=false) const;
    };

    //=======================================================================================
    //! Iterator over the UserUnitCollection
    //======================================================================================
    struct const_iterator : std::iterator<std::forward_iterator_tag, Entry>
    {
    private:
        friend struct UserUnitCollection;

        UnitIteratorOptionsCP           m_options;
        Entry                           m_entry;

        bool    UnitValidForIterator (size_t tableIndex);

        const_iterator (UnitIteratorOptionsCR options);
        const_iterator (bool ascending);

    public:
        //! Advances the iterator to the next in collection.
        DGNPLATFORM_EXPORT const_iterator&  operator ++();

        //! Returns the unit value.
        DGNPLATFORM_EXPORT Entry operator *() const;

        //! Tests if the iterator object on the left side of the operator is not equal to the iterator object on the right side.
        DGNPLATFORM_EXPORT bool operator != (const_iterator const &) const;
        //! Tests if the iterator object on the left side of the operator is equal to the iterator object on the right side.
        DGNPLATFORM_EXPORT bool operator == (const_iterator const & rhs) const {return !(*this != rhs);}

    }; // UnitIterator

    //!  Returns an iterator addressing the first element in the collection
    DGNPLATFORM_EXPORT const_iterator begin () const;

    //!  Returns an iterator that addresses the location succeeding the last element in collection
    DGNPLATFORM_EXPORT const_iterator end () const;
};

/** @endcond */

END_BENTLEY_DGN_NAMESPACE
