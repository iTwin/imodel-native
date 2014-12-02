/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/GPArray.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GPArray::ToElement (EditElementHandleR outEeh, ElementHandleCP templateEh, bool is3d, bool isClosed, DgnModelR model) const
    {
#if defined (NEEDS_WORK_DGNITEM)
    BentleyStatus       status = SUCCESS;
    ElementAgenda       holeAgenda;
    EditElementHandle   solidEeh;
    EditElementHandle   eeh;
    size_t              count = GetCount();
    size_t              breakIndex;

    // NOTE: For grouped hole create to succeed we must construct closed loops...detect multiple loops and override isClosed...
    isClosed = (isClosed ? true : this->FindMajorBreakAfter (0, breakIndex));

    for (int i=0; (size_t)i < count && SUCCESS == status; )
        {
        EditElementHandle  tmpEeh;

        switch (GetCurveType (i))
            {
            case GPCurveType::Ellipse:
                {
                DConic4d    conic;
                DEllipse3d  ellipse;
                bool        isEllipse;

                if (SUCCESS != (status = GetConicOrEllipse (&i, &conic, &ellipse, isEllipse, !is3d)))
                    break;

                if (!isEllipse)
                    {
                    MSBsplineCurve  curve;

                    if (SUCCESS != curve.InitFromDConic4d (conic))
                        {
                        status = ERROR;
                        break;
                        }

                    // Don't create complex shape for single closed element (or single closed element loop)...
                    bool    closed = (isClosed && !eeh.IsValid () && ((i && IsMajorBreak (i-1)) || i == count) && curve.IsPhysicallyClosed (1.0e-5));

                    if (closed)
                        status = (BentleyStatus) curve.CopyClosed (curve);

                    if (SUCCESS == status)
                        status = (BentleyStatus) BSplineCurveHandler::CreateBSplineCurveElement (tmpEeh, NULL, curve, is3d, model);

                    curve.ReleaseMem ();

                    break;
                    }

                // Don't create complex shape for single closed element (or single closed element loop)...
                bool    closed = (isClosed && !eeh.IsValid () && ((i && IsMajorBreak (i-1)) || i == count) && ellipse.isFullEllipse ());

                if (closed)
                    status = EllipseHandler::CreateEllipseElement (tmpEeh, NULL, ellipse, is3d, model);
                else
                    status = ArcHandler::CreateArcElement (tmpEeh, NULL, ellipse, is3d, model);
                break;
                }

            case GPCurveType::Bezier:
            case GPCurveType::BSpline:
                {
                MSBsplineCurve  curve;

                if (SUCCESS != (status = GetBCurve (&i, &curve)))
                    break;

                // Don't create complex shape for single closed element (or single closed element loop)...
                bool    closed = (isClosed && !eeh.IsValid () && ((i && IsMajorBreak (i-1)) || i == count) && curve.IsPhysicallyClosed (1.0e-5));

                if (closed)
                    status = (BentleyStatus) curve.CopyClosed (curve);

                if (SUCCESS == status)
                    status = (BentleyStatus) BSplineCurveHandler::CreateBSplineCurveElement (tmpEeh, NULL, curve, is3d, model);

                curve.ReleaseMem ();

                break;
                }

            case GPCurveType::LineString:
                {
                int         nPoints;
                DPoint3d    points[MAX_VERTICES];

                if (SUCCESS != (status = GetLineString (&i, points, &nPoints, MAX_VERTICES)))
                    break;

                // Don't create complex shape for single closed element (or single closed element loop)...
                bool    closed = (isClosed && !eeh.IsValid () && ((i && IsMajorBreak (i-1)) || i == count) && LegacyMath::RpntEqual (&points[0], &points[nPoints-1]));

                if (2 == nPoints)
                    {
                    DSegment3d  segment;

                    segment.Init (points[0], points[1]);

                    status = LineHandler::CreateLineElement (tmpEeh, NULL, segment, is3d, model);
                    }
                else if (closed)
                    {
                    status = ShapeHandler::CreateShapeElement (tmpEeh, NULL, points, nPoints, is3d, model);
                    }
                else
                    {
                    status = LineStringHandler::CreateLineStringElement (tmpEeh, NULL, points, nPoints, is3d, model);
                    }

                break;
                }

            default:
                {
                BeAssert (false);
                i++;
                break;
                }
            }

        if (eeh.IsValid ())
            {
            status = ChainHeaderHandler::AddComponentElement (eeh, tmpEeh);
            }
        else
            {
            // Don't create complex chain header for single element (or single element loop)...
            if ((i && IsMajorBreak (i-1)) || i == count)
                {
                eeh.SetElementDescr (tmpEeh.ExtractElementDescr().get(), false);
                }
            else
                {
                ChainHeaderHandler::CreateChainHeaderElement (eeh, NULL, isClosed, is3d, model);

                status = ChainHeaderHandler::AddComponentElement (eeh, tmpEeh);
                }
            }

        if (SUCCESS != status)
            continue;

        if ((i && IsMajorBreak (i-1)) || (i == count && solidEeh.IsValid ()))
            {
            if (solidEeh.IsValid ())
                holeAgenda.InsertElemDescr (eeh.ExtractElementDescr().get());
            else
                solidEeh.SetElementDescr (eeh.ExtractElementDescr().get(), false);
            }
        }

    if (SUCCESS == status)
        {
        if (solidEeh.IsValid ())
            {
            if (holeAgenda.IsEmpty ())
                eeh.SetElementDescr (solidEeh.ExtractElementDescr().get(), false);
            else
                status = GroupedHoleHandler::CreateGroupedHoleElement (eeh, solidEeh, holeAgenda);
            }
        else
            {
            ChainHeaderHandler::AddComponentComplete (eeh); // no-op if not a complex chain...
            }
        }

    if (SUCCESS != status || !eeh.IsValid ())
        return ERROR;

    // Now use handler interfaces to make the new element match the template...
    if (templateEh)
        ElementPropertiesSetter::ApplyTemplate (eeh, *templateEh);

    outEeh.SetElementDescr (eeh.ExtractElementDescr().get(), false);
#endif

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GPArray::ToGroupElement (EditElementHandleR outEeh, ElementHandleCP templateEh, bool is3d, bool closePhysicallyClosed, DgnModelR model) const
    {
#if defined (NEEDS_WORK_DGNITEM)
    BentleyStatus       status = SUCCESS;
    EditElementHandle   eeh;

    // NOTE: closePhysicallyClosed needed for drop of filled rsc fonts...closed line strings -> shapes...
    for (int i=0, count = GetCount(); i<count && SUCCESS == status; )
        {
        EditElementHandle  tmpEeh;

        switch (GetCurveType (i))
            {
            case GPCurveType::Ellipse:
                {
                DEllipse3d  ellipse;

                if (SUCCESS != (status = GetEllipse (&i, &ellipse)))
                    break;

                bool    closed = (closePhysicallyClosed && ellipse.isFullEllipse ());

                if (closed)
                    status = EllipseHandler::CreateEllipseElement (tmpEeh, NULL, ellipse, is3d, model);
                else
                    status = ArcHandler::CreateArcElement (tmpEeh, NULL, ellipse, is3d, model);
                break;
                }

            case GPCurveType::Bezier:
            case GPCurveType::BSpline:
                {
                MSBsplineCurve  curve;

                if (SUCCESS != (status = GetBCurve (&i, &curve)))
                    break;

                bool    closed = (closePhysicallyClosed && curve.IsPhysicallyClosed (1.0e-5));

                if (closed)
                    status = (BentleyStatus) curve.CopyClosed (curve);

                if (SUCCESS == status)
                    status = (BentleyStatus) BSplineCurveHandler::CreateBSplineCurveElement (tmpEeh, NULL, curve, is3d, model);

                curve.ReleaseMem ();

                break;
                }

            case GPCurveType::LineString:
                {
                int         nPoints;
                DPoint3d    points[MAX_VERTICES];

                if (SUCCESS != (status = GetLineString (&i, points, &nPoints, MAX_VERTICES)))
                    {
                    //  We added this change because we were afraid to fix DgnShxFile.cpp in the branch.
                    //  We should eliminate this workaround in the trunk after fixing DgnShxFile.cpp.
                    //  The bogus code in DgnShxFile.cpp creates a bogus 1-point linestring prior to calling
                    //  AddEllipse.
                    //
                    //  Without this change some characters from Shx fonts are never generated. Once example
                    //  is the lower case i in the extslim2 font.
                    BeAssert (SUCCESS == status);
                    i++;
                    status = SUCCESS;
                    continue;
                    }

                bool    closed = (closePhysicallyClosed && LegacyMath::RpntEqual (&points[0], &points[nPoints-1]));

                if (2 == nPoints)
                    {
                    DSegment3d  segment;

                    segment.Init (points[0], points[1]);

                    status = LineHandler::CreateLineElement (tmpEeh, NULL, segment, is3d, model);
                    }
                else if (closed)
                    {
                    status = ShapeHandler::CreateShapeElement (tmpEeh, NULL, points, nPoints, is3d, model);
                    }
                else
                    {
                    status = LineStringHandler::CreateLineStringElement (tmpEeh, NULL, points, nPoints, is3d, model);
                    }

                break;
                }

            default:
                {
                BeAssert (false);
                i++;
                break;
                }
            }

        if (eeh.IsValid ())
            {
            status = NormalCellHeaderHandler::AddChildElement (eeh, tmpEeh);
            }
        else
            {
            if (i == count)
                {
                eeh.SetElementDescr (tmpEeh.ExtractElementDescr().get(), false);
                }
            else
                {
                NormalCellHeaderHandler::CreateOrphanCellElement (eeh, NULL, is3d, model);

                status = NormalCellHeaderHandler::AddChildElement (eeh, tmpEeh);
                }
            }
        }

    if (SUCCESS != status || !eeh.IsValid ())
        return ERROR;

    NormalCellHeaderHandler::AddChildComplete (eeh); // no-op if not a cell...

    // Now use handler interfaces to make the new element match the template...
    if (templateEh)
        ElementPropertiesSetter::ApplyTemplate (eeh, *templateEh);

    outEeh.SetElementDescr (eeh.ExtractElementDescr().get(), false);
#endif

    return SUCCESS;
    }

