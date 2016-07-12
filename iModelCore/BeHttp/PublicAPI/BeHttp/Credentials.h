/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeHttp/Credentials.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeHttp/Http.h>
#include <Bentley/WString.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(Credentials)

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct Credentials
{
private:
    Utf8String m_username;
    Utf8String m_password;

    BEHTTP_EXPORT static void Clear(Utf8StringR string);

public:
    //! Emty credentials
    Credentials() {}
    //! Create credentials with username and password.
    //! Use std::move to pass parameters so that their memory could be safely cleared afterwards
    Credentials(Utf8String username, Utf8String password) : m_username(std::move(username)), m_password(std::move(password)){}

    //! Destructor clears credentials memory before exiting
    ~Credentials() {Clear(m_username); Clear(m_password);}

    bool IsEmpty() const {return m_username.empty() && m_password.empty();}
    bool IsValid() const {return !m_username.empty() && !m_password.empty();}

    void SetUsername(Utf8String username) {Clear(m_username); m_username = std::move(username);}
    void SetPassword(Utf8String password) {Clear(m_password); m_password = std::move(password);}

    Utf8StringCR GetUsername() const {return m_username;}
    Utf8StringCR GetPassword() const {return m_password;}

    bool operator == (const Credentials& other) const {return m_username == other.m_username && m_password == other.m_password;}
    bool operator != (const Credentials& other) const {return !(*this == other);}
};

END_BENTLEY_HTTP_NAMESPACE
