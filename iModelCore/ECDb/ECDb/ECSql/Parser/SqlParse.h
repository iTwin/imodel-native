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


#ifndef _CONNECTIVITY_SQLPARSE_HXX
#define _CONNECTIVITY_SQLPARSE_HXX

#include "SqlNode.h"
#include "IParseContext.h"
#include "SqlBison.h"
#include "SqlTypes.h"

#include <map>
namespace connectivity
    {
    class OSQLScanner;
    class SQLError;

    //==========================================================================
    //= OParseContext
    //==========================================================================
    class OOO_DLLPUBLIC_DBTOOLS OParseContext : public IParseContext
        {
        public:
            OParseContext();

            virtual ~OParseContext();
            // retrieves language specific error messages
            virtual Utf8String getErrorMessage(ErrorCode _eCodes) const;

            // retrieves language specific keyword strings (only ASCII allowed)
            virtual Utf8String getIntlKeywordAscii(InternationalKeyCode _eKey) const;

            // finds out, if we have an international keyword (only ASCII allowed)
            virtual InternationalKeyCode getIntlKeyCode(const Utf8String& rToken) const;

            // determines the default international setting
            static const ::com::sun::star::lang::Locale& getDefaultLocale();

            /** set's the default locale which should be used when analyzing strings
                <p>If no locale is set, and any method which needs a locale is called, a default
                (en-US) is used.</p>
                <p>If, while parsing, the locale can be obtained from other sources (such as the number format
                set for a table column), the preferred locale is ignored.</p>
                */
            static void setDefaultLocale(const ::com::sun::star::lang::Locale& _rLocale);

            /** get's a locale instance which should be used when parsing in the context specified by this instance
                <p>if this is not overridden by derived classes, it returns the static default locale.</p>
                */
            virtual ::com::sun::star::lang::Locale getPreferredLocale() const;
        };

    //==========================================================================
    // OSQLParseNodesContainer
    // grabage collection of nodes
    //==========================================================================
    class OSQLParseNodesContainer
        {
        ::std::vector< OSQLParseNode* > m_aNodes;
        public:
            OSQLParseNodesContainer();
            ~OSQLParseNodesContainer();

            void push_back(OSQLParseNode* _pNode);
            void erase(OSQLParseNode* _pNode);
            bool empty() const;
            void clear();
            void clearAndDelete();
            size_t size() const { return m_aNodes.size(); }
            OSQLParseNode* front() const { return m_aNodes.front(); }
        };


    //==========================================================================
    //= OSQLParser
    //==========================================================================
    /** Parser for SQL92
    */
    class OSQLParser
        {
        friend class OSQLParseNode;
        friend struct SQLParseNodeParameter;

        private:
            typedef ::std::map< sal_uInt32, OSQLParseNode::Rule >   RuleIDMap;
            //    static parts for parsers
            static sal_uInt32 s_nRuleIDs[OSQLParseNode::rule_count + 1];
            static RuleIDMap s_aReverseRuleIDLookup;
            static OParseContext s_aDefaultContext;
            std::unique_ptr<OSQLScanner> m_scanner;

            // informations on the current parse action
            const IParseContext* m_pContext;
            std::unique_ptr<OSQLParseNode> m_pParseTree;    // result from parsing
            Utf8String m_sFieldName;    // current field name for a predicate
            Utf8String m_sErrorMessage;// current error msg
            RefCountedPtr< ::com::sun::star::beans::XPropertySet > m_xField;        // current field
        public:
            // if NULL, a default context will be used
            // the context must live as long as the parser
            explicit  OSQLParser(const IParseContext* _pContext = nullptr);
            ~OSQLParser();

            // Parsing an SQLStatement
            std::unique_ptr<OSQLParseNode> parseTree(Utf8String& rErrorMessage, Utf8String const& rStatement, sal_Bool bInternational = sal_False);

            OSQLScanner* GetScanner() { return m_scanner.get(); }
            // Access to the context
            const IParseContext& getContext() const { return *m_pContext; }

            /// access to the SQLError instance owned by this parser
            //const SQLError& getErrorHelper() const;

            // TokenIDToStr: Token-Name zu einer Token-Nr.
            static Utf8String TokenIDToStr(sal_uInt32 nTokenID, const IParseContext* pContext = NULL);

            // StrToTokenID: Token-Nr. zu einem Token-Namen.
            // static sal_uInt32 StrToTokenID(const Utf8String & rName);

            // RuleIDToStr gibt den zu einer RuleID gehoerenden Utf8String zurueck
            // (Leerstring, falls nicht gefunden)
            static Utf8CP RuleIDToStr(sal_uInt32 nRuleID);

            // StrToRuleID berechnet zu einem Utf8String die RuleID (d.h. ::com::sun::star::sdbcx::Index in yytname)
            // (0, falls nicht gefunden). Die Suche nach der ID aufgrund eines Strings ist
            // extrem ineffizient (sequentielle Suche nach Utf8String)!
            static sal_uInt32 StrToRuleID(const Utf8String & rValue);

            static OSQLParseNode::Rule RuleIDToRule(sal_uInt32 _nRule);

            // RuleId mit enum, wesentlich effizienter
            static sal_uInt32 RuleID(OSQLParseNode::Rule eRule);

            void error(const sal_Char* pFormat);
            int SQLlex(YYSTYPE* val);
            //#ifdef YYBISON
            void setParseTree(OSQLParseNode * pNewParseTree);

            // Is the parse in a special mode?
            // Predicate chack is used to check a condition for a field
            sal_Bool inPredicateCheck() const { return m_xField.IsValid(); }
            const Utf8String& getFieldName() const { return m_sFieldName; }

            void reduceLiteral(OSQLParseNode*& pLiteral, sal_Bool bAppendBlank);

            // pCompre will be deleted if it is not used
            sal_Int16 buildPredicateRule(OSQLParseNode*& pAppend, OSQLParseNode* pLiteral, OSQLParseNode*& pCompare, OSQLParseNode* pLiteral2 = NULL);

            sal_Int16 buildLikeRule(OSQLParseNode*& pAppend, OSQLParseNode*& pLiteral, const OSQLParseNode* pEscape);
   
            //#else
            //#endif
        };
    }


#endif //_CONNECTIVITY_SQLPARSE_HXX
