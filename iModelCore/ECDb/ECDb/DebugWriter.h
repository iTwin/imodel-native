/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DebugWriter.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbInternalTypes.h"
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct DebugWriter : NonCopyableClass
    {
    struct IndentBlock : NonCopyableClass
        {
        typedef std::unique_ptr<IndentBlock> Ptr;
        private:
            DebugWriter& m_writer;
        public:
            IndentBlock(DebugWriter& writer);
            ~IndentBlock();
        };

    private:
        int m_currentIndent;
        int m_indentSize;
        Utf8String m_final;
        Utf8String m_lineBuffer;
        Utf8String m_formatBuffer;
        std::vector<std::pair<int, Utf8String>> m_buffer;

    private:
        void VSWrite(Utf8CP fmt, va_list args);

    public:
        DebugWriter(int indentSize = 3);
        ~DebugWriter() {}

        void AppendLine(Utf8CP fmt, ...);
        IndentBlock::Ptr CreateIndentBlock();
        void Indent();
        void UnIndent();
        bool Empty() const;
        void Clear();
        void Flush();
        Utf8CP ToString();
    };
 END_BENTLEY_SQLITE_EC_NAMESPACE