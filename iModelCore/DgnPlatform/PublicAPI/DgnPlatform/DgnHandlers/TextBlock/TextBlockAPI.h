/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/TextBlock/TextBlockAPI.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

//! @addtogroup TextModule
//!
//! The text module refers to element-based text, including the classes and interfaces that can be used to work both with text elements, and other element types that expose text.
//!
//! <h2>Overview</h2>
//!
//! Text in MicroStation takes many forms. For example, there are free-standing text elements, dimensions, notes, tags, as well as many other custom element types that contain text. The bulk of the text module documentation refers to a relatively abstract form of text that is supported by all of the above: the TextBlock. Free-standing text is represented by text node elements and text elements (henceforth referred to simply as text elements), and aside from being compatible with TextBlock are also input to the TextString object, which can be used to draw and measure text. Text in public interfaces is typically represented by the TextBlock class. TextBlock, aside from being a container for formatting and strings, can perform layout, and computes the origins of runs and lines based on formatting options.
//!
//! @note While MicroStation still supports locale-encoded fonts (e.g. MicroStation RSC fonts and non-Unifont SHX fonts), and the underlying file format still stores text in the locale of its font, the API strives to only deal in Unicode (specifically, UCS-2 encoding), and it will do required conversions only when necessary. Keep in mind that just because you can provide a Unicode string doesn't mean that the font you provided allows the piece of text to display all of the characters.
//!
//! <h3>Generic Query/Edit</h3>
//! While text elements are used to store generic free-standing text, there are many other element types that can expose text for purposes of querying and/or editing. If you want to work with text at a level above specific element types, use the ITextQuery and ITextEdit interfaces (e.g. dynamic cast the handler to ITextQuery or ITextEdit; if you have an ElementHandle, see ElementHandle::GetITextQuery and ElementHandle::GetITextEdit). This utilizes TextBlock to transfer text data back and forth. The ITextQuery interface can also be used to identify free-standing text elements via ITextQuery::IsTextElement; this can be used to determine if an element can simply expose text, as opposed to actually being text.
//!
//! <h3>Modifying Text</h3>
//! The primary class used for modifying text element data (including reading/writing free-standing text elements) is TextBlock. It is a high-level object that allows for mixed formatting and layout (e.g. for multiple lines), as well as many operations to modify its data. In the vast majority of cases, you will want to use TextBlock to read/write even text elements. A TextBlock will directly read/write both type 17 text elements and type 7 text node elements as appropriate. A TextBlock can also be provided to other high-level interfaces, and element types such as dimensions can process a TextBlock and create appropriate dimension elements.
//!
//! <h3>Drawing and Measuring Text</h3>
//! The primary class used for drawing (and measuring) text is TextString. It is a performance-oriented immutable single-line collection of like-formatted characters, and can be created from text elements or from raw information (e.g. a Unicode string and TextStringProperties). A TextString can be thought of as analagous to a type 17 text element, although it cannot be used to generate such an element.
//!
//! <h2>Text Block API</h2>
//!
//! The Text Block API (centered around the TextBlock class) is meant to be the full-featured way to read, modify, and create text in MicroStation. It should replace all mdlText_*, mdlTextNode_*, and line arranger functionality. While this API was targetted at dealing with text elements, it is generic enough to support other element types that can expose text, even if they do not directly work with text elements. You should never attempt to deal directly with text elements (via the handler or otherwise); this API will ensure valid elements are written to the file, and will work around bad data if possible when reading from the file.<br>
//! <br>
//! It is important to note that TextBlock objects are meant to be short-lived, and are only valid in the context of a single DgnModel. Storing TextBlock objects for long periods of time can potentially invalidate cached data, and attempting to mix and match TextBlock objects from different caches can lead to (at least) unit and color problems.<br>
//! <br>
//! The implementation details of the C++ objects that represent the pieces of the DOM are hidden, and the only way to 'own' any TextBlock-related objects to ask a factory method to give you a smart pointer. In most circumstances, providing an object you created to TextBlock will cause it to clone the object and own the copy; thus you cannot provide TextBlock an object, and modify it later hoping it will modify any internals of the TextBlock.<br>
//! <br>
//! The classes you will most often use are:
//! <ul>
//!   <li>Caret - used to navigate a TextBlock's DOM</li>
//!   <li><i>DOM Objects</i>
//!       <ul>
//!       <li>TextBlock - a container for TextBlockProperties and one or more Paragraph objects</li>
//!       <li>Paragraph - a container for ParagraphProperties and one or more Run objects</li>
//!       <li>Run - the most atomic unit in the DOM (contains at least a RunProperties object)
//!           <ul>
//!           <li>CharStream - a single-line single-format collection of characters</li>
//!           <li>Fraction - a numerator and denominator (can have either or both)
//!               <ul>
//!                   <li>DiagonalBarFraction - numerator and denominator separated by a diagonal line</li>
//!                   <li>HorizontalBarFraction - center-aligned numerator and denominator separated by a horizontal line</li>
//!                   <li>NoBarFraction - left-aligned numerator and denominator (NOT separated by a horizontal line)</li>
//!               </ul>
//!           </li>
//!           <li>WhiteSpace - a special class of non-printable characters that affect layout
//!               <ul>
//!                   <li>ParagraphBreak - ends a paragraph (also implicitly ends a line)</li>
//!                   <li>LineBreak - ends a line; next line remains in the same paragraph</li>
//!                   <li>Tab - moves next run to the next tab stop</li>
//!               </ul>
//!           </li>
//!           </ul>
//!       </li>
//!       </ul>
//!   </li>
//!   <li><i>Property Objects</i>
//!       <ul>
//!           <li>TextBlockProperties - formatting properties for TextBlock</li>
//!           <li>ParagraphProperties - formatting properties for Paragraph</li>
//!           <li>RunProperties - formatting properties for Run (and its sub-classes)</li>
//!       </ul>
//!   </li>
//! </ul>
//!
//! <h3>TextBlock as a DOM (Document Object Model)</h3>
//! The TextBlock class represents a DOM of paragraphs, lines, and runs. A line is merely a layout concept, and is not directly exposed, so most discussion only involves TextBlock as a whole (which contains one or more paragraphs), paragraphs (which can be thought of as containing one or more runs), and runs. TextBlock as a whole is your primary mechanism for accessing document properties, as well as appending and modifying text. Paragraphs are relatively thin wrappers, and exist primarily to hold a small set of per-paragraph properties. Runs represent the most atomic units of the DOM, and can never span lines (or paragraphs). In a word-wrapped document, runs will be provided based on how the layout split the runs for word-wrapping.
//!
//! <h3>Navigating the DOM</h3>
//! The primary object for navigating the DOM is the Caret. Just as with a text editor, a Caret is an indicator between two characters that indicates a position. This particular implementation tracks a single character index, but is generally treated as if the caret sits just before the character it indicates. Most methods used to edit a TextBlock accept one or more Caret objects to identify the region being modified. Caret objects are intrinsically associated with a TextBlock, and have getters to provide the current TextBlock, Paragraph, and Run they point at (if available). It also has several methods for moving carets in both directions.<br>
//!
//! <h2>Using TextBlock...</h2>
//!
//! <h3>...To Read Text</h3>
//! To read text, you should acquire a TextBlock through ITextQuery, and then create one or more carets to traverse the DOM, extracting data as necessary. The pseudocode below shows a common loop to process a TextBlock:<br>
//! <br>
//! @code
//! // Ensure that the element can expose text.
//! //  If you only want to process free-standing text elements, you can check textQueryHandler->IsTextElement (), which implies a single piece of text.
//! ITextQueryCP textQueryHandler = eh.GetITextQuery ();
//! if (NULL == textQueryHandler)
//!     return;
//!
//! // Some elements expose multiple pieces of text (e.g. dimensions); in general, you must be prepared to accept multiple pieces of text, unless textQueryHandler->IsTextElement ().
//! //  Some element handlers (or associated APIs) will provide pieces of text via TextBlock through their own more specific getter methods, bestowing specific meanings to each piece of text.
//! T_ITextPartIdPtrVector textPartIds;
//! textQueryHandler->GetTextPartIds (eh, *ITextQueryOptions::CreateDefault (), textPartIds);
//!
//! for each (ITextPartIdPtr textPartId in textPartIds)
//!     {
//!     // Ask the handler for the specific piece of text.
//!     //  The TextBlock is in no way directly tied to the underlying textual data, and you must use ITextEdit::ReplaceText to actually modify the element.
//!     TextBlockPtr textBlock = textQueryHandler->GetTextPart (eh, *textPartId);
//!
//!     // Each paragraph has unique properties, so to consume all properties, you must iterate paragraphs.
//!     //  You could also simply iterate runs and continually check to see if the paragraph or paragraph properties has changed.
//!     for (CaretPtr paragraphCaret = textBlock->CreateStartCaret (); !paragraphCaret->IsAtEnd (); paragraphCaret->MoveToNextParagraph ())
//!         {
//!         // Perform logic to deal with the new paragraph; get the current paragraph using paragraphCaret->GetCurrentParagraphCP ().
//!
//!         CaretPtr runCaret = paragraphCaret->Clone ();
//!         do
//!             {
//!             // Perform logic to deal with the new run; get the current run using runCaret->GetCurrentRunCP ().
//!             // You will likely want to find a way to switch on run type in order to gather specific information about each (e.g. CharStream, Fraction, ParagraphBreak, LineBreak, and Tab).
//!             }
//!         while (SUCCESS == runCaret->MoveToNextRunInParagraph ());
//!         }
//!     }
//! @endcode
//!
//! <h3>...To Create Text</h3>
//! Free-standing text elements can be created by first creating a TextBlock, setting properties and adding runs, and then generating an element from it. See TextBlock::Create, and TextHandlerBase::CreateElement. TextBlock contains methods for adding specific kinds of runs (e.g. TextBlock::AppendText and TextBlock::AppendParagraphBreak). Additionally, the only DOM object you will create directly is the TextBlock itself; all other DOM objects will be created on your behalf (e.g. runs and paragraphs). The instance of the TextBlock stores a ParagraphProperties and a RunProperties that will be used when appending paragraphs and runs. All DOM objects added will assume these properties until you change them.<br>
//! <br>
//! @code
//! DgnTextStylePtr myTextStyle = DgnTextStyle::GetByName (...);
//! TextBlockPtr    textBlock   = TextBlock::Create (*myTextStyle, myDgnCache);
//!
//! // Non-formatting information, such as origin and rotation, are accessed directly on TextBlock.
//! textBlock->SetUserOrigin (...);
//! textBlock->SetOrientation (...);
//!
//! // Append the paragraphs, setting unique properties, and then adding their child runs.
//! foreach (ParagraphToAdd)
//!     {
//!     if (!textBlock->IsEmpty ())
//!         textBlock->AppendParagraphBreak (...);
//!
//!     // If paragraph properties have changed...
//!     ParagraphPropertiesPtr paraProps = ParagraphProperties::Create (...); // or textBlock->GetParagraphPropertiesForAdd ().Clone ();
//!     paraProps->Set... (...);
//!     ...
//!     textBlock->SetParagraphPropertiesForAdd (*paraProps);
//!
//!     // Append the runs.
//!     foreach (RunToAdd in ParagraphToAdd)
//!         {
//!         // If run properties have changed...
//!         RunPropertiesPtr runProps = RunProperties::Create (...); // or textBlock->GetRunPropertiesForAdd ().Clone ();
//!         runProps->Set... (...);
//!         ...
//!         textBlock->SetRunPropertiesForAdd (*runProps);
//!
//!         textBlock->AppendText (...);
//!         // -or-
//!         textBlock->AppendFraction (...);
//!         // -or-
//!         textBlock->AppendLineBreak (...);
//!         // -or-
//!         textBlock->AppendTab (...);
//!         }
//!     }
//!
//! EditElementHandle eeh;
//! if (TextBlock::TO_ELEMENT_RESULT_Success != TextHandlerBase::CreateElement (eeh, NULL, *textBlock))
//!     return ERROR;
//!
//! return SUCCESS;
//! @endcode

#include <DgnPlatform/DgnCore/RunPropertiesBase.h>

#include "TextAPICommon.h"

#include "Caret.h"
#include "CharStream.h"
#include "Fraction.h"
#include "Indentation.h"
#include "Paragraph.h"
#include "Run.h"
#include "RunProperties.h"
#include "TextBlock.h"
#include "TextBlockIterators.h"
#include "WhiteSpace.h"

//__PUBLISH_SECTION_END__

#include "ITextListener.h"
#include "Line.h"
#include "TextBlockNode.h"

//__PUBLISH_SECTION_START__

/** @endcond */
