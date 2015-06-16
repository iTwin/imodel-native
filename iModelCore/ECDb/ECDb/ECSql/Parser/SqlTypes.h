/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/Parser/SqlTypes.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <Bentley/Bentley.h>
#include <stdlib.h>
#include "TextEncoding.h"

#define OOO_DLLPUBLIC_DBTOOLS 
////////////////////////OSL//////////////////////////////////////////////////
//#if OSL_DEBUG_LEVEL > 0
//
//#define _OSL_DEBUG_ONLY(f)    (f)
//#define _OSL_ASSERT(c, f, l) \
//    do \
//    {  \
//        if (!(c) && _OSL_GLOBAL osl_assertFailedLine(f, l, 0)) \
//            _OSL_GLOBAL osl_breakDebug(); \
//    } while (0)
//
//#define _OSL_ENSURE(c, f, l, m) \
//    do \
//    {  \
//        if (!(c) && _OSL_GLOBAL osl_assertFailedLine(f, l, m)) \
//            _OSL_GLOBAL osl_breakDebug(); \
//    } while (0)
//
//#else
//
//#define _OSL_DEBUG_ONLY(f)            ((void)0)
//#define _OSL_ASSERT(c, f, l)        ((void)0)
//#define _OSL_ENSURE(c, f, l, m)        ((void)0)
#define OSL_DEBUG_LEVEL 3
#define OSL_ASSERT(X) BeAssert( X)
//#define OSL_FAIL(X, Y) BeAssert( X && Y);
#define OSL_FAIL(X) BeAssert( false && X);

#define OSL_ENSURE(X, Y) BeAssert( X && Y);
#define OSL_VERIFY(c) do { if (!(c)) OSL_ASSERT(0); } while (0)
#define OSL_PRECOND(c, m)      OSL_ENSURE(c, m)
#define OSL_POSTCOND(c, m)     OSL_ENSURE(c, m)

#define RTL_LOGFILE_CONTEXT( instance, name )  ((void)0)
#define RTL_LOGFILE_CONTEXT_AUTHOR( instance, project, author, name )  ((void)0)
#define RTL_LOGFILE_CONTEXT_TRACE( instance, message )  ((void)0)
#define RTL_LOGFILE_CONTEXT_TRACE1( instance, frmt, arg1 ) ((void)0)
#define RTL_LOGFILE_CONTEXT_TRACE2( instance, frmt, arg1, arg2 ) ((void)0)
#define RTL_LOGFILE_CONTEXT_TRACE3( instance, frmt, arg1, arg2 , arg3 ) ((void)0)
#define OSL_TRACE(X, Y) 
#define TODO_ConvertCode() BeAssert(false && "Todo"); 
#define DBG_UNHANDLED_EXCEPTION()
//#define RTL_CONSTASCII_USTRINGPARAM( constAsciiStr ) constAsciiStr, ((sal_Int32)(sizeof(constAsciiStr)-1)), RTL_TEXTENCODING_ASCII_US
#define RTL_CONSTASCII_USTRINGPARAM( constAsciiStr ) constAsciiStr, BentleyCharEncoding::Utf8
////////////////
/** Enum defining UNO_QUERY and UNO_REF_QUERY for implicit interface query.
*/
enum UnoReference_Query
    {
    /** This enum value can be used for implicit interface query.
    */
    UNO_QUERY,
    /** This enum value can be used for implicit interface query.
    */
    UNO_REF_QUERY
    };
/** Enum defining UNO_QUERY_THROW and UNO_REF_QUERY_THROW for implicit interface query.
    If the demanded interface is unavailable, then a RuntimeException is thrown.
    */
enum UnoReference_QueryThrow
    {
    /** This enum value can be used for implicit interface query.
    */
    UNO_QUERY_THROW,
    /** This enum value can be used for implicit interface query.
    */
    UNO_REF_QUERY_THROW
    };
/** Enum defining UNO_SET_THROW for throwing if attempts are made to assign a <NULL/>
    interface

    @since UDK 3.2.8
    */
enum UnoReference_SetThrow
    {
    UNO_SET_THROW
    };
/////////////////////////SAL////////////////////////////////////////////////
#define SAL_CALL
#define SAL_THROW(X) 
typedef  int sal_Int32;
typedef  bool sal_Bool;
typedef  Utf8Char sal_Char;
typedef  WChar sal_Unicode;
typedef  uint32_t sal_uInt32;

typedef int16_t sal_Int16;
typedef  uint16_t sal_uInt16;
const sal_Bool sal_True = true;
const sal_Bool sal_False = false;
#define SAL_MAX_ENUM 0x7fffffff
///////////////////////////RTL/////////////////////////////////////////////


struct Utf8StringBuffer
    {
    private:
        Utf8String m_buffer;
    public:
        Utf8StringBuffer (size_t reserve)
            {
            m_buffer.reserve (reserve);
            }
        Utf8StringBuffer (Utf8StringCR str)
            {
            m_buffer = str;
            }

        Utf8StringBuffer ()
            {
            }
        size_t size () const { return m_buffer.size (); }

        Utf8String makeStringAndClear ()
            {
            Utf8String tmp = m_buffer;
            m_buffer.clear ();
            return tmp;
            }
        void append (Utf8Char c)
            {
            m_buffer.append (&c, 1);
            }
        void append (Utf8StringCR str)
            {
            m_buffer.append (str);
            }

        void operator = (Utf8StringCR str)
            {
            m_buffer = str;
            }
        void appendInt32 (sal_uInt32 i)
            {
            Utf8String n;
            n.Sprintf ("%d ", i);
            m_buffer.append (n);
            }
        void appendAscii (Utf8CP str, sal_Int32 len)
            {
            BeAssert (strlen (str) != len);
            m_buffer.append (str);
            }
        void appendAscii (Utf8CP c)
            {
            m_buffer.append (c);
            }

        sal_Char charAt (size_t index)
            {
            return m_buffer[index];
            }
        void setCharAt (size_t index, sal_Char c)
            {
            BeAssert (m_buffer.size () > index && index >= 0);
            m_buffer[index] = c;
            }
    };

//struct WStringBuffer
//{
//private:
//    WString m_buffer;
//public:
//    WStringBuffer(WStringCR str)
//        : m_buffer(str)
//        {
//        }
//    WStringBuffer(size_t reserve)
//        {
//        m_buffer.reserve(reserve);
//        }
//    WStringBuffer()
//        {
//        }
//    WString makeStringAndClear()
//        {
//        WString tmp = m_buffer;
//        m_buffer.clear();
//        return tmp;
//        }
//    size_t size() const { return m_buffer.size();}
//    void append(WChar c)
//        {        
//        m_buffer.append (&c, 1);
//        }
//    void append(WStringCR str)
//        {
//        m_buffer.append (str);
//        }
//    void appendAscii(Utf8CP c)
//        {
//        m_buffer.AppendUtf8(c);
//        }
//    void operator = (WStringCR str)
//        {
//        m_buffer = str;
//        }
//    void operator = (Utf8StringCR str)
//        {
//        m_buffer.AssignUtf8(str.c_str());
//        }
//    void appendAscii(Utf8CP str, sal_Int32 len)
//        {
//        BeAssert( strlen(str) != len);
//        m_buffer.AppendUtf8(str);
//        }
//    sal_Unicode charAt(size_t index)
//        {
//        return m_buffer[index];
//        }
//    void setCharAt( size_t index, sal_Unicode c)
//        {
//        BeAssert (m_buffer.size() > index && index >=  0);
//        m_buffer[index] = c;
//        }
//};
//struct ECSqlStringHelper
//    {
//    public :
//        static sal_Unicode toChar(WStringCR s)
//            {
//            if(s.empty())
//                return 0;
//            return s[0];
//            }
//    };
struct Utf8StringHelper
    {
    static sal_Char toChar (Utf8StringCR s)
        {
        if (s.empty ())
            return 0;
        return s[0];
        }
    static Utf8String createFromAscii (const char * ascii)
        {
        return Utf8String (ascii);
        }
    static bool compareToAscii (Utf8StringCR str, const char * ascii)
        {
        return str.Equals (createFromAscii (ascii));
        }
    static Utf8String createString (const char* str)
        {
        return Utf8String (str);
        }

    static Utf8String createString (const char* str, size_t len, int16_t encoding)
        {
        BeAssert (encoding != RTL_TEXTENCODING_UTF8);
        BeAssert (strlen (str) != len);
        return Utf8String (str);
        }
    //static Utf8String createString(const WString& str, size_t len, Int16 encoding)
    //    {
    //    BeAssert(encoding != RTL_TEXTENCODING_UTF8);
    //    BeAssert(str.size() != len);
    //    return Utf8String(str);
    //    }
    static Utf8String replace (Utf8StringCR str, sal_Char findChar, sal_Char replaceWith)
        {
        BeAssert (false && "Implement this");
        Utf8Char a[2] = {findChar, '\0'};
        Utf8Char b[2] = {replaceWith, '\0'};
        Utf8String aResult (str);
        aResult.ReplaceAll (a, b);
        return aResult;
        }

    };
//struct WStringHelper
//    {
//    public :
//        static sal_Unicode toChar(WStringCR s)
//            {
//            if(s.empty())
//                return 0;
//            return s[0];
//            }
//        static WString createFromAscii(const char * ascii)
//            {
//            return WString(ascii, BentleyCharEncoding::Utf8);
//            }
//        static bool compareToAscii(WStringCR str, const char * ascii)
//            {
//            return str.Equals(createFromAscii(ascii));
//            }
//        static WString createString(const char* str, size_t len, Int16 encoding)
//            {
//            //if (encoding != RTL_TEXTENCODING_UTF8)
//            //    {
//            //    printf("");
//            //    }
//            //BeAssert(encoding != RTL_TEXTENCODING_UTF8);
//
//            return WString(str, BentleyCharEncoding::Utf8).substr(0, len);
//            }
//        static WString createString(const Utf8String& str, size_t len, Int16 encoding)
//            {
//            BeAssert(encoding != RTL_TEXTENCODING_UTF8);
//            BeAssert(str.size() != len);
//            return WString(str.c_str(), BentleyCharEncoding::Utf8);
//            }
//        static WString replace(WStringCR str, sal_Char findChar, sal_Char replaceWith)
//            {
//            BeAssert(false && "Implement this");
//            Utf8Char a[2] ={findChar, '\0'};
//            Utf8Char b[2] ={replaceWith, '\0'};
//            Utf8String aResult (str);
//            aResult.ReplaceAll(a, b);
//            return WString(aResult.c_str(), BentleyCharEncoding::Utf8);
//            }
//    };
namespace com
    {
    namespace sun
        {
        namespace star
            {
            struct Exception
                {
                };
            namespace container
                {
                struct XNameAccess : RefCountedBase
                    {
                    };
                }
            namespace i18n
                {
                struct XCharacterClassification : RefCountedBase
                    {
                    };
                struct XLocaleData : RefCountedBase
                    {
                    // getLocaleItem()
                    };
                }
            namespace lang
                {
                struct Locale : RefCountedBase
                    {
                    public:
                        Locale (){}
                        Locale (Utf8StringCR language, Utf8StringCR country, Utf8StringCR avariant){}
                    };
                struct XMultiServiceFactory : RefCountedBase
                    {
                    public:
                        static RefCountedPtr<XMultiServiceFactory> CreateInstance ()
                            {
                            return new XMultiServiceFactory ();
                            }
                    };
                }
            namespace beans
                {
                struct XPropertySetInfo : RefCountedBase
                    {
                    };
                struct XPropertySet : RefCountedBase
                    {
                    public:
                        RefCountedPtr<XPropertySetInfo> getPropertySetInfo (){ return NULL; }
                    };

                }
            namespace uno
                {
                struct Any : RefCountedBase
                    {
                    };
                }
            namespace util
                {
                struct XNumberFormatter : RefCountedBase
                    {
                    public:
                        sal_Int32 convertStringToNumber (sal_Int32 _nKey, Utf8StringCR _sValue){ BeAssert (false); return 0; }
                    };
                }
            namespace sdbc
                {
                struct XConnection : RefCountedBase
                    {
                    };
                struct XDatabaseMetaData : RefCountedBase
                    {
                    public:
                        bool supportsCatalogsInDataManipulation () const { return false; }
                        bool supportsSchemasInDataManipulation () const { return false; }
                    };
                struct SQLException : RefCountedBase
                    {
                    };
                }
            }
        }
    }
//namespace osl 
//    {
//    struct Mutex
//        {
//        };
//    struct MutexGuard
//        {
//        public:
//            MutexGuard(Mutex& mutex)
//                {
//                }
//        };
//    }
namespace comphelper
    {
    struct ComponentContext
        {
        };
    }

namespace dbtools
    {
    struct DatabaseMetaData
        {
        public:
            DatabaseMetaData (
                const RefCountedPtr< ::com::sun::star::sdbc::XConnection >& _connection){}
            DatabaseMetaData (const DatabaseMetaData& _copyFrom){}
            DatabaseMetaData& operator=(const DatabaseMetaData& _copyFrom){ return *this; }
            /** determines whether or not the instances is based on a valid connection

                As long as this method returns true<TRUE/>, you should expect all other
                methods throwing an SQLException when called.
                */
            bool    isConnected () const;

            /** resets the instance so that it's based on a new connection
            */
            inline  void    reset (const RefCountedPtr< ::com::sun::star::sdbc::XConnection >& _connection)
                {
                *this = DatabaseMetaData (_connection);
                }

            /// wraps XDatabaseMetaData::getIdentifierQuoteString
            const Utf8String  getIdentifierQuoteString () const{ return "'"; }

            /// wraps XDatabaseMetaData::getCatalogSeparator
            const Utf8String  getCatalogSeparator () const{ return "."; }

            /** determines whether the database supports sub queries in the FROM part
                of a SELECT clause are supported.
                @throws ::com::sun::star::sdbc::SQLException
                with SQLState 08003 (connection does not exist) if the instances was
                default-constructed and does not have a connection, yet.
                */
            bool supportsSubqueriesInFrom () const{ return true; }

            /** checks whether the database supports primary keys

                Since there's no dedicated API to ask a database for this, a heuristics needs to be applied.
                First, the <code>PrimaryKeySupport<code> settings of the data source is examined. If it is <TRUE/>
                or <FALSE/>, then value is returned. If it is <NULL/>, then the database meta data are examined
                for support of core SQL grammar, and the result is returned. The assumption is that a database/driver
                which supports core SQL grammar usually also supports primary keys, and vice versa. At least, experience
                shows this is true most of the time.
                */
            bool supportsPrimaryKeys () const{ return true; }

            /** determines whether names in the database should be restricted to SQL-92 identifiers

                Effectively, this method checks the EnableSQL92Check property of the data source settings,
                if present.
                */
            bool restrictIdentifiersToSQL92 () const{ return false; }

            /** determines whether when generating SQL statements, an AS keyword should be generated
                before a correlation name.

                E.g., it determines whether <code>SELECT * FROM table AS correlation_name</code> or
                <code>SELECT * FROM table correlation_name</code> is generated.
                */
            bool generateASBeforeCorrelationName () const{ return true; }

            /** should date time be escaped like '2001-01-01' => #2001-01-01#
            */
            bool shouldEscapeDateTime () const{ return true; }

            /** auto increment columns should be automaticly used as primary key.
            */
            bool isAutoIncrementPrimaryKey () const{ return true; }

            /** determines the syntax to use for boolean comparison predicates

                @see ::com::sun::star::sdb::BooleanComparisonMode
                */
            sal_Int32
                getBooleanComparisonMode () const{ return true; }

            /** determines in relations are supported.
            *
            * \return <TRUE/> when relations are supported, otherwise <FALSE/>
            */
            bool supportsRelations () const{ return true; }

            /** determines if column alias names can be used in the order by clause.
            *
            * \return <TRUE/> when relations are supported, otherwise <FALSE/>
            */
            bool supportsColumnAliasInOrderBy () const { return true; }

            /** determines whether user administration is supported for the database

                User administration support is controlled by the availability of the XUsersSupplier
                interface, and it returning a non-NULL users container.

                @param _rContext
                the component context we operate in. Might be needed to create the
                css.sdbc.DriverManager instance.
                */
            bool    supportsUserAdministration (const ::comphelper::ComponentContext& _rContext) const { return true; }

            /** determines whether in the application UI, empty table folders (aka catalogs/schemas) should be displayed
            */
            bool displayEmptyTableFolders () const{ return false; }

            /** determines that threads are supported.
            *
            * \return <TRUE/> when threads are supported, otherwise <FALSE/>
            */
            bool supportsThreads () const { return false; }
        };
    struct DBTypeConversion
        {
        };

    //----------------------------------------------------------------------------------
    /** standard SQLStates to be used with an SQLException

        Extend this list whenever you need a new state ...

        @see http://msdn.microsoft.com/library/default.asp?url=/library/en-us/odbc/htm/odbcodbc_error_codes.asp
        */
    enum StandardSQLState
        {
        SQL_WRONG_PARAMETER_NUMBER,     // 07001
        SQL_INVALID_DESCRIPTOR_INDEX,   // 07009
        SQL_UNABLE_TO_CONNECT,          // 08001
        SQL_NUMERIC_OUT_OF_RANGE,       // 22003
        SQL_INVALID_DATE_TIME,          // 22007
        SQL_INVALID_CURSOR_STATE,       // 24000
        SQL_TABLE_OR_VIEW_EXISTS,       // 42S01
        SQL_TABLE_OR_VIEW_NOT_FOUND,    // 42S02
        SQL_INDEX_ESISTS,               // 42S11
        SQL_INDEX_NOT_FOUND,            // 42S12
        SQL_COLUMN_EXISTS,              // 42S21
        SQL_COLUMN_NOT_FOUND,           // 42S22
        SQL_GENERAL_ERROR,              // HY000
        SQL_INVALID_SQL_DATA_TYPE,      // HY004
        SQL_OPERATION_CANCELED,         // HY008
        SQL_FUNCTION_SEQUENCE_ERROR,    // HY010
        SQL_INVALID_CURSOR_POSITION,    // HY109
        SQL_INVALID_BOOKMARK_VALUE,     // HY111
        SQL_FEATURE_NOT_IMPLEMENTED,    // HYC00
        SQL_FUNCTION_NOT_SUPPORTED,     // IM001
        SQL_CONNECTION_DOES_NOT_EXIST,  // 08003

        SQL_ERROR_UNSPECIFIED = SAL_MAX_ENUM    // special value indicating that an SQLState is not to be specified
        };

    }

struct XQueriesSupplier : public RefCountedBase
    {
    public:
        XQueriesSupplier (const RefCountedPtr< ::com::sun::star::sdbc::XConnection >& _rxConnection, UnoReference_Query q) {}
        com::sun::star::container::XNameAccess getQueries () { return com::sun::star::container::XNameAccess (); };
    };

#define PROPERTY_ID_QUERYTIMEOUT                    1
#define PROPERTY_ID_MAXFIELDSIZE                    2
#define PROPERTY_ID_MAXROWS                         3
#define PROPERTY_ID_CURSORNAME                      4
#define PROPERTY_ID_RESULTSETCONCURRENCY            5
#define PROPERTY_ID_RESULTSETTYPE                   6
#define PROPERTY_ID_FETCHDIRECTION                  7
#define PROPERTY_ID_FETCHSIZE                       8
#define PROPERTY_ID_ESCAPEPROCESSING                9
#define PROPERTY_ID_USEBOOKMARKS                    10
// Column
#define PROPERTY_ID_NAME                            11
#define PROPERTY_ID_TYPE                            12
#define PROPERTY_ID_TYPENAME                        13
#define PROPERTY_ID_PRECISION                       14
#define PROPERTY_ID_SCALE                           15
#define PROPERTY_ID_ISNULLABLE                      16
#define PROPERTY_ID_ISAUTOINCREMENT                 17
#define PROPERTY_ID_ISROWVERSION                    18
#define PROPERTY_ID_DESCRIPTION                     19
#define PROPERTY_ID_DEFAULTVALUE                    20

#define PROPERTY_ID_REFERENCEDTABLE                 21
#define PROPERTY_ID_UPDATERULE                      22
#define PROPERTY_ID_DELETERULE                      23
#define PROPERTY_ID_CATALOG                         24
#define PROPERTY_ID_ISUNIQUE                        25
#define PROPERTY_ID_ISPRIMARYKEYINDEX               26
#define PROPERTY_ID_ISCLUSTERED                     27
#define PROPERTY_ID_ISASCENDING                     28
#define PROPERTY_ID_SCHEMANAME                      29
#define PROPERTY_ID_CATALOGNAME                     30

#define PROPERTY_ID_COMMAND                         31
#define PROPERTY_ID_CHECKOPTION                     32
#define PROPERTY_ID_PASSWORD                        33
#define PROPERTY_ID_RELATEDCOLUMN                   34

#define PROPERTY_ID_FUNCTION                        35
#define PROPERTY_ID_TABLENAME                       36
#define PROPERTY_ID_REALNAME                        37
#define PROPERTY_ID_DBASEPRECISIONCHANGED           38
#define PROPERTY_ID_ISCURRENCY                      39
#define PROPERTY_ID_ISBOOKMARKABLE                  40

#define PROPERTY_ID_INVALID_INDEX                   41
#define PROPERTY_ID_HY010                           43
#define PROPERTY_ID_LABEL                           44
#define PROPERTY_ID_DELIMITER                       45
#define PROPERTY_ID_FORMATKEY                       46
#define PROPERTY_ID_LOCALE                          47
#define PROPERTY_ID_IM001                           48

#define PROPERTY_ID_AUTOINCREMENTCREATION           49

#define PROPERTY_ID_PRIVILEGES                      50
#define PROPERTY_ID_HAVINGCLAUSE                    51

#define PROPERTY_ID_ISSIGNED                        52
#define PROPERTY_ID_AGGREGATEFUNCTION               53
#define PROPERTY_ID_ISSEARCHABLE                    54

#define PROPERTY_ID_APPLYFILTER                     55
#define PROPERTY_ID_FILTER                          56
#define PROPERTY_ID_MASTERFIELDS                    57
#define PROPERTY_ID_DETAILFIELDS                    58
#define PROPERTY_ID_FIELDTYPE                       59
#define PROPERTY_ID_VALUE                           60
#define PROPERTY_ID_ACTIVE_CONNECTION               61


USING_NAMESPACE_BENTLEY

