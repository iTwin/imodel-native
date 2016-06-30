/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/CachedResponseKey.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Persistence/CachedResponseKey.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CachedResponseKey::CachedResponseKey() {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CachedResponseKey::CachedResponseKey(ECInstanceKeyCR parent, Utf8StringCR name, ECInstanceKeyCR holder) :
m_parent(parent),
m_name(name),
m_holder(holder.IsValid() ? holder : parent)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKeyCR CachedResponseKey::GetParent() const
    {
    return m_parent;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR CachedResponseKey::GetName() const
    {
    return m_name;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKeyCR CachedResponseKey::GetHolder() const
    {
    return m_holder;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CachedResponseKey::SetHolder(ECInstanceKey holder)
    {
    m_holder = holder;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool CachedResponseKey::IsValid() const
    {
    return m_parent.IsValid();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool CachedResponseKey::operator==(const CachedResponseKey& other) const
    {
    return
        m_parent == other.m_parent &&
        m_name == other.m_name &&
        m_holder == other.m_holder;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              
+---------------+---------------+---------------+---------------+---------------+------*/
bool CachedResponseKey::operator<(const CachedResponseKey& other) const
    {
    if (m_parent != other.m_parent)
        return m_parent < other.m_parent;

    if (m_holder != other.m_holder)
        return m_holder < other.m_holder;

    return m_name.CompareTo(other.m_name) < 0;
    }