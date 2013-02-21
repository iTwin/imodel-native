/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/StandardCustomAttributeHelper.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/NonCopyableClass.h>
#include <Bentley/DateTime.h>
#include <ECObjects/ECObjects.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
//=======================================================================================    
//! DateTimeInfo contains the meta data that can be assigned to an ECProperty of type 
//! 'DateTime'.
//! @bsiclass
//=======================================================================================    
struct DateTimeInfo
    {
private: 
    bool m_isKindNull;
    bool m_isComponentNull;
    DateTime::Info m_info;

    static const DateTime::Kind DEFAULT_KIND;
    static const DateTime::Component DEFAULT_COMPONENT;
    static const DateTime::Info s_default;

public:
    //***** Construction ******
//__PUBLISH_SECTION_END__
    //Intentionally use the compiler-generated versions of copy constructor, assignment operator, and destructor
//__PUBLISH_SECTION_START__

    //! Initializes a new instance of the DateTimeInfo type.
    ECOBJECTS_EXPORT DateTimeInfo ();
    DateTimeInfo (bool isKindNull, DateTime::Kind kind, bool isComponentNull, DateTime::Component component); 

    ECOBJECTS_EXPORT bool IsKindNull () const;
    ECOBJECTS_EXPORT bool IsComponentNull () const;
    ECOBJECTS_EXPORT DateTime::Info const& GetInfo () const;

    ECOBJECTS_EXPORT DateTime::Info GetInfo (bool useDefaultIfUnset) const; 
    ECOBJECTS_EXPORT static DateTime::Info const& GetDefault (); 

    //! Checks whether the RHS object matches this object.
    //! @remarks If one of the members
    //!          of this object is null, the RHS counterpart is ignored and the
    //!          members are considered matching.
    //! @param[in] rhs RHS
    //! @return true, if the RHS matches this object. false otherwise
    ECOBJECTS_EXPORT bool IsMatchedBy (DateTime::Info const& rhs) const;

    //! Generates a text representation of this object.
    //! @return Text representation of this object
    ECOBJECTS_EXPORT WString ToString () const;
    };

//=======================================================================================    
//! StandardCustomAttributeHelper provides APIs to access items of the Bentley standard schemas
//! @bsiclass
//=======================================================================================    
struct StandardCustomAttributeHelper : NonCopyableClass
    {
private:
    //static class
    StandardCustomAttributeHelper ();
    ~StandardCustomAttributeHelper ();

public:
    //! Retrieves the DateTimeInfo meta data from the specified DateTime ECProperty.
    //! @param[out] dateTimeInfo the retrieved DateTimeInfo meta data
    //! @param[in] dateTimeProperty the DateTime ECProperty from which the meta data is to be retrieved
    //! @return true if the ECProperty contains the DateTimeInfo meta data, false if the
    //!         ECProperty doesn't contain the DateTimeInfo meta data or in case of errors.
    ECOBJECTS_EXPORT static bool TryGetDateTimeInfo (DateTimeInfo& dateTimeInfo, ECPropertyCR dateTimeProperty);
    };

END_BENTLEY_ECOBJECT_NAMESPACE

//__PUBLISH_SECTION_END__
