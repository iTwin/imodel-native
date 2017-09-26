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


#ifndef _CONNECTIVITY_SQLSCAN_HXX
#define _CONNECTIVITY_SQLSCAN_HXX
#pragma once

#include <stdarg.h>
#include "IParseContext.h"
#include "SqlTypes.h"
#include "SqlParse.h"

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif


namespace connectivity
    {
    //==========================================================================
    //= OSQLScanner
    //==========================================================================
    /** Scanner for SQL92
    */
    class OOO_DLLPUBLIC_DBTOOLS OSQLScanner
        {
        private:
            const IParseContext* m_pContext; // context for parse, knows all international stuff
            const Utf8String m_sStatement; // statement to parse
            Utf8String m_sErrorMessage;
            sal_Int32 m_nCurrentPos; // next position to read from the statement
            sal_Bool m_bInternational; // do we have a statement which may uses
            sal_Int32 m_nRule; // rule to be set
            OSQLParseNodesContainer m_pGarbageCollector;
            void SetRule(sal_Int32 nRule) { m_nRule = nRule; }

        public:
            yyscan_t  yyscanner; //do not add m_ with this var as it used in macros;

        public:
            OSQLScanner(Utf8CP rNewStatement, const IParseContext* pContext, sal_Bool bInternational);
            virtual ~OSQLScanner();
            OSQLParseNodesContainer& GetContainer() { return m_pGarbageCollector; }
            inline static void * operator new(size_t nSize) { return malloc(nSize); }
            inline static void * operator new(size_t, void* _pHint) { return _pHint; }
            inline static void operator delete(void * pMem) { free(pMem); }
            inline static void operator delete(void *, void*) {}
            virtual void SQLyyerror(const char *fmt);
            virtual void output(sal_Int32) { OSL_ASSERT("Internal error in sdblex.l: output not possible"); }
            virtual void ECHO(void) { OSL_ASSERT("Internal error in sdblex.l: ECHO not possible"); }
            virtual IParseContext::InternationalKeyCode getInternationalTokenID(const char* sToken) const;
            // setting the new information before scanning
            const Utf8String& getErrorMessage() const { return m_sErrorMessage; }
            sal_Int32 SQLyygetc(void);
            Utf8String getStatement() const { return m_sStatement; }
            sal_Int32 SQLlex(YYSTYPE* val);
            // set this as scanner for flex
            // rules settings
            sal_Int32    GetCurrentRule() const;
            sal_Int32    GetGERRule() const;
            sal_Int32    GetENGRule() const;
            sal_Int32    GetSQLRule() const;
            sal_Int32    GetDATERule() const;
            sal_Int32    GetSTRINGRule() const;
            inline sal_Int32 GetCurrentPos() const { return m_nCurrentPos; }

            OSQLParseNode* NewNode(const sal_Char* pNewValue, SQLNodeType eNodeType, sal_uInt32 nNodeID = 0);
            OSQLParseNode* NewNode(Utf8String const& _rNewValue, SQLNodeType eNodeType, sal_uInt32 nNodeID = 0);


        };
    }

#endif
