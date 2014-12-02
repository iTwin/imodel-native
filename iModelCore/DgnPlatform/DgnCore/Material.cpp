/*----------------------------------------------------------------------+
|
|   $Source: DgnCore/Material.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

#define RETURN_IF_FALSE(val)    if (!(val)) { return false; }

static WCharCP    PALETTELEGACYNAME_Internal        = L"$(_DGNFILE)\\";
static WCharCP    PALETTELEGACYNAME_Dgnlib          = L"$(_DGNLIB)";

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/11
//---------------------------------------------------------------------------------------
WString Material::ParseLegacyPaletteName (WCharCP legacyName)
    {
    if (0 == BeStringUtilities::Wcsnicmp (legacyName, PALETTELEGACYNAME_Dgnlib, wcslen (PALETTELEGACYNAME_Dgnlib)))
        {
        WString inputName, library, paletteName;

        inputName           = legacyName;
        size_t tokenBegin   = inputName.find (L"|") + 1;
        size_t tokenEnd     = inputName.find (L"|", tokenBegin + 1);
        library             = inputName.substr (tokenBegin, tokenEnd - tokenBegin);
        paletteName         = inputName.substr (tokenEnd + 1);

        return BeFileName::GetFileNameWithoutExtension (paletteName.c_str());
        }

    if (0 == BeStringUtilities::Wcsnicmp (legacyName, PALETTELEGACYNAME_Internal, wcslen (PALETTELEGACYNAME_Internal)))
        {
        WString paletteName = legacyName + wcslen (PALETTELEGACYNAME_Internal);

        return BeFileName::GetFileNameWithoutExtension (paletteName.c_str());
        }

    return BeFileName::GetFileNameWithoutExtension (legacyName);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
Material::Material (DgnProjectR dgnProject)
    :
    m_dgnProject (&dgnProject),
    m_sentToQV (false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     08/09
+---------------+---------------+---------------+---------------+---------------+------*/
Material::~Material ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialPtr Material::Create (DgnProjectR dgnProject)
    {
    return new Material (dgnProject);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialPtr Material::Create (MaterialCR initFrom, DgnProjectR dgnProject)
    {
    MaterialPtr result = new Material (dgnProject);
    result->Copy (initFrom);
    result->SetDgnProject (dgnProject);
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/212
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialCP Material::FromRendMatId (uintptr_t rendMatId)
    {
    return reinterpret_cast <MaterialCP> (rendMatId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void Material::SetDgnProject (DgnProjectR dgnProject)
    {
    m_dgnProject = &dgnProject;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void Material::Copy (MaterialCR rhs)
    {
    m_name          = rhs.m_name;
    m_palette       = rhs.m_palette;
    m_dgnProject    = rhs.m_dgnProject;
    m_id            = rhs.m_id;
    m_settings.Copy (rhs.m_settings);

    // sent to QV intentionally left out
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     04/10
//---------------------------------------------------------------------------------------
double Material::GetLayerScaleInUORs (MaterialMapLayerCR layer) const
    {
    double result = 1.0;
    switch (layer.GetUnits ())
        {
        // Note: master and subunits read "active" i.e. master file - seems stupid, but matches pre-real world unit behavior.
        default:
        case MapUnits::MasterUnits:
        case MapUnits::SubUnits:
        case MapUnits::Meters:
        case MapUnits::Millimeters:
        case MapUnits::Feet:
        case MapUnits::Inches:
            BeAssert(false && "additional case in GetLayerScaleInUORs");
            break;

        case MapUnits::Relative:
            return 1.0;
        }

    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool Material::Equals (MaterialCR rhs, bool testDgnProject) const
    {
    if (testDgnProject)
        {
        RETURN_IF_FALSE (rhs.m_dgnProject == m_dgnProject);
        }

    RETURN_IF_FALSE (rhs.m_name.EqualsI     (m_name.c_str()));
    RETURN_IF_FALSE (rhs.m_palette.EqualsI  (m_palette.c_str()));
    RETURN_IF_FALSE (rhs.m_settings.Equals  (m_settings));

    // sent to QV and id intentionally left out

    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void Material::InitDefaults (DgnProjectR dgnProject)
    {
    m_name          = "";
    m_palette       = "";
    m_dgnProject    = &dgnProject;
    m_sentToQV      = false;
    m_id.Invalidate();
    m_settings.InitDefaults();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
DgnMaterialId   Material::GetId () const            { return m_id; }
void            Material::SetId (DgnMaterialId id)  { m_id = id; }

/*=================================================================================**//**
* @bsiclass                                                     MattGooding     09/09
+===============+===============+===============+===============+===============+======*/
struct MaterialCache : public BeSQLite::DbAppData
{
private:
    MaterialCache (DgnProjectR host);

    bmap <DgnMaterialId, MaterialPtr>       m_materials;
    DgnProjectP                             m_host;

    void                                    CacheMaterials();

public:
    virtual ~MaterialCache ()                                                           { }

    virtual void                            _OnCleanup (BeSQLiteDbR host) override      { delete this; }
    static BeSQLite::DbAppData::Key const&  GetKey ()                                   { static BeSQLite::DbAppData::Key s_key; return s_key; }

    static MaterialCache*                   GetCache (DgnProjectR host);
    MaterialP                               FindById (DgnMaterialId id);
    BentleyStatus                           FindByName (MaterialList& list, Utf8CP name);
};

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialCache::MaterialCache (DgnProjectR host)
    {
    m_host = &host;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     02/13
//---------------------------------------------------------------------------------------
void MaterialCache::CacheMaterials ()
    {
    BeAssert (m_materials.empty());

    HighPriorityOperationBlock  highPriority; // see comments in BeSQLite.h
    Utf8String                  xml;
    DgnMaterials            materials = m_host->Materials();

    for (auto const& entry : materials.MakeIterator())
        {
        if (BE_SQLITE_ROW != materials.QueryProperty (xml, entry.GetId(), DgnMaterialProperty::XmlMaterial()))
            {
            BeAssert (false);
            continue;
            }

        MaterialPtr result = MaterialManager::GetManagerR().CreateFromXml (xml.c_str(), *m_host);
        if (result.IsNull())
            {
            BeAssert (false);
            continue;
            }

        result->GetNameR() = entry.GetName();
        result->GetPaletteR() =  entry.GetPalette();
        result->SetId (entry.GetId());

        m_materials[entry.GetId()] = result;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialCache* MaterialCache::GetCache (DgnProjectR host)
    {
    MaterialCache* cache = reinterpret_cast <MaterialCache *> (host.AppData().Find(MaterialCache::GetKey()));
    if (NULL == cache)
        {
        cache = new MaterialCache (host);
        cache->CacheMaterials();
        host.AppData().Add(MaterialCache::GetKey(), cache);
        }
    return cache;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     02/13
//---------------------------------------------------------------------------------------
BentleyStatus MaterialCache::FindByName (MaterialList& list, Utf8CP name)
    {
    if (!name || !*name)
        return ERROR;

    for (auto const& entry : m_materials)
        {
        if (!entry.second->GetName().EqualsI (name))
            continue;

        list.push_back (entry.second.get());
        }

    return list.empty() ? ERROR : SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     02/13
//---------------------------------------------------------------------------------------
MaterialP MaterialCache::FindById (DgnMaterialId id)
    {
    if (!id.IsValid())
        return NULL;

    auto iter = m_materials.find (id);
    return m_materials.end() != iter ? iter->second.get() : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialManager::MaterialManager ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialManager& MaterialManager::GetManagerR ()
    {
    static MaterialManager s_materialManager;
    return s_materialManager;
    }

#define MAX_LegacyMaterialName  30
#define LINKAGEID_Material      20314

/*=================================================================================**//**
* @bsiclass                                                     MattGooding     10/09
+===============+===============+===============+===============+===============+======*/
struct MaterialLinkage
    {
    LinkageHeader               m_header;
    struct
        {
        short                   m_nameSize;
        char                    m_name[MAX_LegacyMaterialName];
        } m_data;
    short                       m_padding[4];
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     02/10
//---------------------------------------------------------------------------------------
static DgnMaterialId findMaterialAttachmentId (ElementHandleCR eh)
    {
    Display_attribute attribute;
    if (!mdlElement_displayAttributePresent (eh.GetElementCP(), MATERIAL_ATTRIBUTE, &attribute))
        return DgnMaterialId ();

    return DgnMaterialId (attribute.attr_data.material.materialId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     02/10
//---------------------------------------------------------------------------------------
BentleyStatus MaterialManager::SetMaterialAttachmentId (EditElementHandleR eeh, DgnMaterialId id)
    {
    Display_attribute attribute;

    // strip any external attachment to stop their being both.
    for (ElementLinkageIterator iter = eeh.BeginElementLinkages(); iter.IsValid(); iter.ToNext())
        {
        if (LINKAGEID_Material == iter.GetLinkage()->primaryID)
            {
            eeh.RemoveElementLinkage (iter);
            break;
            }
        }

    DgnV8ElementBlank elementCopy;
    eeh.GetElementCP()->CopyTo (elementCopy);

    if (!mdlElement_displayAttributePresent (&elementCopy, MATERIAL_ATTRIBUTE, &attribute))
        {
        if (!id.IsValid())
            return SUCCESS;

        Display_attribute_material materialData;
        materialData.materialId = id.GetValue();

        if (SUCCESS != mdlElement_displayAttributeCreate (&attribute, MATERIAL_ATTRIBUTE, sizeof (materialData), reinterpret_cast <UShort *> (&materialData.materialId)))
            return ERROR;

        StatusInt status = mdlElement_displayAttributeAdd (&elementCopy, &attribute);

        if (SUCCESS == status)
            eeh.ReplaceElement (&elementCopy);

        return SUCCESS == status ? SUCCESS : ERROR;
        }

    if (!id.IsValid())
        {
        bool deleted = mdlElement_displayAttributeRemove (&elementCopy, MATERIAL_ATTRIBUTE);

        if (deleted)
            eeh.ReplaceElement (&elementCopy);

        return deleted ? SUCCESS : ERROR;
        }

    attribute.attr_data.material.materialId = id.GetValue();

    StatusInt status = mdlElement_displayAttributeReplace (&elementCopy, MATERIAL_ATTRIBUTE, &attribute);

    if (SUCCESS == status)
        eeh.ReplaceElement (&elementCopy);

    return SUCCESS == status ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialCP MaterialManager::FindMaterialAttachment (ElementHandleCR eh)
    {
    DgnProjectP project = eh.GetDgnProject();
    if (NULL == project)
        return NULL;

    return FindMaterial (findMaterialAttachmentId (eh), *project);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     02/13
//---------------------------------------------------------------------------------------
BentleyStatus MaterialManager::FindMaterial (MaterialList& materials, Utf8CP name, DgnProjectR source)
    {
    if (!name || !*name)
        return ERROR;

    MaterialCache* cache = MaterialCache::GetCache (source);
    if (NULL == cache)
        {
        BeAssert (false);
        return ERROR;
        }

    return cache->FindByName (materials, name);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/10
//---------------------------------------------------------------------------------------
MaterialCP MaterialManager::FindMaterial (DgnMaterialId id, DgnProjectR source)
    {
    if (!id.IsValid())
        return NULL;

    MaterialCache* cache = MaterialCache::GetCache (source);
    if (NULL == cache)
        {
        BeAssert (false);
        return NULL;
        }

    return cache->FindById (id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef NOTYET
MaterialCP MaterialManager::FindMaterialBySymbology (MaterialSearchStatus* searchStatus, LevelId levelId, UInt32 rawColor, DgnModelR renderDgnModel, ViewContextP context)
    {
    AssignmentCacheMap* assignmentCache = NULL != context ? context->GetMaterialAssignmentCacheP () : NULL;

    if (NULL != assignmentCache)
        {
        AssignmentCacheMap::const_iterator iter = assignmentCache->find (AssignmentCacheNode (levelId, rawColor, renderDgnModel));
        if (assignmentCache->end () != iter)
            {
            if (searchStatus) { *searchStatus = iter->second.m_status; }
            return iter->second.m_material;
            }
        }

    DgnProjectP renderDgnFile = renderDgnModel.GetDgnProject ();
    if (NULL == renderDgnFile)
        {
        if (NULL != assignmentCache)
            assignmentCache->insert (AssignmentCacheMap::value_type (AssignmentCacheNode (levelId, rawColor, renderDgnModel), AssignmentCacheResult (NULL, MaterialSearchStatus::InvalidDgnModel)));
        if (searchStatus) { *searchStatus = MaterialSearchStatus::InvalidDgnModel; }
        return NULL;
        }

    Utf8CP levelName;
    DgnLevels::Level const& levelHandle = renderDgnFile->GetDgnProject().Levels().QueryLevelById(levelId);
    if (!levelHandle.IsValid () || NULL == (levelName = levelHandle.GetName ()))
        {
        if (NULL != assignmentCache)
            assignmentCache->insert (AssignmentCacheMap::value_type (AssignmentCacheNode (levelId, rawColor, renderDgnModel), AssignmentCacheResult (NULL, MaterialSearchStatus::InvalidLevelId)));
        if (searchStatus) { *searchStatus = MaterialSearchStatus::InvalidLevelId; }
        return NULL;
        }

    MaterialTableCP         table;
    MaterialAssignmentCP    assignment;
    WString                 levelNameW (levelName, true); // string conversion

    if (NULL != (table = GetActiveTableCP (renderDgnModel)) && NULL != (assignment = table->FindAssignmentBySymbology (levelNameW.c_str(), rawColor, *renderDgnFile)))
        {
        MaterialSearchStatus localSearchStatus;
        MaterialCP result = FindMaterialFromAssignment (&localSearchStatus, *assignment, renderDgnModel);
        if (NULL != assignmentCache)
            assignmentCache->insert (AssignmentCacheMap::value_type (AssignmentCacheNode (levelId, rawColor, renderDgnModel), AssignmentCacheResult (result, localSearchStatus)));
        if (searchStatus) { *searchStatus = localSearchStatus; }
        return result;
        }

    ElementId levelMaterial = levelHandle.GetMaterial ();
    if (INVALID_ELEMENTID != levelMaterial && 0 != levelMaterial)
        {
        MaterialSearchStatus localSearchStatus;
        MaterialCP result = FindMaterial (&localSearchStatus, MaterialId (levelMaterial), *renderDgnFile, renderDgnModel);
        if (NULL != assignmentCache)
            assignmentCache->insert (AssignmentCacheMap::value_type (AssignmentCacheNode (levelId, rawColor, renderDgnModel), AssignmentCacheResult (result, localSearchStatus)));
        if (searchStatus) { *searchStatus = localSearchStatus; }
        return result;
        }

    if (searchStatus) { *searchStatus = MaterialSearchStatus::NotFound; }
    if (NULL != assignmentCache)
        assignmentCache->insert (AssignmentCacheMap::value_type (AssignmentCacheNode (levelId, rawColor, renderDgnModel), AssignmentCacheResult (NULL, MaterialSearchStatus::NotFound)));

    return NULL;
    }
#endif


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialMapCP    Material::GetGeometryMap () const
    {
    return GetSettings().GetMaps().GetMapCP (MaterialMap::MAPTYPE_Geometry);     
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialQvGeometryMap::MaterialQvGeometryMap (MaterialMapCR map)
    {
    MaterialMapLayerCR topLayer = map.GetLayers().GetTopLayer();

    m_cellName      = topLayer.GetFileName();
    m_useCellColors = topLayer.UseCellColors();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialQvGeometryMap::~MaterialQvGeometryMap ()
    {
    T_HOST.GetGraphicsAdmin()._DeleteTexture ((uintptr_t) this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool        MaterialQvGeometryMap::Matches (MaterialMapCR map) const
    {
    MaterialMapLayerCR topLayer = map.GetLayers().GetTopLayer();
     
    return m_cellName == topLayer.GetFileName() && m_useCellColors == topLayer.UseCellColors ();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool            Material::NeedsQvGeometryTexture ()  const
    {    
    MaterialMapCP        map;

    return NULL != (map = GetGeometryMap ()) && (m_qvGeometryMap.IsNull () || !m_qvGeometryMap->Matches (*map));
    }

uintptr_t       Material::GetQvGeometryTexture () const   {    return (m_qvGeometryMap.IsNull()) ? 0 : (uintptr_t) &(*m_qvGeometryMap); }
void            Material::ClearQvGeometryTexture () const {    m_qvGeometryMap = NULL; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       Material::GetGeometryMapDefinition (EditElementHandleR elemHandle, bool& useCellColors) const
    {
    BeAssert(false && "in GetGeometryMapDefinition");
    MaterialMapCP           map = NULL;
#if defined(NOTNOW)
    DgnProjectR             dgnProject = GetDgnProjectR();
    ElementRefP             defElemRef = NULL;
    SharedCellDefinitions*  scDefs = NULL;

    if (NULL == (map = GetGeometryMap()) ||
        NULL == (scDefs = dgnFile->GetSharedCellDefinitions()) ||
        NULL == (defElemRef = scDefs->Search (map->GetLayers().GetTopLayer().GetFileName().c_str())))
        return ERROR;


    elemHandle.SetElementRef (defElemRef, &GetDgnModelR());
#endif

    if (elemHandle.GetElementCP()->Is3d())
        {
        TransformInfo   tInfo;

        tInfo.GetTransformR().ScaleMatrixColumns (1.0, 1.0, 1.0E-8);
        elemHandle.GetHandler().ApplyTransform (elemHandle, tInfo);
        }
    useCellColors = map->GetLayers().GetTopLayer().UseCellColors();

#ifdef NEEDS_WORK
    // By convention (from area patterning) pattern maps ignore "point" line elements.
    for (MSElementDescrP pChild = elemHandle.GetElementDescrP()->h.firstElem, pNext; NULL != pChild; pChild = pNext)
        {
        int             nPoints;
        DPoint3d        points[2];

        pNext = pChild->h.next;
        if (LINE_ELM == pChild->el.GetLegacyType() &&
            SUCCESS == mdlLinear_directExtract (points, &nPoints, &pChild->el, elemHandle.GetDgnModel()) &&
            points[0].isEqual (&points[1]))
            pChild->RemoveElement ();
        }
#endif

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            Material::DefineQvGeometryTexture () const
    {
    MaterialMapCP        map;

    if (NULL != (map = GetGeometryMap()))
        m_qvGeometryMap = MaterialQvGeometryMap::Create (*map);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialQvGeometryMapPtr    MaterialQvGeometryMap::Create (MaterialMapCR map)
    {
    MaterialQvGeometryMapPtr    geomMap = new MaterialQvGeometryMap(map);

    return geomMap;
    }


END_BENTLEY_DGNPLATFORM_NAMESPACE
#if defined (NEEDS_WORK_MATERIAL)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      03/13
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialUVDetail::MaterialUVDetail ()
    {
    m_origin.Zero ();
    m_size.Zero ();
    m_rMatrix.InitIdentity ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      03/13
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialUVDetail::Copy (MaterialUVDetailCR rhs)
    {
    m_eh                = rhs.m_eh;
    m_origin            = rhs.m_origin;
    m_size              = rhs.m_size;
    m_rMatrix           = rhs.m_rMatrix;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      03/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool MaterialUVDetail::Equals (MaterialUVDetailCR rhs)
    {
    if (m_eh.IsValid () != rhs.m_eh.IsValid ())
        return false;
    else if (m_eh.IsValid () && m_eh.GetElementId () != rhs.m_eh.GetElementId ())
        return false;
    RETURN_IF_FALSE (rhs.m_origin.IsEqual (m_origin, 1.0E-4));
    RETURN_IF_FALSE (rhs.m_size.IsEqual (m_size, 1.0E-4));
    RETURN_IF_FALSE (rhs.m_rMatrix.IsEqual (m_rMatrix, 1.0E-4));
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      03/13
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialUVDetailPtr MaterialUVDetail::Create ()
    {
    return new MaterialUVDetail ();
    }

DPoint3dCR   MaterialUVDetail::GetOrigin () const                           { return m_origin;}
void         MaterialUVDetail::SetOrigin (DPoint3dCR origin)                { m_origin = origin;}

RotMatrixCR  MaterialUVDetail::GetRMatrix () const                          { return m_rMatrix;}
void         MaterialUVDetail::SetRMatrix (RotMatrixCR rMatrix)             { m_rMatrix = rMatrix;}

DPoint3dCR   MaterialUVDetail::GetSize () const                             { return m_size;}
void         MaterialUVDetail::SetSize (DPoint3dCR size)                    { m_size = size;}

ElementHandleCR  MaterialUVDetail::GetElementHandle () const                {return m_eh;}
void             MaterialUVDetail::SetElementHandle (ElementHandleCR eh)    { m_eh = eh;}

QVAliasMaterialIdCP  MaterialUVDetail::GetQVAliasMaterialId  () const                   {return m_appQvId.get ();}
void                 MaterialUVDetail::SetQVAliasMaterialId  (QVAliasMaterialIdP qvId)  {m_appQvId = qvId;} 

#endif
