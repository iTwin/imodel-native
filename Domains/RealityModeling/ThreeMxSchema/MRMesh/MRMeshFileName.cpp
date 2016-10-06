/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/MRMeshFileName.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "..\ThreeMxSchemaInternal.h"

#include    <windows.h>
#include    <regex>


USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AppendSeparator (Utf8StringR path, bool isUrl)
    {
    if (isUrl)
        {
        if (path.empty() || (*(path.end()-1) != '/'))
            path.append ("/");
        }
    else
        {
        // Non URL separators are platform specific
        if (path.empty() || (*(path.end()-1) != DIR_SEPARATOR_CHAR))
            path.append (DIR_SEPARATOR);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                   06/2016
//---------------------------------------------------------------------------------------
MRMeshFileName::MRMeshFileName (Utf8StringCR name)
: Utf8String(name)
    {
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                   06/2016
//---------------------------------------------------------------------------------------
MRMeshFileName::MRMeshFileName (Utf8CP name)
: Utf8String(name)
    {
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                   06/2016
//---------------------------------------------------------------------------------------
MRMeshFileName::MRMeshFileName (Utf8StringCR name, bool pathOnly)
: Utf8String (name)
    {
    // The value of the flag indicates that we only want the dev and path or equivalent http or Stream-X portion
    if (pathOnly)
        StripOutFileName();

    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                   06/2016
//---------------------------------------------------------------------------------------
MRMeshFileName::MRMeshFileName (Utf8CP name, bool pathOnly)
    {
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                   06/2016
//---------------------------------------------------------------------------------------
bool MRMeshFileName::ParseUrl(Utf8StringP protocol, Utf8StringP server, Utf8StringP remainder) const
    {
    BeAssert(IsUrl());

    Utf8String Url (*this);

    // An URL must first specify the protocol (all character preceeding ':'
    size_t doublePointLocation = Url.find_first_of (':');

    if ((std::string::npos == doublePointLocation) || (0 == doublePointLocation))
        return false;

    if (NULL != protocol)
        *protocol = Url.substr(0, doublePointLocation);

    Url = Url.substr(doublePointLocation + 1);

    // The next two characters must be double forward slashes
    if (0 != Url.find("//"))
        return false;

    Url = Url.substr(2);

    // The server name is from first remaining character to first forward slash. This server name may contain the port specification if one is present.
    size_t forwardSlashLocation = Url.find_first_of("/");
    if ((std::string::npos == forwardSlashLocation) || (0 == forwardSlashLocation))
        return false;

    if (NULL != server)
        *server = Url.substr(0, forwardSlashLocation);

    Url = Url.substr(forwardSlashLocation + 1);

    if (NULL != remainder)
        *remainder = Url;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                   06/2016
// I know that this is probably already coded somewhere in the WSClient but the additional
// dependency was just not worth not writing the function locally.
//---------------------------------------------------------------------------------------
bool MRMeshFileName::ParseWSGUrl(Utf8StringP protocol, Utf8StringP server, Utf8StringP version, Utf8StringP repositoryName, Utf8StringP schemaName, Utf8StringP className, Utf8StringP objectId, bool* contentFlag) const
    {
    Utf8String Url (*this);

    Utf8String remainder;

    if (!ParseUrl(protocol, server, &remainder))
        return false;

    // Then the version string
    size_t forwardSlashLocation = remainder.find_first_of("/");
    if ((std::string::npos == forwardSlashLocation) || (0 == forwardSlashLocation))
        return false;
    
    if (NULL != version)
        *version = remainder.substr(0, forwardSlashLocation);

    remainder = remainder.substr(forwardSlashLocation + 1);

    // Normally the remainder will contain 'Repositories'. 
    // This method only supports the Repository API ... 
    size_t repositoryLocation = repositoryLocation = remainder.find("Repositories/");

    if (0 != repositoryLocation)
        return false;

    remainder = remainder.substr(13);

    // Next follows the repository name
    forwardSlashLocation = remainder.find_first_of("/");
    if ((std::string::npos == forwardSlashLocation) || (0 == forwardSlashLocation))
        return false;

    if (NULL != repositoryName)
        *repositoryName = remainder.substr(0, forwardSlashLocation);

    remainder = remainder.substr(forwardSlashLocation + 1);

    // Next follows the schema name
    forwardSlashLocation = remainder.find_first_of("/");
    if ((std::string::npos == forwardSlashLocation) || (0 == forwardSlashLocation))
        return false;

    if (NULL != schemaName)
        *schemaName = remainder.substr(0, forwardSlashLocation);

    remainder = remainder.substr(forwardSlashLocation + 1);

    // Next follows the class name
    forwardSlashLocation = remainder.find_first_of("/");
    if ((std::string::npos == forwardSlashLocation) || (0 == forwardSlashLocation))
        return false;

    if (NULL != className)
        *className = remainder.substr(0, forwardSlashLocation);

    remainder = remainder.substr(forwardSlashLocation + 1);

    // Next follows the object id
    forwardSlashLocation = remainder.find_first_of("/");
    if (std::string::npos == forwardSlashLocation) 
        {
        // The absence of a slash indicates there are no content flag and that the rest of the string is the class name
        if (NULL != objectId)
            *objectId = remainder;

        if (NULL != contentFlag)
            *contentFlag = false; 
        }
    else
        {
        if (0 == forwardSlashLocation)
            return false;

        if (NULL != objectId)
            *objectId = remainder.substr(0, forwardSlashLocation);

        remainder = remainder.substr(forwardSlashLocation + 1);

        // And finally if the remainder is the content flag then we indicate content.
        // Note we do not tolerate other possible argument
        if (NULL != contentFlag)
            *contentFlag = false;

        if ("$file" == remainder)
            {
            if (NULL != contentFlag)
                *contentFlag = true;
            }
        else if (remainder.size() != 0) // The remainder is not '$file' yet it is not empty ... invalid.
            return false;
            
        }            
   
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                   06/2016
//---------------------------------------------------------------------------------------
bool MRMeshFileName::IsUrl() const
    {
    return (0 == find("http:") || 0 == find("https:"));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                   06/2016
//---------------------------------------------------------------------------------------
bool MRMeshFileName::IsS3MXUrl() const
    {
    // Check for the presence of the S3MX plugin signature
    Utf8String protocol;
    Utf8String server;
    Utf8String WSGversion;
    Utf8String repository;
    Utf8String schemaName;
    Utf8String className;
    Utf8String objectId;
    bool contentFlag;

    if (!IsUrl())
        return false;

    if (!ParseWSGUrl(&protocol, &server, &WSGversion, &repository, &schemaName, &className, &objectId, &contentFlag))
        return false;

    if (!("http" == protocol) && !("https" == protocol))
        return false;

    if (!("S3MX" == schemaName))
        return false;

    if (!("Document" == className))
        return false;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                   06/2016
//---------------------------------------------------------------------------------------
bool MRMeshFileName::BuildWSGUrl(Utf8StringCR protocol, Utf8StringCR server, Utf8StringCR version, Utf8StringCR repositoryName, Utf8StringCR schemaName, Utf8StringCR className, Utf8StringCR objectId, bool contentFlag)
    {
    clear();

    *this = protocol + "://" + server + "/" + version + "/Repositories/" + repositoryName + "/" + schemaName + "/" + className + "/" + objectId;

    if (contentFlag)
        *this = *this + "/$file";

    return true;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                   06/2016
//---------------------------------------------------------------------------------------
bool MRMeshFileName::AppendToS3MXFileName(Utf8StringCR additionalPath)

    {
    Utf8String protocol;
    Utf8String server;
    Utf8String WSGversion;
    Utf8String repository;
    Utf8String schemaName;
    Utf8String className;
    Utf8String objectId;
    bool contentFlag;

    ParseWSGUrl(&protocol, &server, &WSGversion, &repository, &schemaName, &className, &objectId, &contentFlag);
  
    BeAssert(("http" == protocol) || ("https" == protocol));  
    BeAssert("S3MX" == schemaName);
    BeAssert("Document" == className);


    // Although the object ID is Url encoded we only care for slashes at the moment and we will only decode the ~2F sequence
    // Now we replace all ~2F by directory slashes
    objectId.ReplaceAll("~2F", "/");

    AppendSeparator(objectId, true);
    objectId.append(additionalPath);

    // Recode back
    objectId.ReplaceAll("/", "~2F");

    // Even if the /$file content flag was not present we add it ... it would not make sense not to have it.
    BuildWSGUrl(protocol, server, WSGversion, repository, schemaName, className, objectId, true);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                   06/2016
//---------------------------------------------------------------------------------------
void MRMeshFileName::AppendToPath (Utf8StringCR val)
    {
    // Stream-X URL require different processing
    if (IsS3MXUrl())
        AppendToS3MXFileName(val);
    else
        {
        // General case applicable to local file name or plain http names.
        if (!empty())
            AppendSeparator (*this, IsUrl());

        append (val);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshFileName::StripOutFileName()
    {
    if (IsS3MXUrl())
        {
        Utf8String protocol;
        Utf8String server;
        Utf8String WSGversion;
        Utf8String repository;
        Utf8String schemaName;
        Utf8String className;
        Utf8String objectId;
        bool contentFlag;
    
        ParseWSGUrl(&protocol, &server, &WSGversion, &repository, &schemaName, &className, &objectId, &contentFlag);
    
        BeAssert(("http" == protocol) || ("https" == protocol));  
        BeAssert("S3MX" == schemaName);
        BeAssert("Document" == className);
        BeAssert(contentFlag);
    
    
        // Although the object ID is Url encoded we only care for slashes at the moment and we will only decode the ~2F sequence
        // Now we replace all ~2F by directory slashes
        objectId.ReplaceAll("~2F", "/");
    
        // remove trailing slash is any
        if (objectId[objectId.size() - 1] == '/')
            objectId = objectId.substr(0, objectId.size() - 1);

        // Find the last '/'
        size_t lastPosition = objectId.find_last_of ('/');
        if (std::string::npos != lastPosition) 
            objectId = objectId.substr(0, lastPosition);

        // Recode back
        objectId.ReplaceAll("/", "~2F");
    
        // Even if the /$file content flag was not present we add it ... it would not make sense not to have it.
        BuildWSGUrl(protocol, server, WSGversion, repository, schemaName, className, objectId, true);
    }
    else if (IsUrl())
        {
        // remove trailing slash is any
        if ((*this)[this->size() - 1] == '/')
            *this = this->substr(0, this->size() - 1);

        // Find the last '/'
        size_t lastPosition = this->find_last_of ('/');
        if (std::string::npos != lastPosition) 
            *this = this->substr(0, lastPosition);
        }
    else
        {
        WString decodedWideObject;
        BeStringUtilities::Utf8ToWChar(decodedWideObject, c_str());

        // It must be a plain local file name ... we get help from already coded BeFileName.
        BeFileName fileName (BeFileName::DevAndDir, decodedWideObject.c_str());

        BeStringUtilities::WCharToUtf8(*this, fileName.c_str());
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String MRMeshFileName::GetFileNameWithoutExtension() const
    {
    Utf8String name;

    if (!IsS3MXUrl())
        {
        WString decodedWideObject;
        BeStringUtilities::Utf8ToWChar(decodedWideObject, c_str());

        // It must be a plain local file name ... we get help from already coded BeFileName.
        BeFileName fileName (decodedWideObject.c_str());

        WString tempName = fileName.GetFileNameWithoutExtension();

        BeStringUtilities::WCharToUtf8(name, tempName.c_str());
        }
    else
        {
        Utf8String protocol;
        Utf8String server;
        Utf8String WSGversion;
        Utf8String repository;
        Utf8String schemaName;
        Utf8String className;
        Utf8String objectId;
        bool contentFlag;
    
        ParseWSGUrl(&protocol, &server, &WSGversion, &repository, &schemaName, &className, &objectId, &contentFlag);
    
        BeAssert(("http" == protocol) || ("https" == protocol));  
        BeAssert("S3MX" == schemaName);
        BeAssert("Document" == className);
        BeAssert(contentFlag);
    
    
        // Although the object ID is Url encoded we only care for slashes at the moment and we will only decode the ~2F sequence
        // Now we replace all ~2F by directory slashes
        objectId.ReplaceAll("~2F", "/");
    
        // remove trailing slash is any
        if (objectId[objectId.size() - 1] == '/')
            objectId = objectId.substr(0, objectId.size() - 1);

        // Find the last '/'
        size_t lastPositionOfSlash = objectId.find_last_of ('/');
        if (std::string::npos != lastPositionOfSlash && 0 != lastPositionOfSlash) 
            objectId = objectId.substr(lastPositionOfSlash + 1);

        // Remove extension is any
        size_t lastPositionOfDot = objectId.find_last_of ('.');
        if (std::string::npos != lastPositionOfDot) 
            objectId = objectId.substr(0, lastPositionOfDot);

        name = objectId;
        }

    return name;
    }
