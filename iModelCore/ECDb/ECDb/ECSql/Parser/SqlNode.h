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

    enum SQLNodeType {
        SQL_NODE_RULE,
        SQL_NODE_LISTRULE,
        SQL_NODE_COMMALISTRULE,
        SQL_NODE_KEYWORD,
        SQL_NODE_COMPARISON,
        SQL_NODE_NAME,
        SQL_NODE_ARRAY_INDEX,
        SQL_NODE_DOTLISTRULE,
        SQL_NODE_STRING,
        SQL_NODE_INTNUM,
        SQL_NODE_APPROXNUM,
        SQL_NODE_EQUAL,
        SQL_NODE_LESS,
        SQL_NODE_GREAT,
        SQL_NODE_LESSEQ,
        SQL_NODE_GREATEQ,
        SQL_NODE_NOTEQUAL,
        SQL_NODE_PUNCTUATION,
        SQL_NODE_AMMSC,
        SQL_NODE_ACCESS_DATE,
        SQL_NODE_DATE,
        SQL_NODE_BITWISE_NOT,
        SQL_NODE_BITWISE_OR,
        SQL_NODE_BITWISE_AND,
        SQL_NODE_BITWISE_SHIFT_LEFT,
        SQL_NODE_BITWISE_SHIFT_RIGHT,
        SQL_NODE_ARROW,
        SQL_NODE_CONCAT,
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

       public:
            enum Rule
                {
                all_or_any_predicate = 0,
                aggregate_fct,
                as,
                assignment,
                assignment_commalist,
                between_predicate,
                between_predicate_part_2,
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
                collate_clause,
                collating_function,
                column,
                column_commalist,
                column_ref,
                column_ref_commalist,
                comparison,
                comparison_predicate,
                comparison_predicate_part_2,
                concatenation,
                cross_union,
                cte,
                cte_block_list,
                cte_column_list,
                cte_table_name,
                datetime_factor,
                datetime_primary,
                datetime_term,
                datetime_value_exp,
                datetime_value_fct,
                delete_statement_searched,
                derived_column,
                dynamic_parameter_specification,
                ecrelationship_join,
                ecsqloptions_clause,
                else_clause,
                existence_test,
                existing_window_name,
                factor,
                fct_spec,
                fetch_first_clause,
                fetch_first_row_count,
                first_or_last_value,
                first_or_last_value_function,
                first_or_next,
                from_clause,
                function_args_commalist,
                iif_spec,
                in_line_window_specification,
                in_predicate,
                in_predicate_part_2,
                insert_atom,
                insert_atom_commalist,
                insert_statement,
                join_condition,
                join_type,
                joined_table,
                lead_or_lag,
                lead_or_lag_extent,
                lead_or_lag_function,
                like_predicate,
                limit_offset_clause,
                manipulative_statement,
                member_function_call,
                named_columns_join,
                new_window_name,
                non_join_query_exp,
                non_join_query_primary,
                non_join_query_term,
                nth_row,
                nth_value_function,
                ntile_function,
                num_value_exp,
                number_of_tiles,
                offset_row_count,
                op_relationship_direction,
                opt_asc_desc,
                opt_collate_clause,
                opt_column_commalist,
                opt_column_ref_commalist,
                opt_cte_recursive,
                opt_disqualify_polymorphic_constraint,
                opt_disqualify_primary_join,
                opt_ecsqloptions_clause,
                opt_escape,
                opt_existing_window_name,
                opt_extract_value,
                opt_fetch_first_row_count,
                opt_filter_clause,
                opt_function_arg,
                opt_group_by_clause,
                opt_having_clause,
                opt_lead_or_lag_function,
                opt_limit_offset_clause,
                opt_member_function_args,
                opt_null_order,
                opt_only,
                opt_only_all,
                opt_order_by_clause,
                opt_pragma_for,
                opt_pragma_func,
                opt_pragma_set,
                opt_pragma_set_val,
                opt_result_offset_clause,
                opt_where_clause,
                opt_window_clause,
                opt_window_frame_clause,
                opt_window_frame_exclusion,
                opt_window_partition_clause,
                ordering_spec,
                opt_optional_prop,
                ordering_spec_commalist,
                other_like_predicate_part_2,
                outer_join_type,
                parameter,
                pragma,
                pragma_path,
                pragma_value,
                predicate_check,
                property_path,
                property_path_entry,
                qualified_class_name,
                qualified_join,
                query_term,
                range_variable,
                rank_function_type,
                result_offset_clause,
                row_or_rows,
                row_value_constructor,
                row_value_constructor_commalist,
                rtreematch_predicate,
                scalar_exp,
                scalar_exp_commalist,
                search_condition,
                searched_case,
                searched_when_clause,
                searched_when_clause_list,
                select_statement,
                select_sublist,
                selection,
                simple_value_specification,
                single_select_statement,
                sql_not,
                subquery,
                table_exp,
                table_node,
                table_node_path,
                table_node_path_entry,
                table_node_ref,
                table_node_with_opt_member_func_call,
                table_primary_as_range_column,
                table_ref,
                table_ref_commalist,
                tablespace_qualified_class_name,
                term,
                term_add_sub,
                test_for_null,
                type_predicate,
                unary_predicate,
                unique_test,
                update_statement_searched,
                value_creation_fct,
                value_exp,
                value_exp_commalist,
                value_exp_primary,
                values_commalist,
                values_or_query_spec,
                where_clause,
                window_clause,
                window_definition,
                window_definition_list,
                window_frame_between,
                window_frame_bound,
                window_frame_bound_1,
                window_frame_bound_2,
                window_frame_clause,
                window_frame_exclusion,
                window_frame_extent,
                window_frame_following,
                window_frame_preceding,
                window_frame_start,
                window_frame_units,
                window_function,
                window_function_type,
                window_name,
                window_name_or_specification,
                window_partition_clause,
                window_partition_column_reference,
                window_partition_column_reference_list,
                window_specification,
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
