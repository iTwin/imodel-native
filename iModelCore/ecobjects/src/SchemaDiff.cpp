/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SchemaDiff.cpp $
|
|   $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ECObjects/SchemaDiff.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

const WCharCP	SchemaDiffType::ECDIFF = L"@ECDiff@";
const WCharCP	SchemaDiffType::NAME = L"Name";
const WCharCP	SchemaDiffType::DISPLAY_LABEL = L"DisplayLabel";
const WCharCP	SchemaDiffType::DESCRIPTION = L"Description";
const WCharCP	SchemaDiffType::VERSION_MAJOR = L"VersionMajor";
const WCharCP	SchemaDiffType::VERSION_MINOR = L"VersionMinor";
const WCharCP	SchemaDiffType::CLASSES = L"Classes";
const WCharCP	SchemaDiffType::REFERENCES = L"References";
const WCharCP	SchemaDiffType::IS_CUSTOMATTRIBUTE_CLASS = L"IsCustomAttributeClass";
const WCharCP	SchemaDiffType::IS_STRUCT = L"IsStruct";
const WCharCP	SchemaDiffType::IS_DOMAIN_CLASS = L"IsDomainClass";
const WCharCP	SchemaDiffType::IS_RELATIONSHIP_CLASS = L"IsRelationshipClass";
const WCharCP	SchemaDiffType::BASECLASSES = L"BaseClasses";
const WCharCP	SchemaDiffType::PROPERTIES = L"Properties";
const WCharCP	SchemaDiffType::IS_ARRAY = L"IsArray";
const WCharCP	SchemaDiffType::TYPENAME = L"TypeName";
const WCharCP	SchemaDiffType::ARRAYINFO = L"ArrayInfo";
const WCharCP	SchemaDiffType::IS_READONLY = L"IsReadOnly";
const WCharCP	SchemaDiffType::IS_PRIMITIVE = L"IsPrimitive";
const WCharCP	SchemaDiffType::RELATIONSHIP_INFO = L"RelationshipInfo";
const WCharCP	SchemaDiffType::IS_OVERRIDEN = L"IsOverriden";
const WCharCP	SchemaDiffType::STRENGTH = L"Strength";
const WCharCP	SchemaDiffType::STRENGTH_DIRECTION = L"StrengthDirection";
const WCharCP	SchemaDiffType::SOURCE = L"Source";
const WCharCP	SchemaDiffType::TARGET = L"Target";
const WCharCP	SchemaDiffType::CARDINALITY = L"Cardinality";
const WCharCP	SchemaDiffType::IS_POLYMORPHIC = L"IsPolymorphic";
const WCharCP	SchemaDiffType::ROLE_LABEL = L"RoleLabel";
const WCharCP	SchemaDiffType::STRENGTH_DIRECTION_BACKWARD = L"Backward";
const WCharCP	SchemaDiffType::STRENGTH_DIRECTION_FORWARD = L"Forward";
const WCharCP	SchemaDiffType::STRENGTH_TYPE_EMBEDDING = L"Embedding";
const WCharCP	SchemaDiffType::STRENGTH_TYPE_HOLDING = L"Holding";
const WCharCP	SchemaDiffType::STRENGTH_TYPE_REFERENCING = L"Referencing";
const WCharCP	SchemaDiffType::MAXOCCURS = L"MaxOccurs";
const WCharCP	SchemaDiffType::MINOCCURS = L"MinOccurs";
const WCharCP	SchemaDiffType::CUSTOMATTRIBUTES = L"CustomAttributes";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffValue::ECDiffValue()
    : m_type (VALUETYPE_Nil), m_valueInt64(0),m_binarySize(0)
    {
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffValue::SetValue (WCharCP v) 
    {
    if (v == NULL)
        m_valueString = L"<nil>";
    else
        m_valueString = v ; 
    m_type = VALUETYPE_String; 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffValue::SetValue (Byte const* v, size_t size) 
    { 
    m_type = VALUETYPE_Binary;
    m_valueBinary = (Byte*)malloc(size);
    memcpy (m_valueBinary, v, size);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffValue::SetValue (WStringCR v) { m_valueString = v ; m_type = VALUETYPE_String; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffValue::SetValue (int32_t v) { m_valueInt32 = v ; m_type = VALUETYPE_Int32; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffValue::SetValue (int64_t v) { m_valueInt64 = v ; m_type = VALUETYPE_Int64; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffValue::SetValue (double v) { m_valueDouble = v ; m_type = VALUETYPE_Double; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffValue::SetValue (bool v) { m_valueBool = v ; m_type = VALUETYPE_Boolean; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffValue::SetDateTimeValue(int64_t v) { m_valueDateTime = v ; m_type = VALUETYPE_DateTimeTicks; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffValue::SetNil()
    {
    m_valueInt64 = 0 , m_valueString.clear(); 
    m_type = VALUETYPE_Nil;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Byte const* ECDiffValue::GetBinary(size_t& size) const
    { 
    BeAssert(m_type == VALUETYPE_Binary); 
    size= m_binarySize;
    return m_valueBinary;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP ECDiffValue::GetValueWCharCP() const
    { 
    BeAssert(m_type == VALUETYPE_String); 
    return m_valueString.c_str();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t ECDiffValue::GetValueInt32() const
    { 
    BeAssert(m_type == VALUETYPE_Int32); 
    return m_valueInt32;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t ECDiffValue::GetValueInt64() const
    { 
    BeAssert(m_type == VALUETYPE_Int64); 
    return m_valueInt64;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDiffValue::GetValueBool() const
    { 
    BeAssert(m_type == VALUETYPE_Boolean); 
    return m_valueBool;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
double ECDiffValue::GetValueDouble() const
    { 
    BeAssert(m_type == VALUETYPE_Double); 
    return m_valueDouble;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDiffValue::IsValueNill() const { return m_type == VALUETYPE_Nil; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffValue::Clear()
    {
    if (m_type == ECDiffValue::VALUETYPE_Binary)
        free(m_valueBinary);
    m_type = VALUETYPE_Nil;
    m_valueString.clear();
    m_valueInt64 = 0;
    m_binarySize = 0;
    m_valueBinary = NULL;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDiffValue::IsEqual(ECDiffValue const& value)
    {
    if (m_type != value.m_type)
        return false;
    switch(m_type)
        {
    case VALUETYPE_Boolean:
        return m_valueBool == value.m_valueBool;
    case VALUETYPE_Double:
        return m_valueDouble == value.m_valueDouble;
    case VALUETYPE_Int32:
        return m_valueInt32 == value.m_valueInt32;
    case VALUETYPE_Int64:
        return m_valueInt64 == value.m_valueInt64;
    case VALUETYPE_String:
        return m_valueString == value.m_valueString;
    case VALUETYPE_Nil:
        return true;
        }
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECDiffValue::ToString() const
    {
    WString out;
    switch (m_type)
        {
    case VALUETYPE_Boolean:
        out.Sprintf (L"%ls", m_valueBool?L"true" :L"false"); break;
    case VALUETYPE_Double:
        out.Sprintf (L"%f", m_valueDouble); break;
    case VALUETYPE_Int32:
        out.Sprintf (L"%d", m_valueInt32); break;
    case VALUETYPE_Int64:
        out.Sprintf (L"%lld", m_valueInt64); break;
    case VALUETYPE_String:
        out.Sprintf (L"\"%ls\"", m_valueString.c_str()); break;
    case VALUETYPE_Nil:
        out = L""; break;
        }
    return out;
    }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Affan.Khan      02/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffNode::Write (WStringR out, int indent) const
    {
    WString prefix;
    WString eol = L"\r\n";
    int i = indent;
    while (i-- >= 1)
        prefix.append(L"   ");
    indent = indent + 1;
    //Left value is new
    if (!m_valueLeft.IsValueNill() && m_valueRight.IsValueNill())
        {
        out.append (L"L");
        out.append (prefix);
        out.append (m_name);
        out.append (L" : ");
        out.append (m_valueLeft.ToString());
        out.append (eol);
        }
    //Right value is new
    else if (m_valueLeft.IsValueNill() && !m_valueRight.IsValueNill())
        {
        out.append (L"R");
        out.append (prefix);
        out.append (m_name);
        out.append (L" : ");
        out.append (m_valueRight.ToString());
        out.append (eol);
        }
    //Conflict
    else if (!m_valueLeft.IsValueNill() && !m_valueRight.IsValueNill())
        {
        out.append (L"!");
        out.append (prefix);
        out.append (m_name);
        out.append (L" : ");
        out.append (m_valueLeft.ToString());
        out.append (L" <=> ");
        out.append (m_valueRight.ToString());
        out.append (eol);
        }
    else // this is a object output its child nodes
        {
        out.append (L" ");
        out.append (prefix);
        out.append (m_name);
        out.append (L" : ");
        out.append (eol);
        for (const_iterator itor = m_childNodeList.begin(); itor != m_childNodeList.end(); ++itor)
            {
            (*itor)->Write (out, indent);
            }
        //out.append (prefix);
        //out.append (L"}");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECDiffNode::Find (WCharCP name)
    {
    ECDiffNodeMap::const_iterator itor = m_childNodeMap.find (name);
    if (itor != m_childNodeMap.end())
        return itor->second;
    return NULL;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNode::ECDiffNode (WCharCP name, ECDiffNodeP parent) 
    : m_parent(parent)
    {
    BeAssert(name != NULL);
    m_name = name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNode::ECDiffNode() : m_name (SchemaDiffType::ECDIFF), m_parent(NULL) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECDiffNode::GetAccessString() const
    {
    if (GetParent() == NULL)
        return GetName();
    return GetParent()->GetName() + L"." + GetName();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffValue& ECDiffNode::GetValueLeft() {return m_valueLeft; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffValue& ECDiffNode::GetValueRight() {return m_valueRight; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffValue& ECDiffNode::GetValue (ValueDirection direction) { return direction== DIRECTION_Left ? GetValueLeft() : GetValueRight();}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDiffNode::SetValue (WCharCP left, WCharCP right)
    {
    if ( left || right )
        {
        m_valueLeft.SetValue (left);
        m_valueRight.SetValue (right);
        return true;
        }
    BeAssert( false && "left and right both values are null");
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDiffNode::SetValue (bool left, bool right)
    {
    m_valueLeft.SetValue (left);
    m_valueRight.SetValue (right);
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDiffNode::SetValue (uint32_t left, uint32_t right)
    {
    m_valueLeft.SetValue ((int32_t)left);
    m_valueRight.SetValue ((int32_t)right);
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNode::const_iterator ECDiffNode::begin() { return m_childNodeList.begin(); }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNode::const_iterator ECDiffNode::end() { return m_childNodeList.end(); }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNode::size_type ECDiffNode::size () { return m_childNodeList.size(); }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeCP ECDiffNode::GetParent() const { return m_parent ; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ECDiffNode::GetName() const { return m_name; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECDiffNode::Add (WCharCP name)
    {
    if (m_childNodeMap.find (name) != m_childNodeMap.end())
        {
        BeAssert(false && "ECDiffNode with same name already exist");
        return NULL;
        }
    ECDiffNodeP node = new ECDiffNode (name, this);
    m_childNodeMap [node->GetName().c_str()] = node;
    m_childNodeList.push_back(node);
    return node;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECDiffNode::GetChild (WStringCR accessString, bool bCreate)
    {
    WString cur, rest;
    size_t n = accessString.find (L".");
    if (n == WString::npos)
        cur = accessString;
    else
        {
        cur = accessString.substr (0, n);
        rest = accessString.substr (n + 1);
        }
    ECDiffNodeMap::const_iterator itor = m_childNodeMap.find (cur.c_str());
    if (itor != m_childNodeMap.end())
        if (rest.empty())
            return itor->second;
        else
            return itor->second->GetChild (rest, bCreate);
    if (bCreate)
        if (rest.empty())
            return Add (cur.c_str());
        else
            return Add (cur.c_str())->GetChild (rest, bCreate);
    return NULL;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECDiffNode::Add (int index)
    {
    WString idx;
    idx.Sprintf (L"%ls[%d]", m_name.c_str(), index);
    if (m_childNodeMap.find (idx.c_str()) != m_childNodeMap.end())
        {
        BeAssert(false && "ECDiffNode with same name already exist");
        return NULL;
        }
    ECDiffNodeP node = new ECDiffNode (idx.c_str(), this);
    m_childNodeMap [node->GetName().c_str()] = node;
    m_childNodeList.push_back(node);
    return node;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffNode::Clear()
    {
    m_childNodeMap.clear();
    for (ECDiffNodeList::const_iterator itor = m_childNodeList.begin(); itor != m_childNodeList.end(); ++itor)
        delete *itor;
    m_childNodeList.clear();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECDiffNode::RemoveIfEmpty (ECDiffNodeP n)
    {
    BeAssert(n != NULL && n->GetParent() == this);
    if (n->IsEmpty())
        {
        Remove(n);
        return NULL;
        }
    return n;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDiffNode::Remove (ECDiffNodeP n)
    {
    for (ECDiffNodeList::iterator itor = m_childNodeList.begin(); itor != m_childNodeList.end(); ++itor)
        if (*itor == n)
            {
                m_childNodeList.erase(itor);
                break;
            }
    m_childNodeMap.erase(n->GetName().c_str());
    delete n;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECDiffNode::ToString() const
    {
    WString out;
    Write (out, 0);
    return out;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNode::~ECDiffNode()
    {
    Clear();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDiffNode::IsEmpty()
    {
    return size() == 0 && GetValueLeft().IsValueNill() && GetValueRight().IsValueNill();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchemaDiff::DiffSchema (ECDiffNode& diff, ECSchemaCR left, ECSchemaCR right)
    {
    if (left.GetName() != right.GetName())
        diff.Add (SchemaDiffType::NAME)->SetValue (left.GetName().c_str(), right.GetName().c_str());

    if (left.GetVersionMajor() != right.GetVersionMajor())
        diff.Add (SchemaDiffType::VERSION_MAJOR)->SetValue (left.GetVersionMajor(), right.GetVersionMajor());

    if (left.GetVersionMinor() != right.GetVersionMinor())
        diff.Add (SchemaDiffType::VERSION_MINOR)->SetValue (left.GetVersionMinor(), right.GetVersionMinor());

    if (left.GetDisplayLabel() != right.GetDisplayLabel())
        diff.Add (SchemaDiffType::DISPLAY_LABEL)->SetValue (left.GetIsDisplayLabelDefined()? left.GetDisplayLabel().c_str() : NULL, right.GetIsDisplayLabelDefined()? right.GetDisplayLabel().c_str(): NULL);

    if (left.GetDescription() != right.GetDescription())
        diff.Add (SchemaDiffType::DESCRIPTION)->SetValue (left.GetDescription().c_str(), right.GetDescription().c_str());
    DiffReferences (diff, left, right);
    DiffCustomAttributes (diff, left, right);
    bset<WCharCP, CompareIWChar> classes;
    ECClassContainerCR classesLeft = left.GetClasses();
    for (ECClassContainer::const_iterator itor = classesLeft.begin(); itor != classesLeft.end(); ++itor)
        if (classes.find ((*itor)->GetName().c_str()) == classes.end())
            classes.insert ((*itor)->GetName().c_str());

    ECClassContainerCR classesRight = right.GetClasses();
    for (ECClassContainer::const_iterator itor = classesRight.begin(); itor != classesRight.end(); ++itor)
        if (classes.find ((*itor)->GetName().c_str()) == classes.end())
            classes.insert ((*itor)->GetName().c_str());

    ECDiffNodeP diffClasses = diff.Add (SchemaDiffType::CLASSES);
    for (bset<WCharCP,  CompareIWChar>::const_iterator itor = classes.begin(); itor != classes.end(); ++itor)
        DiffClass ((*itor), left, right, *diffClasses);

    if (diffClasses->IsEmpty())
        diff.Remove(diffClasses);

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiff::DiffReferences(ECDiffNodeR parentDiff, ECSchemaCR schemaLeft, ECSchemaCR schemaRight)
        {
        ECSchemaReferenceListCR left = schemaLeft.GetReferencedSchemas();
        ECSchemaReferenceListCR right = schemaRight.GetReferencedSchemas();
   
        bset<WString> referenceSchemas;
        WString namespacePrefix;
        for (ECSchemaReferenceList::const_iterator itor = left.begin(); itor != left.end() ; ++itor)
            if (schemaLeft.ResolveNamespacePrefix (*itor->second, namespacePrefix) == ECOBJECTS_STATUS_Success)
                if (referenceSchemas.find (namespacePrefix) == referenceSchemas.end())
                    referenceSchemas.insert(namespacePrefix);

        for (ECSchemaReferenceList::const_iterator itor = right.begin(); itor != right.end() ; ++itor)
            if (schemaRight.ResolveNamespacePrefix (*itor->second, namespacePrefix) == ECOBJECTS_STATUS_Success)
                if (referenceSchemas.find (namespacePrefix) == referenceSchemas.end())
                    referenceSchemas.insert(namespacePrefix);

        if (referenceSchemas.empty())
            return NULL;
    ECDiffNodeP diff= parentDiff.Add (SchemaDiffType::REFERENCES);
        for(bset<WString>::const_iterator itor = referenceSchemas.begin(); itor != referenceSchemas.end(); ++itor)
            {
            ECSchemaCP leftR =  schemaLeft.GetSchemaByNamespacePrefixP (*itor);
            ECSchemaCP rightR =  schemaRight.GetSchemaByNamespacePrefixP (*itor);
            if (leftR && !rightR)
                diff->Add ((*itor).c_str())->GetValueLeft().SetValue (leftR->GetFullSchemaName().c_str());
            else if (!leftR && rightR)
                diff->Add ((*itor).c_str())->GetValueRight().SetValue (rightR->GetFullSchemaName().c_str());
            else
                if (leftR->GetSchemaKey() != rightR->GetSchemaKey())
                    diff->Add ((*itor).c_str())->SetValue (leftR->GetFullSchemaName().c_str(), rightR->GetFullSchemaName().c_str());
            }
        return parentDiff.RemoveIfEmpty (diff);
        }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiff::DiffClass (WCharCP className, ECSchemaCR schemaLeft, ECSchemaCR schemaRight, ECDiffNodeR parentDiff)
    {
    ECClassCP left = schemaLeft.GetClassCP(className);
    ECClassCP right = schemaRight.GetClassCP(className);
 
    if (left && !right)
        return AppendClass (parentDiff, *left, ECDiffNode::DIRECTION_Left);
    if (right && !left)
        return AppendClass (parentDiff, *right, ECDiffNode::DIRECTION_Right);

    ECDiffNodeP diff = parentDiff.Add (className);
    //if (left->GetName() != right->GetName())
    //    diff->Add (SchemaDiffType::NAME)->SetValue (left->GetName().c_str(), right->GetName().c_str());

    if (left->GetDisplayLabel() != right->GetDisplayLabel())
        diff->Add (SchemaDiffType::DISPLAY_LABEL)->SetValue (left->GetIsDisplayLabelDefined()? left->GetDisplayLabel().c_str() : NULL, right->GetIsDisplayLabelDefined()? right->GetDisplayLabel().c_str(): NULL);

    if (left->GetDescription() != right->GetDescription())
        diff->Add (SchemaDiffType::DESCRIPTION)->SetValue (left->GetDescription().c_str(), right->GetDescription().c_str());

    if (left->GetIsCustomAttributeClass() != right->GetIsCustomAttributeClass())
        diff->Add (SchemaDiffType::IS_CUSTOMATTRIBUTE_CLASS)->SetValue (left->GetIsCustomAttributeClass(), right->GetIsCustomAttributeClass());

    if (left->GetIsStruct() != right->GetIsStruct())
        diff->Add (SchemaDiffType::IS_STRUCT)->SetValue (left->GetIsStruct(), right->GetIsStruct());

    if (left->GetIsDomainClass() != right->GetIsDomainClass())
        diff->Add (SchemaDiffType::IS_DOMAIN_CLASS)->SetValue (left->GetIsDomainClass(), right->GetIsDomainClass());

    if (( (left->GetRelationshipClassCP() != NULL))!= (right->GetRelationshipClassCP() != NULL))
        diff->Add (SchemaDiffType::IS_RELATIONSHIP_CLASS)->SetValue (left->GetRelationshipClassCP() != NULL, right->GetRelationshipClassCP() != NULL );

    DiffCustomAttributes (*diff, *left, *right);
    AlignedClasses classes; 
    classes.push_back(left);
    classes.push_back(right);
    DiffBaseClasses(*diff, classes);

    bset<WCharCP, CompareIWChar> properties;
    ECPropertyIterableCR propertiesLeft = left->GetProperties(false);
    for (ECPropertyIterable::const_iterator itor = propertiesLeft.begin(); itor != propertiesLeft.end(); ++itor)
        if (properties.find ((*itor)->GetName().c_str()) == properties.end())
            properties.insert ((*itor)->GetName().c_str());

    ECPropertyIterableCR propertiesRight = right->GetProperties(false);
    for (ECPropertyIterable::const_iterator itor = propertiesRight.begin(); itor != propertiesRight.end(); ++itor)
        if (properties.find ((*itor)->GetName().c_str()) == properties.end())
            properties.insert ((*itor)->GetName().c_str());

    ECDiffNodeP diffProperties = diff->Add (SchemaDiffType::PROPERTIES);
    for (bset<WCharCP,  CompareIWChar>::const_iterator itor = properties.begin(); itor != properties.end(); ++itor)
        DiffProperty ((*itor), *left, *right, *diffProperties);
    diff->RemoveIfEmpty (diffProperties);

    DiffRelationship (*left, *right, *diff);
    return parentDiff.RemoveIfEmpty (diff);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiff::DiffBaseClasses (ECDiffNodeR parentDiff, AlignedClasses const& classes)
    {
    BeAssert (classes.size() == 2);
    AlignedClassMap baseClassMap;
    bvector<WCharCP> baseClassOrderedList;
    for (size_t i = 0 ; i < classes.size(); i++)
        {
        ECClassCP ecclass = classes[i];
        if (ecclass == NULL)
            continue;

        ECBaseClassesList baseClassContainer = ecclass->GetBaseClasses();
        for (ECBaseClassesList::const_iterator baseClassItor = baseClassContainer.begin(); baseClassItor != baseClassContainer.end(); ++baseClassItor)
            {
            ECClassCR baseClass = *(*baseClassItor);
            if (baseClassMap.find (baseClass.GetFullName())== baseClassMap.end())
                baseClassOrderedList.push_back (baseClass.GetFullName());
            AlignedClasses& baseClasses = baseClassMap [baseClass.GetFullName()];
            size_t nNullEntriesRequired = i - baseClasses.size();
            if (nNullEntriesRequired > 0)
                do
                    {
                    baseClasses.push_back (NULL);
                    } while (--nNullEntriesRequired);
                baseClasses.push_back (&baseClass);
            }
        }
    for (bvector<WCharCP>::iterator itor = baseClassOrderedList.begin(); itor != baseClassOrderedList.end(); ++itor)
        if (baseClassMap[*itor].size() < classes.size())
            while (baseClassMap[*itor].size() < classes.size())
                baseClassMap[*itor].push_back(NULL);

    if (baseClassOrderedList.empty())
        return NULL;

    ECDiffNodeP baseClasses = parentDiff.Add (SchemaDiffType::BASECLASSES);
    int index = 0;
    for (bvector<WCharCP>::iterator itor = baseClassOrderedList.begin(); itor != baseClassOrderedList.end(); ++itor, index++)
        {
        ECClassCP left = baseClassMap[*itor][0];
        ECClassCP right = baseClassMap[*itor][1];
        if (left && !right)
            baseClasses->Add (index)->GetValueLeft().SetValue(left->GetFullName());
        else if(!left && right)
            baseClasses->Add (index)->GetValueRight().SetValue(right->GetFullName());
        else
            if (wcscmp (left->GetFullName(), right->GetFullName()) != 0)
                baseClasses->Add (index)->SetValue(left->GetFullName(), right->GetFullName());
        }
    return parentDiff.RemoveIfEmpty (baseClasses);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiff::DiffProperty (WCharCP propertyName, ECClassCR classLeft, ECClassCR classRight, ECDiffNodeR parentDiff)
    {
    ECPropertyCP left = classLeft.GetPropertyP (propertyName);
    ECPropertyCP right = classRight.GetPropertyP (propertyName);
        
    if (left && !right)
        return AppendProperty (parentDiff, *left, ECDiffNode::DIRECTION_Left);
    if (right && !left)
        return AppendProperty (parentDiff, *right, ECDiffNode::DIRECTION_Right);
    ECDiffNodeP diff = parentDiff.Add (propertyName);

    //if (left->GetName() != right->GetName())
    //    diff->Add (SchemaDiffType::NAME)->SetValue (left->GetName().c_str(), right->GetName().c_str());

    if (left->GetDisplayLabel() != right->GetDisplayLabel())
        diff->Add (SchemaDiffType::DISPLAY_LABEL)->SetValue (left->GetIsDisplayLabelDefined()? left->GetDisplayLabel().c_str() : NULL, right->GetIsDisplayLabelDefined()? right->GetDisplayLabel().c_str(): NULL);

    if (left->GetDescription() != right->GetDescription())
        diff->Add (SchemaDiffType::DESCRIPTION)->SetValue (left->GetDescription().c_str(), right->GetDescription().c_str());

    if (left->GetIsArray() != right->GetIsArray())
        diff->Add (SchemaDiffType::IS_ARRAY)->SetValue (left->GetIsArray(), right->GetIsArray());

    if (left->GetIsStruct() != right->GetIsStruct())
        diff->Add (SchemaDiffType::IS_STRUCT)->SetValue (left->GetIsStruct(), right->GetIsStruct());

    if (left->GetTypeName() != right->GetTypeName())
        diff->Add (SchemaDiffType::TYPENAME)->SetValue (left->GetTypeName().c_str(), right->GetTypeName().c_str());

    if (left->GetIsReadOnly() != right->GetIsReadOnly())
        diff->Add (SchemaDiffType::IS_READONLY)->SetValue (left->GetIsReadOnly(), right->GetIsReadOnly());

    if (left->GetIsPrimitive() != right->GetIsPrimitive())
        diff->Add (SchemaDiffType::IS_PRIMITIVE)->SetValue (left->GetIsPrimitive(), right->GetIsPrimitive());

    bool leftOverriden = left->GetBaseProperty() != NULL;
    bool rightOverriden = right->GetBaseProperty() != NULL;
    if (leftOverriden != rightOverriden)
        diff->Add (SchemaDiffType::IS_OVERRIDEN)->SetValue (leftOverriden, rightOverriden);

    DiffArrayBounds(*diff, *left, *right);
    DiffCustomAttributes (*diff, *left, *right);
    return parentDiff.RemoveIfEmpty (diff);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiff::DiffRelationship (ECClassCR classLeft, ECClassCR classRight, ECDiffNodeR parentDiff)
    {
    ECRelationshipClassCP left = classLeft.GetRelationshipClassCP();
    ECRelationshipClassCP right = classRight.GetRelationshipClassCP();
    if (!left && !right)
        return NULL;
    else if (left && !right)
        return AppendRelationship(parentDiff, *left, ECDiffNode::DIRECTION_Left );
    else if (!left && right)
        return AppendRelationship(parentDiff, *right, ECDiffNode::DIRECTION_Right );
        
    ECDiffNodeP diff = parentDiff.Add (SchemaDiffType::RELATIONSHIP_INFO);

    if (left->GetStrength() != right->GetStrength())
        diff->Add (SchemaDiffType::STRENGTH)->SetValue (ToString (left->GetStrength()), ToString (right->GetStrength()));

    if (left->GetStrengthDirection() != right->GetStrengthDirection())
        diff->Add (SchemaDiffType::STRENGTH_DIRECTION)->SetValue (ToString (left->GetStrengthDirection()), ToString (right->GetStrengthDirection()));

    DiffRelationshipConstraint(*diff, left->GetSource(), right->GetSource(), ECRelationshipEnd_Source);
    DiffRelationshipConstraint(*diff, left->GetTarget(), right->GetTarget(), ECRelationshipEnd_Target);
    return parentDiff.RemoveIfEmpty (diff);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiff::DiffRelationshipConstraint(ECDiffNodeR parent, ECRelationshipConstraintCR  left, ECRelationshipConstraintCR  right, ECRelationshipEnd endPoint)
    {
    ECDiffNodeP diff = endPoint == ECRelationshipEnd_Source ? parent.Add (SchemaDiffType::SOURCE) : parent.Add (SchemaDiffType::TARGET);

    if (left.GetCardinality().ToString() != right.GetCardinality().ToString())
        diff->Add (SchemaDiffType::CARDINALITY)->SetValue (left.GetCardinality().ToString().c_str(), right.GetCardinality().ToString().c_str());

    if (left.GetIsPolymorphic() != right.GetIsPolymorphic())
        diff->Add (SchemaDiffType::IS_POLYMORPHIC)->SetValue (left.GetIsPolymorphic(), right.GetIsPolymorphic());

    if (left.GetRoleLabel() != right.GetRoleLabel())
        diff->Add (SchemaDiffType::ROLE_LABEL)->SetValue (left.GetRoleLabel().c_str(), right.GetRoleLabel().c_str());

    DiffCustomAttributes (*diff, left, right);
    bvector<ECRelationshipConstraintCP> constraints;
    constraints.push_back (&left);
    constraints.push_back (&right);
    DiffRelationshipConstraintClasses(*diff, constraints, endPoint);
    return parent.RemoveIfEmpty (diff);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiff::DiffRelationshipConstraintClasses (ECDiffNodeR parentDiff, bvector<ECRelationshipConstraintCP> const& constraints, ECRelationshipEnd endPoint)
    {
    BeAssert (constraints.size() == 2);
    AlignedClassMap constraintClassMap;
    bvector<WCharCP> constraintClassOrderedList;
    for (size_t i = 0 ; i < constraints.size(); i++)
        {
        ECRelationshipConstraintCP ecRelationshipConstraint = constraints[i];
        if (ecRelationshipConstraint == NULL)
            continue;

        ECBaseClassesList constraintClassContainer = ecRelationshipConstraint->GetClasses();
        for (ECBaseClassesList::const_iterator constraintClassItor = constraintClassContainer.begin(); constraintClassItor != constraintClassContainer.end(); ++constraintClassItor)
            {
            ECClassCR baseClass = *(*constraintClassItor);
            if (constraintClassMap.find (baseClass.GetFullName())== constraintClassMap.end())
                constraintClassOrderedList.push_back (baseClass.GetFullName());
            AlignedClasses& baseClasses = constraintClassMap [baseClass.GetFullName()];
            size_t nNullEntriesRequired = i - baseClasses.size();
            if (nNullEntriesRequired > 0)
                do
                    {
                    baseClasses.push_back (NULL);
                    } while (--nNullEntriesRequired);
                baseClasses.push_back (&baseClass);
            }
        }
    for (bvector<WCharCP>::iterator itor = constraintClassOrderedList.begin(); itor != constraintClassOrderedList.end(); ++itor)
        if (constraintClassMap[*itor].size() < constraints.size())
            while (constraintClassMap[*itor].size() < constraints.size())
                constraintClassMap[*itor].push_back(NULL);
    if (constraintClassOrderedList.empty())
        return NULL;

    ECDiffNodeP constraintClasses = parentDiff.Add (SchemaDiffType::CLASSES);
    int index =0;
    for (bvector<WCharCP>::iterator itor = constraintClassOrderedList.begin(); itor != constraintClassOrderedList.end(); ++itor, index++)
        {
        ECClassCP left = constraintClassMap[*itor][0];
        ECClassCP right = constraintClassMap[*itor][1];
        if (left && !right)
            constraintClasses->Add (index)->GetValueLeft().SetValue(left->GetFullName());
        else if (!left && right)
            constraintClasses->Add (index)->GetValueRight().SetValue(right->GetFullName());
        else
            if (wcscmp (left->GetFullName(), right->GetFullName()) != 0)
                constraintClasses->Add (index)->SetValue(left->GetFullName(), right->GetFullName());
        }
    return parentDiff.RemoveIfEmpty (constraintClasses);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiff::AppendRelationship(ECDiffNodeR parent, ECRelationshipClassCR relationship, ECDiffNode::ValueDirection direction)
    {
    ECDiffNodeP diff = parent.Add (SchemaDiffType::RELATIONSHIP_INFO);
    diff->Add (SchemaDiffType::STRENGTH)->GetValue(direction).SetValue (ToString(relationship.GetStrength()));
    diff->Add (SchemaDiffType::STRENGTH_DIRECTION)->GetValue(direction).SetValue (ToString(relationship.GetStrengthDirection()));
    AppendRelationshipConstraint (*diff, relationship.GetSource(), ECRelationshipEnd_Source, direction);
    AppendRelationshipConstraint (*diff, relationship.GetTarget(), ECRelationshipEnd_Target, direction);
    return diff;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiff::AppendRelationshipConstraint(ECDiffNodeR parent, ECRelationshipConstraintCR  relationshipConstraint,ECRelationshipEnd endPoint, ECDiffNode::ValueDirection direction)
    {
    ECDiffNodeP diff = endPoint == ECRelationshipEnd_Source ? parent.Add (SchemaDiffType::SOURCE) : parent.Add (SchemaDiffType::TARGET);
    diff->Add (SchemaDiffType::CARDINALITY)->GetValue(direction).SetValue (relationshipConstraint.GetCardinality().ToString());
    diff->Add (SchemaDiffType::IS_POLYMORPHIC)->GetValue(direction).SetValue (relationshipConstraint.GetIsPolymorphic());
    diff->Add (SchemaDiffType::ROLE_LABEL)->GetValue(direction).SetValue (relationshipConstraint.GetRoleLabel());
    AppendCustomAttributes (*diff, relationshipConstraint, direction);

    if (!relationshipConstraint.GetClasses().empty())
        {
        ECDiffNodeP constraintClasses = diff->Add (SchemaDiffType::CLASSES);
        ECConstraintClassesList classes = relationshipConstraint.GetClasses();
        int index = 0;
        for(ECConstraintClassesList::const_iterator itor = classes.begin(); itor != classes.end(); ++itor, index++)
            constraintClasses->Add (index)->GetValue(direction).SetValue ((*itor)->GetFullName());
        }
    return diff;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP ECSchemaDiff::ToString(ECRelatedInstanceDirection direction)
    {
    switch(direction)
        {
        case ECRelatedInstanceDirection::Backward:
            return SchemaDiffType::STRENGTH_DIRECTION_BACKWARD;
        case ECRelatedInstanceDirection::Forward:
            return SchemaDiffType::STRENGTH_DIRECTION_FORWARD;
        }
    return NULL;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP ECSchemaDiff::ToString(StrengthType type)
    {
    switch(type)
        {
        case STRENGTHTYPE_Embedding:
            return SchemaDiffType::STRENGTH_TYPE_EMBEDDING;
        case STRENGTHTYPE_Holding:
            return SchemaDiffType::STRENGTH_TYPE_HOLDING;
        case STRENGTHTYPE_Referencing:
            return SchemaDiffType::STRENGTH_TYPE_REFERENCING;
        }
    return NULL;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiff::DiffArrayBounds (ECDiffNodeR parent , ECPropertyCR left, ECPropertyCR right)
    {
    if (left.GetIsArray() && !right.GetIsArray())
        return AppendArrayBounds(parent, right, ECDiffNode::DIRECTION_Left);
    else if (!left.GetIsArray() && right.GetIsArray())
        return AppendArrayBounds(parent, right, ECDiffNode::DIRECTION_Right);
    else if (left.GetIsArray() && right.GetIsArray())
        {
        ECDiffNodeP arrayBounds = parent.Add (SchemaDiffType::ARRAYINFO);
        if (left.GetAsArrayProperty()->GetMaxOccurs() != right.GetAsArrayProperty()->GetMaxOccurs())
            arrayBounds->SetValue (left.GetAsArrayProperty()->GetMaxOccurs(), right.GetAsArrayProperty()->GetMaxOccurs());
        if (left.GetAsArrayProperty()->GetMinOccurs() != right.GetAsArrayProperty()->GetMinOccurs())
            arrayBounds->SetValue (left.GetAsArrayProperty()->GetMinOccurs(), right.GetAsArrayProperty()->GetMinOccurs());
        if (arrayBounds->IsEmpty())
            parent.Remove(arrayBounds);
        }
    return NULL;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiff::AppendClass (ECDiffNodeR parent , ECClassCR ecClass, ECDiffNode::ValueDirection direction)
    {
    ECDiffNodeP diff = parent.Add (ecClass.GetName().c_str());
    //diff->Add (SchemaDiffType::NAME)->GetValue(direction).SetValue (ecClass.GetName().c_str());
    if (ecClass.GetIsDisplayLabelDefined())
        diff->Add (SchemaDiffType::DISPLAY_LABEL)->GetValue(direction).SetValue (ecClass.GetDisplayLabel().c_str());
    diff->Add (SchemaDiffType::DESCRIPTION)->GetValue(direction).SetValue (ecClass.GetDescription().c_str());
    diff->Add (SchemaDiffType::IS_CUSTOMATTRIBUTE_CLASS)->GetValue(direction).SetValue (ecClass.GetIsCustomAttributeClass());
    diff->Add (SchemaDiffType::IS_STRUCT)->GetValue(direction).SetValue (ecClass.GetIsStruct());
    diff->Add (SchemaDiffType::IS_DOMAIN_CLASS)->GetValue(direction).SetValue (ecClass.GetIsDomainClass());
    diff->Add (SchemaDiffType::IS_RELATIONSHIP_CLASS)->GetValue(direction).SetValue (ecClass.GetRelationshipClassCP() != NULL);
    AppendCustomAttributes (*diff, ecClass, direction);
    if (!ecClass.GetBaseClasses().empty())
        {
        ECDiffNodeP baseClassesNode = diff->Add (SchemaDiffType::BASECLASSES);
        ECBaseClassesList baseClasses = ecClass.GetBaseClasses();
        int index = 0;
        for(ECBaseClassesList::const_iterator itor = baseClasses.begin(); itor != baseClasses.end(); ++itor, index++)
            baseClassesNode->Add (index)->GetValue(direction).SetValue ((*itor)->GetFullName());
        }

    ECDiffNodeP propertiesNode = diff->Add (SchemaDiffType::PROPERTIES);
    ECPropertyIterable properties = ecClass.GetProperties(false);
    for (ECPropertyIterable::const_iterator itor = properties.begin(); itor != properties.end(); ++itor)
        AppendProperty (*propertiesNode, **itor, direction);
    diff->RemoveIfEmpty (propertiesNode);
    return diff;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiff::AppendProperty (ECDiffNodeR parent , ECPropertyCR ecProperty, ECDiffNode::ValueDirection direction)
    {
    ECDiffNodeP diff = parent.Add (ecProperty.GetName().c_str());
    //diff->Add (SchemaDiffType::NAME)->GetValue(direction).SetValue (ecProperty.GetName().c_str());
    if (ecProperty.GetIsDisplayLabelDefined())
        diff->Add (SchemaDiffType::DISPLAY_LABEL)->GetValue(direction).SetValue (ecProperty.GetDisplayLabel().c_str());

    diff->Add (SchemaDiffType::DESCRIPTION)->GetValue(direction).SetValue (ecProperty.GetDescription().c_str());
    diff->Add (SchemaDiffType::IS_ARRAY)->GetValue(direction).SetValue (ecProperty.GetIsArray());
    diff->Add (SchemaDiffType::IS_STRUCT)->GetValue(direction).SetValue (ecProperty.GetIsStruct());
    diff->Add (SchemaDiffType::TYPENAME)->GetValue(direction).SetValue (ecProperty.GetTypeName().c_str());
    AppendArrayBounds(*diff, ecProperty, direction);
    AppendCustomAttributes (*diff, ecProperty, direction);
    return diff;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiff::AppendArrayBounds (ECDiffNodeR parent , ECPropertyCR ecProperty, ECDiffNode::ValueDirection direction)
    {
    if (ecProperty.GetIsArray())
        {
        ECDiffNodeP arrayBounds = parent.Add (SchemaDiffType::ARRAYINFO);
        arrayBounds->Add (SchemaDiffType::MAXOCCURS)->GetValue(direction).SetValue ((int32_t)ecProperty.GetAsArrayProperty()->GetMaxOccurs());
        arrayBounds->Add (SchemaDiffType::MINOCCURS)->GetValue(direction).SetValue ((int32_t)ecProperty.GetAsArrayProperty()->GetMinOccurs());
        return arrayBounds;
        }
    return NULL;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaDiff::CollectInstanceValues (bmap<WString,ECValue>& valueMap, bset<WString>& accessStrings, IECInstanceCR instance)
    {
    valueMap.clear();
    ECValuesCollectionPtr propertyValues = ECValuesCollection::Create (instance);
    if (propertyValues.IsNull()) 
        return;
    CollectInstanceValues (valueMap, accessStrings, *propertyValues);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaDiff::CollectInstanceValues (bmap<WString,ECValue>& valueMap, bset<WString>& accessStrings, ECValuesCollectionCR values)
    {
    for (ECValuesCollection::const_iterator itor = values.begin(); itor != values.end(); ++itor)
        {
        ECValueAccessorCR valueAccessor = (*itor).GetValueAccessor ();
        WString accessString = valueAccessor.GetManagedAccessString();
        if ((*itor).HasChildValues())
            CollectInstanceValues (valueMap, accessStrings, *(*itor).GetChildValues());
        else
            {
            ECValueCR v = (*itor).GetValue();
            if (!v.IsPrimitive())
                continue;
            if (v.IsBinary() && !v.IsNull())
                {
                size_t size = 0;
                valueMap [accessString.c_str()].SetBinary (v.GetBinary(size), size, true);
                }
            else
                valueMap [accessString.c_str()].From (v);
            if (accessStrings.find (accessString) == accessStrings.end())
                accessStrings.insert(accessString);
            }
        }
    };
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiff::DiffCustomAttributes 
    (
    ECDiffNodeR parentDiff, 
    IECCustomAttributeContainerCR leftContainer, 
    IECCustomAttributeContainerCR rightContainer
    )
    {
    bmap<WCharCP, ECClassCP, CompareWChar> classes;
    ECCustomAttributeInstanceIterable leftCustomAttributes =  leftContainer.GetPrimaryCustomAttributes(false);
    for (ECCustomAttributeInstanceIterable::const_iterator itor = leftCustomAttributes.begin(); itor != leftCustomAttributes.end(); ++itor)
        {
        ECClassCR caClass = (*itor)->GetClass();
        if (classes.find ((*itor)->GetClass().GetName().c_str()) == classes.end())
            classes[caClass.GetName().c_str()] = &caClass;
        }
    ECCustomAttributeInstanceIterable rightCustomAttributes =  rightContainer.GetPrimaryCustomAttributes(false);
    for (ECCustomAttributeInstanceIterable::const_iterator itor = rightCustomAttributes.begin(); itor != rightCustomAttributes.end(); ++itor)
        {
        ECClassCR caClass = (*itor)->GetClass();
        if (classes.find ((*itor)->GetClass().GetName().c_str()) == classes.end())
            classes[caClass.GetName().c_str()] = &caClass;
        }
    if (classes.empty())
        return NULL;

    ECDiffNodeP diff = parentDiff.Add (SchemaDiffType::CUSTOMATTRIBUTES);
    for (bmap<WCharCP, ECClassCP, CompareWChar>::const_iterator itor = classes.begin(); itor != classes.end(); ++itor)
        DiffInstance(*diff, *(itor->second), leftContainer, rightContainer);
    return parentDiff.RemoveIfEmpty (diff);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiff::DiffInstance 
    (
    ECDiffNodeR parentDiff, 
    ECClassCR customAttributeClass, 
    IECCustomAttributeContainerCR leftContainer, 
    IECCustomAttributeContainerCR rightContainer
    )
    {
    IECInstanceCP left = leftContainer.GetPrimaryCustomAttribute (customAttributeClass).get();
    IECInstanceCP right = rightContainer.GetPrimaryCustomAttribute (customAttributeClass).get();
    if (!left && !right)
        return NULL;
    else if (left && !right)
        return AppendInstance (parentDiff, *left, ECDiffNode::DIRECTION_Left);
    else if (!left && right)
        return AppendInstance (parentDiff, *right, ECDiffNode::DIRECTION_Right);

    bmap<WString,ECValue> leftValues;
    bmap<WString,ECValue> rightValues;
    bset<WString> accessStrings;
    CollectInstanceValues (leftValues, accessStrings, *left);
    CollectInstanceValues (rightValues, accessStrings, *right);

    if (!accessStrings.empty())
        {
        ECDiffNodeP diff = parentDiff.Add (customAttributeClass.GetName().c_str());
        for (bset<WString> ::const_iterator itor = accessStrings.begin(); itor != accessStrings.end(); ++itor)
            {
            bmap<WString,ECValue>::const_iterator lItor = leftValues.find (*itor);
            bmap<WString,ECValue>::const_iterator rItor = rightValues.find (*itor);
            if (lItor == leftValues.end() && rItor != rightValues.end())
                SetECValue (*(diff->GetChild ((*itor), true)), rItor->second, ECDiffNode::DIRECTION_Right);
            else if (lItor != leftValues.end() && rItor == rightValues.end())
                SetECValue (*(diff->GetChild ((*itor), true)), lItor->second, ECDiffNode::DIRECTION_Left);
            else
                {
                ECValueCR a = rItor->second;
                ECValueCR b = lItor->second;

                if (!a.Equals (b))
                    {
                    ECDiffNodeP value = diff->GetChild ((*itor), true);
                    SetECValue (*value, rItor->second, ECDiffNode::DIRECTION_Right);
                    SetECValue (*value, lItor->second, ECDiffNode::DIRECTION_Left);
                    }
                }
            }
        return parentDiff.RemoveIfEmpty (diff);
        }
    return NULL;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP  ECSchemaDiff::AppendCustomAttributes 
    (
    ECDiffNodeR parentDiff, 
    IECCustomAttributeContainerCR container,
    ECDiffNode::ValueDirection direction
    )
    {
    ECDiffNodeP diff = parentDiff.Add (SchemaDiffType::CUSTOMATTRIBUTES);
    ECCustomAttributeInstanceIterable customAttributes =  container.GetPrimaryCustomAttributes(false);
    for (ECCustomAttributeInstanceIterable::const_iterator itor = customAttributes.begin(); itor != customAttributes.end(); ++itor)
        AppendInstance (*diff, *(*itor), direction);

    return parentDiff.RemoveIfEmpty (diff);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiff::AppendInstance (ECDiffNodeR parentDiff, IECInstanceCR instance, ECDiffNode::ValueDirection direction)
    {
    ECDiffNodeP diff = parentDiff.Add (instance.GetClass().GetFullName());
    ECValuesCollectionPtr propertyValues = ECValuesCollection::Create (instance);
    if (propertyValues.IsNull()) 
        return NULL;
    return AppendPropertyValues (*diff, *propertyValues, direction);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECDiffNodeP ECSchemaDiff::AppendPropertyValues (ECDiffNodeR parentDiff, ECValuesCollectionCR values, ECDiffNode::ValueDirection direction)
    {
    for (ECValuesCollection::const_iterator itor = values.begin(); itor != values.end(); ++itor)
        {
        ECValueAccessorCR valueAccessor = (*itor).GetValueAccessor ();
        const WString propertyName = valueAccessor.GetPropertyName ();
        ECDiffNodeP diff = parentDiff.Add (propertyName.c_str());
        if ((*itor).HasChildValues())
            AppendPropertyValues (*diff, *(*itor).GetChildValues(), direction);
        else
            {
            ECValueCR v = (*itor).GetValue();
            if (v.IsNull())
                continue;
            if (v.IsPrimitive())
                SetECValue(*diff, v, direction);
            }
        }
    return NULL;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchemaDiff::SetECValue (ECDiffNodeR n, ECValueCR v, ECDiffNode::ValueDirection direction)
    {
    if (v.IsNull())
        {
        n.GetValue(direction).SetNil();
        return true;
        }
    if (v.IsArray())
        {
        BeAssert(false);
        return false;
        }
    switch(v.GetPrimitiveType())
        {
        case PRIMITIVETYPE_Binary:
            {
            size_t size = 0;
            n.GetValue(direction).SetValue (v.GetBinary(size), size);
            break;
            }
        case PRIMITIVETYPE_Boolean:
            n.GetValue (direction).SetValue (v.GetBoolean()); break;
        case PRIMITIVETYPE_DateTime:
            n.GetValue (direction).SetDateTimeValue(v.GetDateTimeTicks()); break;
        case PRIMITIVETYPE_Double:
            n.GetValue (direction).SetValue (v.GetDouble()); break;
        case PRIMITIVETYPE_IGeometry: break;
        case PRIMITIVETYPE_Integer:
            n.GetValue (direction).SetValue (v.GetInteger()); break;
        case PRIMITIVETYPE_Long:
            n.GetValue (direction).SetValue (v.GetLong()); break;
        case PRIMITIVETYPE_Point2D:
            {              
            n.Add (L"x")->GetValue (direction).SetValue (v.GetPoint2D().x); 
            n.Add (L"y")->GetValue (direction).SetValue (v.GetPoint2D().y); 
            break;
            }
        case PRIMITIVETYPE_Point3D:
            {
            n.Add (L"x")->GetValue (direction).SetValue (v.GetPoint3D().x); 
            n.Add (L"y")->GetValue (direction).SetValue (v.GetPoint3D().y); 
            n.Add (L"z")->GetValue (direction).SetValue (v.GetPoint3D().z); 
            break;
            }
        case PRIMITIVETYPE_String:
            n.GetValue (direction).SetValue (v.GetString()); break;
        }
    return true;
    }

END_BENTLEY_ECOBJECT_NAMESPACE

