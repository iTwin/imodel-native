/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/TextBlock/TextContext.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnHandlers/TextBlock/TextBlockAPI.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Venkat.Kalyan   06/07
//=======================================================================================
struct TextContext
    {
    private:    TextBlockCP m_textBlock;
    private:    ParagraphCP m_paragraph;
    private:    LineCP      m_line;

    public:             TextContext     (TextBlockCP textBlock, ParagraphCP paragraph, LineCP line) : m_textBlock (textBlock), m_paragraph (paragraph), m_line (line) { };
    public:             TextContext     () { };

    public: void        SetTextBlock    (TextBlockCP textBlock) { m_textBlock = textBlock; }
    public: TextBlockCP GetTextBlock    () const                { return m_textBlock; }

    public: void        SetParagraph    (ParagraphCP paragraph) { m_paragraph = paragraph; }
    public: ParagraphCP GetParagraph    () const                { return m_paragraph; }

    public: void        SetLine         (LineCP line)           { m_line = line; }
    public: LineCP      GetLine         () const                { return m_line; }

    }; // TextContext

//=======================================================================================
// @bsiclass                                                    Venkat.Kalyan   11/07
//=======================================================================================
struct TextDrawContext : TextContext
    {
    private:    DgnModelP    m_modelRef;
    private:    ViewContextP    m_viewContext;
    private:    bool            m_firstRunInLine;

    public:                 TextDrawContext     (TextBlockCP textBlock, ViewContextP context, DgnModelP modelRef) : TextContext (textBlock, NULL, NULL), m_viewContext (context), m_modelRef (modelRef), m_firstRunInLine (false) { };

    public: DgnModelP    GetDgnModelP        () const                { return m_modelRef; }
    public: ViewContextP    GetViewContextP     () const                { return m_viewContext; }

    public: void            SetFirstRunInLine   (bool firstRunInLine)   { m_firstRunInLine = firstRunInLine; }
    public: bool            IsFirstRunInLine    () const                { return m_firstRunInLine; }

    }; // TextDrawContext

//=======================================================================================
// @bsiclass                                                    Venkat.Kalyan   05/04
//=======================================================================================
struct ProcessContext : TextContext
    {
    private: ProcessLevel m_processLevel;

    public:                 ProcessContext  (TextBlockCP textBlock, ParagraphCP paragraph, LineCP line) : TextContext (textBlock, paragraph, line) { };

    public: void            SetProcessLevel (ProcessLevel processLevel) { m_processLevel = processLevel; }
    public: ProcessLevel    GetProcessLevel () const                    { return m_processLevel; }

    }; // ProcessContext

END_BENTLEY_DGNPLATFORM_NAMESPACE
