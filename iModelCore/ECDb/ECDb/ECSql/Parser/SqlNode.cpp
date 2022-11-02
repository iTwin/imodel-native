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



 // MARKER(update_precomp.py): autogen include statement, do not remove
#include "ECDbPch.h"
#include "ErrorCondition.h"
#include "SqlNode.h"
//#define YYBISON      1
//#ifndef BISON_INCLUDED
//#define BISON_INCLUDED
#include "SqlBison.h"
//#endif
#include "SqlParse.h"
#include "DataType.h"
#include "SqlScan.h"
#include <string.h>
#include <algorithm>
#include <functional>

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star;
using namespace ::dbtools;
using namespace ::comphelper;

extern Utf8String ConvertLikeToken(const ::connectivity::OSQLParseNode* pTokenNode, const ::connectivity::OSQLParseNode* pEscapeNode, sal_Bool bInternational);
namespace
    {
    // -----------------------------------------------------------------------------
    void replaceAndReset(connectivity::OSQLParseNode*& _pResetNode, connectivity::OSQLParseNode* _pNewNode)
        {
        _pResetNode->getParent()->replace(_pResetNode, _pNewNode);
        delete _pResetNode;
        _pResetNode = _pNewNode;
        }
    // -----------------------------------------------------------------------------
    /** quotes a string and search for quotes inside the string and replace them with the new quote
        @param  rValue
        The value to be quoted.
        @param  rQuot
        The quote
        @param  rQuotToReplace
        The quote to replace with
        @return
        The quoted string.
        */
    Utf8String SetQuotation(Utf8StringCR rValue, Utf8StringCR rQuot, Utf8StringCR rQuotToReplace)
        {
        Utf8String rNewValue = rQuot;
        rNewValue += rValue;
        size_t nIndex = (size_t) -1;   // Quotes durch zweifache Quotes ersetzen, sonst kriegt der Parser Probleme

        if (rQuot.size())
            {
            do
                {
                nIndex += 2;
                nIndex = rNewValue.find(rQuot, nIndex);
                if (nIndex != Utf8String::npos)
                    rNewValue = rNewValue.replace(nIndex, rQuot.size(), rQuotToReplace);
                } while (nIndex != Utf8String::npos);
            }

        rNewValue += rQuot;
        return rNewValue;
        }
    }

namespace connectivity
    {

    //=============================================================================
    //=============================================================================
    //= SQLParseNodeParameter
    //=============================================================================
    //-----------------------------------------------------------------------------
    SQLParseNodeParameter::SQLParseNodeParameter(const RefCountedPtr< XConnection >& _rxConnection,
                                                 const RefCountedPtr< XNumberFormatter >& _xFormatter, const RefCountedPtr< XPropertySet >& _xField,
                                                 const Locale& _rLocale, const IParseContext* _pContext,
                                                 bool _bIntl, bool _bQuote, sal_Char _cDecSep, bool _bPredicate, bool _bParseToSDBC)
        :rLocale(_rLocale)
        , aMetaData(_rxConnection)
        , pParser(NULL)
        , pSubQueryHistory(new QueryNameSet)
        , xFormatter(_xFormatter)
        , xField(_xField)
        , m_rContext(_pContext ? (const IParseContext&) (*_pContext) : (const IParseContext&) OSQLParser::s_aDefaultContext)
        , cDecSep(_cDecSep)
        , bQuote(_bQuote)
        , bInternational(_bIntl)
        , bPredicate(_bPredicate)
        , bParseToSDBCLevel(_bParseToSDBC)
        {}

    //-----------------------------------------------------------------------------
    SQLParseNodeParameter::~SQLParseNodeParameter()
        {}

    //=============================================================================
    //= OSQLParseNode
    //=============================================================================
    //-----------------------------------------------------------------------------
    void OSQLParseNode::parseNodeToStr(Utf8String& rString,
                                       const RefCountedPtr< XConnection >& _rxConnection,
                                       const IParseContext* pContext,
                                       sal_Bool _bIntl,
                                       sal_Bool _bQuote) const
        {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseNode::parseNodeToStr");

        parseNodeToStr(
            rString, _rxConnection, NULL, NULL,
            pContext ? pContext->getPreferredLocale() : OParseContext::getDefaultLocale(),
            pContext, _bIntl, _bQuote, '.', false, false);
        }

    //-----------------------------------------------------------------------------
    void OSQLParseNode::parseNodeToPredicateStr(Utf8String& rString,
                                                const RefCountedPtr< XConnection >& _rxConnection,
                                                const RefCountedPtr< XNumberFormatter > & xFormatter,
                                                const ::com::sun::star::lang::Locale& rIntl,
                                                sal_Char _cDec,
                                                const IParseContext* pContext) const
        {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseNode::parseNodeToPredicateStr");

        OSL_ENSURE(xFormatter.IsValid(), "OSQLParseNode::parseNodeToPredicateStr:: no formatter!");

        if (xFormatter.IsValid())
            parseNodeToStr(rString, _rxConnection, xFormatter, NULL, rIntl, pContext, sal_True, sal_True, _cDec, true, false);
        }

    //-----------------------------------------------------------------------------
    void OSQLParseNode::parseNodeToPredicateStr(Utf8String& rString,
                                                const RefCountedPtr< XConnection > & _rxConnection,
                                                const RefCountedPtr< XNumberFormatter > & xFormatter,
                                                const RefCountedPtr< XPropertySet > & _xField,
                                                const ::com::sun::star::lang::Locale& rIntl,
                                                sal_Char _cDec,
                                                const IParseContext* pContext) const
        {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseNode::parseNodeToPredicateStr");

        OSL_ENSURE(xFormatter.IsValid(), "OSQLParseNode::parseNodeToPredicateStr:: no formatter!");

        if (xFormatter.IsValid())
            parseNodeToStr(rString, _rxConnection, xFormatter, _xField, rIntl, pContext, true, true, _cDec, true, false);
        }

    //-----------------------------------------------------------------------------
    void OSQLParseNode::parseNodeToStr(Utf8String& rString,
                                       const RefCountedPtr< XConnection > & _rxConnection,
                                       const RefCountedPtr< XNumberFormatter > & xFormatter,
                                       const RefCountedPtr< XPropertySet > & _xField,
                                       const ::com::sun::star::lang::Locale& rIntl,
                                       const IParseContext* pContext,
                                       bool _bIntl,
                                       bool _bQuote,
                                       sal_Char _cDecSep,
                                       bool _bPredicate,
                                       bool _bSubstitute) const
        {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseNode::parseNodeToStr");

        OSL_ENSURE(_rxConnection.IsValid(), "OSQLParseNode::parseNodeToStr: invalid connection!");

        if (_rxConnection.IsValid())
            {
            Utf8String sBuffer = rString;
            try
                {
                OSQLParseNode::impl_parseNodeToString_throw(sBuffer,
                                                            SQLParseNodeParameter(
                                                                _rxConnection, xFormatter, _xField, rIntl, pContext,
                                                                _bIntl, _bQuote, _cDecSep, _bPredicate, _bSubstitute
                                                            ));
                }
            catch (const SQLException&)
                {
                OSL_ENSURE(false, "OSQLParseNode::parseNodeToStr: this should not throw!");
                // our callers don't expect this method to throw anything. The only known situation
                // where impl_parseNodeToString_throw can throw is when there is a cyclic reference
                // in the sub queries, but this cannot be the case here, as we do not parse to
                // SDBC level.
                }
            rString = sBuffer;
            }
        }

    //-----------------------------------------------------------------------------
    void OSQLParseNode::impl_parseNodeToString_throw(Utf8String& rString, const SQLParseNodeParameter& rParam) const
        {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseNode::getTableRange");
        if (isToken())
            {
            parseLeaf(rString, rParam);
            return;
            }

        // einmal auswerten wieviel Subtrees dieser Knoten besitzt
        size_t nCount = count();

        bool bHandled = false;
        switch (getKnownRuleID())
            {
            // special handling for parameters
                case parameter:
                {
                if (rString.size())
                    rString.append(" ");
                if (nCount == 1)    // ?
                    m_aChildren[0]->impl_parseNodeToString_throw(rString, rParam);
                else if (nCount == 2)    // :Name
                    {
                    m_aChildren[0]->impl_parseNodeToString_throw(rString, rParam);
                    rString.append(m_aChildren[1]->m_aNodeValue);
                    }                    // [Name]
                else
                    {
                    m_aChildren[0]->impl_parseNodeToString_throw(rString, rParam);
                    rString.append(m_aChildren[1]->m_aNodeValue);
                    rString.append(m_aChildren[2]->m_aNodeValue);
                    }
                bHandled = true;
                }
                break;

                // table refs
                case table_ref:
                    if ((nCount == 2) || (nCount == 3) || (nCount == 5))
                        {
                        impl_parseTableRangeNodeToString_throw(rString, rParam);
                        bHandled = true;
                        }
                    break;

                    // table name - might be a query name

                case as:
                    if (rParam.aMetaData.generateASBeforeCorrelationName())
                        rString.append(" AS");
                    bHandled = true;
                    break;

                case like_predicate:
                    // je nachdem ob international angegeben wird oder nicht wird like anders behandelt
                    // interanational: *, ? sind Platzhalter
                    // sonst SQL92 konform: %, _
                    impl_parseLikeNodeToString_throw(rString, rParam);
                    bHandled = true;
                    break;

                case general_set_fct:
                case fct_spec:
                {
                // Funktionsname nicht quoten
                SQLParseNodeParameter aNewParam(rParam);
                aNewParam.bQuote = false;

                m_aChildren[0]->impl_parseNodeToString_throw(rString, aNewParam);
                aNewParam.bQuote = rParam.bQuote;
                //aNewParam.bPredicate = sal_False; // disable [ ] around names // look at i73215 
                Utf8String aStringPara;
                for (sal_uInt32 i = 1; i < nCount; i++)
                    {
                    const OSQLParseNode * pSubTree = m_aChildren[i];
                    if (pSubTree)
                        {
                        pSubTree->impl_parseNodeToString_throw(aStringPara, aNewParam);

                        // bei den CommaListen zwischen alle Subtrees Commas setzen
                        if ((m_eNodeType == SQL_NODE_COMMALISTRULE) && (i < (nCount - 1)))
                            aStringPara.append(",");

                        if ((m_eNodeType == SQL_NODE_DOTLISTRULE) && (i < (nCount - 1)))
                            aStringPara.append(".");

                        }
                    else
                        i++;
                    }
                rString.append(aStringPara);
                bHandled = true;
                }
                break;
                default:
                    break;
            }   // switch ( getKnownRuleID() )

        if (!bHandled)
            {
            for (OSQLParseNodes::const_iterator i = m_aChildren.begin();
                 i != m_aChildren.end();)
                {
                const OSQLParseNode* pSubTree = *i;
                if (!pSubTree)
                    {
                    ++i;
                    continue;
                    }

                SQLParseNodeParameter aNewParam(rParam);

                // don't replace the field for subqueries
                if (rParam.xField.IsValid() && SQL_ISRULE(pSubTree, subquery))
                    aNewParam.xField = NULL;

                // if there is a field given we don't display the fieldname, if there is any
                if (rParam.xField.IsValid() && SQL_ISRULE(pSubTree, column_ref))
                    {
                    sal_Bool bFilter = sal_False;
                    // retrieve the fields name
                    Utf8String aFieldName;
                    if (pSubTree->count())
                        {
                        const OSQLParseNode* pCol = pSubTree->m_aChildren[pSubTree->count() - 1];
                        if (pCol->getTokenValue().EqualsI(aFieldName))
                            bFilter = sal_True;
                        }

                    // ok we found the field, if the following node is the
                    // comparision operator '=' we filter it as well
                    if (bFilter)
                        {
                        if (SQL_ISRULE(this, comparison_predicate))
                            {
                            ++i;
                            if (i != m_aChildren.end())
                                {
                                pSubTree = *i;
                                if (pSubTree && pSubTree->getNodeType() == SQL_NODE_EQUAL)
                                    i++;
                                }
                            }
                        else
                            i++;
                        }
                    else
                        {
                        pSubTree->impl_parseNodeToString_throw(rString, aNewParam);
                        i++;

                        // bei den CommaListen zwischen alle Subtrees Commas setzen
                        if ((m_eNodeType == SQL_NODE_COMMALISTRULE) && (i != m_aChildren.end()))
                            rString.append(",");

                        if ((m_eNodeType == SQL_NODE_DOTLISTRULE) && (i != m_aChildren.end()))
                            rString.append(".");
                        }
                    }
                else
                    {
                    pSubTree->impl_parseNodeToString_throw(rString, aNewParam);
                    i++;

                    // bei den CommaListen zwischen alle Subtrees Commas setzen
                    if ((m_eNodeType == SQL_NODE_COMMALISTRULE) && (i != m_aChildren.end()))
                        {
                        if (SQL_ISRULE(this, value_exp_commalist) && rParam.bPredicate)
                            rString.append(";");
                        else
                            rString.append(",");
                        }

                    if ((m_eNodeType == SQL_NODE_DOTLISTRULE) && (i != m_aChildren.end()))
                        rString.append(".");
                    }
                }
            }
        }
    //-----------------------------------------------------------------------------
    bool OSQLParseNode::impl_parseTableNameNodeToString_throw(Utf8String& rString, const SQLParseNodeParameter& rParam) const
        {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseNode::impl_parseTableNameNodeToString_throw");
        // is the table_name part of a table_ref?
        OSL_ENSURE(getParent(), "OSQLParseNode::impl_parseTableNameNodeToString_throw: table_name without parent?");
        if (!getParent() || (getParent()->getKnownRuleID() != table_ref))
            return false;

        if (!rParam.bParseToSDBCLevel)
            return false;

        if (!rParam.xQueries.IsValid())
            return false;

        return false;
        }

    //-----------------------------------------------------------------------------
    void OSQLParseNode::impl_parseTableRangeNodeToString_throw(Utf8String& rString, const SQLParseNodeParameter& rParam) const
        {}
    //-----------------------------------------------------------------------------
    void OSQLParseNode::impl_parseLikeNodeToString_throw(Utf8String& rString, const SQLParseNodeParameter& rParam) const
        {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseNode::impl_parseLikeNodeToString_throw");
        OSL_ENSURE(count() == 2, "count != 2: Prepare for GPF");

        const OSQLParseNode* pEscNode = NULL;
        const OSQLParseNode* pParaNode = NULL;

        SQLParseNodeParameter aNewParam(rParam);
        //aNewParam.bQuote = sal_True; // why setting this to true? @see http://www.openoffice.org/issues/show_bug.cgi?id=75557

        // if there is a field given we don't display the fieldname, if there are any
        sal_Bool bAddName = sal_True;
        if (rParam.xField.IsValid())
            {
            // retrieve the fields name
            Utf8String aFieldName;
            try
                {
                TODO_ConvertCode();
                }
            catch (Exception&)
                {
                OSL_ENSURE(false, "OSQLParseNode::impl_parseLikeNodeToString_throw Exception occured!");
                }
            if (!m_aChildren[0]->isLeaf())
                {
                const OSQLParseNode* pCol = m_aChildren[0]->getChild(m_aChildren[0]->count() - 1);
                if (pCol->getTokenValue().EqualsI(aFieldName))
                    bAddName = sal_False;
                }
            }

        if (bAddName)
            m_aChildren[0]->impl_parseNodeToString_throw(rString, aNewParam);

        const OSQLParseNode* pPart2 = m_aChildren[1];
        pPart2->getChild(0)->impl_parseNodeToString_throw(rString, aNewParam);
        pPart2->getChild(1)->impl_parseNodeToString_throw(rString, aNewParam);
        pParaNode = pPart2->getChild(2);
        pEscNode = pPart2->getChild(3);

        if (pParaNode->isToken())
            {
            Utf8String aStr = ConvertLikeToken(pParaNode, pEscNode, rParam.bInternational);
            rString.append(" ");
            rString.append(SetQuotation(aStr, Utf8String("\'"), Utf8String("\'\'")));
            }
        else
            pParaNode->impl_parseNodeToString_throw(rString, aNewParam);

        pEscNode->impl_parseNodeToString_throw(rString, aNewParam);
        }

    // -----------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    OSQLParseNode::OSQLParseNode(const sal_Char * pNewValue,
                                 SQLNodeType eNewNodeType,
                                 sal_uInt32 nNewNodeID)
        :m_pParent(NULL)
        , m_aNodeValue(pNewValue)
        , m_eNodeType(eNewNodeType)
        , m_nNodeID(nNewNodeID)
        , m_container(nullptr)
        {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseNode::OSQLParseNode");
#if !NDEBUG
        m_ruleId = getKnownRuleID();
#endif
        OSL_ENSURE(m_eNodeType >= SQL_NODE_RULE && m_eNodeType <= SQL_NODE_CONCAT, "OSQLParseNode: mit unzulaessigem NodeType konstruiert");
        }
    //-----------------------------------------------------------------------------
    OSQLParseNode::OSQLParseNode(Utf8String const&_rNewValue,
                                 SQLNodeType eNewNodeType,
                                 sal_uInt32 nNewNodeID)
        :m_pParent(NULL)
        , m_aNodeValue(_rNewValue)
        , m_eNodeType(eNewNodeType)
        , m_nNodeID(nNewNodeID)
        , m_container(nullptr)
        {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseNode::OSQLParseNode");
#if !NDEBUG
        m_ruleId = getKnownRuleID();
#endif
        OSL_ENSURE(m_eNodeType >= SQL_NODE_RULE && m_eNodeType <= SQL_NODE_CONCAT, "OSQLParseNode: mit unzulaessigem NodeType konstruiert");
        }
    //-----------------------------------------------------------------------------
    OSQLParseNode::~OSQLParseNode()
        {
        for (OSQLParseNodes::const_iterator i = m_aChildren.begin();
             i != m_aChildren.end(); i++)
            delete *i;
        m_aChildren.clear();

        if (m_container)
            m_container->erase(this);
        }
    //-----------------------------------------------------------------------------
    OSQLParseNode* OSQLParseNode::detach()
        {
        if (m_pParent != nullptr)
            {
            auto itor = std::find(m_pParent->m_aChildren.begin(), m_pParent->m_aChildren.end(), this);
            BeAssert(itor != m_pParent->m_aChildren.end());
            m_pParent->m_aChildren.erase(itor);
            m_pParent = nullptr;
            }

        if (m_container != nullptr)
            {
            m_container->erase(this);
            m_container = nullptr;
            }

        return this;
        }

    //-----------------------------------------------------------------------------
    void OSQLParseNode::append(OSQLParseNode* pNewNode)
        {
        BeAssert(pNewNode != nullptr);
        pNewNode->detach(); //dump all ownerships
        pNewNode->setParent(this);
        m_aChildren.push_back(pNewNode);
        }
    //-----------------------------------------------------------------------------
    OSQLParseNode* OSQLParseNode::getByRule(OSQLParseNode::Rule eRule) const
        {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseNode::getByRule");
        OSQLParseNode* pRetNode = 0;
        if (isRule() && OSQLParser::RuleID(eRule) == getRuleID())
            pRetNode = (OSQLParseNode*) this;
        else
            {
            for (OSQLParseNodes::const_iterator i = m_aChildren.begin();
                 !pRetNode && i != m_aChildren.end(); i++)
                pRetNode = (*i)->getByRule(eRule);
            }
        return pRetNode;
        }

#if OSL_DEBUG_LEVEL > 0
    // -----------------------------------------------------------------------------
    void OSQLParseNode::showParseTree(Utf8String& rString) const
        {
        showParseTree(rString, 0);
        }
    // -----------------------------------------------------------------------------
    void OSQLParseNode::showParseTree(Utf8String& _inout_rBuffer, sal_uInt32 nLevel) const
        {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseNode::showParseTree");

        for (sal_uInt32 j = 0; j < nLevel; ++j)
            _inout_rBuffer.append("  ");

        if (!isToken())
            {
            // Regelnamen als rule: ...
            Utf8String ruleIdStr;
            ruleIdStr.Sprintf("RULE_ID: %" PRIu32 " (%s)\n", getRuleID(), OSQLParser::RuleIDToStr(getRuleID()));
            _inout_rBuffer.append(ruleIdStr);

            // hol dir den ersten Subtree
            for (OSQLParseNodes::const_iterator i = m_aChildren.begin();
                 i != m_aChildren.end();
                 ++i
                 )
                 (*i)->showParseTree(_inout_rBuffer, nLevel + 1);
            }
        else
            {
            // ein Token gefunden
            switch (m_eNodeType)
                {

                    case SQL_NODE_KEYWORD:
                        _inout_rBuffer.append("SQL_KEYWORD: ");
                        _inout_rBuffer.append(OSQLParser::TokenIDToStr(getTokenID()));
                        _inout_rBuffer.append("\n");
                        break;

                    case SQL_NODE_COMPARISON:
                        _inout_rBuffer.append("SQL_COMPARISON: ");
                        _inout_rBuffer.append(m_aNodeValue);
                        _inout_rBuffer.append("\n");
                        break;

                    case SQL_NODE_NAME:
                        _inout_rBuffer.append("SQL_NAME: ");
                        _inout_rBuffer.append("\"");
                        _inout_rBuffer.append(m_aNodeValue);
                        _inout_rBuffer.append("\"");
                        _inout_rBuffer.append("\n");
                        break;

                    case SQL_NODE_ARRAY_INDEX:
                        _inout_rBuffer.append("SQL_ARRAY_INDEX: ");
                        _inout_rBuffer.append("\"");
                        _inout_rBuffer.append(m_aNodeValue);
                        _inout_rBuffer.append("\"");
                        _inout_rBuffer.append("\n");
                        break;

                    case SQL_NODE_STRING:
                        _inout_rBuffer.append("SQL_STRING: ");
                        _inout_rBuffer.append("\"");
                        _inout_rBuffer.append(m_aNodeValue);
                        _inout_rBuffer.append("\"");
                        _inout_rBuffer.append("\n");
                        break;

                    case SQL_NODE_INTNUM:
                        _inout_rBuffer.append("SQL_INTNUM: ");
                        _inout_rBuffer.append(m_aNodeValue);
                        _inout_rBuffer.append("\n");
                        break;

                    case SQL_NODE_APPROXNUM:
                        _inout_rBuffer.append("SQL_APPROXNUM: ");
                        _inout_rBuffer.append(m_aNodeValue);
                        _inout_rBuffer.append("\n");
                        break;

                    case SQL_NODE_PUNCTUATION:
                        _inout_rBuffer.append("SQL_PUNCTUATION: ");
                        _inout_rBuffer.append(m_aNodeValue);
                        _inout_rBuffer.append("\n");
                        break;

                    case SQL_NODE_AMMSC:
                        _inout_rBuffer.append("SQL_AMMSC: ");
                        _inout_rBuffer.append(m_aNodeValue);
                        _inout_rBuffer.append("\n");
                        break;

                    case SQL_NODE_EQUAL:
                    case SQL_NODE_LESS:
                    case SQL_NODE_GREAT:
                    case SQL_NODE_LESSEQ:
                    case SQL_NODE_GREATEQ:
                    case SQL_NODE_NOTEQUAL:
                    case SQL_NODE_BITWISE_NOT:
                    case SQL_NODE_BITWISE_OR:
                    case SQL_NODE_BITWISE_AND:
                    case SQL_NODE_BITWISE_SHIFT_LEFT:
                    case SQL_NODE_BITWISE_SHIFT_RIGHT:
                        _inout_rBuffer.append(m_aNodeValue);
                        _inout_rBuffer.append("\n");
                        break;

                    case SQL_NODE_ACCESS_DATE:
                        _inout_rBuffer.append("SQL_ACCESS_DATE: ");
                        _inout_rBuffer.append(m_aNodeValue);
                        _inout_rBuffer.append("\n");
                        break;

                    case SQL_NODE_DATE:
                        _inout_rBuffer.append("SQL_DATE: ");
                        _inout_rBuffer.append(m_aNodeValue);
                        _inout_rBuffer.append("\n");
                        break;

                    case SQL_NODE_CONCAT:
                        _inout_rBuffer.append("||");
                        _inout_rBuffer.append("\n");
                        break;

                    default:
                        OSL_TRACE("-- %i", int(m_eNodeType));
                        OSL_FAIL("OSQLParser::ShowParseTree: unzulaessiger NodeType");
                }
            }
        }
#endif // OSL_DEBUG_LEVEL > 0
    // -----------------------------------------------------------------------------
    // Insert-Methoden
    //-----------------------------------------------------------------------------
    void OSQLParseNode::insert(sal_uInt32 nPos, OSQLParseNode* pNewSubTree)
        {
        BeAssert(pNewSubTree != nullptr);
        pNewSubTree->detach(); //dump all ownerships
        pNewSubTree->setParent(this);
        m_aChildren.insert(m_aChildren.begin() + nPos, pNewSubTree);
        }

    // removeAt-Methoden
    //-----------------------------------------------------------------------------
    OSQLParseNode* OSQLParseNode::removeAt(sal_uInt32 nPos)
        {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseNode::removeAt");
        OSL_ENSURE(nPos < (sal_uInt32) m_aChildren.size(), "Illegal position for removeAt");
        OSQLParseNodes::iterator aPos(m_aChildren.begin() + nPos);
        if (aPos == m_aChildren.end())
            {
            BeAssert(false);
            return nullptr;
            }

        OSQLParseNode* pNode = *aPos;
        pNode->detach();
        m_aChildren.erase(aPos);
        return pNode;
        }

    // Replace methods
    //-----------------------------------------------------------------------------
    OSQLParseNode* OSQLParseNode::replace(OSQLParseNode* pOldSubNode, OSQLParseNode* pNewSubNode)
        {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseNode::replace ");
        OSL_ENSURE(pOldSubNode != NULL && pNewSubNode != NULL, "OSQLParseNode: invalid nodes");
        OSL_ENSURE(pNewSubNode->getParent() == NULL, "OSQLParseNode: node already has getParent");
        OSL_ENSURE(::std::find(m_aChildren.begin(), m_aChildren.end(), pOldSubNode) != m_aChildren.end(),
                   "OSQLParseNode::Replace() Node not element of parent");
        OSL_ENSURE(::std::find(m_aChildren.begin(), m_aChildren.end(), pNewSubNode) == m_aChildren.end(),
                   "OSQLParseNode::Replace() Node already element of parent");

        pOldSubNode->detach();
        pNewSubNode->setParent(this);
        ::std::replace(m_aChildren.begin(), m_aChildren.end(), pOldSubNode, pNewSubNode);
        return pOldSubNode;
        }
    // -----------------------------------------------------------------------------
    void OSQLParseNode::parseLeaf(Utf8String& rString, const SQLParseNodeParameter& rParam) const
        {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseNode::parseLeaf");
        // ein Blatt ist gefunden
        // Inhalt dem Ausgabestring anfuegen
        switch (m_eNodeType)
            {
                case SQL_NODE_KEYWORD:
                {
                if (rString.size())
                    rString.append(" ");

                const Utf8String sT = OSQLParser::TokenIDToStr(m_nNodeID, rParam.bInternational ? &rParam.m_rContext : NULL);
                rString.append(sT);
                }   break;
                case SQL_NODE_STRING:
                {
                if (rString.size())
                    rString.append(" ");
                rString.append(SetQuotation(m_aNodeValue, Utf8StringHelper::createFromAscii("\'"), Utf8StringHelper::createFromAscii("\'\'")));
                break;
                }
                case SQL_NODE_ARRAY_INDEX:
                {
                if (rString.size())
                    rString.append(" ");
                rString.append("[");
                rString.append(m_aNodeValue);
                rString.append("]");
                break;
                }
                case SQL_NODE_NAME:
                    if (rString.size())
                        {
                        switch (rString[rString.size() - 1])
                            {
                                case ' ':
                                case '.': break;
                                default:
                                    if (!rParam.aMetaData.getCatalogSeparator().size()
                                        || rString[rString.size() - 1] != Utf8StringHelper::toChar(rParam.aMetaData.getCatalogSeparator())
                                        )
                                        rString.append(" ");
                                    break;
                            }
                        }
                    if (rParam.bQuote)
                        {
                        if (rParam.bPredicate)
                            {
                            rString.append("[");
                            rString.append(m_aNodeValue);
                            rString.append("]");
                            }
                        else
                            rString.append(SetQuotation(m_aNodeValue,
                                                        rParam.aMetaData.getIdentifierQuoteString(), rParam.aMetaData.getIdentifierQuoteString()));
                        }
                    else
                        rString.append(m_aNodeValue);
                    break;
                case SQL_NODE_ACCESS_DATE:
                    if (rString.size())
                        rString.append(" ");
                    rString.append("#");
                    rString.append(m_aNodeValue);
                    rString.append("#");
                    break;

                case SQL_NODE_INTNUM:
                case SQL_NODE_APPROXNUM:
                {
                Utf8String aTmp = m_aNodeValue;
                if (rParam.bInternational && rParam.bPredicate && rParam.cDecSep != '.')
                    aTmp = Utf8StringHelper::replace(aTmp, '.', rParam.cDecSep);

                if (rString.size())
                    rString.append(" ");
                rString.append(aTmp);

                }   break;
                case SQL_NODE_PUNCTUATION:
                    if (getParent() && SQL_ISRULE(getParent(), cast_spec) && Utf8StringHelper::toChar(m_aNodeValue) == '(') // no spaces in front of '('
                        {
                        rString.append(m_aNodeValue);
                        break;
                        }
                    // fall through
                default:
                    if (rString.size() && Utf8StringHelper::toChar(m_aNodeValue) != '.' && Utf8StringHelper::toChar(m_aNodeValue) != ':')
                        {
                        switch (rString[rString.size() - 1])
                            {
                                case ' ':
                                case '.': break;
                                default:
                                    if (!rParam.aMetaData.getCatalogSeparator().size()
                                        || rString[rString.size() - 1] != Utf8StringHelper::toChar(rParam.aMetaData.getCatalogSeparator())
                                        )
                                        rString.append(" ");
                                    break;
                            }
                        }
                    rString.append(m_aNodeValue);
            }
        }
    // -----------------------------------------------------------------------------
    OSQLParseNode::Rule OSQLParseNode::getKnownRuleID() const
        {
        if (!isRule())
            return UNKNOWN_RULE;
        return OSQLParser::RuleIDToRule(getRuleID());
        }
    // -----------------------------------------------------------------------------
    OSQLParseNodesContainer::OSQLParseNodesContainer()
        {}
    // -----------------------------------------------------------------------------
    OSQLParseNodesContainer::~OSQLParseNodesContainer()
        {}
    // -----------------------------------------------------------------------------
    void OSQLParseNodesContainer::push_back(OSQLParseNode* _pNode)
        {
        m_aNodes.push_back(_pNode);

        }
    // -----------------------------------------------------------------------------
    void OSQLParseNodesContainer::erase(OSQLParseNode* _pNode)
        {
        if (!m_aNodes.empty())
            {
            ::std::vector< OSQLParseNode* >::iterator aFind = ::std::find(m_aNodes.begin(), m_aNodes.end(), _pNode);
            if (aFind != m_aNodes.end())
                m_aNodes.erase(aFind);
            }
        }
    // -----------------------------------------------------------------------------
    void OSQLParseNodesContainer::clear()
        {
        m_aNodes.clear();
        }
    // -----------------------------------------------------------------------------
    void OSQLParseNodesContainer::clearAndDelete()
        {
        // clear the garbage collector
        while (!m_aNodes.empty())
            {
            OSQLParseNode* pNode = m_aNodes[0];
            while (pNode->getParent())
                {
                pNode = pNode->getParent();
                }
            delete pNode;
            }
        }
    }  // namespace connectivity
