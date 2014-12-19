//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLMemFile.h $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCURLMemFile
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HFCURL.h>

//:> URL specification at this level is:
//:> file:{//host/[path] | [//]drive/[path] | [//]relpath}

/**

This class defines the URL descriptor for the "memory" scheme type.  That
kind of URL is used to simulate files through memory buffer wich are accessible
on local access. The structure of the text of the URLs follows one of the following syntaxes:

@code
@i{memory}://@i{host}/[@i{path}]
@end

where the text in italics has to be replaced by corresponding value, and
text between brackets is facultative.

This class adds methods to access the parsed values extracted from the
URL text (the host and path) and to convert the memory buffer into a
standard filename like.

@see HFCURL

*/

class HFCBuffer;

class HFCURLMemFile : public HFCURL
    {
public:

    HDECLARE_CLASS_ID(1309, HFCURL);

    // Define the Scheme label
    static const WString& s_SchemeName()
        {   static const WString Val(L"memory");
        FREEZE_STL_STRING(Val);
        return Val;
        }

    //:> Primary methods

    _HDLLu                      HFCURLMemFile(const WString&        pi_URL,
                                              const HFCPtr<HFCBuffer>&    pi_rpBuffer = HFCPtr<HFCBuffer>());
    _HDLLu                      HFCURLMemFile(const WString&        pi_Host,
                                              const WString&        pi_Path,
                                              const HFCPtr<HFCBuffer>&    pi_rpBuffer = HFCPtr<HFCBuffer>());
    _HDLLu virtual              ~HFCURLMemFile();

    //:> Content access methods

    _HDLLu virtual WString      GetURL() const;
    _HDLLu const WString&       GetHost() const;
    _HDLLu const WString&       GetPath() const;
    _HDLLu WString              GetFilename() const;
    _HDLLu WString              GetExtension() const;

    //:> Overriden methods, used in relative path management

    virtual bool               HasPathTo(HFCURL* pi_pURL);
    virtual WString             FindPathTo(HFCURL* pi_pDest);
    virtual HFCURL*             MakeURLTo(const WString& pi_Path);

    _HDLLu HFCPtr<HFCBuffer>&   GetBuffer();
    void                        SetBuffer(HFCPtr<HFCBuffer>& pi_rpBuffer);

    //:> HFCStat utilities methods
    void                        SetCreationTime(time_t   pi_NewTime);
    time_t                      GetCreationTime() const;

    void                        SetModificationTime(time_t   pi_NewTime);
    time_t                      GetModificationTime() const;


#ifdef __HMR_DEBUG_MEMBER
    virtual void PrintState() const;
#endif

protected:

private:
    friend struct URLMemoryCreator;

    //:> Disabled methods
    HFCURLMemFile(const HFCURLMemFile&);
    HFCURLMemFile& operator=(const HFCURLMemFile&);


    //:> Components of the scheme-specific part of the URL string.
    WString                 m_Host;
    WString                 m_Path;
    HFCPtr<HFCBuffer>       m_pBuffer;
    time_t                  m_creationTime;
    time_t                  m_modificationTime;
    };

#include "HFCURLMemFile.hpp"

