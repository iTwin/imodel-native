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
            IndentBlock(DebugWriter& writer)
                :m_writer(writer)
                {
                writer.Indent();
                }
            ~IndentBlock()
                {
                m_writer.UnIndent();
                }
        };

    private:
        int m_currentIndent;
        Utf8String m_final;
        int m_indentSize;
        Utf8String m_lineBuffer;
        Utf8String m_formatBuffer;
        std::vector<std::pair<int, Utf8String>> m_buffer;

    private:
        void VSWrite(Utf8CP fmt, va_list args)
            {
            m_formatBuffer.clear();
            m_formatBuffer.VSprintf(fmt, args);

            m_lineBuffer.clear();
            for (auto c : m_formatBuffer)
                {
                if (c == '\r')
                    continue;
                if (c == '\n')
                    {
                    m_buffer.push_back(std::make_pair(m_currentIndent, m_lineBuffer));
                    m_lineBuffer.clear();
                    continue;
                    }

                m_lineBuffer.append(1, c);
                }

            if (!m_lineBuffer.empty())
                m_buffer.push_back(std::make_pair(m_currentIndent, m_lineBuffer));
            }

    public:
        DebugWriter(int indentSize = 3)
            :m_currentIndent(0), m_indentSize(indentSize)
            {
            m_lineBuffer.reserve(1024);
            m_formatBuffer.reserve(1024);
            }
        ~DebugWriter() {}

        void AppendLine(Utf8CP fmt, ...)
            {
            va_list args;
            va_start(args, fmt);
            VSWrite(fmt, args);
            va_end(args);
            } 

        IndentBlock::Ptr CreateIndentBlock()
            {
            return IndentBlock::Ptr(new IndentBlock(*this));
            }
        int GetCurrentIndent() const { return m_currentIndent; }
        void Indent() { m_currentIndent++; }
        void UnIndent() { if (m_currentIndent > 0) m_currentIndent--; }
        bool Empty() const { return m_buffer.empty() && m_buffer.empty(); }
        void Clear() { m_currentIndent = 0; m_buffer.clear(); m_final.clear(); }
        void Flush()
            {
            if (m_buffer.empty())
                return;

            std::map<int, Utf8String> padding;
            for (auto& a : m_buffer)
                {
                if (padding.find(a.first) != padding.end())
                    continue;

                padding[a.first] = Utf8String(a.first*m_indentSize, ' ');
                }

            for (auto& a : m_buffer)
                {
                m_final.append(padding[a.first]);
                m_final.append(a.second);
                m_final.append("\r\n");
                }

            m_formatBuffer.clear();
            m_lineBuffer.clear();
            m_buffer.clear();
            }
        Utf8CP ToString()
            {
            Flush();
            return m_final.c_str();
            }
        void fo(){}
    };
 END_BENTLEY_SQLITE_EC_NAMESPACE