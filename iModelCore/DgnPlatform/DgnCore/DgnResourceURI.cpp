/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnResourceURI.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/DgnCore/DgnResourceURI.h>
#include <DgnPlatform/DgnHandlers/DgnECPersistence.h>

#define SCHEME_DgnResourceURI   "dgnresourceuri"
#define SCHEME_DgnResourceURI_Length 14

#define RT_DgnElements  "DgnElements"
#define RT_DgnElements_Length   11
#define RT_ECInstances  "ECInstances"
#define RT_ECInstances_Length   11

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String getEncodedTargetFile (DgnProjectR targetFile)
    {
    Utf8String filename (BeFileName (BeFileName::NameAndExt, targetFile.GetFileName().c_str()));
    filename.append (";");
    filename.append (targetFile.GetDbGuid().ToString());
    return BeStringUtilities::UriEncode(filename.c_str());
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus findElementIdByProvenance (ElementId& eid, Utf8CP oldFileName, Int64 oldElementId, DgnProjectR project)
    {
    DgnProvenances prov = project.Provenance();

    //  Look up the file
    UInt64 oldFileId;
    if (prov.QueryFileId (oldFileId, oldFileName) != SUCCESS)
        return ERROR;

    //  Look up the element
    ElementProvenance eprov (oldFileId, oldElementId);
    eid = prov.GetElement (eprov);
    return eid.IsValid()? SUCCESS: ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnResourceURI::UriToken::GetString() const
    {
    switch (m_type)
        {
        case TYPE_String:   return BeStringUtilities::UriDecode (m_substr.m_start, m_substr.m_end);
        case TYPE_UInt64:   return Utf8PrintfString("%llu", m_uint64);
        }
    return "";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnResourceURI::ParserBase::ParseUInt64 ()
    {
    UInt64 v;
    if (1 != BE_STRING_UTILITIES_UTF8_SSCANF (m_curpos, "%lld", &v))
        return ERROR;

    m_curtok.SetUInt64 (v);        

    ToNextDelimiter();
        
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::ParserBase::ParseString ()
    {
    Utf8CP p = m_curpos;
    ToNextDelimiter();
    m_curtok.SetString (p, m_curpos);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::ParserBase::ToNextDelimiter ()
    {
    // Skip over non-delimiters
    m_curpos += strcspn (m_curpos, m_tokenDelimiters);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::ParserBase::ToStartOfNextToken ()
    {
    // Skip over delimiters
    while (*m_curpos && strchr(m_tokenDelimiters,*m_curpos))
        ++m_curpos;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnResourceURI::ParserBase::ParserBase (Utf8StringCR pathStr, Utf8CP delims) : m_tokenDelimiters(delims)
    {
    m_rawString = pathStr;
    m_curpos = &m_rawString[0];
    if (ispunct(*m_curpos))
        ++m_curpos; // step over initial / or ? or #
    m_curtok.SetError();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnResourceURI::UriToken DgnResourceURI::ParserBase::GetCurToken() const {return m_curtok;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnResourceURI::UriToken DgnResourceURI::ParserBase::ParseNextToken ()
    {
    ToStartOfNextToken();
        
    if (!*m_curpos)
        {
        m_curtok.SetEOS();
        }
    else
        {
        if (ParseUInt64() != SUCCESS)
            {
            ParseString();
            }
        }

    return m_curtok;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnResourceURI::ParserBase::ParseFullECClassName (Utf8StringR ecSchemaName, Utf8StringR ecClassName, Utf8CP str, bool mustHaveSchema)
    {
    Utf8CP colon = strstr (str+1, ":");

    if (colon != nullptr)
        {
        ecSchemaName.assign (str, colon);
        ecClassName.assign (colon+1);
        return SUCCESS;
        }
            
    if (mustHaveSchema)
        return ERROR;
            
    ecSchemaName.clear();
    ecClassName.assign (str);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnResourceURI::PathParser::PathParser (Utf8StringCR str) : DgnResourceURI::ParserBase(str, "/?#") {;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnResourceURI::QueryParser::QueryParser (Utf8StringCR str) : DgnResourceURI::ParserBase(str, "&#") {;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnResourceURI::QueryParser::ParseQueryParameter (UriToken& propname, UriToken& propvalue, char& logical)
    {
    if (*m_curpos == '&')
        logical = '&';
    else
        logical = 0;

    // parse everything up to the = sign
    Utf8CP delims = m_tokenDelimiters;
    m_tokenDelimiters = "&#=";

    propname = ParseNextToken();

    propvalue = ParseNextToken();

    m_tokenDelimiters = delims;

    return (propname.GetType()  == UriToken::TYPE_EOS || propname.GetType() == UriToken::TYPE_Error)
        || (propvalue.GetType() == UriToken::TYPE_EOS || propvalue.GetType() == UriToken::TYPE_Error)? ERROR: SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECHelper
    {
    DgnProjectR m_file;

    ECHelper (DgnProjectR file) : m_file(file) {;}

    ECN::ECClassId FindECClassIdByName (Utf8CP ecSchemaName, Utf8CP ecClassName)
        {
        Statement stmt;
        stmt.Prepare (m_file, "SELECT ECClassId FROM ec_Class JOIN ec_Schema WHERE ec_Class.ECSchemaId = ec_Schema.ECSchemaId AND (ec_Schema.Name = ?1 OR ec_Schema.NamespacePrefix = ?1) AND ec_Class.Name = ?2");
        stmt.BindText (1, ecSchemaName, Statement::MAKE_COPY_No);
        stmt.BindText (2, ecClassName, Statement::MAKE_COPY_No);
        DbResult r = stmt.Step();
        if (BE_SQLITE_ROW != r)
            return (ECN::ECClassId) 0;

        return (ECN::ECClassId) stmt.GetValueInt64(0);
        }

    ECN::ECClassP FindECClassByName (Utf8CP ecSchemaName, Utf8CP ecClassName)
        {
        ECN::ECClassP ecClass = nullptr;
        if (SUCCESS == m_file.GetEC().GetSchemaManager().GetECClass (ecClass, ecSchemaName, ecClassName))
            return ecClass;
        else
            return nullptr;
        }

    ECInstanceId FindECInstanceByPropertyValue (ECN::ECClassCR ecClass, Utf8CP keyname, Utf8CP keyvalue)
        {
        ECSqlSelectBuilder ecSqlBuilder;
        SqlPrintfString whereStr ("%s='%s'", keyname, keyvalue);
        ecSqlBuilder.Select ("ECInstanceId").From (ecClass, false).Where(whereStr);
        ECSqlStatement ecStatement;
        if (ecStatement.Prepare (m_file, ecSqlBuilder.ToString ().c_str ()) != ECSqlStatus::Success || ecStatement.Step() != ECSqlStepStatus::HasRow)
            return ECInstanceId();

        return ecStatement.GetValueId<ECInstanceId> (0);
        }

    static WString GetEcKeyPropertyName (ECN::ECClassR ecClass, WCharCP customAttributeName)
        {
        ECN::IECInstancePtr ca = ecClass.GetCustomAttribute (customAttributeName);
        if (!ca.IsValid())
            return L"";
        ECN::ECValue v;
        ca->GetValue (v, L"PropertyName");
        if (v.IsNull())
            return L"";
        return v.GetString();
        }

    static Utf8String GetEcKeyPropertyName (ECN::ECClassR ecClass, DgnResourceURICreationStrategy strategy)
        {
        WString keyName;
    
        if ((strategy & DGN_RESOURCE_URI_CREATION_STRATEGY_ByBusinessKey) != 0)
            {
            keyName = GetEcKeyPropertyName (ecClass, L"BusinessKeySpecification");
            if (!keyName.empty())
                return Utf8String(keyName);
            }

        if ((strategy & DGN_RESOURCE_URI_CREATION_STRATEGY_ByGlobalId) != 0)
            {
            keyName = GetEcKeyPropertyName (ecClass, L"GlobalIdSpecification");
            if (!keyName.empty())
                return (Utf8String)keyName;
            }

        return "";
        }

    Utf8String GetEcPropertyValue (ECN::ECClassR ecClass, ECInstanceId ecInstanceId, Utf8CP propName)
        {
        ECSqlSelectBuilder ecSqlBuilder;
        SqlPrintfString whereStr ("ECInstanceId=%lld", ecInstanceId.GetValue());
        ecSqlBuilder.From (ecClass, false).Select(propName).Where(whereStr);
        ECSqlStatement ecStatement;
        if (ecStatement.Prepare (m_file, ecSqlBuilder.ToString ().c_str ()) != ECSqlStatus::Success || ecStatement.Step() != ECSqlStepStatus::HasRow)
            return "";
        return ecStatement.GetValueText(0);
        }

#if defined (NEEDS_WORK_DGNITEM)
    ElementId FindElementByPrimaryInstance (ECN::ECClassR ecClass, ECInstanceId ecInstanceId)
        {
        bvector<ElementId> eids;
        m_file.Models().GetElementIdsForECInstance (eids, ecClass.GetId(), ecInstanceId);
        if (eids.size() != 1)
            return ElementId();

        return eids.front();
        }
#endif

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::Builder::InitQuery ()
    {
    if (m_query.empty())
        m_query.append ("?");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnResourceURI DgnResourceURI::Builder::ToDgnResourceURI () const
    {
    DgnResourceURI uri;
    uri.FromEncodedString (ToEncodedString().c_str());
    return uri;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnResourceURI::Builder::ToEncodedString () const
    {
    return m_schemeAndTargetFile + m_path + m_query + m_fragment;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnResourceURI::Builder::MakeFullECClassName (WStringCR schemaName, WStringCR className)
    {
    Utf8String name (schemaName);
    name.append(":");
    name.append (Utf8String (className));
    return name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::Builder::SetTargetFile (DgnProjectR targetFile)
    {
    m_schemeAndTargetFile = SCHEME_DgnResourceURI ":/";
    m_schemeAndTargetFile.append (getEncodedTargetFile (targetFile));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::Builder::AppendEncodedPath (Utf8CP newSegment)
    {
    if (*newSegment != '/')
        m_path.append ("/");
    m_path.append (newSegment);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::Builder::AppendPath (Utf8CP newSegment)
    {
    AppendEncodedPath (BeStringUtilities::UriEncode (newSegment).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::Builder::AppendPath (Utf8StringCR newSegment)
    {
    AppendPath (newSegment.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::Builder::SetEncodedPath (Utf8CP newPath)
    {
    m_path = newPath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::Builder::AppendQueryParameter (Utf8CP key, Utf8CP value)
    {
    InitQuery();
    BeAssert (*key != '?');
    m_query.append(BeStringUtilities::UriEncode(key)).append("=").append(BeStringUtilities::UriEncode(value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::Builder::AppendQueryParameter (Utf8StringCR key, Utf8StringCR value)
    {
    AppendQueryParameter (key.c_str(), value.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::Builder::AppendAndOperator ()
    {
    BeAssert (!m_query.empty());
    m_query.append ("&");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::Builder::SetEncodedQuery (Utf8CP newQuery)
    {
    if (*newQuery == '?')
        m_query = newQuery;
    else
        {
        InitQuery();
        m_query.append (newQuery);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::Builder::SetEncodedFragment (Utf8CP newFragment)
    {
    m_fragment = newFragment;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct URIBuilder
    {
    DgnProjectR m_file;
    DgnResourceURICreationStrategy m_strategy;

    URIBuilder (DgnProjectR f, DgnResourceURICreationStrategy s): m_file(f), m_strategy(s) {;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus MakeInstanceByPropertyValue (DgnResourceURI& uri, ECN::ECClassR ecClass, ECInstanceId ecInstanceId)
        {
        Utf8String className = DgnResourceURI::Builder::MakeFullECClassName (ecClass.GetSchema().GetName(), ecClass.GetName());
        
        ECHelper echelper (m_file);

        Utf8String keyName = echelper.GetEcKeyPropertyName (ecClass, m_strategy);
        if (keyName.empty())
            return ERROR;

        Utf8String keyValue = echelper.GetEcPropertyValue (ecClass, ecInstanceId, keyName.c_str());
        if (keyName.empty())
            return ERROR;

        DgnResourceURI::Builder builder;
        builder.AppendEncodedPath (RT_ECInstances);
        builder.AppendPath (className);
        builder.AppendQueryParameter (keyName, keyValue);

        uri = builder.ToDgnResourceURI();

        return SUCCESS;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus MakeInstanceByECInstanceId (DgnResourceURI& uri, ECN::ECClassR ecClass, ECInstanceId ecInstanceId)
        {
        Utf8String className = DgnResourceURI::Builder::MakeFullECClassName (ecClass.GetSchema().GetName(), ecClass.GetName());
        
        ECHelper echelper (m_file);

        DgnResourceURI::Builder builder;
        builder.AppendEncodedPath (RT_ECInstances);
        builder.AppendPath (className);
        builder.AppendPath ((Utf8CP)Utf8PrintfString("%lld", ecInstanceId.GetValue()));

        uri = builder.ToDgnResourceURI();

        return SUCCESS;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus MakeElementByPrimaryInstancePropertyValue (DgnResourceURI& uri, ECN::ECClassId ecClassId, ECInstanceId ecInstanceId)
        {
        ECN::ECClassP ecClass = nullptr;
        if (m_file.GetEC().GetSchemaManager().GetECClass (ecClass, ecClassId) != SUCCESS)
            return ERROR;

        Utf8String className = DgnResourceURI::Builder::MakeFullECClassName (ecClass->GetSchema().GetName(), ecClass->GetName());

        ECHelper echelper (m_file);
        Utf8String keyName = echelper.GetEcKeyPropertyName (*ecClass, m_strategy);
        if (keyName.empty())
            return ERROR;

        Utf8String keyValue = echelper.GetEcPropertyValue (*ecClass, ecInstanceId, keyName.c_str());
        if (keyName.empty())
            return ERROR;

        DgnResourceURI::Builder builder;
        builder.AppendEncodedPath (RT_DgnElements);
        builder.AppendQueryParameter ("ECClass", className);
        builder.AppendAndOperator();
        builder.AppendQueryParameter (keyName, keyValue);

        uri = builder.ToDgnResourceURI();

        return SUCCESS;
        }

    /*----------------------------------------------------------------------------------*//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus MakeElementByProvenance (DgnResourceURI& uri, ElementId eid)
        {
        Utf8String oldFileName;
        DgnProvenances prov = m_file.Provenance();

        ElementProvenance eprov = prov.QueryProvenance(eid);
        if (!eprov.IsValid() || SUCCESS != prov.QueryFileName (oldFileName, eprov.GetOriginalFileId()))
            return ERROR;

        DgnResourceURI::Builder builder;
        builder.AppendEncodedPath (RT_DgnElements);
        builder.AppendQueryParameter ("SourceId", oldFileName);
        builder.AppendAndOperator();
        builder.AppendQueryParameter ("ElementId", (Utf8CP)Utf8PrintfString("%lld", eprov.GetOriginalElementId()));
    
        uri = builder.ToDgnResourceURI();
        return SUCCESS;
        }

    /*----------------------------------------------------------------------------------*//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus MakeElementById (DgnResourceURI& uri, ElementId eid)
        {
        DgnResourceURI::Builder builder;
        builder.AppendEncodedPath (RT_DgnElements);
        builder.AppendEncodedPath ((Utf8CP)Utf8PrintfString("%lld", eid.GetValue()));
    
        uri = builder.ToDgnResourceURI();

        return SUCCESS;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP DgnResourceURI::GetWellKnownResourceType (WellKnownResourceType t) 
    {
    switch (t)
        {
        case WELL_KNOWN_RESOURCE_TYPE_DgnElements: return RT_DgnElements;
        case WELL_KNOWN_RESOURCE_TYPE_ECInstances: return RT_ECInstances;
        }
    BeAssert (false);
    return "";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnResourceURI::DgnResourceURI ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnResourceURI::ToEncodedString() const 
    {
    Utf8String uriRef (m_path);
    uriRef.append (m_query);
    uriRef.append (m_fragment);
    
    if (m_targetFile.empty())
        return uriRef;

    Utf8String uri (SCHEME_DgnResourceURI ":/");
    uri.append (m_targetFile);
    uri.append (uriRef);
    return uri;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnResourceURI::ParseStatus DgnResourceURI::FromEncodedString (Utf8CP uriString)
    {
    m_path.clear();

    //  Pull off the optional scheme:/filename;fileguid prefix
    Utf8CP s = strstr (uriString, SCHEME_DgnResourceURI ":/");
    if (s != nullptr)
        {
        uriString += SCHEME_DgnResourceURI_Length + 2;  // skip over scheme:/
        // make sure target file is also in there! I hope that detecting a semicolon (%3B) is enough.
        Utf8CP semicolon = strstr (uriString, "%3B");
        if (semicolon == nullptr)
            return PARSE_ERROR_InvalidSyntax;
        Utf8CP slash = strstr (semicolon+3, "/");
        if (slash == nullptr)
            return PARSE_ERROR_InvalidSyntax;
        m_targetFile.assign (uriString, slash);
        uriString = slash;
        }

    //  Parse the path and optional query
    Utf8CP q = strstr (uriString, "?");
    if (q == nullptr)
        {
        m_path = uriString;
        if (*m_path.rbegin() == '/')     // drop trailing /
            m_path.resize (m_path.length() - 1);
        m_query.clear();
        }
    else
        {
        m_path.assign (uriString, q);
        m_query.assign(q);
        }

    if (m_path[0] != '/')
        m_path.insert (m_path.begin(), '/');

    //  Parse the optional fragment
    Utf8CP f = strstr (uriString, "#");
    if (f != nullptr)
        m_fragment = f;
    else
        m_fragment.clear();

    return PARSE_STATUS_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::SetTargetFile (DgnProjectR targetFile)
    {
    m_targetFile = getEncodedTargetFile (targetFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnResourceURI::GetTargetFile (Utf8StringR filename, BeGuid& guid) const
    {
    if (m_targetFile.empty())
        return ERROR;

    Utf8String targetFileDecoded = BeStringUtilities::UriDecode (m_targetFile.c_str());

    size_t semicolon = targetFileDecoded.find (";");
    BeAssert (semicolon != Utf8String::npos && "Invalid target file string was accepted by FromEncodedString");

    filename.assign (targetFileDecoded.substr (0, semicolon));

    guid.FromString (targetFileDecoded.substr (semicolon+1).c_str());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnResourceURI::CheckTargetFileMatches (DgnProjectR file) const
    {
    Utf8String filename;
    BeGuid guid;
    if (GetTargetFile (filename, guid) != SUCCESS)
        return SUCCESS;

    return (guid == file.GetDbGuid())? SUCCESS: ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnResourceURI::GetPath() const
    {
    return m_path;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnResourceURI::GetQuery() const
    {
    return m_query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnResourceURI::GetFragment() const
    {
    return m_fragment;
    }

