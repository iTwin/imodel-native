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
#pragma once
#ifndef _CONNECTIVITY_SQLNODE_HXX
#define _CONNECTIVITY_SQLNODE_HXX

#include "SqlTypes.h"
#include <vector>
#include <functional>
#include <set>
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

    typedef ::std::vector< OSQLParseNode* > OSQLParseNodes;

    enum SQLNodeType
        {
        SQL_NODE_RULE, SQL_NODE_LISTRULE, SQL_NODE_COMMALISTRULE,
        SQL_NODE_KEYWORD, SQL_NODE_COMPARISON, SQL_NODE_NAME, SQL_NODE_ARRAY_INDEX, SQL_NODE_DOTLISTRULE,
        SQL_NODE_STRING, SQL_NODE_INTNUM, SQL_NODE_APPROXNUM,
        SQL_NODE_EQUAL, SQL_NODE_LESS, SQL_NODE_GREAT, SQL_NODE_LESSEQ, SQL_NODE_GREATEQ, SQL_NODE_NOTEQUAL,
        SQL_NODE_PUNCTUATION, SQL_NODE_AMMSC, SQL_NODE_ACCESS_DATE, SQL_NODE_DATE, SQL_NODE_BITWISE_NOT,
        SQL_NODE_BITWISE_OR, SQL_NODE_BITWISE_AND, SQL_NODE_BITWISE_SHIFT_LEFT, SQL_NODE_BITWISE_SHIFT_RIGHT, SQL_NODE_CONCAT
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

        SQLParseNodeParameter(
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
        ~SQLParseNodeParameter();
        };
    class OSQLParseNodesContainer;
    //==========================================================================
    //= OSQLParseNode
    //==========================================================================
    class OOO_DLLPUBLIC_DBTOOLS OSQLParseNode
        {
        friend class OSQLParser;
        friend class OSQLScanner;
        OSQLParseNodes m_aChildren;
        OSQLParseNode* m_pParent;        // pParent fuer Reuckverkettung im Baum
        Utf8String m_aNodeValue;    // Token-Name oder leer bei Regeln oder Utf8String bei
        SQLNodeType m_eNodeType;    // s. o.
        sal_uInt32 m_nNodeID;         // ::com::sun::star::chaos::Rule ID (bei IsRule()) oder Token ID (bei !IsRule())
        OSQLParseNodesContainer* m_container;
        public:
            enum Rule
                {
                all_or_any_predicate = 0,
                as,
                assignment_commalist,
                assignment,
                between_predicate_part_2,
                between_predicate,
                bit_value_fct,
                boolean_factor,
                boolean_primary,
                boolean_term,
                boolean_test,
                cast_spec,
                cast_target_array,
                cast_target_scalar,
                char_factor,
                char_value_exp,
                character_like_predicate_part_2,
                class_name,
                column_commalist,
                column_ref_commalist,
                column_ref,
                column,
                comparison_predicate_part_2,
                comparison_predicate,
                comparison,
                concatenation,
                cross_union,
                cte_block_list,
                cte_column_list,
                cte_table_name,
                cte,
                datetime_factor,
                datetime_primary,
                datetime_term,
                datetime_value_exp,
                datetime_value_fct,
                delete_statement_searched,
                derived_column,
                ecrelationship_join,
                ecsqloptions_clause,
                else_clause,
                existence_test,
                factor,
                fct_spec,
                from_clause,
                function_args_commalist,
                general_set_fct,
                iif_spec,
                in_predicate_part_2,
                in_predicate,
                insert_atom_commalist,
                insert_atom,
                insert_statement,
                join_condition,
                join_type,
                joined_table,
                like_predicate,
                limit_offset_clause,
                manipulative_statement,
                member_function_call,
                named_columns_join,
                non_join_query_exp,
                non_join_query_primary,
                non_join_query_term,
                num_value_exp,
                op_relationship_direction,
                opt_asc_desc,
                opt_column_commalist,
                opt_column_ref_commalist,
                opt_cte_recursive,
                opt_disqualify_polymorphic_constraint,
                opt_disqualify_primary_join,
                opt_ecsqloptions_clause,
                opt_escape,
                opt_group_by_clause,
                opt_having_clause,
                opt_limit_offset_clause,
                opt_member_function_args,
                opt_only,
                opt_order_by_clause,
                opt_pragma_for,
                opt_pragma_func,
                opt_pragma_set_val,
                opt_pragma_set,
                opt_where_clause,
                ordering_spec_commalist,
                ordering_spec,
                other_like_predicate_part_2,
                outer_join_type,
                parameter_ref,
                parameter,
                pragma_path,
                pragma_value,
                pragma,
                predicate_check,
                property_path_entry,
                property_path,
                qualified_class_name,
                qualified_join,
                query_term,
                range_variable,
                row_value_constructor_commalist,
                row_value_constructor,
                rtreematch_predicate,
                scalar_exp_commalist,
                scalar_exp,
                search_condition,
                searched_case,
                searched_when_clause_list,
                searched_when_clause,
                select_statement,
                select_sublist,
                selection,
                single_select_statement,
                sql_not,
                subquery,
                table_exp,
                table_node_path_entry,
                table_node_path,
                table_node_with_opt_member_func_call,
                table_node,
                table_primary_as_range_column,
                table_ref_commalist,
                table_ref,
                tablespace_qualified_class_name,
                term_add_sub,
                term,
                test_for_null,
                type_predicate,
                unary_predicate,
                unique_test,
                update_statement_searched,
                value_exp_commalist,
                value_exp_primary,
                value_exp,
                values_or_query_spec,
                where_clause,
                rule_count,
                UNKNOWN_RULE            // ID indicating that a node is no rule with a matching Rule-enum value (see getKnownRuleID)
                };

            // must be ascii encoding for the value
            OSQLParseNode(const sal_Char* _pValueStr,
                          SQLNodeType _eNodeType,
                          sal_uInt32 _nNodeID = 0);

            OSQLParseNode(Utf8String const& _rValue,
                          SQLNodeType eNewNodeType,
                          sal_uInt32 nNewNodeID = 0);

            virtual ~OSQLParseNode();
            OSQLParseNode* getParent() const { return m_pParent; };
            void setParent(OSQLParseNode* pParseNode) { m_pParent = pParseNode; };
            size_t count() const { return m_aChildren.size(); };
            inline OSQLParseNode* getChild(size_t nPos) const;
            inline OSQLParseNode* getLast() const;
            inline OSQLParseNode* getFirst() const;
            void append(OSQLParseNode* pNewSubTree);
            void insert(sal_uInt32 nPos, OSQLParseNode* pNewSubTree);
            OSQLParseNode* replace(OSQLParseNode* pOldSubTree, OSQLParseNode* pNewSubTree);
            OSQLParseNode* removeAt(sal_uInt32 nPos);
            OSQLParseNode* detach();
            void parseNodeToStr(Utf8String& rString,
                                const RefCountedPtr< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                                const IParseContext* pContext = NULL,
                                sal_Bool _bIntl = sal_False,
                                sal_Bool _bQuote = sal_True) const;

            // quoted und internationalisert
            void parseNodeToPredicateStr(Utf8String& rString,
                                         const RefCountedPtr< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                                         const RefCountedPtr< ::com::sun::star::util::XNumberFormatter > & xFormatter,
                                         const ::com::sun::star::lang::Locale& rIntl,
                                         sal_Char _cDec,
                                         const IParseContext* pContext = NULL) const;

            void parseNodeToPredicateStr(Utf8String& rString,
                                         const RefCountedPtr< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                                         const RefCountedPtr< ::com::sun::star::util::XNumberFormatter > & xFormatter,
                                         const RefCountedPtr< ::com::sun::star::beans::XPropertySet > & _xField,
                                         const ::com::sun::star::lang::Locale& rIntl,
                                         sal_Char _cDec,
                                         const IParseContext* pContext = NULL) const;

            OSQLParseNode* getByRule(OSQLParseNode::Rule eRule) const;

#if OSL_DEBUG_LEVEL > 0
            // zeigt den ParseTree mit tabs und linefeeds
            void showParseTree(Utf8String& rString) const;
            void showParseTree(Utf8String& _inout_rBuf, sal_uInt32 nLevel) const;
#endif

            SQLNodeType getNodeType() const { return m_eNodeType; };
            sal_uInt32 getRuleID() const { return m_nNodeID; }
            Rule getKnownRuleID() const;
            sal_uInt32 getTokenID() const { return m_nNodeID; }
            sal_Bool isRule() const
                {
                return (m_eNodeType == SQL_NODE_RULE) || (m_eNodeType == SQL_NODE_LISTRULE)
                    || (m_eNodeType == SQL_NODE_COMMALISTRULE || m_eNodeType == SQL_NODE_DOTLISTRULE);
                }

            // IsToken tests whether a Node is a Token (Terminal but not a rule)
            sal_Bool isToken() const { return !isRule(); }
            const Utf8String& getTokenValue() const { return m_aNodeValue; }
            void setTokenValue(const Utf8String& rString) { if (isToken()) m_aNodeValue = rString; }
            sal_Bool isLeaf() const { return m_aChildren.empty(); }

        protected:
            // ParseNodeToStr konkateniert alle Token (Blaetter) des ParseNodes
            void parseNodeToStr(Utf8String& rString,
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
            void impl_parseNodeToString_throw(Utf8String& rString, const SQLParseNodeParameter& rParam) const;
            void impl_parseLikeNodeToString_throw(Utf8String& rString, const SQLParseNodeParameter& rParam) const;
            void impl_parseTableRangeNodeToString_throw(Utf8String& rString, const SQLParseNodeParameter& rParam) const;
            bool impl_parseTableNameNodeToString_throw(Utf8String& rString, const SQLParseNodeParameter& rParam) const;

            void parseLeaf(Utf8String& rString, const SQLParseNodeParameter& rParam) const;
#if !NDEBUG
        private:
            Rule m_ruleId;
#endif
        };

    //-----------------------------------------------------------------------------
    inline OSQLParseNode* OSQLParseNode::getChild(size_t nPos) const
        {
        if (nPos >= m_aChildren.size()) {
            OSL_ENSURE(nPos < m_aChildren.size(), "Invalid Position");
        }
        return m_aChildren.at(nPos);
        }
    inline OSQLParseNode* OSQLParseNode::getLast() const
        {
        auto nSize = m_aChildren.size();
        if (nSize > 0)
            return m_aChildren[nSize - 1];

        return nullptr;
        }
    inline OSQLParseNode* OSQLParseNode::getFirst() const
        {
        if (m_aChildren.size() > 0)
            return m_aChildren[0];

        return nullptr;
        }
    // Utility-Methoden zum Abfragen auf bestimmte Rules, Token oder Punctuation:
#define SQL_ISRULE(pParseNode, eRule)           ((pParseNode)->isRule() && (pParseNode)->getRuleID() == OSQLParser::RuleID(OSQLParseNode::eRule))
#define SQL_ISTOKEN(pParseNode, token)          ((pParseNode)->isToken() && (pParseNode)->getTokenID() == SQL_TOKEN_##token)
#define SQL_ISPUNCTUATION(pParseNode, aString)  ((pParseNode)->getNodeType() == SQL_NODE_PUNCTUATION && !(pParseNode)->getTokenValue().Equals(aString))
    }
#endif    //_CONNECTIVITY_SQLNODE_HXX
