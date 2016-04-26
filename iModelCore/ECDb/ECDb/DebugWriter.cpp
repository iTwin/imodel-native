/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DebugWriter.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  04/2016
//+---------------+---------------+---------------+---------------+---------------+------
DebugWriter::IndentBlock::IndentBlock(DebugWriter& writer)
    :m_writer(writer)
    {
    writer.Indent();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  04/2016
//+---------------+---------------+---------------+---------------+---------------+------
DebugWriter::IndentBlock::~IndentBlock()
    {
    m_writer.UnIndent();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  04/2016
//+---------------+---------------+---------------+---------------+---------------+------
void DebugWriter::VSWrite(Utf8CP fmt, va_list args)
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  04/2016
//+---------------+---------------+---------------+---------------+---------------+------
DebugWriter::DebugWriter(int indentSize)
    :m_currentIndent(0), m_indentSize(indentSize)
    {
    m_lineBuffer.reserve(1024);
    m_formatBuffer.reserve(1024);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  04/2016
//+---------------+---------------+---------------+---------------+---------------+------
void DebugWriter::AppendLine(Utf8CP fmt, ...)
    {
    va_list args;
    va_start(args, fmt);
    VSWrite(fmt, args);
    va_end(args);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  04/2016
//+---------------+---------------+---------------+---------------+---------------+------
DebugWriter::IndentBlock::Ptr DebugWriter::CreateIndentBlock()
    {
    return IndentBlock::Ptr(new IndentBlock(*this));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  04/2016
//+---------------+---------------+---------------+---------------+---------------+------
void DebugWriter::Indent() { m_currentIndent++; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  04/2016
//+---------------+---------------+---------------+---------------+---------------+------
void DebugWriter::UnIndent() { if (m_currentIndent > 0) m_currentIndent--; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  04/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool DebugWriter::Empty() const { return m_buffer.empty() && m_buffer.empty(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  04/2016
//+---------------+---------------+---------------+---------------+---------------+------
void DebugWriter::Clear() { m_currentIndent = 0; m_buffer.clear(); m_final.clear(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  04/2016
//+---------------+---------------+---------------+---------------+---------------+------
void DebugWriter::Flush()
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  04/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP DebugWriter::ToString()
    {
    Flush();
    return m_final.c_str();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE