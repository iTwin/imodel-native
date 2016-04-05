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

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct Credentials
    {
    private:
        Utf8String m_username;
        Utf8String m_password;

    private:
        static void Clear (Utf8StringR string);

    public:
        //! Emty credentials
        BEHTTP_EXPORT Credentials ();
        //! Create credentials with username and password.
        //! Use std::move to pass parameters so that their memory could be safely cleared afterwards
        BEHTTP_EXPORT Credentials (Utf8String username, Utf8String password);
        //! Destructor clears credentials memory before exiting
        BEHTTP_EXPORT ~Credentials ();

        BEHTTP_EXPORT bool IsEmpty () const;
        BEHTTP_EXPORT bool IsValid () const;

        BEHTTP_EXPORT void SetUsername (Utf8String username);
        BEHTTP_EXPORT void SetPassword (Utf8String password);

        BEHTTP_EXPORT Utf8StringCR GetUsername () const;
        BEHTTP_EXPORT Utf8StringCR GetPassword () const;

        BEHTTP_EXPORT bool operator == (const Credentials& other) const;
        BEHTTP_EXPORT bool operator != (const Credentials& other) const;
    };

typedef Credentials& CredentialsR;
typedef const Credentials& CredentialsCR;

END_BENTLEY_HTTP_NAMESPACE
