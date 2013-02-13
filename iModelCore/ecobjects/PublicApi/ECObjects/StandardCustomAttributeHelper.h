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

public:
    //***** Construction ******
//__PUBLISH_SECTION_END__
    //Intentionally use the compiler-generated versions of copy constructor, assignment operator, and destructor
//__PUBLISH_SECTION_START__

    DateTimeInfo () : m_isKindNull (true), m_isComponentNull (true) {}
    DateTimeInfo (bool isKindNull, DateTime::Kind kind, bool isComponentNull, DateTime::Component component) 
        : m_isKindNull (isKindNull), m_isComponentNull (isComponentNull), m_info (kind, component) {}

    bool IsKindNull () const {return m_isKindNull;}
    bool IsComponentNull () const {return m_isComponentNull;}
    DateTime::Info const& GetInfo () const {return m_info;}
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
