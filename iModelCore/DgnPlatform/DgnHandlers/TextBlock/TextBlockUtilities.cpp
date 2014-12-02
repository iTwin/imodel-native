/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextBlock/TextBlockUtilities.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- TextBlockUtilities ------------------------------------------------------------------------------------------------------- TextBlockUtilities --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/08
//---------------------------------------------------------------------------------------
DRange3d TextBlockUtilities::ComputeJustificationRange (DRange3dCR nominalRange, DRange3dCR exactRange)
    {
    DRange3d jRange = nominalRange;

    jRange.high.x = exactRange.high.x;

    return jRange;
    }

#if defined (NEEDS_WORK_DGNITEM)
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
void TextBlockUtilities::AppendEdfs (TextBlockR textBlock, WStringCR fullString, TextEDParamCR textEDParams)
    {
    EDFieldVector edFields;
    for (int i = 0; i < textEDParams.numEDFields; ++i)
        edFields.push_back (textEDParams.edField[i]);
    
    TextBlockUtilities::AppendEdfs (textBlock, fullString, edFields);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2011
//---------------------------------------------------------------------------------------
void TextBlockUtilities::AppendEdfs (TextBlockR textBlock, WStringCR fullString, EDFieldVectorCR edFields)
    {
    EDFieldVector sortedEDFields = edFields;
    
    TextString::SortAndValidateEdfs (sortedEDFields, fullString.length ());
        
    if (sortedEDFields.empty ())
        {
        textBlock.AppendText (fullString.c_str ());
        return;
        }
    
    if (sortedEDFields.front ().start > 0)
        textBlock.AppendText (fullString.substr (0, sortedEDFields.front ().start).c_str ());

    for (size_t iEDField = 0; iEDField < sortedEDFields.size (); ++iEDField)
        {
        TextEDFieldCR currEdf = sortedEDFields[iEDField];
                
        textBlock.AppendEnterDataField (fullString.substr (currEdf.start, currEdf.len).c_str (), currEdf.len, (EdfJustification)currEdf.just);
        
        size_t nextStart = (((sortedEDFields.size () - 1) == iEDField) ? fullString.size () : (sortedEDFields[iEDField + 1].start));

        if ((size_t)(currEdf.start + currEdf.len) < nextStart)
            textBlock.AppendText (fullString.substr ((currEdf.start + currEdf.len), nextStart - (currEdf.start + currEdf.len)).c_str ());
        }
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2010
//---------------------------------------------------------------------------------------
DgnGlyphRunLayoutFlags TextBlockUtilities::ComputeRunLayoutFlags (TextParamWideCR textParams, DPoint2dCR fontSize)
    {
    DgnGlyphRunLayoutFlags flags = GLYPH_RUN_LAYOUT_FLAG_None;
    
    if (textParams.flags.vertical)
        flags = (DgnGlyphRunLayoutFlags)(flags | GLYPH_RUN_LAYOUT_FLAG_Vertical);
    
    if (textParams.exFlags.backwards || (fontSize.x < 0.0))
        flags = (DgnGlyphRunLayoutFlags)(flags | GLYPH_RUN_LAYOUT_FLAG_Backwards);
    
    if (textParams.exFlags.upsidedown || (fontSize.y < 0.0))
        flags = (DgnGlyphRunLayoutFlags)(flags | GLYPH_RUN_LAYOUT_FLAG_UpsideDown);
    
    return flags;
    }
