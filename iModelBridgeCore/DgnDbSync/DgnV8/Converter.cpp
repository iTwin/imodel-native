/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include <GeoCoord\BaseGeoCoord.h>

// #define WIP_DUMP

// Via RequiredRepository entry in the DgnV8ConverterDLL Part. The way this piece was designed was that is was so small, every library gets and builds as source.
#include "../../../../V8IModelExtraFiles/V8IModelExtraFiles.h"

#include <VersionedDgnV8Api/PSolid/PSolidCore.h>
#if defined (BENTLEYCONFIG_PARASOLID)
#include <BRepCore/PSolidUtil.h>
#endif

#undef min
#undef max

#undef GetClassName

#include <regex>

// Structured Storage open modes (we use "direct" mode).
//  NB: when opening a root storage for read, you can allow multiple readers.
enum
    {
    ROOTSTORE_OPENMODE_READ = (STGM_SHARE_DENY_WRITE | STGM_READ),
    ROOTSTORE_OPENMODE_WRITE = (STGM_SHARE_EXCLUSIVE | STGM_READWRITE),
    ROOTSTORE_OPENMODE_CREATE = (STGM_SHARE_EXCLUSIVE | STGM_READWRITE | STGM_CREATE),

    //  NB: when opening a sub-storage (in any mode), you *must* specify STGM_SHARE_EXCLUSIVE.
    SUBSTORE_OPENMODE_READ = (STGM_SHARE_EXCLUSIVE | STGM_READ),
    SUBSTORE_OPENMODE_WRITE = (STGM_SHARE_EXCLUSIVE | STGM_READWRITE),
    SUBSTORE_OPENMODE_CREATE = (STGM_SHARE_EXCLUSIVE | STGM_READWRITE | STGM_CREATE),

    STREAM_OPENMODE_READ = (STGM_SHARE_EXCLUSIVE | STGM_READ),
    STREAM_OPENMODE_WRITE = (STGM_SHARE_EXCLUSIVE | STGM_READWRITE),
    STREAM_OPENMODE_CREATE = (STGM_SHARE_EXCLUSIVE | STGM_READWRITE | STGM_CREATE)
    };

enum {MAX_UNSAVED_ELEMENTS = 100000};

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     SamWilson       08/13
+===============+===============+===============+===============+===============+======*/
class SupplyPassword : public DgnV8Api::DgnFileSupplyRights
    {
private:
    CharCP  m_password;

public:
    SupplyPassword (CharCP p) : m_password(p) {;}

    virtual StatusInt   getLicense (DgnV8Api::DgnFileLicenseDef* pLic, bool* userCancel, const Byte*) override
        {
        //  Return the password in scrambled form.
        pLic->keypw.InitFromHash ((Byte*)m_password, strlen(m_password));
        *userCancel = false;
        return SUCCESS;
        }

    bool IsValid() const {return m_password != nullptr;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/13
+---------------+---------------+---------------+---------------+---------------+------*/
static bool matchesString(Utf8StringCR pattern, uint32_t wantMatch, Utf8StringCR str)
    {
    if (wantMatch==0)
        return true;        // wantMatch==0 means that this property should not be checked. Return true, since we have nothing to say about it.

    if (pattern.find_first_of("*?") == Utf8String::npos)
        return (wantMatch==1) == pattern.EqualsI(str);

    return (wantMatch==1) == FileNamePattern::MatchesGlob(BeFileName(str.c_str(),true), WString(pattern.c_str(),true).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      4/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::IsEmbeddedFileName0(wchar_t const* fn)
    {
    return BSISUCCESS == DgnV8Api::DgnFile::ParsePackagedName(nullptr, nullptr, nullptr, fn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::GetFileBaseName(DgnV8FileCR ff)
    {
    Utf8String fn(BeFileName::GetFileNameWithoutExtension(ff.GetEmbeddedName().c_str()));
    size_t idot = fn.find(".");
    if (idot != Utf8String::npos)
        fn.erase(idot);
    return fn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String remapNameString(Utf8String filename, Utf8StringCR name, Utf8StringCR suffix)
    {
    Utf8String fileBase = filename.substr(0, filename.find_first_of(".")) + suffix;
    return name + " [" + fileBase + "]";
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   02/15
//=======================================================================================
struct V8FileSyncInfoIdAppData : DgnV8Api::DgnFileAppData
    {
    StableIdPolicy m_idPolicy;
    RepositoryLinkId m_repositoryLinkId;

    V8FileSyncInfoIdAppData(RepositoryLinkId id, StableIdPolicy policy)
        : m_repositoryLinkId(id), m_idPolicy(policy)
        {}

    static DgnV8Api::DgnFileAppData::Key const& GetKey() {static DgnFileAppData::Key s_key; return s_key;}
    virtual void _OnCleanup(DgnFileR host) override {delete this;}

    void SetRepositoryLink(RepositoryLinkId rlinkId) {m_repositoryLinkId = rlinkId;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::DiscardV8FileSyncInfoAppData(DgnV8FileR file)
    {
    file.DropAppData(V8FileSyncInfoIdAppData::GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
StableIdPolicy Converter::GetIdPolicyFromAppData(DgnV8FileCR file)
    {
    auto appdata = (V8FileSyncInfoIdAppData*)((DgnV8FileR)file).FindAppData(V8FileSyncInfoIdAppData::GetKey());
    return (nullptr == appdata) ? StableIdPolicy::ById : appdata->m_idPolicy;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      04/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::ComputeEffectiveEmbeddedFileName(Utf8StringCR fullName, iModelBridge::Params::FileIdRecipe const* recipe)
    {
    if (nullptr == recipe)
        return fullName;

    Utf8String id(fullName);
    if (recipe->m_ignoreCase)
        id.ToLower();

    if (recipe->m_ignorePackage)
        {
        //  masterfile.i.dgn<n>referencefile.i.dgn
        //                    ^
        auto iSep = id.find(">");
        if (iSep != Utf8String::npos)
            id = id.substr(iSep+1);
        }

    if (recipe->m_ignoreExtension)
        {
        auto iDot = id.find(".");
        if (iDot != Utf8String::npos)
            id = id.substr(0, iDot);
        }

    if (!recipe->m_suffixRegex.empty())
        {
        Utf8String expr(recipe->m_suffixRegex);
        if (recipe->m_ignoreCase)
            expr.ToLower(); // in case regex contains exact-match literals, lower-case it, so that it will match what we did to the id
        std::regex rgx(expr.c_str());
        std::smatch matches;
        std::string str(id.c_str());
        if (std::regex_search(str, matches, rgx))
            {
            auto suffix = id.substr(matches.prefix().length());     // TODO - somehow set this as a property of the RepositoryLink element
            id = matches.prefix().str().c_str();
            }
        }

    return id;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedCPtr<RepositoryLink> Converter::GetRepositoryLinkElement(RepositoryLinkId eid) const
    {
    return GetDgnDb().Elements().Get<RepositoryLink>(eid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryLinkId Converter::WriteRepositoryLink(DgnV8FileR file)
    {
    SyncInfo::V8FileInfo finfo = GetSyncInfo().ComputeFileInfo(file);

    // The RepositoryLink's Code always matches its ExternalSourceAspects's Identifier. This is 
    // a unique, stable identifer for this V8 file.
    Utf8String code = finfo.m_uniqueName;

    Utf8String uri = GetDocumentURNforFile(file);
    if (!iModelBridge::IsNonFileURN(uri) || IsEmbeddedFileName(uri) || BentleyApi::BeFileName::DoesPathExist(BentleyApi::WString(uri.c_str(), true).c_str()))
        uri.clear();    // Don't store filepaths that refer to someone's computer.

    BeFileName localFileName(file.GetFileName().c_str());
    if (file.IsEmbeddedFile())
        {
        // don't include package name
        auto endPackage = localFileName.find(L">");
        if (endPackage != WString::npos)
            {
            auto refname = localFileName.substr(endPackage+1);
            localFileName.assign(refname.c_str());
            }
        }

    auto rlink = iModelBridge::MakeRepositoryLink(GetDgnDb(), _GetParams(), localFileName, code.c_str(), uri.c_str());

    auto rlinkPersist = GetDgnDb().Elements().Get<RepositoryLink>(rlink->GetElementId());
    if (rlinkPersist.IsValid())
        {
        auto hashNew = iModelBridge::ComputeRepositoryLinkHash(*rlink);
        auto hashOld = iModelBridge::ComputeRepositoryLinkHash(*rlinkPersist);
        if (hashNew.GetHashString().Equals(hashOld.GetHashString()))
            {
            // If the link hasn't changed, then don't request locks or write to the BIM.
            _OnFileDiscovered(file);
            return RepositoryLinkId(rlink->GetElementId().GetValue());
            }
        }

    if (!GetDgnDb().BriefcaseManager().IsBulkOperation())
        {
        // Request locks and codes explicity. That is because a bridge can possibly create a RepositoryLink in one of its callbacks
        // such as _OpenSource that is called before we go into bulk insert mode.
        IBriefcaseManager::Request req;
        rlink->PopulateRequest(req, (rlink->GetElementId().IsValid())? BeSQLite::DbOpcode::Update: BeSQLite::DbOpcode::Insert);
        GetDgnDb().BriefcaseManager().Acquire(req).Result();
        }

    auto xsa = SyncInfo::RepositoryLinkExternalSourceAspect::CreateAspect(GetDgnDb(), finfo, _GetIdPolicy(file));
    xsa.AddAspect(*rlink);

    auto rlinkPost = rlink->GetElementId().IsValid()? rlink->Update(): rlink->Insert();

    if (!rlinkPost.IsValid())
        {
        Utf8PrintfString message("Failed to %s RepositoryLink %s", rlink->GetElementId().IsValid() ? "update" : "insert", code.c_str());
        OnFatalError(IssueCategory::Briefcase(), Issue::Error(), message.c_str());
        return RepositoryLinkId();
        }

    _OnFileDiscovered(file);

#ifndef NDEBUG
    {
    // iModelExternalSourceAspect::Dump(*rlinkPost, nullptr, NativeLogging::SEVERITY::LOG_DEBUG);

    BeFileName v8FileName(file.GetFileName().c_str());
    auto f1 = GetSyncInfo().FindFileByFileName(v8FileName);
    BeAssert(f1.IsValid());
    BeAssert(f1.GetFileName().Equals(Utf8String(v8FileName)));

    auto rlink = GetRepositoryLinkElement(file);
    BeAssert(rlink.IsValid());

    auto aspect = SyncInfo::RepositoryLinkExternalSourceAspect::GetAspect(*rlink);
    BeAssert(aspect.IsValid() && "Root file should have a RepositoryLinkExternalSourceAspect");
    BeAssert(aspect.GetFileName().Equals(Utf8String(v8FileName)) && "RepositoryLinkExternalSourceAspect should store V8 filename");
    }
#endif

    return RepositoryLinkId(rlinkPost->GetElementId().GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::SetRepositoryLinkInAppData(DgnV8FileCR file, RepositoryLinkId rlinkId) const
    {
    auto appdata = (V8FileSyncInfoIdAppData*) const_cast<DgnV8FileR>(file).FindAppData(V8FileSyncInfoIdAppData::GetKey());
    if (nullptr == appdata)
        {
        appdata = new V8FileSyncInfoIdAppData(rlinkId, _GetIdPolicy(file));
        const_cast<DgnV8FileR>(file).AddAppData(V8FileSyncInfoIdAppData::GetKey(), appdata);
        }
    appdata->m_repositoryLinkId = rlinkId;

    _OnFileDiscovered(file);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryLinkId Converter::GetRepositoryLinkId(DgnV8FileCR file) const
    {
    SyncInfo::V8FileInfo finfo = const_cast<Converter*>(this)->GetSyncInfo().ComputeFileInfo(file);
    auto aspect = SyncInfo::RepositoryLinkExternalSourceAspect::FindAspectByIdentifier(GetDgnDb(), finfo.m_uniqueName);
    if (aspect.IsValid())
        {
        _OnFileDiscovered(file);
        return aspect.GetRepositoryLinkId();
        }
    LOG.tracev("Writing syncinfo %s as %s", finfo.m_v8Name.c_str(), finfo.m_uniqueName.c_str());
    return const_cast<Converter*>(this)->WriteRepositoryLink(const_cast<DgnV8FileR>(file));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_OnFileDiscovered(DgnV8FileCR file) const
    {
    if (std::find(m_v8Files.begin(), m_v8Files.end(), &const_cast<DgnV8FileR>(file)) == m_v8Files.end())
        {
        auto fileP = &const_cast<DgnV8FileR>(file);
        m_v8Files.push_back(fileP);
        _KeepFileAlive(file);

        if (!file.IsEmbeddedFile())
            m_v8FilesByName[BentleyApi::BeFileName(file.GetFileName().c_str())] = fileP;
        else
            {
            SyncInfo::V8FileInfo finfo = const_cast<RootModelConverter*>(this)->GetSyncInfo().ComputeFileInfo(file);
            m_embeddedFilesSeen[finfo.m_uniqueName] = fileP;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::DgnFilePtr Converter::OpenAndRegisterV8FileForDrawings(DgnV8Api::DgnFileStatus& openStatus, BeFileNameCR fn)
    {
    Bentley::DgnFilePtr newV8File = OpenDgnV8File(openStatus, fn);
    if (!newV8File.IsValid())
        return nullptr;

    GetRepositoryLinkId(*newV8File);

    return newV8File;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::RemapViewName(Utf8StringCR name, DgnV8FileR v8File, Utf8StringCR suffix)
    {
    return remapNameString(GetFileBaseName(v8File), name, suffix);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::RemapModelName(Utf8StringCR name, DgnV8FileR v8file, Utf8StringCR suffix)
    {
    return remapNameString(GetFileBaseName(v8file), name, suffix);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/13
+---------------+---------------+---------------+---------------+---------------+------*/
static uint32_t queryXmlRuleStringAttribute(Utf8String& value, BeXmlNode& xml, Utf8CP optionName)
    {
    if (BEXML_Success == xml.GetAttributeStringValue(value, optionName))
        return 1;
    Utf8String notOptionName("not_");
    notOptionName.append(optionName);
    return (BEXML_Success == xml.GetAttributeStringValue(value, notOptionName.c_str())) ? 2 : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ImportRule::InitFromXml(BeXmlNode& xml)
    {
    m_matchOnBase.file = queryXmlRuleStringAttribute(m_file, xml, "file");
    m_file.ToLower();

    m_matchOnBase.name = queryXmlRuleStringAttribute(m_name, xml, "name");
    m_name.ToLower();

    auto thenClause = xml.GetFirstChild();
    if (thenClause == nullptr)
        return;

    m_hasNewName = (BEXML_Success == thenClause->GetAttributeStringValue(m_newName, "newName"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/13
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String ruleToString(Utf8CP name, uint32_t wantMatch, Utf8StringCR value)
    {
    if (wantMatch==0)
        return "";

    Utf8String str(" ");
    str.append(name);
    str.append((wantMatch==1)? "=": "!=");
    str.append(value);
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImportRule::Matches(Utf8StringCR name, DgnV8FileCR file) const
    {
    if (m_matchOnBase.file && !matchesString(m_file, m_matchOnBase.file, Converter::GetFileBaseName(file)))
        return false;

    if (m_matchOnBase.name && !matchesString(m_name, m_matchOnBase.name, name))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ImportRule::ToString() const
    {
    Utf8String str("If ");
    str.append(ruleToString("file", m_matchOnBase.file, m_file));
    str.append(ruleToString("name", m_matchOnBase.name, m_name));
    str.append(" then");

    if (m_hasNewName)
        str.append(" newName=").append(m_newName);

    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ImportRule::ComputeNewName(Utf8StringR newName, DgnV8FileCR ff) const
    {
    if (!m_hasNewName)
        return BSIERROR;

    if (newName.empty())
        {
        if (m_newName.empty())
            return BSIERROR;

        newName = m_newName;
        newName.Trim();
        }

    static Utf8CP fileStr = "%file";
    size_t pos = newName.find(fileStr);
    if (pos != Utf8String::npos)
        newName.replace(pos, strlen(fileStr), Converter::GetFileBaseName(ff));

    return BentleyApi::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ImportRule::ComputeNewName(Utf8StringR newName, DgnV8ModelCR fm) const
    {
    if (!m_hasNewName)
        return BSIERROR;

    newName = m_newName;
    newName.Trim();

    Utf8String oldName(fm.GetModelName());

    static Utf8CP modelStr = "%model";
    size_t pos = newName.find(modelStr);
    if (pos != Utf8String::npos)
        newName.replace(pos, strlen(modelStr), oldName);

    return ComputeNewName(newName, *fm.GetDgnFileP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ParseModelConfig(Config& config)
    {
    if (config.GetDom() == nullptr)
        return;

    BeXmlDom::IterableNodeSet nodes;
    config.GetDom()->SelectNodes(nodes, "/ImportConfig/Models/ImportRules/If", nullptr);

    for (auto node : nodes)
        {
        ImportRule rule;
        rule.InitFromXml(*node);
        m_modelImportRules.push_back(rule);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ParseLevelConfigAndUpdateParams(Config& config)
    {
    if (_GetParams().GetCopyLevel() != Params::CopyLevel::UseConfig)
        return;

    Params::CopyLevel v = Params::CopyLevel::Never;
    Utf8String viewTypes = m_config.GetXPathString("/ImportConfig/Levels/@copy", "");
    if (viewTypes.EqualsI("always"))
        v = Params::CopyLevel::Always;
    else if (viewTypes.EqualsI("ifdifferent"))
        v = Params::CopyLevel::IfDifferent;
    _GetParamsR().SetCopyLevel(v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      04/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Converter::SearchForMatchingRule(ImportRule& entryOut, DgnV8ModelCR oldModel)
    {
    Utf8String modelName(oldModel.GetModelName());
    for (auto const& rule : m_modelImportRules)
        {
        if (rule.Matches(modelName, *oldModel.GetDgnFileP()))
            {
            entryOut = rule;
            LOG_MODEL.tracev("Model %s <- Rule %s", modelName.c_str(), rule.ToString().c_str());
            return BentleyApi::SUCCESS;
            }
        }
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::_ComputeModelName(DgnV8ModelCR v8Model)
    {
    Utf8String dgnDbModelName(v8Model.GetModelName());

    ImportRule modelMergeEntry;
    if (SearchForMatchingRule(modelMergeEntry, v8Model) == BentleyApi::SUCCESS)
        {
        Utf8String computedName;
        if (SUCCESS == modelMergeEntry.ComputeNewName(computedName, v8Model))
            {
            dgnDbModelName = computedName;
            }
        }

    if (dgnDbModelName.empty())		// This happens for V7 files
        dgnDbModelName = GetFileBaseName(*v8Model.GetDgnFileP());

    if (!_GetNamePrefix().empty())
        dgnDbModelName.insert(0, _GetNamePrefix().c_str());

    DgnModels& models = m_dgndb->Models();
    models.ReplaceInvalidCharacters(dgnDbModelName, models.GetIllegalCharacters(), '_');

#if 0
    Utf8String uniqueName(dgnDbModelName);
    Utf8String suffix;
    for (int i=0; models.QueryModelId(DgnModel::CreateModelCode(uniqueName)).IsValid(); )
        {
        uniqueName = RemapModelName(dgnDbModelName, *v8Model.GetDgnFileP(), suffix);

        // _RemapModelName will append the file name, which could re-introduce new illegal characters.
        models.ReplaceInvalidCharacters(uniqueName, models.GetIllegalCharacters(), '_');

        suffix.Sprintf("-%d", ++i);
        }

    return uniqueName;
#else
    return dgnDbModelName;
#endif
    }

typedef bmap <DgnV8Api::ModelId, DgnV8Api::DgnModelType> T_ModelTypeMap;

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   02/15
//=======================================================================================
struct ModelTypeAppData : DgnV8Api::DgnFileAppData
    {
    // For every 2d model with DgnModelType::Normal, there will be an entry in this map telling what output model type we want.
    T_ModelTypeMap      m_modelTypeMap;
    bool                m_modelsScanned;

    ModelTypeAppData ()
        {
        m_modelsScanned = false;
        }

    static DgnV8Api::DgnFileAppData::Key const& GetKey() {static DgnFileAppData::Key s_key; return s_key;}

    void    SetEffectiveModelType (DgnV8Api::ModelId modelId, DgnV8Api::DgnModelType modelType)
        {
        m_modelTypeMap[modelId] = modelType;
        }

    bool    HasEffectiveModelType (DgnV8Api::ModelId modelId)
        {
        return (m_modelTypeMap.end() != m_modelTypeMap.find (modelId));
        }

public:
    virtual void _OnCleanup(DgnFileR host) override
        {
        delete this;
        }

    static ModelTypeAppData& Get(DgnV8FileR v8DgnFile)
        {
        ModelTypeAppData* mtAppData = dynamic_cast <ModelTypeAppData*> (v8DgnFile.FindAppData (ModelTypeAppData::GetKey()));
        if (nullptr == mtAppData)
            v8DgnFile.AddAppData (ModelTypeAppData::GetKey(), (mtAppData = new ModelTypeAppData ()));
        return *mtAppData;
        }

    static DgnV8Api::DgnModelType GetEffectiveModelType (DgnV8FileR v8DgnFile, DgnV8Api::ModelId modelId, DgnV8Api::DgnModelType defaultModelType)
        {
        ModelTypeAppData*    mtAppData;
        if (nullptr != (mtAppData = dynamic_cast <ModelTypeAppData*> (v8DgnFile.FindAppData (ModelTypeAppData::GetKey()))))
            {
            auto iterator = mtAppData->m_modelTypeMap.find (modelId);
            if (mtAppData->m_modelTypeMap.end() != iterator)
                return iterator->second;
            }

        // It gets here for all 3d models, and 2d models that we haven't overridden the modelType.
        return defaultModelType;
        }

    static DgnV8Api::DgnModelType GetEffectiveModelType (DgnV8ModelR v8DgnModel)
        {
        DgnV8FileR          v8DgnFile = *v8DgnModel.GetDgnFileP();
        DgnV8Api::ModelId   modelId   = v8DgnModel.GetModelId();
        return GetEffectiveModelType (v8DgnFile, modelId, v8DgnModel.GetModelType());
        }

    static void     SetEffectiveModelType (DgnV8ModelR v8DgnModel, DgnV8Api::DgnModelType modelType)
        {
        DgnV8FileR          v8DgnFile = *v8DgnModel.GetDgnFileP();
        DgnV8Api::ModelId   modelId   = v8DgnModel.GetModelId();

        ModelTypeAppData*    mtAppData;
        if (nullptr == (mtAppData = dynamic_cast <ModelTypeAppData*> (v8DgnFile.FindAppData (ModelTypeAppData::GetKey()))))
            {
            v8DgnFile.AddAppData (ModelTypeAppData::GetKey(), (mtAppData = new ModelTypeAppData ()));
            }

        mtAppData->SetEffectiveModelType (modelId, modelType);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void     Converter::CopyEffectiveModelType(DgnV8ModelR newModel, DgnV8ModelR oldModel)
    {
    ModelTypeAppData::SetEffectiveModelType(newModel, ModelTypeAppData::GetEffectiveModelType(oldModel));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void     Converter::SetEffectiveModelType(DgnV8ModelR newModel, DgnV8Api::DgnModelType modelType)
    {
    ModelTypeAppData::SetEffectiveModelType(newModel, modelType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void     Converter::ClassifyNormal2dModel(DgnV8ModelR v8Model, ModelTypeAppData& mtAppData)
    {
    // We consider the presence of a GCS to be evidence that it is a spatial model, so get the model so we can see if it has a GCS.

    // need the control elements filled to find the GCS.
    v8Model.FillCache(DgnV8Api::DgnModelSections::ControlElements);

    // This is the only case where we are making a change: A 2d Normal model without a GCS is considered to be DgnModelType::Drawing.
    if (nullptr == Bentley::GeoCoordinates::DgnGCS::FromModel (&v8Model, true))
        mtAppData.SetEffectiveModelType (v8Model.GetModelId(), DgnV8Api::DgnModelType::Drawing);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void     Converter::Classify2dModelIfNormal(DgnV8ModelR v8Model, ModelTypeAppData* mtAppData)
    {
    // if normal models are considered spatial, we consider all Normal models to be Normal.
    if (_ConsiderNormal2dModelsSpatial())
        return;

    // we are only concerned with 2d models of DgnModelType::Normal.
    if ( v8Model.Is3D() || (DgnV8Api::DgnModelType::Normal != v8Model.GetModelType()) )
        return;

    if (nullptr == mtAppData)
        mtAppData = &ModelTypeAppData::Get(*v8Model.GetDgnFileP());

    // if a conversion type was already forced in because the model was referenced from the root model, don't consider.
    if (mtAppData->HasEffectiveModelType (v8Model.GetModelId()))
        return;

    ClassifyNormal2dModel(v8Model, *mtAppData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void     Converter::ClassifyNormal2dModels (DgnV8FileR v8DgnFile)
    {
    // The reason for this method is that some V8 files have 2d models that should be considered to be Drawing models, but they
    // have DgnModelType::Normal instead of DgnModelType::Drawing. That is because DgnModelType::Drawing and DgnModelType::Sheet
    // did not exist in the earliest V8 file formats.

    // The converter will convert 2d Models of DgnModelType::Normal to Spatial models, promoting them to 3d in the process. So
    // this method traverses the models and changes 2d models of DgnModelType::Normal to DgnModelType::Drawing if that shouldn't happen.

    // If we have already considered this file, skip it.
    ModelTypeAppData& mtAppData = ModelTypeAppData::Get(v8DgnFile);
    if (mtAppData.m_modelsScanned)
        return;

    // set it to scanned.
    mtAppData.m_modelsScanned = true;

    // if normal models are considered spatial, we don't really need to scan - we consider all Normal models to be Normal.
    if (_ConsiderNormal2dModelsSpatial())
        return;

    //  A V7 file has only one model. The DgnV8Api::ModelIndexItem::Is3D fucntion for a V7 file returns an incorrect value, so don't use it.
    DgnV8Api::DgnFileFormatType   format;
    if (SUCCESS == v8DgnFile.GetVersion (&format, NULL, NULL) && DgnV8Api::DgnFileFormatType::V7 == format)
        {
        auto model = v8DgnFile.FindLoadedModelById(v8DgnFile.GetDefaultModelId());
        if (nullptr == model)
            return;
        if (!model->Is3D())
            mtAppData.SetEffectiveModelType (model->GetModelId(), DgnV8Api::DgnModelType::Drawing);
        return;
        }

    // Note that this loop only classifies models that are in the model index. The converter will classify non-indexed models
    // when it encounters them as reference attachments to other drawings and sheets.
    for (DgnV8Api::ModelIndexItem const& item : v8DgnFile.GetModelIndex())
        {
        // we are only concerned with 2d models of DgnModelType::Normal.
        if ( item.Is3D() || (DgnV8Api::DgnModelType::Normal != item.GetModelType()) )
            continue;

        // if a conversion type was already forced in because the model was referenced from the root model, don't consider.
        if (mtAppData.HasEffectiveModelType (item.GetModelId()))
            continue;

        Bentley::DgnModelPtr v8Model = v8DgnFile.LoadModelById(item.GetModelId());

        if (!v8Model.IsValid())
            continue;

        ClassifyNormal2dModel(*v8Model, mtAppData);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool        RootModelConverter::_ConsiderNormal2dModelsSpatial ()
    {
    // m_considerNormal2dModelsSpatial is set in the RootModelConverter constructor so we don't have to continually check the Config object.
    return m_considerNormal2dModelsSpatial;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void        RootModelConverter::ForceAttachmentsToSpatial (Bentley::DgnAttachmentArrayR attachments)
    {
    // If we get here, we are converting a Spatial model. Unless their DgnModelType Drawing or Sheet, all of its attachments should be considered spatial.
    for (DgnV8Api::DgnAttachment* attachment : attachments)
        {
        DgnV8ModelP  dgnV8Model = attachment->GetDgnModelP();
        if (nullptr == dgnV8Model)
            continue;
        if (dgnV8Model->Is3D())
            continue;

        // If it is DgnModelType::Normal, "lock it down" as that so that a later scan of all models can't change it to Drawing.
        if (DgnV8Api::DgnModelType::Normal == dgnV8Model->GetModelType())
            ModelTypeAppData::SetEffectiveModelType (*dgnV8Model, DgnV8Api::DgnModelType::Normal);

        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnClassId Converter::_ComputeModelType(DgnV8ModelR v8Model)
    {
    enum class ModelType { Physical, Sheet, Drawing };
    ModelType thisModelType = ModelType::Physical;

    // In case we haven't done so before, classify the 2d DgnModelType::Normal models.
    DgnV8FileR dgnV8File = *v8Model.GetDgnFileP();
    ClassifyNormal2dModels (dgnV8File);

    // Get the effective model type. It can be stored in the FileAppData if we know what we want, else we use the DgnModelType from the DgnV8 ModelInfo
    DgnV8Api::DgnModelType v8ModelType = ModelTypeAppData::GetEffectiveModelType (v8Model);
    if (DgnV8Api::DgnModelType::Sheet == v8ModelType)
        thisModelType = ModelType::Sheet;
    else if (DgnV8Api::DgnModelType::Drawing == v8ModelType)
        thisModelType = ModelType::Drawing;

    Utf8String className;
    switch (thisModelType)
        {
        case ModelType::Physical:
            className = BIS_CLASS_PhysicalModel;
            break;

        case ModelType::Sheet:
            className = BIS_CLASS_SheetModel;
            break;

        case ModelType::Drawing:
            switch (GetV8NamedViewTypeOfFirstAttachment(v8Model))
                {
                case V8NamedViewType::Section:     className = BIS_CLASS_SectionDrawingModel; break;
                case V8NamedViewType::Plan:        className = BIS_CLASS_SectionDrawingModel; break; // *** WIP_CONVERTER_CVE
                case V8NamedViewType::Elevation:   className = BIS_CLASS_SectionDrawingModel; break; // *** WIP_CONVERTER_CVE
                case V8NamedViewType::Detail:      className = BIS_CLASS_SectionDrawingModel; break; // *** WIP_CONVERTER_CVE
                default:                           className = BIS_CLASS_DrawingModel;
                }
            break;

        default:
            BeAssert(false && "Missing mapping from DgnV8 model type to DgnDb model type");
            return DgnClassId();
        }

    DgnClassId classId(m_dgndb->Schemas().GetClassId(BIS_ECSCHEMA_NAME, className.c_str()));
    BeAssert(classId.IsValid());
    return classId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::IsV8DrawingModel (DgnV8FileR v8File, DgnV8Api::ModelIndexItem const& item)
    {
    // whether we should treat it as a DrawingModel is determined by whether it's a 2D, whether it came from a DgnAttachment to the root model, etc.
    return DgnV8Api::DgnModelType::Drawing == ModelTypeAppData::GetEffectiveModelType (v8File, item.GetModelId(), item.GetModelType());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::IsV8DrawingModel (DgnV8ModelR v8Model)
    {
    // whether we should treat it as a DrawingModel is determined by whether it's a 2D, whether it came from a DgnAttachment to the root model, etc.
    return DgnV8Api::DgnModelType::Drawing == ModelTypeAppData::GetEffectiveModelType (v8Model);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::_IsModelPrivate(DgnV8ModelCR v8Model)
    {
    return v8Model.GetModelInfo().GetIsHidden();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2014
//---------------------------------------------------------------------------------------
static UnitDefinition fromV8(DgnV8Api::UnitDefinition const& v8Def)
    {
    return UnitDefinition(UnitBase(v8Def.GetBase()), UnitSystem(v8Def.GetSystem()), v8Def.GetNumerator(), v8Def.GetDenominator(), Utf8String(v8Def.GetLabelCP()).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId Converter::_MapModelIntoProject(DgnV8ModelR v8Model, Utf8CP newName, DgnV8Api::DgnAttachment const* attachment)
    {
    return CreateModelFromV8Model(v8Model, newName, _ComputeModelType(v8Model), attachment);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DrawingPtr Converter::CreateDrawing(Utf8CP codeValue, Utf8CP userLabel)
    {
    DocumentListModelPtr drawingListModel = m_dgndb->Models().Get<DocumentListModel>(m_drawingListModelId);
    if (!drawingListModel.IsValid())
        return nullptr;

    DgnCode drawingCode = !Utf8String::IsNullOrEmpty(codeValue)? Drawing::CreateUniqueCode(*drawingListModel, codeValue): DgnCode::CreateEmpty();
    DrawingPtr drawing = Drawing::Create(*drawingListModel, drawingCode.GetValueUtf8CP());
    if (!drawing.IsValid())
        return nullptr;

    if (userLabel)
        drawing->SetUserLabel(userLabel);
    return drawing;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
SectionDrawingPtr Converter::CreateSectionDrawing(Utf8CP codeValue, Utf8CP userLabel)
    {
    DocumentListModelPtr drawingListModel = m_dgndb->Models().Get<DocumentListModel>(m_drawingListModelId);
    if (!drawingListModel.IsValid())
        return nullptr;

    DgnCode drawingCode = !Utf8String::IsNullOrEmpty(codeValue)? Drawing::CreateUniqueCode(*drawingListModel, codeValue): DgnCode::CreateEmpty();
    SectionDrawingPtr drawing = SectionDrawing::Create(*drawingListModel, drawingCode.GetValueUtf8CP());
    if (!drawing.IsValid())
        return nullptr;

    if (userLabel)
        drawing->SetUserLabel(userLabel);
    return drawing;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
Sheet::ElementPtr Converter::CreateSheet(Utf8CP name, DgnV8ModelCR v8Model)
    {
    auto const& modelInfo = v8Model.GetModelInfo();

    DocumentListModelPtr sheetListModel = m_dgndb->Models().Get<DocumentListModel>(m_sheetListModelId);
    if (!sheetListModel.IsValid())
        return nullptr;

    auto v8SheetDef = modelInfo.GetSheetDefCP();
    if (nullptr == v8SheetDef)
        {
        BeDataAssert(false);
        ReportError(IssueCategory::Unknown(), Issue::ConvertFailure(), "missing sheet def");
        return nullptr;
        }

    double scale = SheetsComputeScale(v8Model);

    DPoint2d size;
    v8SheetDef->GetSize(size.x, size.y);
    double toMeters = ComputeUnitsScaleFactor(v8Model) * v8Model.GetModelInfo().GetAnnotationScaleFactor();

    size.x *= toMeters;
    size.y *= toMeters;

    DgnCode sheetCode = Sheet::Element::CreateUniqueCode(*sheetListModel, name);
    Sheet::ElementPtr sheet = Sheet::Element::Create(*sheetListModel, scale, size, sheetCode.GetValueUtf8CP());
    if (!sheet.IsValid())
        return nullptr;

    sheet->SetUserLabel(name);
    return sheet;
    }

/*---------------------------------------------------------------------------------**//**
* Given a DgnV8Model and a new model name, create a new DgnModel in the DgnDb and return its DgnModelId.
* @bsimethod                                    Sam.Wilson      09/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId Converter::CreateModelFromV8Model(DgnV8ModelCR v8Model, Utf8CP newName, DgnClassId classId, DgnV8Api::DgnAttachment const* attachment)
    {
    ModelHandlerP handler = dgn_ModelHandler::Model::FindHandler(*m_dgndb, classId);
    if (nullptr == handler)
        {
        BeAssert(false);
        ReportError(IssueCategory::Unknown(), Issue::ConvertFailure(), "bad model type");
        return DgnModelId();
        }

    DgnElementId modeledElementId;

    if (m_dgndb->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingModel) == classId)
        {
        auto suggestedLabel = GetSuggestedUserLabel(v8Model, newName);
        auto modelCode = ModelHasNoCode(v8Model)? nullptr: newName;
        DrawingCPtr drawing = CreateDrawingAndInsert(modelCode, suggestedLabel);
        if (!drawing.IsValid())
            {
            BeAssert(false);
            ReportError(IssueCategory::Unknown(), Issue::ConvertFailure(), "error creating Drawing");
            return DgnModelId();
            }

        modeledElementId = drawing->GetElementId();
        }
    else if (m_dgndb->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_SectionDrawingModel) == classId)
        {
        auto suggestedLabel = GetSuggestedUserLabel(v8Model, newName);
        auto modelCode = ModelHasNoCode(v8Model)? nullptr: newName;
        SectionDrawingCPtr drawing = CreateSectionDrawingAndInsert(modelCode, suggestedLabel);
        if (!drawing.IsValid())
            {
            BeAssert(false);
            ReportError(IssueCategory::Unknown(), Issue::ConvertFailure(), "error creating SectionDrawing");
            return DgnModelId();
            }

        modeledElementId = drawing->GetElementId();
        }
    else if (m_dgndb->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_SheetModel) == classId)
        {
        auto sheet = CreateSheetAndInsert(newName, v8Model);
        if (!sheet.IsValid())
            {
            BeAssert(false);
            ReportError(IssueCategory::Unknown(), Issue::ConvertFailure(), "error creating Sheet");
            return DgnModelId();
            }

        modeledElementId = sheet->GetElementId();
        }
    else if (m_dgndb->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_PhysicalModel) == classId)
        {
        SubjectCR parentSubject = _GetSpatialParentSubject();

        DgnCode partitionCode = PhysicalPartition::CreateCode(parentSubject, newName);
        Utf8String qualifiedName(newName);
        if (GetDgnDb().Elements().QueryElementIdByCode(partitionCode).IsValid())
            {
            // if the model's own name is not unique among all references attachments of the parent, then
            //  fall back on filename-modelname.
            qualifiedName = GetFileBaseName(*v8Model.GetDgnFileP());
            qualifiedName.append("-");
            qualifiedName.append(newName);

            partitionCode = PhysicalPartition::CreateCode(parentSubject, qualifiedName.c_str());
            if (GetDgnDb().Elements().QueryElementIdByCode(partitionCode).IsValid())
                {
                //  If somehow!?! even filename-modelname is not unique among the parent's attachments, then just
                //  generate a (meaningless) unique code.
                partitionCode = PhysicalPartition::CreateUniqueCode(parentSubject, qualifiedName.c_str());
                }
            }

        PhysicalPartitionPtr ed = PhysicalPartition::Create(parentSubject, partitionCode.GetValueUtf8CP());

        Json::Value modelProps(Json::nullValue);
        auto isRef = parentSubject.GetSubjectJsonProperties(Subject::json_Model()).GetMember("Type") == "References";
        modelProps["Content"] = isRef? "Reference": "Master";
        ed->SetPhysicalPartitionJsonProperties(PhysicalPartition::json_Model(), modelProps);

        PhysicalPartitionCPtr partition = ed->InsertT<PhysicalPartition>();
        if (!partition.IsValid())
            {
            BeAssert(false);
            ReportError(IssueCategory::Unknown(), Issue::ConvertFailure(), "error creating PhysicalPartition for PhysicalModel");
            return DgnModelId();
            }

        modeledElementId = partition->GetElementId();

        }
    else
        {
        BeAssert(false && "Need to manufacture a DgnElement for the DgnModel to model");
        ReportError(IssueCategory::Unknown(), Issue::ConvertFailure(), "Unhandled model type");
        return DgnModelId();
        }
    iModelBridge::InsertElementHasLinksRelationship(GetDgnDb(), modeledElementId, GetRepositoryLinkId(*v8Model.GetDgnFileP()));

    DgnModelPtr model = handler->Create(DgnModel::CreateParams(*m_dgndb, classId, modeledElementId, nullptr));
    if (!model.IsValid())
        {
        BeAssert(false);
        ReportError(IssueCategory::Unknown(), Issue::CantCreateModel(), newName);
        return DgnModelId();
        }

    GeometricModelP geometricModel = model->ToGeometricModelP();
    if (geometricModel != nullptr)
        {
        DgnV8Api::ModelInfo const& v8ModelInfo = _GetModelInfo(v8Model);
        auto& displayInfo = geometricModel->GetFormatterR();

        displayInfo.SetUnits(fromV8(v8ModelInfo.GetMasterUnit()), fromV8(v8ModelInfo.GetSubUnit()));
        displayInfo.SetRoundoffUnit(v8ModelInfo.GetRoundoffUnit(), v8ModelInfo.GetRoundoffRatio());
        displayInfo.SetLinearUnitMode(DgnUnitFormat(v8ModelInfo.GetLinearUnitMode()));
        displayInfo.SetLinearPrecision(PrecisionFormat(v8ModelInfo.GetLinearPrecision()));
        displayInfo.SetAngularMode(AngleMode(v8ModelInfo.GetAngularMode()));
        displayInfo.SetAngularPrecision(AnglePrecision(v8ModelInfo.GetAngularPrecision()));
        displayInfo.SetDirectionMode(DirectionMode(v8ModelInfo.GetDirectionMode()));
        displayInfo.SetDirectionClockwise(v8ModelInfo.GetDirectionClockwise());
        displayInfo.SetDirectionBaseDir(v8ModelInfo.GetDirectionBaseDir());
        }

    /* WIP: move description to modeled element instead of model.
    Utf8String descr;
    wchar_t wDesc[DgnV8Api::MAX_MODEL_DESCR_LENGTH+1];
    if (v8Model.GetModelDescription(wDesc) == SUCCESS && 0 != *wDesc)
        descr = Utf8String(wDesc);
        */

    if (m_dgndb->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_SheetModel) == classId)
        {
        }

    model->SetIsPrivate(_IsModelPrivate(v8Model));
    auto modelStatus = model->Insert();
    if (modelStatus != DgnDbStatus::Success)
        {
        BeAssert((DgnDbStatus::LockNotHeld != modelStatus) && "Failed to get or retain necessary locks");
        BeAssert(false);
        ReportError(IssueCategory::Unknown(), Issue::ConvertFailure(), Utf8String(v8Model.GetModelNameCP()).c_str());
        return DgnModelId();
        }

    return model->GetModelId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void tweakTextIndentationLinkageData(DgnV8Api::EditElementHandle& eeh, DgnV8Api::ElementLinkageIterator& il)
    {
    size_t entireLnkageSize = 2*DgnV8Api::LinkageUtil::GetWords(il.GetLinkage());   // includes the LinkageHeader
    size_t linkageDataSize = entireLnkageSize - sizeof(DgnV8Api::LinkageHeader);    // just the data
    ScopedArray<Byte> bytes(linkageDataSize);
    memcpy(bytes.GetData(), il.GetData(), linkageDataSize);

    /*
    if (true)
        {
        BentleyApi::DataInternalizer reader ((byte*)bytes.GetData(), linkageDataSize);

        int offset = 0;

        UInt16 dummyWord;
        reader.get (&dummyWord);
        printf ("[%d] pad %d\n", offset, dummyWord);
        offset += sizeof(dummyWord);

        reader.get (&dummyWord);
        printf ("[%d] pad %d\n", offset, dummyWord);
        offset += sizeof(dummyWord);

        double d;
        reader.get (&d);
        printf ("[%d] firstLineIndent=%0.17lg\n", offset, d);
        offset += sizeof(d);

        reader.get (&d);
        printf ("[%d] paragraphIndent=%0.17lg\n", offset, d);
        offset += sizeof(d);

        int tabCount;
        reader.get (&tabCount);
        printf ("[%d] tabCount=%d\n", offset, tabCount);
        offset += sizeof(tabCount);
        }
        */


    //  Here's the comment from MdlTextInternal.cpp:
    //
    // In the old days, we had a linkage structure that went like this:
    //
    //  {
    //  LinkageHeader           linkHeader; // 2 words
    //      {
    //00   ** 2 Words Padding **
    //04   double      firstLineIndent;    // 4 words
    //12   double      paragraphIndent;    // 4 words
    //20   int         tabCount;           // 2 words
    //24   ** 2 Words Padding **
    //28   double      tabStops[1];        // 4 words
    //     }
    //  }
    //
    //  And simply cast it in/out of the actual element linkage.
    //  Thus, there are 2 words of padding between linkHeader and firstLineIndent, as well as 2 words of padding between tabCount and TabStops.

    auto p = bytes.GetData();
    memset(p, 0, 4);
    memset(p+24, 0, 4);

    int tabCount = *(int*)(p+20);
    double* tabStops = (double*)(p+28);

    Byte* tabStopEnd = (Byte*)(tabStops + tabCount);
    Byte* linkageDataEnd = (Byte*)p + linkageDataSize;

    if (tabStopEnd < linkageDataEnd)
        memset(tabStopEnd, 0, linkageDataEnd-tabStopEnd);

    eeh.ReplaceElementLinkage(il, *il.GetLinkage(), bytes.GetData());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::_TweakLinkageForComparisonAndHashPurposes(DgnV8Api::EditElementHandle& eeh, DgnV8Api::ElementLinkageIterator& il)
    {
    // *** WIP_CONVERTER: Use DataDef to convert from file to memory format

    switch (il.GetLinkage()->primaryID)
        {
        case 22544:
            {
            tweakTextIndentationLinkageData(eeh, il);
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::_TweakElementForComparisonAndHashPurposes(DgnV8Api::EditElementHandle& eeh, DgnV8Api::MSElement const& elementData)
    {
    // *** WIP_CONVERTER: Let the Handler Extension do this
    switch (elementData.ehdr.type)
        {
        default:
            return;

        case DgnV8Api::TEXT_ELM:
        case DgnV8Api::TEXT_NODE_ELM:
        case DgnV8Api::ATTRIBUTE_ELM:
        case DgnV8Api::DIMENSION_ELM:
            break;
        }

    auto tDscr = DgnV8Api::MSElementDescr::Allocate(elementData, eeh.GetDgnModelP());
    eeh.SetElementDescr(tDscr, true, false);

    auto tweakedElementData = eeh.GetElementP();

    switch (elementData.ehdr.type)
        {
        case DgnV8Api::TEXT_NODE_ELM:
            {
            // *** WIP_CONVERTER: text node number means nothing in graphite. On the other hand, a customer converter might use this information ...
            tweakedElementData->text_node_2d.nodenumber = 0;
            break;
            }

        case DgnV8Api::ATTRIBUTE_ELM:
            {
            // 'version' is an internal concept in DGN tags. It affects how the tag element handler loads the stored data. The results of conversion are not affected by this.
            tweakedElementData->attrElm.version = 0;
            break;
            }

        case DgnV8Api::DIMENSION_ELM:
            {
            // 'version' is an internal concept in DGN dimensions. It affects how the dimension element handler loads the stored data. The results of conversion are not affected by this.
            tweakedElementData->dim.version = 0;
            break;
            }

        case DgnV8Api::TEXT_ELM:
            break; // deal with various Text linkages below

        default:
            {
            BeAssert(false && "must handle each type selected in switch above");
            return;
            }
        }

    for (auto li = eeh.BeginElementLinkages(); li != eeh.EndElementLinkages(); ++li)
        _TweakLinkageForComparisonAndHashPurposes(eeh, li);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::CaptureFileDiscard(DgnV8Api::DgnFile& ff)
    {
    // *** WIP_CONVERTER - record discard in syncinfo?
    ReportIssueV(IssueSeverity::Info, IssueCategory::Filtering(), Issue::FileFilteredOut(), Utf8String(ff.GetFileName().c_str()).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::CaptureModelDiscard(DgnV8Api::DgnModel& fm)
    {
    ReportIssueV(IssueSeverity::Info, IssueCategory::Filtering(), Issue::ModelFilteredOut(), IssueReporter::FmtModel(fm).c_str());
    // *** WIP_CONVERTER - record discard in syncinfo?
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::IsV8Format(DgnV8FileR dgnFile) const
    {
    DgnV8Api::DgnFileFormatType format;
    dgnFile.GetVersion(&format, nullptr, nullptr);
    return (DgnV8Api::DgnFileFormatType::V8 == format);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
StableIdPolicy Converter::_GetIdPolicy(DgnV8FileCR dgnFile) const
    {
    // if they said to never use v8 ElementIds, return hash regardless of format
    if (StableIdPolicy::ByHash == _GetParams().GetStableIdPolicy())
        return StableIdPolicy::ByHash;

    // some files have stable ids
    DgnV8Api::DgnFileFormatType format = DgnV8Api::DgnFileFormatType::V8;
    const_cast<DgnV8FileR>(dgnFile).GetVersion(&format, nullptr, nullptr);
    switch (format)
        {
        case DgnV8Api::DgnFileFormatType::V8:
        case DgnV8Api::DgnFileFormatType::DWG:
        case DgnV8Api::DgnFileFormatType::DXF:
        case DgnV8Api::DgnFileFormatType::IFC:
            return StableIdPolicy::ById;
        }

    // others do not
    return StableIdPolicy::ByHash;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFilePtr Converter::OpenDgnV8File(DgnV8Api::DgnFileStatus &openStatus, BeFileNameCR inputFile, Utf8CP password)
    {
    auto doc = DgnV8Api::DgnDocument::CreateFromFileName(openStatus, inputFile, nullptr, Bentley::DEFDGNFILE_ID, DgnV8Api::DgnDocument::FetchMode::Read, DgnV8Api::DgnDocument::FetchOptions::Default);
    if (doc == nullptr)
        return nullptr;

    auto fullSpec = doc->GetMonikerPtr()->ResolveFileName();

    DgnFilePtr file = DgnV8Api::DgnFile::GetOpenedFileArray().GetOpenFileByName (fullSpec.c_str(), nullptr, -1, false, false);

    if (file.IsValid())
        return file;

    file = DgnV8Api::DgnFile::Create(*doc, DgnV8Api::DgnFileOpenMode::ReadOnly);

    if (!file.IsValid())
        return nullptr;

    SupplyPassword supplyPw(password);
    DgnV8Api::DgnFileLoadContext v8LoadContext(&supplyPw);

    openStatus = (DgnV8Api::DgnFileStatus) file->LoadFile(nullptr, &v8LoadContext, true);

    if (DgnV8Api::DGNFILE_STATUS_Success == openStatus && !file->HasDigitalRight(DgnV8Api::DgnFile::DIGITAL_RIGHT_Export))
        openStatus = DgnV8Api::DGNFILE_ERROR_RightNotGranted;

    file->SetShareFlag(true);     // TRICKY: If a drawing or sheet references a model in this file, we want it to find our copy of it, not load another copy.

    return (SUCCESS != openStatus) ? nullptr : file;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFilePtr Converter::OpenDgnV8File(DgnV8Api::DgnFileStatus &openStatus, BeFileNameCR inputFile)
    {
    return OpenDgnV8File(openStatus, inputFile, _GetParams().GetPassword().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
Converter::Converter(Params const& params)
    : m_issueReporter(params.GetReportFileName()), m_config(params.GetConfigFile(), *this),
    m_syncInfo(*this), m_selectAllQueryV8(nullptr), m_linkConverter(nullptr), m_elementConverter(nullptr), m_elementAspectConverter(nullptr)
    {
    m_lineStyleConverter = LineStyleConverter::Create(*this);
    m_currIdPolicy = StableIdPolicy::ById;

    m_haveCreatedThumbnails = false;

    m_rootTrans.InitIdentity();
    m_rootTransChange.InitIdentity();

    m_monitor = new Monitor;

    // read config file, if supplied
    if (!m_config.GetInstanceFilename().empty())
        m_config.ReadFromXmlFile();

    m_addDebugDgnCodes = m_config.GetOptionValueBool("SaveIdsAsCodes", false);

    if (DgnV8Api::ConfigurationManager::IsVariableDefined(L"DGNV8_SaveIdsAsCodes")) // so developers can turn this on without changing default config file
        m_addDebugDgnCodes = true;

    ParseModelConfig(m_config);
    if (!params.GetIsPowerplatformBased())
        Converter::InitV8ForeignFileTypes (this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
Converter::~Converter()
    {
    if (m_issueReporter.HasIssues())
        m_issueReporter.CloseReport();

    ClearV8ProgressMeter();

    if (nullptr != m_linkConverter)
        delete m_linkConverter;

    for (BeFileNameCR pathToDelete : m_tempFontFiles)
        pathToDelete.BeDeleteFile();

    if (nullptr != m_elementConverter)
        delete m_elementConverter;

    if (nullptr != m_elementAspectConverter)
        delete m_elementAspectConverter;

    Converter::InitV8ForeignFileTypes (nullptr);

    // This is a hack - When a schema that references the ECv3CustomAttributes schema is imported, the copy being held by this helper receives an ECSchemaId.  Since that is a static
    // helper, if the converter attempts to import that schema into a different ecdb, it will fail.
    BentleyApi::ECN::ConversionCustomAttributeHelper::Reset();

    // Need to clear out the NamedGroupInfo so it doesn't affect other files
    V8NamedGroupInfo::Reset();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  01/15
//---------------------------------------------------------------------------------------
Transform Converter::ComputeAttachmentTransform(TransformCR parentTransform, DgnAttachmentCR v8attachment)
    {
    Transform atrans;
    v8attachment.GetTransformToParent((Bentley::TransformR)atrans, /*scaleZfor2dRef*/false);
    return Transform::FromProduct(parentTransform, atrans);
    }

#ifdef WIP_DUMP

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  03/17
//---------------------------------------------------------------------------------------
static Utf8String getIndent(int indent)
    {
    Utf8String s;
    while (indent--)
        s.append("    ");
    return s;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  03/17
//---------------------------------------------------------------------------------------
static Utf8String getDescr(DgnElementCR el)
    {
    auto ifp = dynamic_cast<InformationPartitionElement const*>(&el);
    if (nullptr != ifp)
        return ifp->GetDescription();
    auto subj = dynamic_cast<Subject const*>(&el);
    if (nullptr != subj)
        return subj->GetDescription();
    return "";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  03/17
//---------------------------------------------------------------------------------------
static void dumpElement(DgnElementCR el)
    {
    printf("%s", Converter::IssueReporter::FmtElement(el).c_str());
    if (!el.GetCode().GetValue().empty())
        printf(" Code:[%s]", el.GetCode().GetValueUtf8CP());
    if (0 != *el.GetUserLabel())
        printf(" UserLabel:[%s]", el.GetUserLabel());
    auto descr = getDescr(el);
    if (!descr.empty())
        printf(" Descr:[%s]", descr.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  03/17
//---------------------------------------------------------------------------------------
static void dumpPartitionRepositoryLink(DgnElementCR el)
    {
    BeSQLite::EC::ECSqlStatement stmt;
    stmt.Prepare(el.GetDgnDb(), "SELECT TargetECInstanceId FROM " BIS_SCHEMA(BIS_REL_ElementHasLinks) " WHERE SourceECInstanceId=?");
    stmt.BindId(1, el.GetElementId());
    while (BE_SQLITE_ROW == stmt.Step())
        {
        auto rl = el.GetDgnDb().Elements().Get<RepositoryLink>(stmt.GetValueId<DgnElementId>(0));
        if (rl.IsValid())
            {
            printf (" ---originates-from--> ");
            dumpElement(*rl);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  03/17
//---------------------------------------------------------------------------------------
static void dumpParentAndChildren(DgnElementCR el, int indent)
    {
    auto in = getIndent(indent);

    printf("%s", in.c_str());

    dumpElement(el);

    auto subj = dynamic_cast<Subject const*>(&el);
    if (nullptr != subj)
        printf(" %s", Json::FastWriter::ToString(subj->GetSubjectJsonProperties()).c_str());

    dumpPartitionRepositoryLink(el);

    puts("");

    auto childids = el.QueryChildren();
    for (auto childid : childids)
        {
        auto child = el.GetDgnDb().Elements().GetElement(childid);
        if (child.IsValid())
            dumpParentAndChildren(*child, indent+1);
        else
            printf("%s   %llx - Missing child??!!\n", in.c_str(), childid.GetValueUnchecked());
        }
    }

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::_OnConversionComplete()
    {
    if (!m_onConversionStartCalled)
        {
        LOG.error ("_OnConversionComplete called without _OnConversionStart");
        BeAssert(false && "_OnConversionComplete called without _OnConversionStart");
        return;
        }
    GetChangeDetector()._Cleanup(*this);

#if defined (BENTLEYCONFIG_PARASOLID)
    // NOTE: Terminate V8 frustum before trying to use DgnDb to render thumbnails...
    if (DgnV8Api::PSolidKernelManager::IsSessionStarted())
        DgnV8Api::PSolidKernelManager::StopSession();

    DgnDbApi::PSolidKernelManager::SetExternalFrustrum(false);
#endif

    if (!IsUpdating())
        OnCreateComplete();
    else
        OnUpdateComplete();

    if (!m_config.GetInstanceFilename().empty() && m_config.GetOptionValueBool("EmbedConfigFile", false))
        {
        BeSQLite::DbResult result;
        m_dgndb->EmbeddedFiles().Import(Utf8String(m_config.GetInstanceFilename().GetFileNameWithoutExtension().c_str()).c_str(), m_config.GetInstanceFilename().GetNameUtf8().c_str(), "xml");
    }

#ifdef WIP_DUMP
    dumpParentAndChildren(*GetDgnDb().Elements().GetRootSubject(), 0);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/16
+---------------+---------s------+---------------+---------------+---------------+------*/
void Converter::CopyExpirationDate(DgnV8FileR dgnfile)
    {
    DgnV8Api::DgnFileLicense lic;
    Bentley::dgnFileObj_getCurrentLicense(&lic, &dgnfile);

    if (lic.expiry == 0)
        return;

    DateTime expirationDate;
    DateTime::FromUnixMilliseconds(expirationDate, (UInt64)lic.expiry);
    if (expirationDate.IsValid())
        GetDgnDb().SaveExpirationDate(expirationDate);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr Converter::GetJobHierarchySubject()
    {
    auto const& jobsubj = _GetJobSubject();
    auto childids = jobsubj.QueryChildren();
    for (auto childid : childids)
        {
        auto subj = GetDgnDb().Elements().Get<Subject>(childid);
        if (subj.IsValid() && subj->GetSubjectJsonProperties(Subject::json_Model()).GetMember("Type") == "Hierarchy")
            return subj;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ValidateJob()
    {
    LOG.trace ("Converter::ValidateJob");

    auto const& jobsubj = _GetJobSubject();
    if (!jobsubj.GetElementId().IsValid())
        {
        BeAssert(false && "job subject must be persistent in the BIM");
        LOG.error ("Job subject must be persistent in the BIM.");
        _OnFatalError();
        return;
        }
    auto jchildids = jobsubj.QueryChildren();
    auto hcount = 0;
    for (auto jchildid : jchildids)
        {
        auto subj = GetDgnDb().Elements().Get<Subject>(jchildid);
        if (subj.IsValid() && subj->GetSubjectJsonProperties(Subject::json_Model()).GetMember("Type") == "Hierarchy")
            ++hcount;
        }
    if (hcount != 1)
        {
        BeAssert(false && "there should be exactly 1 job hierarchy subject under the job subject");
        LOG.error ("There should be exactly 1 job hierarchy subject under the job subject.");
        _OnFatalError();
        return;
        }

    auto hchildids = jobsubj.QueryChildren();
    auto rcount = 0;
    for (auto hchildid : hchildids)
        {
        auto subj = GetDgnDb().Elements().Get<Subject>(hchildid);
        if (subj.IsValid() && subj->GetSubjectJsonProperties(Subject::json_Model()).GetMember("Type") == "References")
            ++rcount;
        }
    if ((rcount != 0) && (rcount != 1))
        {
        BeAssert(false && "there should be 0 or 1 references subject under the hierarchy subject");
        LOG.error ("There should be 0 or 1 references subject under the hierarchy subject.");
        _OnFatalError();
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mayuresh.Kanade                 04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void getOutlierElementInfo (const bvector<BeInt64Id>& elementOutliers, DgnDbCP db, Utf8String& message, Converter* converter)
{
    for (auto eid : elementOutliers)
    {
        auto outlierElement = db->Elements ().GetElement ((DgnElementId)eid.GetValue ());
        if (outlierElement.IsValid ())
        {
            BentleyApi::BeSQLite::EC::ECSqlStatement estmt;
            auto status = estmt.Prepare (*db, "SELECT Identifier FROM " BIS_SCHEMA (BIS_CLASS_ExternalSourceAspect) " AS xsa WHERE (xsa.Element.Id=?)");

            estmt.BindInt64 (1, eid.GetValue ());
            int64_t v8ElementId = 0;

            if (BentleyApi::BeSQLite::BE_SQLITE_ROW == estmt.Step ())
                v8ElementId = estmt.GetValueInt64 (0);

            auto v8ModelInfo = std::get <1> (SyncInfo::V8ModelExternalSourceAspect::GetAspect (*outlierElement->GetModel ()));
            Utf8String sourceModelName;
            Utf8String sourceFileName;
            if (v8ModelInfo.IsValid())
                {
                sourceModelName = v8ModelInfo.GetV8ModelName();
                RepositoryLinkId id = v8ModelInfo.GetRepositoryLinkId();
                auto repositoryLinkElement = converter->GetRepositoryLinkElement(id);
                auto v8FileInfo = SyncInfo::RepositoryLinkExternalSourceAspect::GetAspect(*repositoryLinkElement);
                sourceFileName = v8FileInfo.GetFileName();
                }
            else
                {
                sourceModelName = "<unknown>";
                sourceFileName = "<unknown>";
                }
            message.Sprintf(" Element Id: %lld, Model Name: %s, File Name: %s", v8ElementId, sourceModelName.c_str(), sourceFileName.c_str());
        }
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::OnCreateComplete()
    {
    _EmbedFonts();

    size_t      outlierCount;
    DRange3d    rangeWithOutliers;
    bvector<BeInt64Id> elementOutliers;
    m_dgndb->GeoLocation().InitializeProjectExtents(&rangeWithOutliers, &elementOutliers);

    if (elementOutliers.size () > 0)
        {
        Utf8String message;
        getOutlierElementInfo (elementOutliers, m_dgndb.get (), message, this);
        ReportAdjustedProjectExtents (elementOutliers.size (), m_dgndb->GeoLocation ().GetProjectExtents (), rangeWithOutliers, message);
        }

    // *** NEEDS WORK: What is this for? m_rootScaleFactor is never set anywhere in the converter
    //m_dgndb->SaveProperty(PropertySpec("SourceRootScaleFactor", "dgn_Proj"), &m_rootScaleFactor, sizeof(m_rootScaleFactor));

    if (m_defaultViewId.IsValid())
        {
        // check and add the DefaultView ID if not found in DB:
        PropertySpec    prop = DgnViewProperty::DefaultView ();
        DgnElementId    existingId;
        if (m_dgndb->QueryProperty(&existingId, sizeof(existingId), prop) != DbResult::BE_SQLITE_OK || !existingId.IsValid())
            m_dgndb->SaveProperty (prop, &m_defaultViewId, sizeof(m_defaultViewId));
        }
    // else
    //  ensureAUserView

    StopWatch timer(true);
    GenerateThumbnailsWithExceptionHandling();
    ConverterLogging::LogPerformance(timer, "Creating thumbnails");

    timer.Start();
    GenerateRealityModelTilesets();
    //GenerateWebMercatorModel();
    ConverterLogging::LogPerformance(timer, "Creating reality model tilesets");

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::OnUpdateComplete()
    {
    // *** WIP_UPDATER - update thumbnails for views with modified models
    if (m_elementsConverted != 0)
        GenerateThumbnailsWithExceptionHandling();
    if (m_modelsRequiringRealityTiles.size() != 0)
        GenerateRealityModelTilesets();

    auto extents = m_dgndb->GeoLocation().GetProjectExtents();

    size_t      outlierCount;
    DRange3d    rangeWithOutliers;

    bvector<BeInt64Id> elementOutliers;
    auto calculated = m_dgndb->GeoLocation().ComputeProjectExtents(&rangeWithOutliers, &elementOutliers);

    if (!extents.IsEqual(calculated))
        {
        m_dgndb->GeoLocation().SetProjectExtents(calculated);
        if (elementOutliers.size () > 0)
            {
            Utf8String message;
            getOutlierElementInfo (elementOutliers, m_dgndb.get (), message, this);
            ReportAdjustedProjectExtents (elementOutliers.size (), m_dgndb->GeoLocation ().GetProjectExtents (), rangeWithOutliers, message);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::_OnElementConverted(DgnElementId elementId, DgnV8EhCP v8eh, ChangeOperation changeOperation)
    {
    ++m_elementsConverted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::_OnElementBeforeDelete(DgnElementId elementId)
    {
    if (nullptr != m_linkConverter)
        m_linkConverter->RemoveLinksOnElement(elementId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ReportDgnFileStatus(DbResult fileStatus, BeFileNameCR projectFileName)
    {
    auto category = Converter::IssueCategory::DiskIO();
    auto issue = Converter::Issue::Error();
    switch (fileStatus)
        {
        case BE_SQLITE_ERROR_FileNotFound:
            issue = Converter::Issue::FileNotFound();
            break;


        case BE_SQLITE_CORRUPT:
        case BE_SQLITE_ERROR_NoPropertyTable:
            issue = Converter::Issue::NotADgnDb();
            break;

        case BE_SQLITE_ERROR_InvalidProfileVersion:
        case BE_SQLITE_ERROR_ProfileUpgradeFailed:
        case BE_SQLITE_ERROR_ProfileTooOld:
            category = Converter::IssueCategory::Compatibility();
            issue = Converter::Issue::Error();
            break;
        }

    ReportError(category, issue, projectFileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Converter::AttachSyncInfo()
    {

    if (BentleyApi::SUCCESS != m_syncInfo.Initialize(GetDgnDb()))
        {
        ReportIssue(Converter::IssueSeverity::Fatal, Converter::IssueCategory::Sync(), Converter::Issue::CantOpenSyncInfo(), "");
        return OnFatalError();
        }

    m_dgndb->SaveChanges();
    BeAssert(!WasAborted());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::GetOrCreateJobPartitions()
    {
    InitGroupModel();
    InitDrawingListModel();
    InitSheetListModel();

    InitUncategorizedCategory(); // Create the "Default" category for elements that can't be categorized without application/domain help
    InitUncategorizedDrawingCategory();
    InitBusinessKeyCodeSpec();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::_OnConversionStart()
    {
    if (m_onConversionStartCalled)
        {
        BeAssert(false && "Call _OnConversionStart only once");
        return;
        }
    m_onConversionStartCalled = true;

    BeAssert(_GetParams().GetAssetsDir().DoesPathExist());

    SetV8ProgressMeter();

    ParseLevelConfigAndUpdateParams(m_config);

    GetChangeDetector()._Prepare(*this);

    InitLinkConverter();

    m_dgndb->AddIssueListener(m_issueReporter.GetECDbIssueListener());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Ramanujam.Raman                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::InitLinkConverter()
    {
    if (m_linkConverter)
        delete m_linkConverter;

    m_linkConverter = new LinkConverter(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Converter::InitGroupModel()
    {
    DgnCode partitionCode = GroupInformationPartition::CreateCode(GetJobSubject(), SubjectConvertedGroupsCode);
    DgnElementId partitionId = m_dgndb->Elements().QueryElementIdByCode(partitionCode);
    m_groupModelId = DgnModelId(partitionId.GetValueUnchecked());

    if (m_groupModelId.IsValid())
        return BentleyStatus::SUCCESS;

    GroupInformationPartitionPtr ed = GroupInformationPartition::Create(GetJobSubject(), partitionCode.GetValueUtf8CP());
    GroupInformationPartitionCPtr partition = ed->InsertT<GroupInformationPartition>();
    if (!partition.IsValid())
        return BentleyStatus::ERROR;

    GenericGroupModelPtr groupModel = GenericGroupModel::CreateAndInsert(*partition);
    if (!groupModel.IsValid())
        return BentleyStatus::ERROR;

    m_groupModelId = groupModel->GetModelId();
    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Converter::InitDrawingListModel()
    {
    DgnCode partitionCode = GroupInformationPartition::CreateCode(GetJobSubject(), SubjectConvertedDrawingsCode);
    DgnElementId partitionId = m_dgndb->Elements().QueryElementIdByCode(partitionCode);
    m_drawingListModelId = DgnModelId(partitionId.GetValueUnchecked());

    if (m_drawingListModelId.IsValid())
        return BentleyStatus::SUCCESS;

    DocumentPartitionPtr ed = DocumentPartition::Create(GetJobSubject(), partitionCode.GetValueUtf8CP());
    DocumentPartitionCPtr partition = ed->InsertT<DocumentPartition>();
    if (!partition.IsValid())
        return BentleyStatus::ERROR;

    DocumentListModelPtr drawingListModel = DocumentListModel::CreateAndInsert(*partition);
    if (!drawingListModel.IsValid())
        return BentleyStatus::ERROR;

    m_drawingListModelId = drawingListModel->GetModelId();
    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Converter::InitSheetListModel()
    {
    DgnCode partitionCode = GroupInformationPartition::CreateCode(GetJobSubject(), SubjectConvertedSheetsCode);
    DgnElementId partitionId = m_dgndb->Elements().QueryElementIdByCode(partitionCode);
    m_sheetListModelId = DgnModelId(partitionId.GetValueUnchecked());

    if (m_sheetListModelId.IsValid())
        return BentleyStatus::SUCCESS;

    DocumentPartitionPtr ed = DocumentPartition::Create(GetJobSubject(), partitionCode.GetValueUtf8CP());
    DocumentPartitionCPtr partition = ed->InsertT<DocumentPartition>();
    if (!partition.IsValid())
        return BentleyStatus::ERROR;

    DocumentListModelPtr sheetListModel = DocumentListModel::CreateAndInsert(*partition);
    if (!sheetListModel.IsValid())
        return BentleyStatus::ERROR;

    m_sheetListModelId = sheetListModel->GetModelId();
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            12/2017
//---------------+---------------+---------------+---------------+---------------+-------
DefinitionModelPtr Converter::GetJobDefinitionModel()
    {
    if (m_jobDefinitionModelId.IsValid())
        return m_dgndb->Models().Get<DefinitionModel>(m_jobDefinitionModelId);

    SubjectCR job = GetJobSubject();
    DgnCode partitionCode = DefinitionPartition::CreateCode(job, SubjectDefinitionsCode);
    DgnElementId partitionId = m_dgndb->Elements().QueryElementIdByCode(partitionCode);
    m_jobDefinitionModelId = DgnModelId(partitionId.GetValueUnchecked());
    if (m_jobDefinitionModelId.IsValid())
        return m_dgndb->Models().Get<DefinitionModel>(m_jobDefinitionModelId);

    DefinitionPartitionPtr ed = DefinitionPartition::Create(job, partitionCode.GetValueUtf8CP());
    DefinitionPartitionCPtr partition = ed->InsertT<DefinitionPartition>();
    if (!partition.IsValid())
        return DefinitionModelPtr();

    DefinitionModelPtr defModel = DefinitionModel::CreateAndInsert(*partition);
    if (!defModel.IsValid())
        return DefinitionModelPtr();

    m_jobDefinitionModelId = defModel->GetModelId();
    return defModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isChildOfJob(DgnDbR db, DgnElementId elId, DgnElementId jobSubjectId)
    {
    if (elId == db.Elements().GetRootSubjectId())
        return false;
    auto el = db.Elements().GetElement(elId);
    if (!el.IsValid())
        return false;
    auto thisParent = el->GetParentId();
    if (!thisParent.IsValid())
        return isChildOfJob(db, el->GetModel()->GetModeledElementId(), jobSubjectId);
    if (thisParent == jobSubjectId)
        return true;
    return isChildOfJob(db, thisParent, jobSubjectId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::IsBimModelAssignedToJobSubject(DgnModelId mid) const
    {
    auto  model = m_dgndb->Models().GetModel(mid);
    if (!model.IsValid())
        return false;
    return isChildOfJob(*m_dgndb, model->GetModeledElementId(), GetJobSubject().GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::IsV8ModelAssignedToJobSubject(DgnV8ModelCR v8Model) const
    {
    auto existingModelId = FindFirstModelInSyncInfo(v8Model);
    if (!existingModelId.IsValid())
        return true;
    return IsBimModelAssignedToJobSubject(existingModelId);
    }

static const Utf8CP s_codeSpecName = "DgnV8"; // TBD: One CodeSpec per V8 file?

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::InitBusinessKeyCodeSpec()
    {
    m_businessKeyCodeSpecId = m_dgndb->CodeSpecs().QueryCodeSpecId(s_codeSpecName);
    if (!m_businessKeyCodeSpecId.IsValid())
        {
        CodeSpecPtr codeSpec = CodeSpec::Create(*m_dgndb, s_codeSpecName);
        BeAssert(codeSpec.IsValid());
        if (codeSpec.IsValid())
            {
            codeSpec->Insert();
            m_businessKeyCodeSpecId = codeSpec->GetCodeSpecId();
            }
        }

    BeAssert(m_businessKeyCodeSpecId.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Converter::CreateCode(Utf8StringCR value) const
    {
    auto codeSpec = m_dgndb->CodeSpecs().GetCodeSpec(m_businessKeyCodeSpecId);
    BeDataAssert(codeSpec.IsValid());
    return codeSpec.IsValid() ? codeSpec->CreateCode(_GetSpatialParentSubject(), value) : DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Converter::CreateDebuggingCode(DgnV8EhCR v8eh)
    {
    return WantDebugCodes() ? CreateCode(Utf8PrintfString("%sDgnV8-%lld-%lld", _GetNamePrefix().c_str(), GetRepositoryLinkId(*v8eh.GetDgnFileP()).GetValueUnchecked(), v8eh.GetElementId())) : DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                                   Krischan.Eberle   10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Converter::GetECContentOfElement(V8ElementECContent& content, DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm, bool isNewElement)
    {
    //gather primary and secondary instances first (and hold them as Ptrs to ensure lifetime after iterated instances are out of scope
    //primary instances need to processed before secondary instances
    content.m_primaryV8Instance = nullptr;
    content.m_v8ElementType = V8ElementTypeHelper::GetType(v8eh);
    content.m_elementConversionRule = BisConversionRule::ToDefaultBisClass;
    RepositoryLinkId v8FileId = GetRepositoryLinkId(*v8eh.GetDgnModelP()->GetDgnFileP());
    content.m_namedGroupsWithOwnershipHintPerFile = nullptr;
    V8NamedGroupInfo::TryGetNamedGroupsWithOwnershipHint(content.m_namedGroupsWithOwnershipHintPerFile, v8FileId);

    DgnModelR targetModel = v8mm.GetDgnModel();

    if (!m_skipECContent)
        {
        DgnV8Api::FindInstancesScopePtr scope = CreateFindInstancesScope(v8eh);
        for (DgnV8Api::DgnECInstance* v8Instance : DgnV8Api::DgnECManager::GetManager().FindInstances(*scope, GetSelectAllV8ECQuery()))
            {
            ECClassName v8ClassName(v8Instance->GetClass());
            if (0 == BeStringUtilities::Strnicmp("EWR", v8ClassName.GetSchemaName(), 3))
                v8ClassName = ECClassName("EWR", v8ClassName.GetClassName());

            BisConversionRule conversionRule;
            bool hasSecondary;
            if (!V8ECClassInfo::TryFind(conversionRule, hasSecondary, GetDgnDb(), v8ClassName.GetClassFullName()))
                {
                continue;
                }

            if (BisConversionRuleHelper::IgnoreInstance(conversionRule))
                {
                DgnV8ModelCP v8Model = v8eh.GetDgnModelP();
                Utf8String warning;
                warning.Sprintf("Skipped v8 %s ECInstance (%s) on v8 Element '%s'in v8 model '%s' because its class was ignored during schema conversion. See ECSchema conversion log entries above.",
                                v8ClassName.GetClassFullName().c_str(), Utf8String(v8Instance->GetInstanceId().c_str()).c_str(),
                                IssueReporter::FmtElement(v8eh).c_str(),
                                v8Model != nullptr ? IssueReporter::FmtModel(*v8Model).c_str() : "nullptr");

                ReportIssue(IssueSeverity::Warning, IssueCategory::Sync(), Issue::Message(), warning.c_str());
                continue;
                }

            // In the situation where a class can be used as both a primary instance and a secondary instance, the conversionRule is going to come back is the Element rule and not ToAspectOnly.
            // Therefore, if we already have a primary instance set and the hasSecondary flag indicates that this ecClass also has ToAspect rule, then we just set it as a secondaryInstance.

            if (BisConversionRuleHelper::IsSecondaryInstance(conversionRule) || (hasSecondary && !DgnV8Api::DgnECManager::GetManager().GetPrimaryInstanceId(v8eh, false, true).Equals(v8Instance->GetInstanceId())))
            //if (BisConversionRuleHelper::IsSecondaryInstance(conversionRule) || (hasSecondary && content.HasPrimaryInstance()))
                {
                content.m_secondaryV8Instances.push_back(std::make_pair(v8Instance, BisConversionRule::ToAspectOnly));
                }
            else if (IsUpdating() && BisConversionRule::ToDefaultBisBaseClass == conversionRule)
                {
                // We must be trying to do an update, and we found a primary ECInstance from a class that had no instances when we first converted it.
                // If this is an existing element, then it needs to be added as a secondary instance as we cannot change the element's class.  If it is a new element, then it is added as the primary instance

                const bool namedGroupOwnsMembersFlag = content.m_namedGroupsWithOwnershipHintPerFile != nullptr && content.m_namedGroupsWithOwnershipHintPerFile->find(v8eh.GetElementId()) != content.m_namedGroupsWithOwnershipHintPerFile->end();
                BisConversionTargetModelInfo targetModelInfo(targetModel);
                BisConversionRule rule = BisConversionRuleHelper::ConvertToBisConversionRule(content.m_v8ElementType, targetModelInfo, namedGroupOwnsMembersFlag, !isNewElement);
                if (isNewElement)
                    {
                    ECClassName previousECClass = BisConversionRuleHelper::GetElementBisBaseClassName(conversionRule);
                    ECClassName newECClass = BisConversionRuleHelper::GetElementBisBaseClassName(rule);
                    if (previousECClass != newECClass)
                        {
                        Utf8String msg;
                        DgnV8ModelCP v8Model = v8eh.GetDgnModelP();
                        msg.Sprintf("Skipped v8 ECInstance (%s) on v8 Element '%s' in v8 model '%s' because its base class has changed since the initial schema import.  Previous ECClass was '%s'.  It now requires '%s'.",
                                    Utf8String(v8Instance->GetInstanceId().c_str()).c_str(),
                                    IssueReporter::FmtElement(v8eh).c_str(),
                                    v8Model != nullptr ? IssueReporter::FmtModel(*v8Model).c_str() : "nullptr",
                                    previousECClass.GetClassFullName().c_str(), newECClass.GetClassFullName().c_str());
                        ReportIssueV(IssueSeverity::Error, IssueCategory::Unsupported(), Issue::Message(), nullptr, msg.c_str());
                        return BentleyApi::ERROR;
                        }
                    content.m_primaryV8Instance = v8Instance;
                    content.m_elementConversionRule = rule;
                    }
                else
                    content.m_secondaryV8Instances.push_back(std::make_pair(v8Instance, rule));
                }
            else
                {
                content.m_primaryV8Instance = v8Instance;
                content.m_elementConversionRule = conversionRule;
                }
            }
        }

    if (!content.HasPrimaryInstance())
        {
        bool namedGroupOwnsMembersFlag = false;
        if (content.m_namedGroupsWithOwnershipHintPerFile != nullptr)
            namedGroupOwnsMembersFlag = content.m_namedGroupsWithOwnershipHintPerFile->find(v8eh.GetElementId()) != content.m_namedGroupsWithOwnershipHintPerFile->end();
        BisConversionTargetModelInfo targetModelInfo(targetModel);
        content.m_elementConversionRule = BisConversionRuleHelper::ConvertToBisConversionRule(content.m_v8ElementType, targetModelInfo, namedGroupOwnsMembersFlag);
        if (content.m_elementConversionRule == BisConversionRule::ToPhysicalElement)
            content.m_elementConversionRule = BisConversionRule::ToPhysicalObject; // ToPhysicalElement can only be used if the element has an ECInstance.  Otherwise, it needs the non-abstract generic::PhysicalObject rule
        else if (content.m_elementConversionRule == BisConversionRule::ToGroup)
            content.m_elementConversionRule = BisConversionRule::ToGenericGroup; // ToGroup can only be used if the element has an ECInstance.  Otherwise, it needs the non-abstract Generic:Group class.
        }

    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson          03/17
//---------------------------------------------------------------------------------------
static bool wouldBe3dMismatch(ElementConversionResults const& results, ResolvedModelMapping const& v8mm)
    {
    if (!results.m_element.IsValid())
        return false;
    auto gs = results.m_element->ToGeometrySource();
    if (nullptr == gs)
        return false;
    return gs->Is3d() != v8mm.GetDgnModel().Is3dModel();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8ElementExternalSourceAspect  Converter::AddOrUpdateV8ElementExternalSourceAspect(DgnElementR el, SyncInfo::V8ElementExternalSourceAspectData const& elprov)
    {
    auto sourceId = !elprov.m_v8IdPath.empty()? elprov.m_v8IdPath: SyncInfo::V8ElementExternalSourceAspect::FormatSourceId(elprov.m_v8Id);
    SyncInfo::V8ElementExternalSourceAspect aspect = SyncInfo::V8ElementExternalSourceAspect::GetAspectForEdit(el, sourceId);
    if (aspect.IsValid())
        {
        BeAssert(!elprov.m_v8IdPath.empty() || aspect.GetV8ElementId() == elprov.m_v8Id);
        aspect.Update(elprov.m_prov);
        return SyncInfo::V8ElementExternalSourceAspect();
        }

    aspect = SyncInfo::V8ElementExternalSourceAspect::CreateAspect(elprov, GetDgnDb());
    if (aspect.AddAspect(el) != DgnDbStatus::Success)
        aspect.Invalidate();
    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8ElementExternalSourceAspect Converter::WriteV8ElementExternalSourceAspect(DgnElementId bimElementId, DgnV8EhCR v8eh, DgnModelId scope, Utf8StringCR sourceIdPathIn, Utf8StringCR propsJson)
    {
    auto el = GetDgnDb().Elements().GetElement(bimElementId)->CopyForEdit();
    SyncInfo::ElementProvenance newProv(v8eh, GetSyncInfo(), GetCurrentIdPolicy());
    Utf8String sourceIdPath = !sourceIdPathIn.empty()? sourceIdPathIn: SyncInfo::V8ElementExternalSourceAspect::FormatSourceId(v8eh);
    SyncInfo::V8ElementExternalSourceAspectData elprov(scope, sourceIdPath, newProv, propsJson);
    auto aspect = AddOrUpdateV8ElementExternalSourceAspect(*el, elprov);
    el->Update();
    return aspect;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------------------------------------------------------------------------------
BentleyStatus Converter::ConvertElement(ElementConversionResults& results, DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm, DgnCategoryId defaultCategoryId,
                                        bool ignoreElementWithoutECInstance, bool isNewElement)
    {
    V8ElementECContent ecContent;
    GetECContentOfElement(ecContent, v8eh, v8mm, isNewElement);

    bool hasPrimaryInstance = ecContent.m_primaryV8Instance != nullptr;
    bool hasSecondaryInstances = !ecContent.m_secondaryV8Instances.empty();

    DgnModelR targetModel = v8mm.GetDgnModel();

    //if element doesn't have any ECInstances and is a non-graphical element, just skip the element
    if (!hasPrimaryInstance && !hasSecondaryInstances && (ignoreElementWithoutECInstance || ecContent.m_v8ElementType == V8ElementType::NonGraphical))
        {
        LOG.tracev("Skipping element %s %s as it is either non-graphical or has no EC content", Converter::IssueReporter::FmtElement(v8eh).c_str(), Converter::IssueReporter::FmtModel(*v8eh.GetDgnModelP()).c_str());
        return BentleyApi::SUCCESS;
        }

    LOG.tracev("Converting element %s %s", Converter::IssueReporter::FmtElement(v8eh).c_str(), Converter::IssueReporter::FmtModel(*v8eh.GetDgnModelP()).c_str());
    ConvertToDgnDbElementExtension* upx = ConvertToDgnDbElementExtension::Cast(v8eh.GetHandler());
    if (nullptr != upx)
        {
        if (ConvertV8TextToDgnDbExtension::Result::SkipElement == upx->_PreConvertElement(v8eh, *this, v8mm))
            {
            return BSIERROR;
            }
        }
    for (auto xdomain : XDomainRegistry::s_xdomains)
        {
        if (XDomain::Result::SkipElement == xdomain->_PreConvertElement(v8eh, *this, v8mm))
            {
            return BSIERROR;
            }
        }

    DgnCategoryId categoryId;
    DgnClassId elementClassId;
    DgnCode elementCode;

    // Let a converter extension attempt to determine basic element parameters. Otherwise, fall back and try to figure something out ourselves.
    if (nullptr != upx)
        upx->_DetermineElementParams(elementClassId, elementCode, categoryId, v8eh, *this, ecContent.m_primaryV8Instance.get(), v8mm);

    for (auto xdomain : XDomainRegistry::s_xdomains)
        xdomain->_DetermineElementParams(elementClassId, elementCode, categoryId, v8eh, *this, ecContent.m_primaryV8Instance.get(), v8mm);

    if (!categoryId.IsValid())
        {
        categoryId = defaultCategoryId;
        if (!categoryId.IsValid())
            {
            Utf8String instanceStr = "";
            if (hasPrimaryInstance)
                {
                instanceStr.Sprintf("  -- Primary instance: %s (%s)", Utf8String(ecContent.m_primaryV8Instance->GetClass().GetName().c_str()).c_str(), Utf8String(ecContent.m_primaryV8Instance->GetInstanceId().c_str()).c_str());
                }
            ReportIssue(IssueSeverity::Warning, IssueCategory::CorruptData(), Issue::ConvertFailure(), Utf8PrintfString("[%s] - Invalid level %d%s - Using Default Uncategorized category instead.", IssueReporter::FmtElement(v8eh).c_str(), v8eh.GetElementCP()->ehdr.level, instanceStr.c_str()).c_str());
            categoryId = m_uncategorizedCategoryId;
            }
        }

    if (!elementClassId.IsValid())
        {
        elementClassId = _ComputeElementClass(v8eh, ecContent, v8mm);
        if (!elementClassId.IsValid())
            {
            BeAssert(false);
            LOG.debugv("Failed to compute element class for %s %s", Converter::IssueReporter::FmtElement(v8eh).c_str(), Converter::IssueReporter::FmtModel(*v8eh.GetDgnModelP()).c_str());
            return BSIERROR;
            }
        }

    if (!elementCode.IsValid())
        elementCode = _ComputeElementCode(v8eh, ecContent);

    if (ecContent.m_v8ElementType == V8ElementType::Graphical)
        {
        if (BentleyApi::SUCCESS != _CreateElementAndGeom(results, v8mm, elementClassId, hasPrimaryInstance, categoryId, elementCode, v8eh))
            {
            LOG.debugv("Failed to create element and geom for %s %s", Converter::IssueReporter::FmtElement(v8eh).c_str(), Converter::IssueReporter::FmtModel(*v8eh.GetDgnModelP()).c_str());
            return BSIERROR;
            }
        if (wouldBe3dMismatch(results, v8mm))
            {
            ReportIssue(IssueSeverity::Info, IssueCategory::Unsupported(), Issue::UnsupportedPrimaryInstance(), IssueReporter::FmtElement(v8eh).c_str());
            elementClassId = ComputeElementClassIgnoringEcContent(v8eh, v8mm);
            auto was = m_skipECContent;
            m_skipECContent= true;
            results.m_element = nullptr;
            results.m_v8PrimaryInstance = V8ECInstanceKey();
            hasPrimaryInstance = false;
            auto res = _CreateElementAndGeom(results, v8mm, elementClassId, hasPrimaryInstance, categoryId, elementCode, v8eh);
            m_skipECContent = was;
            ecContent.m_secondaryV8Instances.push_back(std::make_pair(ecContent.m_primaryV8Instance, BisConversionRule::ToAspectOnly));
            hasSecondaryInstances = true;
            if (BentleyApi::SUCCESS != res)
                {
                LOG.debugv("Failed to create element and geom after 3d mismatch for %s %s", Converter::IssueReporter::FmtElement(v8eh).c_str(), Converter::IssueReporter::FmtModel(*v8eh.GetDgnModelP()).c_str());
                return BSIERROR;
                }
            }
        }
    else
        {
        //for non-graphical elements, we create the Element right here
        ElementHandlerP elementHandler = dgn_ElementHandler::Element::FindHandler(GetDgnDb(), elementClassId);
        if (nullptr == elementHandler)
            {
            BeAssert(false);
            LOG.debugv("Failed to find element handler for %s %s", Converter::IssueReporter::FmtElement(v8eh).c_str(), Converter::IssueReporter::FmtModel(*v8eh.GetDgnModelP()).c_str());
            return BSIERROR;
            }

        DgnModelId targetModelId = targetModel.GetModelId();
        if (V8ElementType::NamedGroup == ecContent.m_v8ElementType)
            targetModelId = GetTargetModelForNamedGroup(v8eh, v8mm, elementClassId);

        results.m_element = elementHandler->Create(DgnElement::CreateParams(GetDgnDb(), targetModelId, elementClassId, elementCode));
        if (!results.m_element.IsValid())
            {
            LOG.debugv("Failed to create element from element handler for %s %s", Converter::IssueReporter::FmtElement(v8eh).c_str(), Converter::IssueReporter::FmtModel(*v8eh.GetDgnModelP()).c_str());
            return BSIERROR;
            }

        // Apparently we convert non-graphical V8 elements into physical DgnDb elements (with no geometry).
        // This occurs when an ECClass is attached to both graphical and non-graphical elems in v8.
        auto geomEl = results.m_element->ToGeometrySourceP();
        if (nullptr != geomEl)
            geomEl->SetCategoryId(categoryId);
        }

    // Code above created element(s) matching the class/code/category that a converter extension specified; let it fill in its own element, either keeping or replacing the proxy graphics we generated above.
    if (nullptr != upx)
        upx->_ProcessResults(results, v8eh, v8mm, *this);

    for (auto xdomain : XDomainRegistry::s_xdomains)
        xdomain->_ProcessResults(results, v8eh, v8mm, *this);

    BeAssert(results.m_element.IsValid());// At this point results.m_element is expected to be valid

    if (hasPrimaryInstance)
        {
        if (nullptr == m_elementConverter)
            m_elementConverter = new ElementConverter(*this);
        m_elementConverter->ConvertToElementItem(results, ecContent.m_primaryV8Instance.get(), &ecContent.m_elementConversionRule);

        Bentley::WString displayLabel;
        ecContent.m_primaryV8Instance->GetDisplayLabel(displayLabel);
        results.m_element->SetUserLabel(Utf8String(displayLabel.c_str()).c_str());

        if (ecContent.m_v8ElementType == V8ElementType::NamedGroup)
            OnNamedGroupConverted(v8eh, results.m_element->GetElementClassId());
        rapidjson::Document propsData(rapidjson::kObjectType);
        if (!results.m_jsonProps.empty())
            propsData.Parse(results.m_jsonProps.c_str());
        propsData.AddMember("PrimaryInstance", rapidjson::Value(Utf8String(ecContent.m_primaryV8Instance->GetInstanceId().c_str()).c_str(), propsData.GetAllocator()), propsData.GetAllocator());
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        propsData.Accept(writer);
        results.m_jsonProps = buffer.GetString();
        }
    //item or aspects only if there was an ECInstance on the element at all
    if (hasSecondaryInstances)
        {
        if (nullptr == m_elementAspectConverter)
            m_elementAspectConverter = new ElementAspectConverter(*this);

        IChangeDetector::SearchResults changeInfo;
        if (!isNewElement)
            GetChangeDetector()._IsElementChanged(changeInfo, *this, v8eh, v8mm);

        if (BentleyApi::SUCCESS != m_elementAspectConverter->ConvertToAspects(&changeInfo.m_v8ElementAspect, results, ecContent.m_secondaryV8Instances, isNewElement))
            {
            LOG.errorv("Failed to create elementaspect for %s %s", Converter::IssueReporter::FmtElement(v8eh).c_str(), Converter::IssueReporter::FmtModel(*v8eh.GetDgnModelP()).c_str());
            return BSIERROR;
            }
        }

    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------+---------------+---------------+---------------+---------------+-------
DgnClassId Converter::_ComputeElementClass(DgnV8EhCR v8eh, V8ElementECContent const& ecContent, ResolvedModelMapping const& v8mm)
    {
    ECClassName elementClassName;
    if (ecContent.HasPrimaryInstance())
        elementClassName = ECClassName(ecContent.m_primaryV8Instance->GetClass());
    else
        elementClassName = BisConversionRuleHelper::GetElementBisBaseClassName(ecContent.m_elementConversionRule);

    if (0 == BeStringUtilities::Strnicmp(elementClassName.GetSchemaName(), "EWR", 3))
        elementClassName = ECClassName("EWR", elementClassName.GetClassName());

    ECN::ECClassId classId = m_dgndb->Schemas().GetClassId(elementClassName.GetSchemaName(), elementClassName.GetClassName());
    return DgnClassId(classId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      1/17
//---------------+---------------+---------------+---------------+---------------+-------
DgnClassId Converter::ComputeElementClassIgnoringEcContent(DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm)
    {
    V8ElementECContent ignoreEcContent;
    ignoreEcContent.m_v8ElementType = V8ElementTypeHelper::GetType(v8eh);
    ignoreEcContent.m_elementConversionRule = v8mm.GetDgnModel().Is3dModel() ? BisConversionRule::ToPhysicalObject : BisConversionRule::ToDrawingGraphic;
    return _ComputeElementClass(v8eh, ignoreEcContent, v8mm);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------+---------------+---------------+---------------+---------------+-------
DgnCode Converter::_ComputeElementCode(DgnV8EhCR v8eh, V8ElementECContent const& ecContent)
    {
    DgnCode elementCode;

    //if primary instance has a business key, retrieve it and use it as Element code
    if (ecContent.HasPrimaryInstance())
        elementCode = TryGetBusinessKey(*ecContent.m_primaryV8Instance);

    if (!elementCode.IsValid())
        elementCode = CreateDebuggingCode(v8eh);

    return elementCode;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------+---------------+---------------+---------------+---------------+-------
void Converter::OnNamedGroupConverted(DgnV8EhCR v8eh, DgnClassId elementClassId)
    {
    // check whether this is a named group with an ownership hint. In that case we will later create a dedicated
    // relationship between the elements of the group members
    BECN::ECClassCP elementClass = GetDgnDb().Schemas().GetClass(ECN::ECClassId(elementClassId.GetValue()));
    bool namedGroupHasOwnershipFlag = DetermineNamedGroupOwnershipFlag(*elementClass);
    if (namedGroupHasOwnershipFlag)
        V8NamedGroupInfo::AddNamedGroupWithOwnershipHint(v8eh, *this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------+---------------+---------------+---------------+---------------+-------
DgnModelId Converter::GetTargetModelForNamedGroup(DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm, DgnClassId elementClassId)
    {
    DgnModelR targetModel = v8mm.GetDgnModel();
    DgnModelId targetModelId = targetModel.GetModelId();

    BECN::ECClassCP elementClass = GetDgnDb().Schemas().GetClass(ECN::ECClassId(elementClassId.GetValue()));
    bool namedGroupHasOwnershipFlag = DetermineNamedGroupOwnershipFlag(*elementClass);
    if (!namedGroupHasOwnershipFlag)
        return m_groupModelId;

    // if the V8NamedGroup is in the dictionary model we need to change that to the first child's model instead
    if (DgnModel::DictionaryId() == targetModelId)
        return FindModelIdForNamedGroup(v8eh);

    return targetModelId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnModelId Converter::FindModelIdForNamedGroup(DgnV8EhCR v8eh)
    {
    DgnModelId modelId;
    DgnV8Api::NamedGroupPtr ng = nullptr;
    DgnV8Api::DgnModel* defaultModel = v8eh.GetDgnModelP()->GetDgnFileP()->FindLoadedModelById(v8eh.GetDgnModelP()->GetDgnFileP()->GetDefaultModelId());

    if (DgnV8Api::NamedGroupStatus::NG_Success != DgnV8Api::NamedGroup::Create(ng, v8eh, defaultModel))
        return modelId;

    uint32_t graphicCount;
    uint32_t groupMembers;
    ng->GetMemberCount(&graphicCount, &groupMembers);
    for (uint32_t i = 0; i< graphicCount; i++)
        {
        DgnV8Api::NamedGroupMember* child = ng->GetMember(i);
        DgnV8Api::ElementHandle memberEh(child->GetElementRef());
        DgnElementId childElementId;
        if (!TryFindElement(childElementId, memberEh))
            continue;

        DgnElementCPtr childElement = GetDgnDb().Elements().GetElement(childElementId);
        if (!childElement.IsValid())
            continue;
        return childElement->GetModelId();
        }

    DgnV8Api::NamedGroup::ElementRefList groups;
    ng->GetContainedGroups(groups);
    for (uint32_t i = 0; i < groupMembers; i++)
        {
        DgnV8Api::ElementHandle containedGroup(groups[i]);
        DgnModelId subId = FindModelIdForNamedGroup(containedGroup);
        if (subId.IsValid())
            return subId;
        }
    return modelId;

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
DgnCode Converter::TryGetBusinessKey(ECObjectsV8::IECInstanceCR v8ECInstance)
    {
    //look up business key ca
    ECObjectsV8::IECInstancePtr caInstance = v8ECInstance.GetClass().GetCustomAttribute(L"BusinessKeySpecification");
    if (caInstance == nullptr)
        return DgnCode();

    ECObjectsV8::ECValue bkPropNameV;
    if (ECObjectsV8::ECOBJECTS_STATUS_Success != caInstance->GetValue(bkPropNameV, L"PropertyName") && bkPropNameV.IsNull())
        return DgnCode();

    //look up value of property marked as business key prop
    ECObjectsV8::ECValue bkValue;
    if (v8ECInstance.GetValue(bkValue, bkPropNameV.GetString()) != ECObjectsV8::ECOBJECTS_STATUS_Success)
        return DgnCode();

    return bkValue.IsNull() ? DgnCode() : CreateCode(bkValue.GetUtf8CP());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
//static
bool Converter::DetermineNamedGroupOwnershipFlag(BECN::ECClassCR ecClass)
    {
    return !ecClass.Is(BIS_ECSCHEMA_NAME, BIS_CLASS_GroupInformationElement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------------------------------------------------------------------------------
DgnV8Api::ECQuery const& Converter::GetSelectAllV8ECQuery() const
    {
    if (m_selectAllQueryV8 == nullptr)
        m_selectAllQueryV8 = DgnV8Api::ECQuery::CreateQuery(DgnV8Api::ECQUERY_PROCESS_SearchAllExtrinsic);

    return *m_selectAllQueryV8;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------------------------------------------------------------------------------
DgnV8Api::FindInstancesScopePtr Converter::CreateFindInstancesScope(DgnV8EhCR v8Element)
    {
    return DgnV8Api::FindInstancesScope::CreateScope(v8Element, DgnV8Api::FindInstancesScopeOption());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::AnnounceConversionResults(ElementConversionResults& results, DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm,
                                                  IChangeDetector::SearchResults const& updatePlan, bool isParentElement)
    {
    if (!results.m_element.IsValid() || !results.m_element->GetElementId().IsValid())
        {
        ++m_elementsDiscarded;
        results.m_wasDiscarded = true;
        return;
        }

    DgnElementId elementId = results.m_element->GetElementId();

    DgnElementR element = *results.m_element;
    BeSQLite::EC::ECInstanceKey bisElementKey = element.GetECInstanceKey();
    RepositoryLinkId fileId = GetRepositoryLinkId(*v8eh.GetDgnFileP());
    if (results.m_v8PrimaryInstance.IsValid())
        ECInstanceInfo::Insert(GetDgnDb(), fileId, results.m_v8PrimaryInstance, bisElementKey, true);

    auto sourceId = SyncInfo::V8ElementExternalSourceAspect::FormatSourceId(v8eh.GetElementId());
    SyncInfo::V8ElementExternalSourceAspect aspect = SyncInfo::V8ElementExternalSourceAspect::GetAspectForEdit(element, sourceId);

    rapidjson::Document propData = aspect.GetProperties();
    auto& allocator = propData.GetAllocator();
    if (!results.m_v8SecondaryInstanceMappings.empty() && !propData.HasMember("SecondaryInstances"))
        {
        rapidjson::Value tmp;
        tmp.SetObject();
        propData.AddMember("SecondaryInstances", tmp, allocator);
        }

    for (bpair<V8ECInstanceKey,BECN::IECInstancePtr> const& v8SecondaryInstanceMapping : results.m_v8SecondaryInstanceMappings)
        {
        BECN::IECInstanceCR aspect = *v8SecondaryInstanceMapping.second;
        BeSQLite::EC::ECInstanceId aspectId;
        if (SUCCESS != BeSQLite::EC::ECInstanceId::FromString(aspectId, aspect.GetInstanceId().c_str()))
            {
            BeAssert(false && "Could not convert IECInstance's instance id to a BeSQLite::EC::ECInstanceId.");
            continue;
            }

        ECInstanceInfo::Insert(GetDgnDb(), fileId, v8SecondaryInstanceMapping.first, BeSQLite::EC::ECInstanceKey(aspect.GetClass().GetId(), aspectId), false);
        auto& secondary = propData["SecondaryInstances"];
        if (!secondary.HasMember(v8SecondaryInstanceMapping.first.GetInstanceId()))
            secondary.AddMember(rapidjson::StringRef(v8SecondaryInstanceMapping.first.GetInstanceId()), rapidjson::Value(aspectId.GetValue()), allocator);
        }

    for (ElementConversionResults& child : results.m_childElements)
        AnnounceConversionResults(child, v8eh, v8mm, updatePlan, false);

    ChangeOperation changeOperation = (IChangeDetector::ChangeType::Update == updatePlan.m_changeType)?
                                        ChangeOperation::Update : ChangeOperation::Create;

    if (isParentElement)
        m_linkConverter->ConvertLinksOnElement(&v8eh, elementId, changeOperation);

    if (!results.m_v8SecondaryInstanceMappings.empty())
        {
        aspect.SetProperties(propData);
        element.Update();
        }
    _OnElementConverted(elementId, &v8eh, changeOperation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Converter::InsertResults(ElementConversionResults& results, SyncInfo::V8ElementExternalSourceAspectData const& elprov)
    {
    if (!results.m_element.IsValid())
        return DgnDbStatus::Success;

    DgnDbStatus stat;
    DgnCode code = results.m_element->GetCode();

    results.m_mapping = AddOrUpdateV8ElementExternalSourceAspect(*results.m_element, elprov);

    auto result = m_dgndb->Elements().Insert(*results.m_element, &stat);

    if (DgnDbStatus::DuplicateCode == stat)
        {
        Utf8String duplicateMessage;
        duplicateMessage.Sprintf("Duplicate element code '%s' ignored", code.GetValueUtf8().c_str());
        ReportIssue(IssueSeverity::Info, IssueCategory::InconsistentData(), Issue::Message(), duplicateMessage.c_str());

        DgnDbStatus stat2 = results.m_element->SetCode(DgnCode::CreateEmpty()); // just leave the code null
        BeAssert(DgnDbStatus::Success == stat2);
        result = m_dgndb->Elements().Insert(*results.m_element, &stat);
        }

    if (DgnDbStatus::Success != stat)
        {
        BeAssert((DgnDbStatus::LockNotHeld != stat) && "Failed to get or retain necessary locks");
        BeAssert(false);
        ReportIssue(IssueSeverity::Warning, IssueCategory::Unsupported(), Issue::ConvertFailure(), IssueReporter::FmtElement(*results.m_element).c_str());
        return stat;
        }

    results.m_element = result->CopyForEdit(); // Note that we don't plan to modify the result after this. We just
                                              // want the output to reflect the outcome. Since, we have a non-const
                                              // pointer, we have to make a copy.
    if (result.IsValid() && LOG_IS_SEVERITY_ENABLED(LOG_TRACE))
        LOG.tracev("Insert %s into %s", IssueReporter::FmtElement(*result).c_str(), IssueReporter::FmtModel(*results.m_element->GetModel()).c_str());

    _GetChangeDetector().OnElementSeen(*this, results.m_element.get());

    for (ElementConversionResults& child : results.m_childElements)
        {
        if (!child.m_element.IsValid())
            continue;
        child.m_element->SetParentId(results.m_element->GetElementId(), m_dgndb->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements));
        auto status = InsertResults(child, elprov);
        if (DgnDbStatus::Success != status)
            return status;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr Converter::MakeCopyForUpdate(DgnElementCR newEl, DgnElementCR originalEl)
    {
    // *** This copying is necessary only because originalEl has the ElementId and newEl does (or may) not.
    //      We can't set the elementid. We can only copy it.
    BeAssert(originalEl.GetElementId().IsValid());

    DgnElementPtr writeEl = originalEl.CopyForEdit();   // writeEl will have originalEl's ElementId
    writeEl->CopyFrom(newEl);                            // writeEl now gets newEl's data (other than ElementId)
    writeEl->CopyAppDataFrom(newEl);                     // writeEl also gets newEl's appdata

    // The Code may have changed. (Note: _CopyFrom zeroes out the original Code, so we have to assign it even if unchanged.)
    DgnCode code = originalEl.GetCode();
    DgnCode newCode = newEl.GetCode();
    if (newCode.IsValid() && newCode != code)
        code = newCode;
    writeEl->SetCode(code);

    return writeEl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Converter::UpdateResultsForOneElement(ElementConversionResults& conversionResults, DgnElementId existingElementId, SyncInfo::V8ElementExternalSourceAspectData const& elprov)
    {
    if (!conversionResults.m_element.IsValid() || !existingElementId.IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::BadArg;
        }

    DgnElementCPtr el = m_dgndb->Elements().GetElement(existingElementId);

    if (!el.IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::BadArg;
        }

    if (el->GetElementClassId() != conversionResults.m_element->GetElementClassId())
        {
        ReportIssueV(IssueSeverity::Error, IssueCategory::Unsupported(), Issue::UpdateDoesNotChangeClass(), nullptr,
            m_issueReporter.FmtElement(*el).c_str(), conversionResults.m_element->GetElementClass()->GetECSqlName().c_str());
        }

    DgnElementPtr writeEl = MakeCopyForUpdate(*conversionResults.m_element, *el);

    conversionResults.m_mapping = AddOrUpdateV8ElementExternalSourceAspect(*writeEl, elprov);

    DgnDbStatus stat;
    DgnElementCPtr result = m_dgndb->Elements().Update(*writeEl, &stat);
    if (!result.IsValid())
        return stat;

    conversionResults.m_element = result->CopyForEdit();// Note that we don't plan to modify the result after this. We just
                                                        // want the output to reflect the outcome. Since, we have a non-const
                                                        // pointer, we have to make a copy.

    _GetChangeDetector().OnElementSeen(*this, conversionResults.m_element.get());

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Converter::UpdateResultsForChildren(ElementConversionResults& parentConversionResults, SyncInfo::V8ElementExternalSourceAspectData const& parentelprov)
    {
    if (parentConversionResults.m_childElements.empty())
        return DgnDbStatus::Success;

    if (!parentConversionResults.m_element.IsValid() || !parentConversionResults.m_element->GetElementId().IsValid())
        {
        BeAssert(false && "input should be persistent parent element");
        return DgnDbStatus::BadArg;
        }
    DgnElementId parentId = parentConversionResults.m_element->GetElementId();

    // Make sure the parentid property is set on all of the children.
    for (auto& newChild : parentConversionResults.m_childElements)
        newChild.m_element->SetParentId(parentId, m_dgndb->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements));

    // While we could just delete all existing children and insert all new ones, we try to do better.
    // If we can figure out how the new children map to existing children, we can update them.

    // Note that in the update logic below we don't delete existing children that were not mapped.
    // Instead, we just refrain from calling the change detector's _OnElementSeen method on unmatched child elements.
    // That will allow the updater in its final phase to infer that they should be deleted.

    // The best way is if an extension sets up the DgnElementId of the child elements in parentConversionResults.
    auto const& firstChild = parentConversionResults.m_childElements.front();
    if (firstChild.m_element.IsValid() && firstChild.m_element->GetElementId().IsValid())
        {
        auto existingChildIdSet = parentConversionResults.m_element->QueryChildren();
        for (auto& childRes : parentConversionResults.m_childElements)
            {
            if (!childRes.m_element.IsValid())
                continue;
            auto existingChildElementId = childRes.m_element->GetElementId();
            auto iFound = existingChildIdSet.find(existingChildElementId);
            if (iFound != existingChildIdSet.end())
                {
                UpdateResults(childRes, existingChildElementId, parentelprov);
                // *** WIP_CONVERTER - bail out if any child update fails?
                }
            }
        return DgnDbStatus::Success;
        }

    // If we have to guess, we just try to match them up 1:1 in sequence.
    bvector<DgnElementId> existingChildIds;
    CachedStatementPtr stmt = GetDgnDb().Elements().GetStatement("SELECT Id FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE ParentId=?");
    stmt->BindId(1, parentId);
    while (BE_SQLITE_ROW == stmt->Step())
        existingChildIds.push_back(stmt->GetValueId<DgnElementId>(0));

    size_t count = std::min(existingChildIds.size(), parentConversionResults.m_childElements.size());
    size_t i = 0;
    for ( ; i<count; ++i)
        {
        UpdateResults(parentConversionResults.m_childElements.at(i), existingChildIds.at(i), parentelprov);
        // *** WIP_CONVERTER - bail out if any child update fails?
        }

    for ( ; i < parentConversionResults.m_childElements.size(); ++i)
        {
        InsertResults(parentConversionResults.m_childElements.at(i), parentelprov);
        // *** WIP_CONVERTER - bail out if any child update fails?
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Converter::UpdateResults(ElementConversionResults& conversionResults, DgnElementId existingElementId, SyncInfo::V8ElementExternalSourceAspectData const& elprov)
    {
    auto status = UpdateResultsForOneElement(conversionResults, existingElementId, elprov);
    if (DgnDbStatus::Success != status)
        {
        BeAssert((DgnDbStatus::LockNotHeld != status) && "Failed to get or retain necessary locks");
        return status;
        }
    return UpdateResultsForChildren(conversionResults, elprov);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ProcessConversionResults(ElementConversionResults& conversionResults, IChangeDetector::SearchResults const& csearch,
                                         DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm)
    {
    if (IChangeDetector::ChangeType::None == csearch.m_changeType)
        {
        _GetChangeDetector()._OnElementSeen(*this, csearch.GetExistingElementId());
        conversionResults.m_mapping = csearch.m_v8ElementAspect;

        auto allFromSameSource = iModelExternalSourceAspect::GetSelectFromSameSource(GetDgnDb(), csearch.m_v8ElementAspect, BeSQLite::EC::IECSqlBinder::MakeCopy::No);
        while(BE_SQLITE_ROW == allFromSameSource->Step())
            _GetChangeDetector()._OnElementSeen(*this, allFromSameSource->GetValueId<DgnElementId>(0));

/*<=*/  return;
        }

    // NB! The "scope" for this conversion must be an object in the BIM that coresponds to the *source* model that contains
    //     the v8 element. We may decide to write the element itself to a different model.
    auto scopeId = conversionResults.m_scope.IsValid()? conversionResults.m_scope.GetDgnModel().GetModelId(): v8mm.GetDgnModel().GetModelId();
    SyncInfo::V8ElementExternalSourceAspectData elprov(scopeId, v8eh.GetElementId(), csearch.m_currentElementProvenance, conversionResults.m_jsonProps);

    if (IChangeDetector::ChangeType::Update == csearch.m_changeType)
        {
        conversionResults.m_mapping = csearch.m_v8ElementAspect;
        if (v8eh.GetElementType() != DgnV8Api::RASTER_FRAME_ELM &&
            !(v8eh.GetElementType() == DgnV8Api::EXTENDED_ELM &&  // Quickly reject non-106
             DgnV8Api::ElementHandlerManager::GetHandlerId(v8eh) == DgnV8Api::PointCloudHandler::GetElemHandlerId()))
            {
            UpdateResults(conversionResults, csearch.GetExistingElementId(), elprov);
            }
        else
            {
            _GetChangeDetector()._OnElementSeen(*this, csearch.GetExistingElementId());      // NEEDS WORK: Do I really need to presere this for the special case of raster elements?
            }
        }
    else
        {
        BeAssert(IChangeDetector::ChangeType::Insert == csearch.m_changeType);
        InsertResults(conversionResults, elprov);
        if (!conversionResults.m_element.IsValid())
            return;
        }

    AnnounceConversionResults(conversionResults, v8eh, v8mm, csearch);

    if (BSISUCCESS != iModelBridge::SaveChangesToConserveMemory(GetDgnDb(), nullptr, 100000))
        {
        OnFatalError(IssueCategory::DiskIO(), Issue::Error(), "SavePoint failed");
        }
    m_hadAnyChanges = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2019
//---------------+---------------+---------------+---------------+---------------+-------
void Converter::AddModelRequiringRealityTiles(DgnModelId id, Utf8StringCR sourceFile, RepositoryLinkId fileId)
    {
    m_modelsRequiringRealityTiles.Insert(id, bpair<Utf8String, RepositoryLinkId>(sourceFile, fileId));

    BeFileName file(sourceFile.c_str());
    SyncInfo::UriContentInfo currentInfo;
    currentInfo.GetInfo(sourceFile);       // TODO: What to do if we can't get current info (e.g., server is down and we can't get eTag?)

    bool isUpdate = false;

    DgnElementId modeledElementId(DgnElementId(id.GetValue()));
    Utf8String fileName = Utf8String(sourceFile.c_str());
    auto existingAspect = SyncInfo::UriExternalSourceAspect::GetAspect(*GetDgnDb().Elements().GetElement(modeledElementId));
    if (existingAspect.IsValid())
        {
        SyncInfo::UriContentInfo existingInfo;
        existingAspect.GetInfo(existingInfo);

        if (existingInfo.IsEqual(currentInfo))
            return;

        isUpdate = true;
        }

    auto el = GetDgnDb().Elements().GetForEdit<DgnElement>(modeledElementId);
    if (isUpdate)
        {
        auto aspect = SyncInfo::UriExternalSourceAspect::GetAspectForEdit(*el);
        aspect.SetInfo(currentInfo);
        }
    else
        {
        auto aspect = SyncInfo::UriExternalSourceAspect::CreateAspect(fileId, sourceFile.c_str(), currentInfo, "", *this);
        aspect.AddAspect(*el);
        }
    el->Update();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Keith.Bentley                   02 / 15
//---------------------------------------------------------------------------------------
BentleyStatus SpatialConverterBase::DoConvertSpatialElement(ElementConversionResults& results, DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm, bool isNewElement)
    {
    if (WasAborted())
        return BSISUCCESS;

    ReportProgress();

    // Convert raster elements
    if (v8eh.GetElementType() == DgnV8Api::RASTER_FRAME_ELM)
        {
        return _ConvertRasterElement(v8eh, v8mm, true, isNewElement);
        }

    // Convert point cloud elements
    if (v8eh.GetElementType() == DgnV8Api::EXTENDED_ELM &&  // Quickly reject non-106
        DgnV8Api::ElementHandlerManager::GetHandlerId(v8eh) == DgnV8Api::PointCloudHandler::GetElemHandlerId())
        {
        return _ConvertPointCloudElement(v8eh, v8mm, true, isNewElement);
        }

    return ConvertElement(results, v8eh, v8mm, GetSyncInfo().GetCategory(v8eh, v8mm), false, isNewElement);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Keith.Bentley                   02 / 15
//---------------------------------------------------------------------------------------
void Converter::DoConvertControlElement(ElementConversionResults& results, DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm, bool isNewElement)
    {
    if (WasAborted())
        return;

    ReportProgress();

    ConvertElement(results, v8eh, v8mm, GetSyncInfo().GetCategory(v8eh, v8mm), false, isNewElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ConvertLevels(DgnV8EhCR v8eh)
    {
    ConvertDrawingLevel(*v8eh.GetDgnFileP(), GetV8Level(v8eh));

    // *** TRICKY: Keep this consistent with V8GraphicsCollector::ProcessElement
    for (DgnV8Api::ChildElemIter childIt(v8eh, DgnV8Api::ExposeChildrenReason::Query); childIt.IsValid(); childIt = childIt.ToNext())
        {
        ConvertLevels(childIt);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::DoConvertDrawingElement(ElementConversionResults& results, DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm, bool isNewElement)
    {
    if (WasAborted())
        return;

    ReportProgress();


    DgnCategoryId catid = GetSyncInfo().GetCategory(v8eh, v8mm);
    ConvertElement(results, v8eh, v8mm, catid, false, isNewElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialConverterBase::_ConvertSpatialElement(ElementConversionResults& results, DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm)
    {
//TODO    if (_FilterElement(v8eh))
//TODO        return;

    IChangeDetector::SearchResults changeInfo;
    BentleyStatus stat = SUCCESS;
    if (GetChangeDetector()._IsElementChanged(changeInfo, *this, v8eh, v8mm))
        {
        stat = DoConvertSpatialElement(results, v8eh, v8mm, (IChangeDetector::ChangeType::Insert == changeInfo.m_changeType));
        }

    if (SUCCESS == stat)
        ProcessConversionResults(results, changeInfo, v8eh, v8mm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::_ConvertDrawingElement(DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm)
    {
    ElementConversionResults results;

    IChangeDetector::SearchResults changeInfo;
    if (GetChangeDetector()._IsElementChanged(changeInfo, *this, v8eh, v8mm))
        {
        DoConvertDrawingElement(results, v8eh, v8mm, (IChangeDetector::ChangeType::Insert == changeInfo.m_changeType));
        }

    ProcessConversionResults(results, changeInfo, v8eh, v8mm);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::_ConvertControlElement(ElementConversionResults& results, DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm)
    {
//TODO    if (_FilterElement(v8eh))
//TODO        return;

    if (v8eh.GetElementType() == DgnV8Api::REFERENCE_ATTACH_ELM)
        return;

    IChangeDetector::SearchResults changeInfo;
    if (GetChangeDetector()._IsElementChanged(changeInfo, *this, v8eh, v8mm))
        {
        DoConvertControlElement(results, v8eh, v8mm, (IChangeDetector::ChangeType::Insert == changeInfo.m_changeType));
        }

    ProcessConversionResults(results, changeInfo, v8eh, v8mm);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Keith.Bentley                   02 / 15
//---------------------------------------------------------------------------------------
void SpatialConverterBase::ConvertElementList(DgnV8Api::PersistentElementRefList* list, ResolvedModelMapping const& v8mm)
    {
    if (nullptr == list)
        return;

    //bool isGraphics = list->_IsGraphicsList();
    bool isGraphics = (nullptr != dynamic_cast<DgnV8Api::GraphicElementRefList*>(list));


    for (auto v8El : *list)
        {
        BeAssert(v8El != nullptr);
        DgnV8Api::EditElementHandle v8eh(v8El);
        ElementConversionResults results;
        if (isGraphics)
            _ConvertSpatialElement(results, v8eh, v8mm);
        else
            _ConvertControlElement(results, v8eh, v8mm);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/15
+---------------+---------------+---------------+---------------+---------------+------*/
double Converter::ComputeUnitsScaleFactor(DgnV8ModelCR v8Model)
    {
    Bentley::ModelInfoCR      modelInfo = v8Model.GetModelInfo();
    Bentley::UnitDefinitionCR v8Storage = modelInfo.GetStorageUnit();

    if (DgnV8Api::UnitBase::Meter != v8Storage.GetBase())
        return 0.0; // DANGER! - this file cannot be converted.

    UnitDefinition existingUnits((UnitBase) v8Storage.GetBase(),(UnitSystem) v8Storage.GetSystem(), v8Storage.GetNumerator(), v8Storage.GetDenominator(), nullptr);

    // for DgnDb, all coordinates are stored as meters. Determine the ratio of DgnV8 storage units to meters
    auto toMeters = existingUnits.ToMeters() / modelInfo.GetUorPerStorage();

    if (v8Model.GetModelType() == DgnV8Api::DgnModelType::Sheet)
        {
        // Note: the V8 annotation scale factor a big number. Normally, V8 uses this to scale small things like line styles and text up so that they look right in big sheets.
        auto sheetscale = v8Model.GetModelInfo().GetAnnotationScaleFactor();
        if ((0 != BeNumerical::Compare(sheetscale, 1.0)) && (0 != BeNumerical::Compare(sheetscale, 0.0)))
            {
            // This appears to be a sheet that is as big as the designs that it shows.
            // We don't want that in the BIM. We want sheets to have sensible (paper) sizes. Then views of designs
            // are scaled down to the sheets. So, we build the scale-down factor into the sheet model's overall
            // V8->BIM transform.
            // Note that we divide by the V8 annotation scale factor here, in order to scale the sheet itself DOWN.
            toMeters /= sheetscale;
            }
        }

    return toMeters;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/15
+---------------+---------------+---------------+---------------+---------------+------*/
Transform Converter::ComputeUnitsScaleTransform(DgnV8ModelCR v8Model)
    {
    double scale = ComputeUnitsScaleFactor(v8Model);
    Transform uorToMeter = Transform::FromIdentity();
    uorToMeter.ScaleMatrixColumns(scale, scale, scale);

    // for sheets, we need to shift the origin to the lower left corner and potentially rotate to align the model with the sheet axes
    if (v8Model.GetModelType() != DgnV8Api::DgnModelType::Sheet)
        return uorToMeter;

    auto v8SheetDef = v8Model.GetModelInfo().GetSheetDefCP();
    if (nullptr == v8SheetDef)
        return uorToMeter;

    Bentley::DPoint2d org;
    v8SheetDef->GetOrigin(org);
    Transform sheetTrans;
    sheetTrans.InverseOf(Transform::From(RotMatrix::FromAxisAndRotationAngle(2,v8SheetDef->GetRotation()), DPoint3d::From(org.x,org.y,0)));
    return Transform::FromProduct(uorToMeter, sheetTrans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedModelMapping Converter::FindModelByExternalAspect(DgnV8ModelRefCR v8ModelRef, TransformCR trans)
    {
    DgnV8ModelP v8Model = v8ModelRef.GetDgnModelP();
    if (nullptr == v8Model)
        {
        BeAssert(false);
        return ResolvedModelMapping();
        }

    SyncInfo::V8ModelExternalSourceAspectIteratorByV8Id it(*GetRepositoryLinkElement(*v8Model->GetDgnFileP()), *v8Model);
    for (auto entry=it.begin(); entry!=it.end(); ++entry)
        {
        if (Converter::IsTransformEqualWithTolerance(entry->GetTransform(),trans))
            {
            auto model = m_dgndb->Models().GetModel(entry->GetModelId());
            if (!model.IsValid())
                continue;
            return ResolvedModelMapping(*model, *v8Model, *entry, v8ModelRef.AsDgnAttachmentCP());
            }
        }

    return ResolvedModelMapping();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      1/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::TryFindElement(DgnElementId& elementId, DgnV8EhCR eh)
    {
    auto aspect = FindFirstElementMappedTo(*eh.GetDgnModelP(), eh.GetElementId(), nullptr);
    if (!aspect.IsValid())
        return false;
    elementId = aspect.GetElementId();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8ElementExternalSourceAspect Converter::FindFirstElementMappedTo(DgnModelId modelId, DgnV8Api::ElementId v8ElementId,
                                                                  IChangeDetector::T_SyncInfoElementFilter* filter)
    {
    SyncInfo::V8ElementExternalSourceAspectIteratorByV8Id elements(*GetDgnDb().Models().GetModel(modelId), v8ElementId);
    auto entry = elements.begin();
    if (nullptr != filter)
        {
        while ((entry != elements.end()) && !(*filter)(entry, *this))
            ++entry;
        }
    if (elements.end() != entry)
        return *entry;

    return SyncInfo::V8ElementExternalSourceAspect();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8ElementExternalSourceAspect Converter::FindFirstElementMappedTo(DgnV8ModelCR v8Model, DgnV8Api::ElementId v8ElementId,
                                                               IChangeDetector::T_SyncInfoElementFilter* filter)
    {
    auto repositoryLinkEl = GetRepositoryLinkElement(*v8Model.GetDgnFileP());
    SyncInfo::V8ModelExternalSourceAspectIteratorByV8Id modelIter(*repositoryLinkEl, v8Model);
    auto modelAspectEntry = modelIter.begin();
    if (modelAspectEntry == modelIter.end())
        return SyncInfo::V8ElementExternalSourceAspect();

    return FindFirstElementMappedTo(modelAspectEntry->GetModelId(), v8ElementId, filter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
SyncInfo::V8ElementExternalSourceAspect Converter::_FindFirstElementMappedTo(DgnV8Api::DisplayPath const& proxyPath, bool tail,
                                                               IChangeDetector::T_SyncInfoElementFilter* filter)
    {
    ElementRefP targetEl;
    int pathPos = proxyPath.GetCount();
    if (pathPos == 0)
        {
        BeAssert(false);
        return SyncInfo::V8ElementExternalSourceAspect();
        }
    --pathPos;
    if (!tail && pathPos > 0)
        --pathPos;
    targetEl = proxyPath.GetPathElem(pathPos);

    if (nullptr == targetEl)
        return SyncInfo::V8ElementExternalSourceAspect();

    DgnV8ModelRefR v8Model = *proxyPath.GetRoot();
    if (nullptr == v8Model.GetDgnModelP())
        return SyncInfo::V8ElementExternalSourceAspect();

    return FindFirstElementMappedTo(*v8Model.GetDgnModelP(), targetEl->GetElementId(), filter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr Converter::CreateNewElement(DgnModel& model, DgnClassId elementClassId, DgnCategoryId categoryId, DgnCode code, Utf8CP label)
    {
    ElementHandlerP elementHandler = dgn_ElementHandler::Element::FindHandler(model.GetDgnDb(), elementClassId);

    if (nullptr == elementHandler)
        {
        BeAssert(false);
        return nullptr;
        }

    if (!categoryId.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    DgnElementPtr el = elementHandler->Create(DgnElement::CreateParams(model.GetDgnDb(), model.GetModelId(), elementClassId, code, label));
    auto geomSource = el->ToGeometrySourceP();

    if (nullptr == geomSource)
        return nullptr;

    geomSource->SetCategoryId(categoryId);
    return el;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_AddResolvedModelMapping(ResolvedModelMapping const& v8mm)
    {
    m_v8ModelMappings.insert(v8mm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedModelMapping RootModelConverter::_GetResolvedModelMapping(DgnV8ModelRefCR v8ModelRef, TransformCR trans)
    {
    DgnV8ModelR v8Model = *v8ModelRef.GetDgnModelP();

    bool foundOne = false;
    for (auto thisModel : FindResolvedModelMappings(v8Model)) // all unique transforms of attachments of this model
        {
        foundOne = true; // We have mapped this DgnV8 model to a DgnDb model.

        //  See if this particular mapping is based on the same transform.
        if (IsTransformEqualWithTolerance(thisModel.GetTransform(),trans))
            return thisModel;
        }

    // Didn't find an already-resolved mapping to this model with this transform. Try looking it up by XSA.
    ResolvedModelMapping alreadyConverted = FindModelByExternalAspect(v8ModelRef, trans);
    if (alreadyConverted.IsValid())
        {
        _AddResolvedModelMapping(alreadyConverted);
        GetChangeDetector()._OnModelSeen(*this, alreadyConverted);
        return alreadyConverted;
        }

    // Didn't find a record of converting this model with this transform => Convert it now.

    Utf8String newModelName;
    if (foundOne)
        {
        // We already have this V8Model mapped into the DgnDb, but with a different transform than what the caller wants to use.
        // => We must make a new, transformed copy of the DgnV8 model.
        DgnAttachmentCP thisAttachment = v8ModelRef.AsDgnAttachmentCP();
        if (nullptr == thisAttachment)
            {
            BeAssert(false && "transformed copies are for attachments only!");
            return ResolvedModelMapping();
            }

        bool base = true;
        while (nullptr != thisAttachment)
            {
            // create the new model name from the logical names of the attachment path
            Utf8String attachName((0 == *thisAttachment->GetLogicalName()) ? _ComputeModelName(*thisAttachment->GetDgnModelP()) : Utf8String(thisAttachment->GetLogicalName()));

            if (base)
                {
                newModelName = attachName;
                base = false;
                }
            else
                newModelName = attachName + "@" + newModelName;

            thisAttachment = thisAttachment->GetParentDgnAttachmentCP();
            }

        DgnModels& models = m_dgndb->Models();
        models.ReplaceInvalidCharacters(newModelName, models.GetIllegalCharacters(), '_');
        }
    else
        {
        newModelName = _ComputeModelName(v8Model).c_str();
        }

    // Map in the DgnV8 model.
    DgnModelId modelId = _MapModelIntoProject(v8Model, newModelName.c_str(), v8ModelRef.AsDgnAttachmentCP());
    if (!modelId.IsValid())
        {
        return ResolvedModelMapping();
        }

    DgnModelPtr model = m_dgndb->Models().GetModel(modelId);
    if (!model.IsValid())
        {
        ReportError(IssueCategory::Unknown(), Issue::ConvertFailure(), IssueReporter::FmtModel(v8Model).c_str());
        OnFatalError();
        return ResolvedModelMapping();
        }
    BeAssert(model->GetRefCount() > 0); // DgnModels holds references to all models that it loads

    auto modeledElement = m_dgndb->Elements().GetElement(model->GetModeledElementId())->CopyForEdit();
    auto modelAspect = SyncInfo::V8ModelExternalSourceAspect::CreateAspect(v8Model, trans, *this);
    modelAspect.AddAspect(*modeledElement);
    auto updatedModelElement = modeledElement->Update();
    UNUSED_VARIABLE(updatedModelElement);
    BeAssert(updatedModelElement.IsValid());

    ResolvedModelMapping v8mm(*model, v8Model, modelAspect, v8ModelRef.AsDgnAttachmentCP());

    _AddResolvedModelMapping(v8mm);

    GetChangeDetector()._OnModelInserted(*this, v8mm);
    GetChangeDetector()._OnModelSeen(*this, v8mm);
    m_monitor->_OnModelInserted(v8mm);

    if (LOG_MODEL_IS_SEVERITY_ENABLED(NativeLogging::LOG_TRACE))
        {
        LOG_MODEL.tracev("+ %s -> %s", IssueReporter::FmtModel(v8Model).c_str(), IssueReporter::FmtModel(*model).c_str());
        LOG_MODEL.trace(modelAspect.FormatForDump(*m_dgndb, true, true).c_str());
        }

    return v8mm;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedModelMapping RootModelConverter::MapDgnV8ModelToDgnDbModel(DgnV8ModelR v8Model, TransformCR trans, DgnModelId targetModelId)
    {
    // This function is like GetResolvedModelMapping, except that caller already knows the target model

    // *************************************************************
    // *************************************************************
    // *************************************************************
    // ******************       NEEDS WORK      ********************
    // *************************************************************
    // *************************************************************
    // This function and GetResolvedModelMapping have the same initial look-up logic.
    // Factor that out into a function that both can call.
    // *************************************************************
    // *************************************************************
    // *************************************************************
    if (IsUpdating())
        {
        ResolvedModelMapping res = FindModelByExternalAspect(v8Model, trans);
        if (res.IsValid())
            {
            if (!_FindResolvedModelMapping(v8Model, trans).IsValid())
                _AddResolvedModelMapping(res);
            GetChangeDetector()._OnModelSeen(*this, res);
            return res;
            }

        // not found in syncinfo => treat as insert
        }

    bool foundOne = false;
    for (auto thisModel : FindResolvedModelMappings(v8Model))   // all unique transforms of attachments of this model
        {
        foundOne = true; // We have mapped this DgnV8 model to a DgnDb model.

        //  See if this particular mapping is based on the same transform.
        if (Converter::IsTransformEqualWithTolerance(thisModel.GetTransform(),trans))
            return thisModel;
        }

    // This is the first time we've seen this (attachment of) this V8 model. Create a new mapping for it.
    auto model = m_dgndb->Models().GetModel(targetModelId);
    if (!model.IsValid())
        return ResolvedModelMapping();

    auto modeledElement = m_dgndb->Elements().GetElement(model->GetModeledElementId())->CopyForEdit();
    auto modelAspect = SyncInfo::V8ModelExternalSourceAspect::CreateAspect(v8Model, trans, *this);
    modelAspect.AddAspect(*modeledElement);
    auto updatedModelElement = modeledElement->Update();
    BeAssert(updatedModelElement.IsValid());

    ResolvedModelMapping v8mm(*model, v8Model, modelAspect, nullptr);

    _AddResolvedModelMapping(v8mm);

    GetChangeDetector()._OnModelInserted(*this, v8mm);
    GetChangeDetector()._OnModelSeen(*this, v8mm);

    return v8mm;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedModelMapping RootModelConverter::_FindResolvedModelMapping(DgnV8ModelR v8Model, TransformCR trans)
    {
    for (auto thisModel : FindResolvedModelMappings(v8Model)) // finds all unique transforms of attachments of this model
        {
        //  See if this mapping is based on the same transform.
        if (Converter::IsTransformEqualWithTolerance(thisModel.GetTransform(),trans))
            return thisModel;
        }

    return ResolvedModelMapping();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+--------------+---------------+---------------+---------------+---------------+------*/
bvector<ResolvedModelMapping> RootModelConverter::FindResolvedModelMappings(DgnV8ModelR v8Model)
    {
    auto fid = GetRepositoryLinkId(*v8Model.GetDgnFileP());
    if (!fid.IsValid())
        return bvector<ResolvedModelMapping>();
    SyncInfo::V8ModelExternalSourceAspectIteratorByV8Id modelXsaIterator(*GetRepositoryLinkElement(fid), v8Model);
    auto mrec = modelXsaIterator.begin();
    if (mrec == modelXsaIterator.end())
        return bvector<ResolvedModelMapping>();
    ResolvedModelMapping key(*(DgnModelP)nullptr, v8Model, *mrec, nullptr); // tricky: we construct an invalid mapping, just for search purposes. We know that only the v8model id and v8 file syncinfo id are used.
    auto range = m_v8ModelMappings.equal_range(key); // finds all unique transforms of attachments of this model
    bvector<ResolvedModelMapping> found;
    for (auto i = range.first; i != range.second; ++i)
        found.push_back(*i);
    return found;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool RootModelConverter::IsLessInMappingOrder(DgnV8ModelP a, DgnV8ModelP b)
    {
    auto fid1 = GetRepositoryLinkId(*a->GetDgnFileP());
    auto fid2 = GetRepositoryLinkId(*b->GetDgnFileP());
    if (fid1 < fid2)
        return true;
    if (fid1 > fid2)
        return false;
    return a->GetModelId() < b->GetModelId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+--------------+---------------+---------------+---------------+---------------+------*/
ResolvedModelMapping RootModelConverter::_FindFirstResolvedModelMapping(DgnV8ModelR v8Model)
    {
    auto range = FindResolvedModelMappings(v8Model); // finds all unique transforms of attachments of this model
    return !range.empty()? range.front(): ResolvedModelMapping();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::PopulateRangePartIdMap()
    {
    for (ElementIteratorEntryCR geomPartEntry : m_dgndb->Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_GeometryPart)))
        {
        DgnGeometryPartCPtr geomPart = m_dgndb->Elements().Get<DgnGeometryPart>(geomPartEntry.GetElementId());
        if (!geomPart.IsValid())
            continue;

        Utf8String codeValue = geomPart->GetCode().GetValueUtf8CP();
        if (codeValue.empty())
            codeValue = QueryGeometryPartTag(geomPart->GetId());

        if (codeValue.empty())
            continue; //Null codes are valid.

        if (nullptr == strstr(codeValue.c_str(), "CvtV8"))
            continue;

        GetRangePartIdMap().insert(Converter::RangePartIdMap::value_type(PartRangeKey(geomPart->GetBoundingBox()), geomPart->GetId()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Ramanujam.Raman                      05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::EmbedSpecifiedFiles()
    {
    SetStepName(ProgressMessage::STEP_EMBEDDING_FILES());

    bvector<BeFileName> embedFiles = _GetParams().GetEmbedFiles();

    BeFileNameCR embedDir = _GetParams().GetEmbedDir();
    if (!embedDir.IsEmpty())
        {
        if (!embedDir.IsDirectory() || !embedDir.DoesPathExist())
            ReportIssueV(IssueSeverity::Warning, IssueCategory::MissingData(), Issue::EmbeddedDirOrFileIsInvalid(), nullptr, Utf8String(embedDir.c_str()).c_str());
        else
            {
            WString embedDirectoryWildcard = embedDir;
            BeFileName::AppendToPath(embedDirectoryWildcard, L"*.*");

            BeFileListIterator embeddedFileIter(embedDirectoryWildcard, false);
            BeFileName embedFile;
            while (SUCCESS == embeddedFileIter.GetNextFileName(embedFile))
                {
                embedFiles.push_back(embedFile);
                }
            }
        }

    for (BeFileNameCR embedFile : embedFiles)
        {
        if (embedFile.IsDirectory() || !embedFile.DoesPathExist())
            {
            ReportIssueV(IssueSeverity::Warning, IssueCategory::MissingData(), Issue::EmbeddedDirOrFileIsInvalid(), nullptr, Utf8String(embedFile.c_str()).c_str());
            continue;
            }

        Utf8String pathName(embedFile.GetNameUtf8());
        Utf8String typeName(BeFileName::GetExtension(embedFile.GetName()).c_str());
        Utf8String descr(BeFileName::GetFileNameAndExtension(embedFile.GetName()).c_str());

        DbResult result = m_dgndb->EmbeddedFiles().Import(pathName.c_str(), pathName.c_str(), typeName.c_str(), descr.c_str());
        if (BE_SQLITE_OK != result)
            {
            ReportIssueV(IssueSeverity::Warning, IssueCategory::Unknown(), Issue::EmbedFileError(), nullptr, Utf8String(embedFile.c_str()).c_str());
            continue;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2013
//---------------------------------------------------------------------------------------
void Converter::EmbedFilesInSource(BeFileNameCR rootFileName)
    {
    // Expect to be able to open the root store.
    IStoragePtr rootStg;
    HRESULT hr = ::StgOpenStorageEx(rootFileName.c_str(), ROOTSTORE_OPENMODE_READ, STGFMT_ANY, 0, NULL, 0, IID_IStorage, (void**) &rootStg);

    // DgnV7 files can come in here, which are not structured stores.
    if (E_NOINTERFACE == hr)
        return;

    if (UNEXPECTED_CONDITION(FAILED(hr)))
        return;

    // Use Bern's API for decoding file name from stream name.
    V8IModelExtraFiles::EmbeddedFileListPtr v8EmbeddedFiles = V8IModelExtraFiles::EmbeddedFileList::ReadFromPackagedIModel(rootStg);
    if (UNEXPECTED_CONDITION(NULL == v8EmbeddedFiles.get()))
        return;

    // See if there's an optional ExtraFiles storage.
    // This must be done after the call to V8IModelExtraFiles::EmbeddedFileList::ReadFromPackagedIModel, because it requires an exclusive lock on this storage.
    IStoragePtr extraFilesStg;
    hr = rootStg->OpenStorage(V8IModelExtraFiles::EXTRAFILES_STORAGE_NAME, NULL, SUBSTORE_OPENMODE_READ, NULL, 0, &extraFilesStg);
    if (FAILED(hr))
        return;

    for (auto fileIter = v8EmbeddedFiles->cbegin(); v8EmbeddedFiles->cend() != fileIter; ++fileIter)
        {
        Utf8String fileName((*fileIter)->GetFileName());

        // Choosing to have first-in wins, instead of renaming.
        if (m_dgndb->EmbeddedFiles().QueryFile(fileName.c_str()).IsValid())
            {
            ReportIssueV(IssueSeverity::Warning, IssueCategory::Unsupported(), Issue::EmbeddedFileAlreadyExists(), nullptr, fileName.c_str());
            continue;
            }

        // Get an IStream for this object.
        IStreamPtr extraFileStream;
        hr = extraFilesStg->OpenStream((*fileIter)->GetStreamName().c_str(), NULL, STREAM_OPENMODE_READ, 0, &extraFileStream);
        if (UNEXPECTED_CONDITION(FAILED(hr)))
            continue;

        // Determine stream size.
        STATSTG extraFileStreamStat;
        hr = extraFileStream->Stat(&extraFileStreamStat, STATFLAG_NONAME);
        if (UNEXPECTED_CONDITION(FAILED(hr)))
            continue;

        // Read the entire stream into memory so we can embed the blob.
        // We still support 32-bit importers, so be mindful (e.g. don't crash) with large files.
        try
            {
            ScopedArray<Byte> extraFileData(static_cast<size_t>(extraFileStreamStat.cbSize.QuadPart));
            ULONG numBytesRead;
            hr = extraFileStream->Read(extraFileData.GetData(), (ULONG) extraFileStreamStat.cbSize.QuadPart, &numBytesRead);
            if (UNEXPECTED_CONDITION(FAILED(hr)) || UNEXPECTED_CONDITION(numBytesRead != extraFileStreamStat.cbSize.QuadPart))
                continue;

            BentleyApi::Utf8String typeName(BeFileName::GetExtension((*fileIter)->GetFileName()).c_str());

            // Make the stub for the DB embedded file (no API to directly import a buffer).
            DbResult dbres = m_dgndb->EmbeddedFiles().AddEntry(fileName.c_str(), typeName.c_str(), NULL);
            if (UNEXPECTED_CONDITION(BE_SQLITE_OK != dbres))
                continue;

            // Save the stream's bytes into the DB embedded file.
            dbres = m_dgndb->EmbeddedFiles().Save(extraFileData.GetData(), extraFileStreamStat.cbSize.QuadPart, fileName.c_str());
            if (UNEXPECTED_CONDITION(BE_SQLITE_OK != dbres))
                continue;
            }
        catch (std::bad_alloc)
            {
            ReportIssueV(IssueSeverity::Warning, IssueCategory::Unsupported(), Issue::EmbeddedFileTooBig(), nullptr, fileName.c_str());
            continue;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId Converter::GetOrCreateDrawingCategoryId(DefinitionModelR model, Utf8StringCR categoryName, DgnSubCategory::Appearance const& appear)
    {
    DgnCode categoryCode = DrawingCategory::CreateCode(model, categoryName);
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(model.GetDgnDb(), categoryCode);
    if (categoryId.IsValid())
        return categoryId;

    DrawingCategory category(model, categoryName, DgnCategory::Rank::Application);
    category.Insert(appear, nullptr);
    return category.GetCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId Converter::GetOrCreateSubCategoryId(DgnCategoryId categoryId, Utf8CP subCatName, DgnSubCategory::Appearance const& appear)
    {
    DgnCode subCategoryCode = DgnSubCategory::CreateCode(GetDgnDb(), categoryId, subCatName);

    DgnSubCategoryId id = DgnSubCategory::QuerySubCategoryId(GetDgnDb(), subCategoryCode);
    if (id.IsValid())
        return id;

    DgnSubCategory newSubCategory(DgnSubCategory::CreateParams(GetDgnDb(), categoryId, subCatName, appear));
    newSubCategory.Insert();
    return newSubCategory.GetSubCategoryId();
    }

// DGNV8_ELEMENTHANDLER_EXTENSION_DEFINE_MEMBERS(ConvertToDgnDbElementExtension);
// Could not find a convenient way to use this DgnV8 macro because it expands to directly use DgnV8 namespaces, which can't work here.
DgnV8Api::Handler::Extension::Token& ConvertToDgnDbElementExtension::z_GetConvertToDgnDbElementExtensionToken() { static DgnV8Api::Handler::Extension::Token* s_token = 0; if (0 == s_token) s_token = NewToken(); return *s_token; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
ConverterLibrary::ConverterLibrary(DgnDbR bim, RootModelSpatialParams& params) : T_Super(params)
    {
    m_dgndb = &bim;

    m_changeDetector.reset(new ChangeDetector);

    AttachSyncInfo();

    m_dgndb->AddIssueListener(m_issueReporter.GetECDbIssueListener());
    InitLinkConverter();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterLibrary::RecordFileMapping(DgnV8FileR v8File)
    {
    GetRepositoryLinkId(v8File);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterLibrary::SetRootModelAndSubject(DgnV8ModelR rootV8Model, SubjectCR jobSubject)
    {
    m_rootModelRef = &rootV8Model;
    m_rootFile = rootV8Model.GetDgnFileP();

    GetRepositoryLinkId(*m_rootFile);
    FindSpatialV8Models(*GetRootModelP());
    FindV8DrawingsAndSheets();

    m_isRootModelSpatial = ShouldConvertToPhysicalModel(rootV8Model);

    _ComputeCoordinateSystemTransform();

    Transform jobTrans = iModelBridge::GetSpatialDataTransform(_GetParams(), jobSubject);
    if (jobTrans.IsIdentity())
        return;
    m_rootTrans = BentleyApi::Transform::FromProduct(jobTrans, m_rootTrans); // NB: pre-multiply!
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
ResolvedModelMapping ConverterLibrary::RecordModelMapping(DgnV8ModelR sourceV8Model, DgnModelR targetBimModel, BentleyApi::TransformCP transform)
    {
    BentleyApi::Transform defaultTransform;
    if (nullptr == transform)
        {
        if (&sourceV8Model == m_rootModelRef)
            transform = &m_rootTrans;
        else
            {
            defaultTransform = ComputeUnitsScaleTransform(sourceV8Model);
            transform = &defaultTransform;
            }
        }

    auto v8mm = FindResolvedModelMapping(sourceV8Model, *transform);
    if (v8mm.IsValid())
        return v8mm;

    GetRepositoryLinkId(*sourceV8Model.GetDgnFileP());

    auto modeledElement = m_dgndb->Elements().GetElement(targetBimModel.GetModeledElementId())->CopyForEdit();
    auto modelAspect = SyncInfo::V8ModelExternalSourceAspect::CreateAspect(sourceV8Model, transform? *transform: Transform::FromIdentity(), *this);
    modelAspect.AddAspect(*modeledElement);
    auto updatedModelElement = modeledElement->Update();
    BeAssert(updatedModelElement.IsValid());

    v8mm = ResolvedModelMapping(targetBimModel, sourceV8Model, modelAspect, nullptr);

    m_v8ModelMappings.insert(v8mm);

    GetChangeDetector()._OnModelInserted(*this, v8mm);
    GetChangeDetector()._OnModelSeen(*this, v8mm);

    BeAssert(FindResolvedModelMapping(sourceV8Model, *transform).IsValid());

    if (&sourceV8Model == m_rootModelRef)
        m_rootModelMapping = v8mm;

    return v8mm;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterLibrary::RecordLevelMappingForModel(DgnV8Api::LevelId sourceV8LevelId, DgnSubCategoryId targetBimSubCategory, DgnV8ModelRefR sourceV8Model)
    {
    auto v8Level = sourceV8Model.GetLevelCache().GetLevel(sourceV8LevelId);
    m_syncInfo.InsertLevel(targetBimSubCategory, *sourceV8Model.GetDgnModelP(), v8Level);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterLibrary::RecordLevelMappingForModel(DgnV8Api::LevelId sourceV8LevelId, DgnSubCategoryId targetBimSubCategory, DgnV8FileR sourceV8File)
    {
    auto v8Level = sourceV8File.GetLevelCacheR().GetLevel(sourceV8LevelId);
    RepositoryLinkId v8fileId = GetRepositoryLinkId(sourceV8File);
    m_syncInfo.InsertLevel(targetBimSubCategory, sourceV8File.GetDictionaryModel(), v8Level);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
ElementConversionResults ConverterLibrary::ConvertElement(DgnV8EhCR v8Element, ResolvedModelMapping const* rmm)
    {
    ResolvedModelMapping defaultModelMapping;
    if (nullptr == rmm)
        {
        defaultModelMapping = _FindFirstResolvedModelMapping(*v8Element.GetDgnModelP());
        rmm = &defaultModelMapping;
        }
    ElementConversionResults results;
    Converter::ConvertElement(results, v8Element, *rmm, GetSyncInfo().GetCategory(v8Element, *rmm), false, true);
    return results;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterLibrary::SetChangeDetector(bool isUpdate)
    {
    _SetChangeDetector(isUpdate);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ConverterLibrary::ConvertAllDrawingsAndSheets()
    {
    if (!m_jobSubject.IsValid())
        {
        BeAssert(false && "You must register your job subject first");
        return BSIERROR;
        }

    GetOrCreateJobPartitions();

    _ImportDrawingAndSheetModels(m_rootModelMapping);
    if (WasAborted())
        return BSIERROR;
    _ConvertDrawings();
    if (WasAborted())
        return BSIERROR;
    _ConvertSheets();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool ResolvedModelMapping::operator< (ResolvedModelMapping const &o) const
    {
    if (!IsValid())
        return false;
    auto fid1 = m_aspect.GetRepositoryLinkId();
    auto fid2 = o.m_aspect.GetRepositoryLinkId();
    if (fid1 < fid2)
        return true;
    if (fid1 > fid2)
        return false;
    return m_aspect.GetV8ModelId() < o.m_aspect.GetV8ModelId();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/17
+---------------+---------------+---------------+---------------+---------------+------*/
BisConversionTargetModelInfo::BisConversionTargetModelInfo(DgnModelCR model)
    {
    if (model.IsDictionaryModel())
        m_modelType = ModelType::Dictionary;
    else if (model.Is3dModel())
        m_modelType = ModelType::ThreeD;
    else
        m_modelType = ModelType::TwoD;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2018
//---------------+---------------+---------------+---------------+---------------+-------
bool Converter::ShouldImportSchema(Utf8StringCR fullSchemaName, DgnV8ModelR v8Model)
    {
    if (!_ShouldImportSchema(fullSchemaName, v8Model))
        return false;

    for (auto v : m_schemaImportVerifiers)
        {
        if (!v->_ShouldImportSchema(fullSchemaName, v8Model))
            return false;
        }

    // This is a hack - there are schemas that reference the ECXD provided RfaElementSchema.  That schema cannot be located which means any schema that references it will fail to
    // de-serialize.  So we screen for known schemas that reference it.
    if (fullSchemaName.EqualsI("TargetRfaElement__Family___x5706____x684C__.01.00"))
        return false;
    return true;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Abeesh.Basheer           08/2018
//---------------+---------------+---------------+---------------+---------------+-------
bool            Converter::IsTransformEqualWithTolerance(TransformCR lhs, TransformCR rhs)
    {
    return iModelBridge::AreTransformsEqual(lhs, rhs);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2018
//---------------+---------------+---------------+---------------+---------------+-------
void Converter::CheckForAndSaveChanges()
    {
    m_hadAnyChanges |= m_dgndb->Txns().HasChanges();
    if (BSISUCCESS != iModelBridge::SaveChanges(*m_dgndb))
        {
        OnFatalError(IssueCategory::DiskIO(), Issue::Error(), "CheckForAndSaveChanges failed.");
        }
    }


END_DGNDBSYNC_DGNV8_NAMESPACE

