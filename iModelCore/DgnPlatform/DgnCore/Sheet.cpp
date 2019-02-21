/*-------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Sheet.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

BEGIN_SHEET_NAMESPACE
namespace Handlers
{
HANDLER_DEFINE_MEMBERS(Element);
HANDLER_DEFINE_MEMBERS(AttachmentElement)
HANDLER_DEFINE_MEMBERS(Model)

}
END_SHEET_NAMESPACE

USING_NAMESPACE_SHEET

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Sheet::Element::CreateCode(DocumentListModelCR model, Utf8StringCR name)
    {
    return CodeSpec::CreateCode(BIS_CODESPEC_Sheet, *model.GetModeledElement(), name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Sheet::Element::CreateUniqueCode(DocumentListModelCR model, Utf8CP baseName)
    {
    DgnDbR db = model.GetDgnDb();
    DgnCode code = CreateCode(model, baseName);
    if (!db.Elements().QueryElementIdByCode(code).IsValid())
        return code;

    int counter=1;
    do  {
        Utf8PrintfString name("%s-%d", baseName, counter);
        code = CreateCode(model, name.c_str());
        counter++;
        } while (db.Elements().QueryElementIdByCode(code).IsValid());

    return code;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementPtr Sheet::Element::Create(DocumentListModelCR model, double scale, DPoint2dCR size, Utf8StringCR name)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(Handlers::Element::GetHandler());

    if (!model.GetModelId().IsValid() || !classId.IsValid() || name.empty())
        {
        BeAssert(false);
        return nullptr;
        }

    auto sheet = new Element(CreateParams(db, model.GetModelId(), classId, CreateCode(model, name)));
    sheet->SetScale(scale);
    sheet->SetWidth(size.x);
    sheet->SetHeight(size.y);
    return sheet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementPtr Sheet::Element::Create(DocumentListModelCR model, double scale, DgnElementId sheetTemplate, Utf8StringCR name)
    {
    DgnDbR db = model.GetDgnDb();       
    DgnClassId classId = db.Domains().GetClassId(Handlers::Element::GetHandler());

    if (!model.GetModelId().IsValid() || !classId.IsValid() || name.empty())
        {
        BeAssert(false);
        return nullptr;
        }

    auto sheet = new Element(CreateParams(db, model.GetModelId(), classId, CreateCode(model, name)));
    sheet->SetScale(scale);
    sheet->SetTemplate(sheetTemplate);

#ifdef WIP_SHEETS
    sheet->SetHeight(sheetTemplateElem->GetHeight());
    sheet->SetWidth(sheetTemplateElem->GetWidth());
    sheet->SetBorder(sheetTemplateElem->GetBorder());
#endif
    BeAssert(false && "WIP_SHEETS - templates");
    return sheet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ViewAttachment::ViewAttachment(DgnDbR db, DgnModelId model, DgnViewId viewId, DgnCategoryId cat, Placement2dCR placement) : T_Super(CreateParams(db, model, QueryClassId(db), cat, placement))
    {
    SetAttachedViewId(viewId);
    SetCode(GenerateDefaultCode());
    SetScale(ComputeScale(db, viewId, placement.GetElementBox()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ViewAttachment::ViewAttachment(DgnDbR db, DgnModelId model, DgnViewId viewId, DgnCategoryId cat, DPoint2dCR origin, double scale) : T_Super(CreateParams(db, model, QueryClassId(db), cat, ComputePlacement(db, viewId, origin, scale)))
    {
    SetAttachedViewId(viewId);
    SetCode(GenerateDefaultCode());
    SetScale(scale);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Placement2d ViewAttachment::ComputePlacement(DgnDbR db, DgnViewId viewId, DPoint2dCR origin, double scale)
    {
    auto viewDef = db.Elements().Get<ViewDefinition>(viewId);
    if (!viewDef.IsValid())
        {
        BeAssert(false);
        return Placement2d{};
        }
    auto viewExtents = viewDef->GetExtents();

    Placement2d placement;
    placement.SetOrigin(origin);

    ElementAlignedBox2d box;
    box.low.Zero();
    box.high.x = viewExtents.x / scale;
    box.high.y = viewExtents.y / scale;
    placement.SetElementBox(box);
    
    return placement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
double ViewAttachment::ComputeScale(DgnDbR db, DgnViewId viewId, ElementAlignedBox2dCR placement)
    {
    auto viewDef = db.Elements().Get<ViewDefinition>(viewId);
    if (!viewDef.IsValid())
        {
        BeAssert(false);
        return 1.0;
        }

    return viewDef->GetExtents().x / placement.GetWidth();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewAttachment::CheckValid() const
    {
    if (!GetAttachedViewId().IsValid())
        return DgnDbStatus::ViewNotFound;

    return GetModel()->IsSheetModel() ? DgnDbStatus::Success : DgnDbStatus::WrongModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/17
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr ViewAttachment::GetClip() const
    {
    return m_jsonProperties.isMember(json_clip()) ? ClipVector::FromJson(m_jsonProperties[json_clip()]) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewAttachment::SetClip(ClipVectorCR clipVector)
    {
    m_jsonProperties[json_clip()] = clipVector.ToJson();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewAttachment::ClearClip()
    {
    m_jsonProperties.removeMember(json_clip());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Sheet::Model::_OnInsert()
    {
    if (!GetDgnDb().Elements().Get<Sheet::Element>(GetModeledElementId()).IsValid())
        {
        BeAssert(false && "A SheetModel should be modeling a Sheet element");
        return DgnDbStatus::BadElement;
        }

    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
ModelPtr Sheet::Model::Create(ElementCR sheet)
    {
    DgnDbR db = sheet.GetDgnDb();
    ModelHandlerR handler = Handlers::Model::GetHandler();
    DgnClassId classId = db.Domains().GetClassId(handler);

    if (!classId.IsValid() || !sheet.GetElementId().IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    DgnModelPtr model = handler.Create(DgnModel::CreateParams(db, classId, sheet.GetElementId()));
    if (!model.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return dynamic_cast<ModelP>(model.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::ViewControllerPtr SheetViewDefinition::_SupplyController() const
    {
    return new Sheet::ViewController(*this);
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   02/17
//=======================================================================================
struct RectanglePoints
{
    template<typename T> static void Init(T& pts, double xlow, double ylow, double xhigh, double yhigh)
        {
        pts[0].x = pts[3].x = pts[4].x = xlow;
        pts[0].y = pts[1].y = pts[4].y = ylow;
        pts[1].x = pts[2].x = xhigh; 
        pts[2].y = pts[3].y = yhigh;
        }

    DPoint2d m_pts[5];

    // If context supplied, points will be transformed to view coords.
    RectanglePoints(double xlow, double ylow, double xhigh, double yhigh, ViewContextCP context=nullptr)
        {
        if (nullptr != context)
            {
            DPoint3d pts[5];
            Init(pts, xlow, ylow, xhigh, yhigh);
            for (auto& pt : pts)
                pt.z = 0.0;

            context->WorldToView(m_pts, pts, 5);
            }
        else
            {
            Init(m_pts, xlow, ylow, xhigh, yhigh);
            }
        }

    template<typename T> RectanglePoints(T const& range) : RectanglePoints(range.low.x, range.low.y, range.high.x, range.high.y) { }

    operator DPoint2dP() {return m_pts;}
    operator DPoint2dCP() const {return m_pts;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DgnElementId> Sheet::Model::GetSheetAttachmentIds() const
    {
    bvector<DgnElementId>   attachIds;
    // Scan for viewAttachments...
    auto stmt =  GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_ViewAttachment) " WHERE Model.Id=?");
    stmt->BindId(1, GetModelId());

    while (BE_SQLITE_ROW == stmt->Step())
        attachIds.push_back(stmt->GetValueId<DgnElementId>(0));

    return attachIds;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ViewDefinitionCPtr> Sheet::Model::GetSheetAttachmentViews(DgnDbR db) const
    {
    bvector<DgnElementId> attachmentIds = GetSheetAttachmentIds();
    bvector<ViewDefinitionCPtr> attachmentViews;

    for (auto& attachmentId : attachmentIds)
        {
        auto attachmentElement = GetDgnDb().Elements().Get<Sheet::ViewAttachment>(attachmentId);
        if (!attachmentElement.IsValid())
            {
            BeAssert(false);
            continue;
            }
        
        auto viewDefinition = GetDgnDb().Elements().Get<ViewDefinition>(attachmentElement->GetAttachedViewId());
        if (!viewDefinition.IsValid())
            {
            BeAssert(false);
            continue;
            }
        attachmentViews.push_back(viewDefinition);
        }

    return attachmentViews;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                     04/
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d Sheet::Model::GetSheetSize() const
    {
    // Get the Sheet::Element to extract the sheet size
    auto sheetElement = GetDgnDb().Elements().Get<Sheet::Element>(GetModeledElementId());
    if (!sheetElement.IsValid())
        {
        BeAssert(false); // this is fatal
        return DPoint2d::From(0.0, 0.0);
        }
    
    return DPoint2d::From(sheetElement->GetWidth(), sheetElement->GetHeight());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d Sheet::Model::GetSheetExtents() const
    {
    DPoint2d size = GetSheetSize();
    return AxisAlignedBox3d(DPoint3d::FromZero(), DPoint3d::From(size.x, size.y, 0.0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::ViewController::_LoadState()
    {
    auto model = GetViewedModel();  // get the sheet model for this view
    if (nullptr == model || nullptr == model->ToSheetModel())
        {
        BeAssert(false); // what happened?
        return;
        }

    m_size = model->ToSheetModel()->GetSheetSize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
Sheet::Border::Border(ViewContextCP context, DPoint2dCR size)
    {
    // Rect
    RectanglePoints rect(0, 0, size.x, size.y, context);
    memcpy(m_rect, rect.m_pts, 5 * sizeof(DPoint2d));

    // Shadow
    double shadowWidth = .01 * size.Distance(DPoint2d::FromZero());

    DPoint3d shadow[7];
    shadow[0].y = shadow[1].y = shadow[6].y = 0.0;
    shadow[0].x = shadowWidth;
    shadow[1].x = shadow[2].x = size.x;
    shadow[3].x = shadow[4].x = size.x + shadowWidth;
    shadow[2].y = shadow[3].y = size.y - shadowWidth;
    shadow[4].y = shadow[5].y = -shadowWidth;
    shadow[5].x = shadow[6].x = shadowWidth;

    if (nullptr != context)
        {
        for (auto& point : shadow)
            point.z = 0.0;

        context->WorldToView(m_shadow, shadow, 7);
        }
    else
        {
        for (size_t i = 0; i < 7; i++)
            {
            m_shadow[i].x = shadow[i].x;
            m_shadow[i].y = shadow[i].y;
            }
        }
            
    // Gradient
    double keyValues[] = {0.0, 0.5};
    ColorDef keyColors[] = {ColorDef(25,25,25), ColorDef(150,150,150)};

    m_gradient = GradientSymb::Create();
    m_gradient->SetMode(Render::GradientSymb::Mode::Linear);
    m_gradient->SetAngle(-45.0);
    m_gradient->SetKeys(2, keyColors, keyValues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::Border::AddToBuilder(Render::GraphicBuilderR builder) const
    {
    builder.SetSymbology(ColorDef::Black(), ColorDef::Black(), 2, LinePixels::Solid);
    builder.AddLineString2d(5, m_rect, 0.0);

    GraphicParams params;
    params.SetLineColor(ColorDef(25,25,25));
    params.SetGradient(m_gradient.get());

    builder.ActivateGraphicParams(params);
    builder.AddShape2d(7, m_shadow, true, ViewDefinition2d::Get2dFrustumDepth());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
DRange2d Sheet::Border::GetRange() const
    {
    DRange2d shadowRange = DRange2d::From(m_shadow, 7),
             range = DRange2d::From(m_rect, 5);
    range.Extend(shadowRange);
    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::ViewController::_DrawView(ViewContextR context)
    {
    auto model = GetViewedModel();
    if (nullptr == model)
        return;

    context.VisitDgnModel(*model);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId Sheet::Model::FindFirstViewOfSheet(DgnDbR db, DgnModelId mid)
    {
    auto findViewOfSheet = db.GetPreparedECSqlStatement("SELECT sheetView.ECInstanceId FROM bis.SheetViewDefinition sheetView WHERE (sheetView.BaseModel.Id=?)");
    findViewOfSheet->BindId(1, mid);
    return BE_SQLITE_ROW != findViewOfSheet->Step() ?  DgnElementId() : findViewOfSheet->GetValueId<DgnElementId>(0);
    }
