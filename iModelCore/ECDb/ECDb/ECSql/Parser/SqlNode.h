/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _CONNECTIVITY_SQLNODE_HXX
#define _CONNECTIVITY_SQLNODE_HXX


#include "SqlTypes.h"
//#include "connectivity/dbtoolsdllapi.hxx"
//#include "connectivity/dbmetadata.hxx"
//#include <com/sun/star/uno/Reference.hxx>
//#include <com/sun/star/util/XNumberFormatTypes.hpp>
//#include <com/sun/star/beans/XPropertySet.hpp>
#include <vector>
#include <functional>
#include <set>
//#include <boost/shared_ptr.hpp>
//#include <rtl/ustrbuf.hxx>

// forward declarations
//namespace com
//{
//    namespace sun
//    {
//        namespace star
//        {
//            namespace beans
//            {
//                class XPropertySet;
//            }
//            namespace util
//            {
//                class XNumberFormatter;
//            }
//            namespace container
//            {
//                class XNameAccess;
//            }
//        }
//    }
//}

namespace rtl
    {
    class OUStringBuffer;
    }
#define ORDER_BY_CHILD_POS  5
#define TABLE_EXPRESSION_CHILD_COUNT    9

namespace connectivity
    {
    class OSQLParser;
    class OSQLParseNode;
    class IParseContext;

    typedef ::std::vector< OSQLParseNode* >                  OSQLParseNodes;

    enum SQLNodeType    {
        SQL_NODE_RULE, SQL_NODE_LISTRULE, SQL_NODE_COMMALISTRULE,
        SQL_NODE_KEYWORD, SQL_NODE_COMPARISON, SQL_NODE_NAME, SQL_NODE_ARRAY_INDEX, SQL_NODE_DOTLISTRULE,
        SQL_NODE_STRING, SQL_NODE_INTNUM, SQL_NODE_APPROXNUM,
        SQL_NODE_EQUAL, SQL_NODE_LESS, SQL_NODE_GREAT, SQL_NODE_LESSEQ, SQL_NODE_GREATEQ, SQL_NODE_NOTEQUAL,
        SQL_NODE_PUNCTUATION, SQL_NODE_AMMSC, SQL_NODE_ACCESS_DATE, SQL_NODE_DATE, SQL_NODE_CONCAT
        };

    typedef ::std::set< Utf8String >   QueryNameSet;
    //==================================================================
    //= SQLParseNodeParameter
    //==================================================================
    struct OOO_DLLPUBLIC_DBTOOLS SQLParseNodeParameter
        {
        const ::com::sun::star::lang::Locale&    rLocale;
        ::dbtools::DatabaseMetaData             aMetaData;
        OSQLParser*                             pParser;
        ::std::shared_ptr< QueryNameSet >       pSubQueryHistory;
        RefCountedPtr< ::com::sun::star::util::XNumberFormatter >    xFormatter;
        RefCountedPtr< ::com::sun::star::beans::XPropertySet >       xField;
        RefCountedPtr< ::com::sun::star::container::XNameAccess >    xQueries;  // see bParseToSDBCLevel
        const IParseContext& m_rContext;
        sal_Char            cDecSep;
        bool                bQuote : 1;    /// should we quote identifiers?
        bool                bInternational : 1;    /// should we internationalize keywords and placeholders?
        bool                bPredicate : 1;    /// are we going to parse a mere predicate?
        bool                bParseToSDBCLevel : 1;    /// should we create an SDBC-level statement (e.g. with substituted sub queries)?

        SQLParseNodeParameter (
            const RefCountedPtr< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const RefCountedPtr< ::com::sun::star::util::XNumberFormatter >& _xFormatter,
            const RefCountedPtr< ::com::sun::star::beans::XPropertySet >& _xField,
            const ::com::sun::star::lang::Locale& _rLocale,
            const IParseContext* _pContext,
            bool _bIntl,
            bool _bQuote,
            sal_Char _cDecSep,
            bool _bPredicate,
            bool _bParseToSDBC
            );
        ~SQLParseNodeParameter ();
        };

    //==========================================================================
    //= OSQLParseNode
    //==========================================================================
    class OOO_DLLPUBLIC_DBTOOLS OSQLParseNode
        {
        friend class OSQLParser;

        OSQLParseNodes                    m_aChildren;
        OSQLParseNode*                     m_pParent;        // pParent fuer Reuckverkettung im Baum
        Utf8String                          m_aNodeValue;    // Token-Name oder leer bei Regeln oder Utf8String bei
        // Utf8String, INT, usw. -Werten
        SQLNodeType                     m_eNodeType;    // s. o.
        sal_uInt32                        m_nNodeID;         // ::com::sun::star::chaos::Rule ID (bei IsRule()) oder Token ID (bei !IsRule())
        // ::com::sun::star::chaos::Rule IDs und Token IDs koennen nicht anhand des Wertes
        // unterschieden werden, dafuer ist IsRule() abzufragen!

        public:
            enum Rule
                {
                select_statement = 0,
                table_exp,
                table_ref_commalist,
                table_ref,
                catalog_name,
                schema_name,
                table_name,
                opt_column_commalist,
                opt_column_ref_commalist,
                column_commalist,
                column_ref_commalist,
                column_ref,
                opt_order_by_clause,
                ordering_spec_commalist,
                ordering_spec,
                opt_asc_desc,
                where_clause,
                opt_where_clause,
                search_condition,
                comparison_predicate,
                between_predicate,
                like_predicate,
                character_like_predicate_part_2,
                other_like_predicate_part_2,
                opt_escape,
                test_for_null,
                scalar_exp_commalist,
                scalar_exp,
                parameter_ref,
                parameter,
                general_set_fct,
                ecclassid_fct_spec,
                range_variable,
                column,
                delete_statement_positioned,
                delete_statement_searched,
                update_statement_positioned,
                update_statement_searched,
                assignment_commalist,
                assignment,
                values_or_query_spec,
                insert_statement,
                insert_atom_commalist,
                insert_atom,
                predicate_check,
                from_clause,
                qualified_join,
                cross_union,
                select_sublist,
                derived_column,
                column_val,
                fct_spec,
                boolean_term,
                boolean_primary,
                num_value_exp,
                join_type,
                relationship_join,
                op_relationship_direction,
                position_exp,
                extract_exp,
                length_exp,
                char_value_fct,
                predefined_type,
                binary_string_type,
                binary_large_object_string_type,
                in_predicate,
                in_predicate_part_2,
                existence_test,
                unique_test,
                all_or_any_predicate,
                named_columns_join,
                join_condition,
                joined_table,
                boolean_factor,
                sql_not,
                boolean_test,
                manipulative_statement,
                subquery,
                non_join_query_exp,
                value_exp_commalist,
                union_statement,
                outer_join_type,
                char_value_exp,
                term,
                value_exp_primary,
                value_exp,
                selection,
                fold,
                char_substring_fct,
                factor,
                base_table_def,
                base_table_element_commalist,
                data_type,
                column_def,
                table_node,
                query_term,
                non_join_query_term,
                non_join_query_primary,
                as,
                table_primary_as_range_column,
                datetime_primary,
                datetime_value_exp,
                datetime_term,
                datetime_factor,
                datetime_value_fct,
                concatenation,
                char_factor,
                opt_limit_offset_clause,
                limit_offset_clause,
                bit_value_fct,
                comparison,
                comparison_predicate_part_2,
                parenthesized_boolean_value_expression,
                character_string_type,
                between_predicate_part_2,
                opt_group_by_clause,
                opt_having_clause,
                cast_spec,
                opt_column_array_idx,
                property_path,
                property_path_entry,

                rule_count,             // letzter_wert
                UNKNOWN_RULE            // ID indicating that a node is no rule with a matching Rule-enum value (see getKnownRuleID)
                };

            // must be ascii encoding for the value
            OSQLParseNode (const sal_Char* _pValueStr,
                SQLNodeType _eNodeType,
                sal_uInt32 _nNodeID = 0);

            OSQLParseNode (const Utf8String& _rValue,
                SQLNodeType eNewNodeType,
                sal_uInt32 nNewNodeID = 0);

            // Kopiert den entsprechenden ParseNode
            OSQLParseNode (const OSQLParseNode& rParseNode);
            OSQLParseNode& operator=(const OSQLParseNode& rParseNode);

            sal_Bool operator==(OSQLParseNode& rParseNode) const;

            // Destruktor raeumt rekursiv den Baum ab
            virtual ~OSQLParseNode ();

            // Parent gibt den Zeiger auf den Parent zurueck
            OSQLParseNode* getParent () const { return m_pParent; };

            // SetParent setzt den Parent-Zeiger eines ParseNodes
            void setParent (OSQLParseNode* pParseNode) { m_pParent = pParseNode; };

            size_t count () const { return m_aChildren.size (); };
            inline OSQLParseNode* getChild (size_t nPos) const;
            inline OSQLParseNode* getLast () const;
            inline OSQLParseNode* getFirst () const;

            void append (OSQLParseNode* pNewSubTree);
            void insert (sal_uInt32 nPos, OSQLParseNode* pNewSubTree);

            OSQLParseNode* replace (OSQLParseNode* pOldSubTree, OSQLParseNode* pNewSubTree);

            OSQLParseNode* removeAt (sal_uInt32 nPos);

            void replaceNodeValue (const Utf8String& rTableAlias, const Utf8String& rColumnName);

            /** parses the node to a string which can be passed to a driver's connection for execution

                Any particles of the parse tree which represent application-level features - such
                as queries appearing in the FROM part - are subsituted, so that the resulting statement can
                be executed at an SDBC-level connection.

                @param  _out_rString
                is an output parameter taking the resulting SQL statement

                @param  _rxConnection
                the connection relative to which to parse. This must be an SDB-level connection (e.g.
                support the XQueriesSupplier interface) for the method to be able to do all necessary
                substitutions.

                @param _rParser
                the SQLParser used to create the node. This is needed in case we need to parse
                sub queries which are present in the SQL statement - those sub queries need to be parsed,
                too, to check whether they contain nested sub queries.

                @param _pErrorHolder
                takes the error which occured while generating the statement, if any. Might be <NULL/>,
                in this case the error is not reported back, and can only be recognized by examing the
                return value.

                @return
                <TRUE/> if and only if the parsing was successful.<br/>

                Currently, there's only one condition how this method can fail: If it contains a nested
                query which causes a cycle. E.g., consider a statement <code>SELECT * from "foo"</code>,
                where <code>bar </code> is a query defined as <code>SELECT * FROM "bar"</code>, where
                <code>bar</code> is defined as <code>SELECT * FROM "foo"</code>. This statement obviously
                cannot be parsed to an executable statement.

                If this method returns <FALSE/>, you're encouraged to check and handle the error in
                <arg>_pErrorHolder</arg>.
                */
            bool parseNodeToExecutableStatement (Utf8String& _out_rString,
                const RefCountedPtr< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                OSQLParser& _rParser,
                ::com::sun::star::sdbc::SQLException* _pErrorHolder) const;

            void parseNodeToStr (Utf8String& rString,
                const RefCountedPtr< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                const IParseContext* pContext = NULL,
                sal_Bool _bIntl = sal_False,
                sal_Bool _bQuote = sal_True) const;

            // quoted und internationalisert
            void parseNodeToPredicateStr (Utf8String& rString,
                const RefCountedPtr< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                const RefCountedPtr< ::com::sun::star::util::XNumberFormatter > & xFormatter,
                const ::com::sun::star::lang::Locale& rIntl,
                sal_Char _cDec,
                const IParseContext* pContext = NULL) const;

            void parseNodeToPredicateStr (Utf8String& rString,
                const RefCountedPtr< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                const RefCountedPtr< ::com::sun::star::util::XNumberFormatter > & xFormatter,
                const RefCountedPtr< ::com::sun::star::beans::XPropertySet > & _xField,
                const ::com::sun::star::lang::Locale& rIntl,
                sal_Char _cDec,
                const IParseContext* pContext = NULL) const;

            OSQLParseNode* getByRule (OSQLParseNode::Rule eRule) const;

#if OSL_DEBUG_LEVEL > 0
            // zeigt den ParseTree mit tabs und linefeeds
            void showParseTree (Utf8String& rString) const;
            void showParseTree (Utf8StringBuffer& _inout_rBuf, sal_uInt32 nLevel) const;
#endif

            SQLNodeType getNodeType () const { return m_eNodeType; };

            // RuleId liefert die RuleId der Regel des Knotens (nur bei IsRule())
            sal_uInt32 getRuleID () const { return m_nNodeID; }

            /** returns the ID of the rule represented by the node

                If the node does not represent a rule, UNKNOWN_RULE is returned
                */
            Rule getKnownRuleID () const;

            // returns the TokenId of the node's token (only if !isRule())
            sal_uInt32 getTokenID () const { return m_nNodeID; }

            // IsRule tests whether a node is a rule (NonTerminal)
            // ATTENTION: rules can be leaves, for example empty lists
            sal_Bool isRule () const
                {
                return (m_eNodeType == SQL_NODE_RULE) || (m_eNodeType == SQL_NODE_LISTRULE)
                    || (m_eNodeType == SQL_NODE_COMMALISTRULE || m_eNodeType == SQL_NODE_DOTLISTRULE);
                }

            // IsToken tests whether a Node is a Token (Terminal but not a rule)
            sal_Bool isToken () const { return !isRule (); }

            const Utf8String& getTokenValue () const { return m_aNodeValue; }

            void setTokenValue (const Utf8String& rString) { if (isToken ()) m_aNodeValue = rString; }

            sal_Bool isLeaf () const { return m_aChildren.empty (); }

            // negate only a searchcondition, any other rule could cause a gpf
            static void negateSearchCondition (OSQLParseNode*& pSearchCondition, sal_Bool bNegate = sal_False);

            // normalize a logic form
            // e.q. (a or b) and (c or d) <=> a and c or a and d or b and c or b and d
            static void disjunctiveNormalForm (OSQLParseNode*& pSearchCondition);

            //     Simplies logic expressions
            // a * a        = a
            // a + a        = a
            // a * ( a + b) = a
            // a + a * b    = a
            static void absorptions (OSQLParseNode*& pSearchCondition);

            // erase not nessary braces
            static void eraseBraces (OSQLParseNode*& pSearchCondition);

            // makes the logic formula a little more smaller
            static void compress (OSQLParseNode*& pSearchCondition);
            // return the catalog, schema and tablename form this node
            // _pTableNode must be a rule of that above or a SQL_TOKEN_NAME
            static sal_Bool getTableComponents (const OSQLParseNode* _pTableNode,
                Utf8String &_rCatalog,
                Utf8String &_rSchema,
                Utf8String &_rTable
                , const RefCountedPtr< ::com::sun::star::sdbc::XDatabaseMetaData >& _xMetaData);

            // susbtitute all occurences of :var or [name] into the dynamic parameter ?
            // _pNode will be modified if parameters exists
            static void substituteParameterNames (OSQLParseNode* _pNode);

            /** return a table range when it exists.
            */
            static Utf8String getTableRange (const OSQLParseNode* _pTableRef);

        protected:
            // ParseNodeToStr konkateniert alle Token (Blaetter) des ParseNodes
            void parseNodeToStr (Utf8String& rString,
                const RefCountedPtr< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                const RefCountedPtr< ::com::sun::star::util::XNumberFormatter > & xFormatter,
                const RefCountedPtr< ::com::sun::star::beans::XPropertySet > & _xField,
                const ::com::sun::star::lang::Locale& rIntl,
                const IParseContext* pContext,
                bool _bIntl,
                bool _bQuote,
                sal_Char _cDecSep,
                bool _bPredicate,
                bool _bSubstitute) const;

        private:
            void impl_parseNodeToString_throw (Utf8StringBuffer& rString, const SQLParseNodeParameter& rParam) const;
            void impl_parseLikeNodeToString_throw (Utf8StringBuffer& rString, const SQLParseNodeParameter& rParam) const;
            void impl_parseTableRangeNodeToString_throw (Utf8StringBuffer& rString, const SQLParseNodeParameter& rParam) const;

            /** parses a table_name node into a SQL statement particle.
                @return
                <TRUE/> if and only if parsing was successful, <FALSE/> if default handling should
                be applied.
                */
            bool impl_parseTableNameNodeToString_throw (Utf8StringBuffer& rString, const SQLParseNodeParameter& rParam) const;

            Utf8String convertDateTimeString (const SQLParseNodeParameter& rParam, const Utf8String& rString) const;
            Utf8String convertDateString (const SQLParseNodeParameter& rParam, const Utf8String& rString) const;
            Utf8String convertTimeString (const SQLParseNodeParameter& rParam, const Utf8String& rString) const;
            void parseLeaf (Utf8StringBuffer& rString, const SQLParseNodeParameter& rParam) const;
#if !NDEBUG
        private:
            Rule m_ruleId;
#endif
        };

    //-----------------------------------------------------------------------------
    inline OSQLParseNode* OSQLParseNode::getChild (size_t nPos) const
        {
        OSL_ENSURE (nPos < m_aChildren.size (), "Invalid Position");

        //    return m_aChildren[nPos];
        return m_aChildren.at (nPos);
        }
    inline OSQLParseNode* OSQLParseNode::getLast () const
        {
        auto nSize = m_aChildren.size ();
        if (nSize > 0)
            return m_aChildren[nSize - 1];

        return nullptr;
        }
    inline OSQLParseNode* OSQLParseNode::getFirst () const
        {
        if (m_aChildren.size () > 0)
            return m_aChildren[0];

        return nullptr;
        }
    // Utility-Methoden zum Abfragen auf bestimmte Rules, Token oder Punctuation:
#define SQL_ISRULE(pParseNode, eRule)     ((pParseNode)->isRule() && (pParseNode)->getRuleID() == OSQLParser::RuleID(OSQLParseNode::eRule))
#define SQL_ISRULEOR2(pParseNode, e1, e2)  ((pParseNode)->isRule() && ( \
    (pParseNode)->getRuleID () == OSQLParser::RuleID (OSQLParseNode::e1) || \
    (pParseNode)->getRuleID () == OSQLParser::RuleID (OSQLParseNode::e2)))
#define SQL_ISRULEOR3(pParseNode, e1, e2, e3)  ((pParseNode)->isRule() && ( \
    (pParseNode)->getRuleID () == OSQLParser::RuleID (OSQLParseNode::e1) || \
    (pParseNode)->getRuleID () == OSQLParser::RuleID (OSQLParseNode::e2) || \
    (pParseNode)->getRuleID () == OSQLParser::RuleID (OSQLParseNode::e3)))
#define SQL_ISTOKEN(pParseNode, token) ((pParseNode)->isToken() && (pParseNode)->getTokenID() == SQL_TOKEN_##token)
#define SQL_ISTOKENOR2(pParseNode, tok0, tok1) ((pParseNode)->isToken() &&  ( (pParseNode)->getTokenID() == SQL_TOKEN_##tok0 || (pParseNode)->getTokenID() == SQL_TOKEN_##tok1 ))
#define SQL_ISTOKENOR3(pParseNode, tok0, tok1, tok2) ((pParseNode)->isToken() && ( (pParseNode)->getTokenID() == SQL_TOKEN_##tok0 || (pParseNode)->getTokenID() == SQL_TOKEN_##tok1 || (pParseNode)->getTokenID() == SQL_TOKEN_##tok2 ))
#define SQL_ISPUNCTUATION(pParseNode, aString) ((pParseNode)->getNodeType() == SQL_NODE_PUNCTUATION && !Utf8StringHelper::compareToAscii((pParseNode)->getTokenValue(), aString))
    }

#endif    //_CONNECTIVITY_SQLNODE_HXX
