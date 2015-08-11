/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/NativeSqlBuilder.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "../ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECSqlParameter;

//=======================================================================================
// @bsiclass                                             Krischan.Eberle    08/2013
//+===============+===============+===============+===============+===============+======
struct NativeSqlBuilder
    {
    public:
        struct ECSqlParameterIndex
            {
            private:
                int m_index;
                int m_componentIndex;

            public:
                ECSqlParameterIndex (int index, int componentIndex)
                    : m_index (index), m_componentIndex (componentIndex)
                    {}

                int GetIndex () const { return m_index; }
                int GetComponentIndex () const { return m_componentIndex; }
            };

        typedef std::vector<NativeSqlBuilder> List;
        typedef std::vector<List> ListOfLists;

    private:
        Utf8String m_nativeSql;
        std::vector<ECSqlParameterIndex> m_parameterIndexMappings;
        std::vector<Utf8String> m_stack;
    public:
        NativeSqlBuilder ();
        explicit NativeSqlBuilder (Utf8CP initialStr);

        ~NativeSqlBuilder () {}
        NativeSqlBuilder (NativeSqlBuilder const& rhs);
        NativeSqlBuilder& operator= (NativeSqlBuilder const& rhs);
        NativeSqlBuilder (NativeSqlBuilder&& rhs);
        NativeSqlBuilder& operator= (NativeSqlBuilder&& rhs);

        NativeSqlBuilder& Append (NativeSqlBuilder const& builder, bool appendTrailingSpace = false);
        NativeSqlBuilder& Append (Utf8CP str, bool appendTrailingSpace = false);

        void Push (bool clear = true)
            {
            m_stack.push_back (m_nativeSql);

            if (clear)
                m_nativeSql.clear ();
            }
        void Reset ()
            {
            m_nativeSql.clear ();
            m_stack.clear ();
            }
        Utf8String Pop ()
            {
            Utf8String r;
            if (m_stack.empty ())
                {
                BeAssert (false && "Nothing to pop");
                return r;
                }

            r = m_nativeSql;
            m_nativeSql = m_stack.back ();
            m_stack.pop_back ();
            return r;
            }
        //!@param[in] separator The separator used to separate the items in @p builderList. If nullptr is passed,
        //!                     the items will be separated by comma.
        NativeSqlBuilder& Append (List const& builderList, Utf8CP separator = nullptr);

        NativeSqlBuilder& Append (List const& lhsBuilderList, Utf8CP operatorStr, List const& rhsBuilderList, Utf8CP separator = nullptr);
        NativeSqlBuilder& AppendFormatted (Utf8CP format, ...)
            {
            va_list ap;
            va_start (ap, format);
            Append (Utf8PrintfString (format, ap).c_str());
            va_end (ap);
            return *this;
            }
    NativeSqlBuilder& Append (Utf8CP classIdentifier, Utf8CP identifier);
    NativeSqlBuilder& Append (BinarySqlOperator op, bool appendTrailingSpace = true);
    NativeSqlBuilder& Append (BooleanSqlOperator op, bool appendTrailingSpace = true);
    NativeSqlBuilder& Append (SqlSetQuantifier setQuantifier, bool appendTrailingSpace = true);
    NativeSqlBuilder& Append (UnarySqlOperator op, bool appendTrailingSpace = true);
    //!@param[in] ecsqlParameterName Parameter name of nullptr if parameter is unnamed
    NativeSqlBuilder& AppendParameter (Utf8CP ecsqlParameterName, int ecsqlParameterIndex, int ecsqlParameterComponentIndex);
    //!@param[in] ecsqlParameterName Parameter name of nullptr if parameter is unnamed
    NativeSqlBuilder& AppendParameter (Utf8CP ecsqlParameterName, int ecsqlParameterComponentIndex);
    NativeSqlBuilder& Append (ECN::ECClassId ecClassId);

    NativeSqlBuilder& AppendEscaped (Utf8CP identifier);
    NativeSqlBuilder& AppendQuoted (Utf8CP stringLiteral);

    NativeSqlBuilder& AppendSpace ();
    NativeSqlBuilder& AppendComma (bool appendTrailingSpace = true);
    NativeSqlBuilder& AppendDot ();
    NativeSqlBuilder& AppendParenLeft ();
    NativeSqlBuilder& AppendParenRight ();
    NativeSqlBuilder& AppendLine (Utf8CP str)
        {
        return Append (str).AppendEOL ();
        }
    NativeSqlBuilder& AppendEOL () { return Append ("\n"); }
    NativeSqlBuilder& AppendTAB (int count = 1) 
        { 
        for (auto i = 0; i < count; i++)
            Append ("\t"); 

        return *this;
        }
	NativeSqlBuilder& AppendIf(bool appendIf, Utf8CP stringLiteral){ if (appendIf) Append(stringLiteral); return *this; }
	NativeSqlBuilder& AppendEscapedIf(bool escapeIf, Utf8CP identifier){ if (escapeIf) AppendEscaped(identifier); else Append(identifier); return *this; };
    bool IsEmpty () const;
    Utf8CP ToString () const;

    std::vector<ECSqlParameterIndex> const& GetParameterIndexMappings () const { return m_parameterIndexMappings; }

    static List FlattenJaggedList (ListOfLists const& listOfLists, std::vector<size_t> const& indexSkipList);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
