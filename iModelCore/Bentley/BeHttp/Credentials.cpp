/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/Credentials.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <BeHttp/Credentials.h>

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Credentials::Credentials ()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Credentials::Credentials (Utf8String username, Utf8String password) :
m_username (std::move (username)),
m_password (std::move (password))
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Credentials::~Credentials ()
    {
    Clear (m_username);
    Clear (m_password);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Credentials::Clear (Utf8StringR string)
    {
    size_t length = string.length ();
    for (size_t i = 0; i < length; i++)
        {
        string[i] = 0;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool Credentials::IsEmpty () const
    {
    return m_username.empty () && m_password.empty ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Dalius.Dobravolskas             09/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool Credentials::IsValid () const
    {
    return !m_username.empty () && !m_password.empty ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Credentials::SetUsername (Utf8String username)
    {
    Clear (m_username);
    m_username = std::move (username);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void Credentials::SetPassword (Utf8String password)
    {
    Clear (m_password);
    m_password = std::move (password);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR Credentials::GetUsername () const
    {
    return m_username;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR Credentials::GetPassword () const
    {
    return m_password;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool Credentials::operator == (const Credentials& other) const
    {
    return
        m_username == other.m_username &&
        m_password == other.m_password;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool Credentials::operator != (const Credentials& other) const
    {
    return !(*this == other);
    }
