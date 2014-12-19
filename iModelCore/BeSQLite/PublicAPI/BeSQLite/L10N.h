/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeSQLite/L10N.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeSQLite/BeSQLite.h>

// This macro defines a struct called STRUCT_NAME that embeds an enum called STRUCT_NAME::Number. 
// The struct has a GetString function that calls L10N::GetString using the specified NAMESPACE_NAME.
// The struct also has a GetNameSpace function that returns NAMESPACE_NAME as a constant string.
// The CHeaderToXLiff.py program recognizes this macro and knows how to extract messages from the enumerands
// that follow it.
#define BENTLEY_TRANSLATABLE_STRINGS_START(STRUCT_NAME,NAMESPACE_NAME)\
    struct STRUCT_NAME\
        {\
        enum Number : Int32;\
        static Utf8CP     GetNameSpace() {return #NAMESPACE_NAME;}\
        static Utf8String GetString  (Number id) {return BeSQLite::L10N::GetString (GetNameSpace(), id);}\
        static WString    GetStringW (Number id) {return WString (GetString(id).c_str(), BentleyCharEncoding::Utf8);}\
        \
        enum Number : Int32

// Indicates the end of a translatable string enum
#define BENTLEY_TRANSLATABLE_STRINGS_END };

// This macro defines a struct called STRUCT_NAME and an enum called ENUM_NAME. The enum is not embedded in the struct.
// The struct has a GetString function that calls L10N::GetString using the specified NAMESPACE_NAME.
// The struct also has a GetNameSpace function that returns NAMESPACE_NAME as a constant string.
// The CHeaderToXLiff.py program recognizes this macro and knows how to extract messages from the enumerands
// that follow it.
#define BENTLEY_TRANSLATABLE_STRINGS2_START(ENUM_NAME,STRUCT_NAME,NAMESPACE_NAME)\
    enum ENUM_NAME : Int32;\
    struct STRUCT_NAME\
        {\
        static Utf8CP     GetNameSpace() {return #NAMESPACE_NAME;}\
        static Utf8String GetString  (ENUM_NAME id) {return BeSQLite::L10N::GetString (GetNameSpace(), id);}\
        static WString    GetStringW (ENUM_NAME id) {return WString (GetString(id).c_str(), BentleyCharEncoding::Utf8);}\
        };\
    enum ENUM_NAME : Int32

#define BENTLEY_TRANSLATABLE_STRINGS2_END

BEGIN_BENTLEY_SQLITE_NAMESPACE

//=======================================================================================
//! %L10N (chic numeronym for "localization") is used to obtain localized strings from one of 3 SQLang database files. 
//! A SQLang database file is a SQLite3 file that has a table named "l10n_strings" with the following specification:
//! \code
//!   CREATE TABLE l10n_strings (Namespace CHAR, ResName CHAR, Id INTEGER NOT NULL, Value CHAR, PRIMARY KEY (NameSpace,Id),UNIQUE (Namespace,ResName))
//! \endcode
//! Strings can be retrieved by either Namespace/ResName or Namespace/Id, both of which are unique.
//! The %L10N service holds up to 3 SQLang files open: A "culture specific" file that has the strings localized in the native language of the 
//! user. If the culture-specific file is not present, or if a string is not found in it, then the "culture neutral" (e.g. localized for 
//! "French" vs. "French Canadian") SQLang file is used. If the string is not found in either of the localized databases, 
//! the "default" (usually English) SQLang file is used.
//=======================================================================================
struct L10N
{
    //! Set of 1 to 3 SQLang database files.
    //! @ingroup L10NGroup
    struct SqlangFiles
    {
        BeFileName  m_default;
        BeFileName  m_cultureNeutral;
        BeFileName  m_cultureSpecific;

        //! @param[in] defaultSqlangFile The name of the SQLang database file to be used as a fallback in the case where a string cannot be found in 
        //! the culture specific or culture neutral files. Usually this is the English strings database. This parameter 
        //! should specify a valid SQLang database file.
        //! @param[in] cultureNeutral The name of the SQLang database file with culture-neutral strings. This file is used as a fallback when
        //! strings cannot be found in the culture-specific file. If this parameter doesn't specify a valid SQLang database, string
        //! lookups will fall back to the default database.
        //! @param[in] cultureSpecific The name of the SQLang database file with culture-specifc strings. If this parameter
        //! doesn't specify a valid SQLang database, string lookups will fall back to the culture-neutral database.
        SqlangFiles (BeFileName defaultSqlangFile, BeFileName cultureNeutral=BeFileName(NULL), BeFileName cultureSpecific=BeFileName(NULL)) : 
                m_default(defaultSqlangFile), m_cultureNeutral(cultureNeutral), m_cultureSpecific(cultureSpecific) {}
    };

    //! Initialize the L10N service by supplying the names of the SQLang files to be used for localized strings. This method
    //! *must be called once* before attempting to perform any string lookups. Subsequent calls are ignored until #Shutdown is called.
    //! @note More common to override DgnPlatformLib::Host::_SupplySqlangFiles than to call this method directly.
    //! @private
    BE_SQLITE_EXPORT static BentleyStatus Initialize(SqlangFiles const& files);

    //! The Suspend and Resume calls can be used to close and re-open the database files that store the translations.
    //! This is required on iOS 8 when the app suspends and resumes; it is optional elsewhere. On iOS 8 using older hardware (pre-iPad Air), the *.sqlang.db3 files in the app bundle are, for an unknown reason, considred "system" files, and your app is terminated if they're left open while suspended.
    BE_SQLITE_EXPORT static void Suspend();

    //! The Suspend and Resume calls can be used to close and re-open the database files that store the translations.
    //! This is required on iOS 8 when the app suspends and resumes; it is optional elsewhere. On iOS 8 using older hardware (pre-iPad Air), the *.sqlang.db3 files in the app bundle are, for an unknown reason, considred "system" files, and your app is terminated if they're left open while suspended.
    BE_SQLITE_EXPORT static void Resume();

/** @cond BENTLEY_SDK_Publisher */
    //! Close all of the SQLang databases that are currently opened. No L10N services may be called after this method until another call to L10N::Initialize
    BE_SQLITE_EXPORT static void Shutdown();
  
    //! Retrieve a localized string by Namespace and Id.
    //! @param[in] nameSpace the namespace of the string.
    //! @param[in] id the Id of the string.
    //! @param[out] hasString true if the string was successfully found in one of the SQLang files. This is necessary only if you wish to differentiate
    //! between the return of a string whose value is blank and a string that was not found. May be NULL. There is no way to determine from which 
    //! database the string was found.
    BE_SQLITE_EXPORT static Utf8String GetString(Utf8CP nameSpace, int id, bool* hasString=NULL);

    //! Retrieve a localized string by Namespace and Resource Name.
    //! @param[in] nameSpace the namespace of the string.
    //! @param[in] resourceName the Resource Name of the string.
    //! @param[out] hasString true if the string was successfully found in one of the SQLang files. This is necessary only if you wish to differentiate
    //! between the return of a string whose value is blank and a string that was not found. May be NULL. There is no way to determine from which 
    //! database the string was found.
    BE_SQLITE_EXPORT static Utf8String GetString(Utf8CP nameSpace, Utf8CP resourceName, bool* hasString=NULL);

    //! Shortcut to determine whether a string can be found in the L10N databases by NameSpace and Id.
    static bool HasString (Utf8CP nameSpace, int id) {bool hasString; GetString (nameSpace, id, &hasString); return hasString;}

    //! Shortcut to determine whether a string can be found in the L10N databases by NameSpace and Resource Name.
    static bool HasString (Utf8CP nameSpace, Utf8CP resourceName) {bool hasString; GetString (nameSpace, resourceName, &hasString); return hasString;}
/** @endcond */
};

//__PUBLISH_SECTION_END__
//=======================================================================================
//! L10NLookup maintains one to three sqlang db3 connections that are used for looking up localized strings.
// @bsiclass
//=======================================================================================
struct L10NLookup
{
private:
    struct SQLangDb : Db
        {
        DECLARE_KEY_METHOD
        Utf8String m_fileName;
        SQLangDb (BeFileNameCR filename) : m_fileName(filename) {}
        DbResult Open();
        bool HasName() {return m_fileName.length() != 0;}
        Utf8String GetString(Utf8CP scope, int id, bool& hasString);
        Utf8String GetString(Utf8CP scope, Utf8CP name, bool& hasString);
        };

    bool       m_initialized;
    SQLangDb   m_lastResortDb;
    SQLangDb   m_cultureNeutralDb;
    SQLangDb   m_cultureSpecificDb;
    void Initialize();

public: 
    BE_SQLITE_EXPORT L10NLookup(L10N::SqlangFiles const&);
    BE_SQLITE_EXPORT void Suspend();
    BE_SQLITE_EXPORT void Resume();
    BE_SQLITE_EXPORT Utf8String GetString(Utf8CP scope, int id, bool* hasString=NULL);
    BE_SQLITE_EXPORT Utf8String GetString(Utf8CP scope, Utf8CP name, bool* hasString=NULL);

    bool HasString (Utf8CP scope, Utf8CP name) {bool hasString; GetString (scope, name, &hasString); return hasString;}
    bool HasString (Utf8CP scope, int id) {bool hasString; GetString (scope, id, &hasString); return hasString;}
};

//__PUBLISH_SECTION_START__
END_BENTLEY_SQLITE_NAMESPACE
