/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextBlock/AlongTextDependency.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- AlongTextDependency ----------------------------------------------------------------------------------------------------- AlongTextDependency --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/05
//---------------------------------------------------------------------------------------
bool AlongTextDependency::IsRootValid (DisplayPath const & path)
    {
    int count = path.GetCount ();
    
    if (0 == count)
        return false;

    for (int i = 0; i < count; ++i)
        {
        if (SHARED_CELL_ELM == path.GetPathElem(i)->GetLegacyType())
            return false;
        }
    
    // Check for acceptable element type at cursor index.
    ElementRefP elemRef = path.GetCursorElem ();
    if (NULL == elemRef)
        return false;

    switch (elemRef->GetLegacyType())
        {
        case CMPLX_STRING_ELM:
        case CMPLX_SHAPE_ELM:
        case SHAPE_ELM:
        case ELLIPSE_ELM:
        case MULTILINE_ELM:
        case ARC_ELM:
        case CURVE_ELM:
        case BSPLINE_CURVE_ELM:
            {
            return true;
            }
        
        case LINE_ELM:
            {
            DPoint3d        points[2];
            ElementHandle   eh (elemRef);
            
            LineStringUtil::Extract (points, NULL, *eh.GetElementCP (), *eh.GetDgnModelP ());
            
            return !bsiDPoint3d_pointEqual (&points[0], &points[1]);
            }

        case LINE_STRING_ELM:
            {
            // I can understand not supporting zero-length lines (above), but this doesn't quite do that...
            //  But I'm also afraid there's some old fragile code that would break should we skip this (e.g. create in new version, edit in old version)...
            
            ElementHandle           eh          (elemRef);
            int                     numPoints   = LineStringUtil::GetCount (*eh.GetElementCP ());
            ScopedArray<DPoint3d>   points      ((size_t)numPoints);
            
            LineStringUtil::Extract (points.GetData (), &numPoints, *eh.GetElementCP (), *eh.GetDgnModelP ());
            
            for (int i = 0; i < (numPoints - 1); ++i)
                {
                if (!bsiDPoint3d_pointEqual (&points.GetData ()[i], &points.GetData ()[i + 1]))
                    return true;
                }
            
            return false;
            }
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/09
//---------------------------------------------------------------------------------------
BentleyStatus AlongTextDependency::CreateDisplayPath (DisplayPath& path, DgnModelR homeCache) const
    {
    DependencyRoot root;
    DependencyManager::ResolveRefInSameFile (root, &homeCache, m_rootId);
    
    if (NULL == root.ref)
        return ERROR;
    
    path.SetPath (root.ref, &homeCache);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/09
//---------------------------------------------------------------------------------------
bool AlongTextDependency::IsValid (DgnModelR homeCache) const
    {
    if (INVALID_ELEMENTID == m_rootId)
        return false;
    
    DisplayPath path;
    if (SUCCESS != this->CreateDisplayPath (path, homeCache))
        return false;
    
    return AlongTextDependency::IsRootValid (path);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Venkat.Kalyan                   12/2005
//---------------------------------------------------------------------------------------
BentleyStatus AlongTextDependency::DoesElementMatchDependency (ElementHandleCR eh, bool& matches) const
    {
    matches = false;
    
    TextBlockPtr roundTrippedTB = TextBlock::Create (eh);
    
    ParagraphCP firstParagraph = roundTrippedTB->GetParagraph (0);
    if (NULL == firstParagraph)
        { BeAssert (false); return ERROR; }
    
    LineCP firstLine = firstParagraph->GetLine (0);
    if (NULL == firstLine)
        { BeAssert (false); return ERROR; }
    
    RunCP firstRun = firstLine->GetRun (0);
    if (NULL == firstRun)
        { BeAssert (false); return ERROR; }
    
    DPoint3d correctOrigin = firstRun->GetOrigin ();
    
    if (eh.GetElementCP ()->ToText_node_2d().Is3d())
        {
        matches = eh.GetElementCP ()->ToText_node_3d().origin.IsEqual (correctOrigin, mgds_fc_epsilon);
        }
    else
        {
        DPoint2d correctOrigin2d;
        correctOrigin2d.init (&correctOrigin);
        
        matches = eh.GetElementCP ()->ToText_node_2d().origin.IsEqual (correctOrigin2d, mgds_fc_epsilon);
        }

    return SUCCESS;
    }
