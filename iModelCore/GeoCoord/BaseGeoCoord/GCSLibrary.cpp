/*----------------------------------------------------------------------+
|
|   $Source: BaseGeoCoord/GCSLibrary.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/


// NOTE: This file is #included from BaseManagedGCS.cpp


#if defined (__unix__)
#include <unistd.h>
#endif

/*=================================================================================**//**
* UserLibrary class
+===============+===============+===============+===============+===============+======*/
typedef bvector <std::string>       T_StringList;

enum    FileMagicNumbers
    {
    OriginalFormat      = cs_CSDEF_MAGIC,
    ConsolidatedFormat  = 0x02131982,
    };


#if defined (DONTNEED)
// class that makes sure CSMap data is freed.
struct  CSDataHolder
    {
    void    *m_csData;

    CSDataHolder (void* csData) { m_csData = NULL;}
    ~CSDataHolder ()
        {
        CSMAP_FREE_AND_CLEAR (m_csData);
        }
    void* operator->() {return m_csData;}
    void* operator=(void* csData) {return (m_csData = csData);}
    };
#endif

/*=================================================================================**//**
* User provided Geographic Coordinate System library.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct  UserLibrary : public Library
{
private:
WString             m_libraryPath;
WString             m_guiName;
T_WStringVector*    m_csNames;
T_WStringVector*    m_datumNames;
T_WStringVector*    m_ellipsoidNames;
bool                m_consolidatedFormat;
uint32_t            m_ellipsoidStart;
uint32_t            m_datumStart;
uint32_t            m_gcsStart;



WStringP            m_orgFileName;


// Note: This class must support both the old-style User Libraries, which contained only GCS's,
//       and the new style user libraries, which can contain GCS's, Datums, and Ellipsoids.
//       They are distinguished by the "Magic Number" at the beginning.
//
//       The old-style User Libraries magic number was 0x800D0012, and that magic number was
//       followed immediately by GCS data.
//
//       The new-style User libraries magic number is 0x02131982, and that magic number is part
//       of a 16-byte header that has fields for the start of the ellipsoid data, the start of
//       the datum data, and the start of the GCS data.
//
//       Currently, we reserve space at the beginning of the file for 100 Ellipsoids and 250 Datums.
//       The remainder of the file is of variable size and is taken up by GCS Data.
//       That is pretty hokey, and wasteful of disk space, but it makes using the CSMap IO routines
//       much easier, particularly those that update the current contents of the library.

public:
virtual ~UserLibrary() 
    {
    DELETE_AND_CLEAR (m_orgFileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
UserLibrary
(
WString         libraryPath,
WString         guiName
)
    {
    m_libraryPath           = libraryPath;
    m_guiName               = guiName;
    m_csNames               = NULL;
    m_datumNames            = NULL;
    m_ellipsoidNames        = NULL;

    // assume old style until proven otherwise.
    m_consolidatedFormat    = false;
    m_ellipsoidStart        = 0;
    m_datumStart            = 0;
    m_gcsStart              = sizeof (uint32_t);  // positioned just past the magic number.

    m_orgFileName           = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSParameters*   GetCS (WCharCP name) override
    {
    CSDefinition    csDef;

    if (SUCCESS != FindCSDef (&csDef, name))
        return NULL;

    return GetCS (csDef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSParameters*   GetCS (CSDefinition& csDef)
    {
    // We found the CSDefinition in this UserLibrary - we need to see whether the Datum or Ellipsoid is in this UserLibrary also.
    // If it's an old-format library, we can't possibly have Datums or Ellipsoids, so we can just use CScsloc1 (which will look in the system libraries for datums and ellipsoids).
    if (!m_consolidatedFormat)
        return CSMap::CScsloc1 (&csDef);

    CSEllipsoidDef*     localEllipsoid  = NULL;
    CSDatumDef*         localDatum      = NULL;

    // If we have a datum we try to get it from this UserLibrary. If that fails, it (and the Ellipsoid) must be in the system library, so just use CScsloc1.
    if (0 != csDef.dat_knm[0])
        {
        WString datumName (csDef.dat_knm,false);
        // can't find datum in this UserLibrary, so datum must come from the system library, we can just use CScsloc1 to do that.
        if (NULL == (localDatum = this->GetDatum (datumName.c_str())))
            return CSMap::CScsloc1 (&csDef);

        // We have to find the Ellipsoid. It can either be in this library or in the system library.
        WString ellipsoidName (localDatum->ell_knm,false);
        if (NULL == (localEllipsoid = this->GetEllipsoid (ellipsoidName.c_str())))
            localEllipsoid = CS_eldef (localDatum->ell_knm);
        }
    else if (0 != csDef.elp_knm[0])
        {
        // we do not have a datum. If the ellipsoid is in this UserLibrary, we use it. Otherwise, we just use CScsloc1.
        WString ellipsoidName (csDef.elp_knm,false);
        if (NULL == (localEllipsoid = this->GetEllipsoid (ellipsoidName.c_str())))
            return CSMap::CScsloc1 (&csDef);
        }

    CSParameters*   returnCSParams = NULL;
    if (NULL != localEllipsoid)
        {
        returnCSParams = CSMap::CScsloc2 (&csDef, localDatum, localEllipsoid);
        CSMAP_FREE_AND_CLEAR (localEllipsoid);
        }
    CSMAP_FREE_AND_CLEAR (localDatum);

    return returnCSParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WString         GetGUIName () override
    {
    return m_guiName;
    }

/*---------------------------------------------------------------------------------**//**
* NOTE: This always returns either NULL or or a CSParameters that needs to be CSMap::CS_free'd
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSParameters*   GetCS (uint32_t index) override
    {
    GetCSNames();
    if (index >= m_csNames->size())
        return NULL;
    WString csName = (*m_csNames)[index];
    return GetCS (csName.data());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool            CSInLibrary (WCharCP csName) override
    {
    GetCSNames();
    for (WString const& nameString : *m_csNames)
        {
        if (0 == nameString.CompareToI (csName))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       DeleteCS (BaseGCSP gcsToDelete) override
    {
    csFILE*     stream;
    WCharCP     csNameP = gcsToDelete->GetName();
    int32_t        readPosition;
    int32_t        writePosition;

    if (NULL == (stream = OpenLibraryFile (_STRM_BINUP)))
        return GEOCOORDERR_LibraryReadonly;

    // step past the datums and ellipsoids
    readPosition    = m_gcsStart;
    CS_fseek (stream, readPosition, SEEK_SET);
    writePosition   = readPosition;


    AString mbNameToDelete (csNameP);

    // step through the file record by record until we encounter the one we're deleting.
    for (;;)
        {
        int             crypt;
        int             readStatus;
        CSDefinition    readCsDef;

        CS_fseek (stream, readPosition, SEEK_SET);
        if (0 == (readStatus = CS_csrd (stream, &readCsDef, &crypt)))
            break;
        else if (readStatus < 0)
            {
            // sure hope this doesn't happen.
            assert (false);
            CS_fclose (stream);
            return GEOCOORDERR_IOError;
            }

        // if the one we just read is not the one we're trying to delete, write it.
        if (0 != CS_stricmp (mbNameToDelete.c_str(), readCsDef.key_nm))
            {
            // if we haven't yet encountered the one we're trying to delete, then readPosition == writePosition and there's no need to write.
            if (readPosition != writePosition)
                {
                CS_fseek (stream, writePosition, SEEK_SET);
                CS_cswr (stream, &readCsDef, crypt);
                }

            // increment write position.
            writePosition += sizeof (CSDefinition);
            }

        // increment read position
        readPosition += sizeof (CSDefinition);
        }

    // flush, and then truncate the file.
    fflush (stream);
    int fileDescriptor = _fileno (stream);
#ifdef _WIN32
    _chsize (fileDescriptor, writePosition);
#else
    #if defined (ANDROID) || defined (__APPLE__)
        ftruncate (fileDescriptor, (uint32_t)writePosition);
    #else
        ftruncate64 (fileDescriptor, (uint32_t)writePosition);
    #endif
#endif
    CS_fclose(stream);

    // Discard our list of coordinate system names.
    if (NULL != m_csNames)
        {
        delete m_csNames;
        m_csNames = NULL;
        }

    // remove the coordinate system from the cache.
    LibraryManager::Instance()->RemoveFromCache (gcsToDelete->GetName());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       CreateNewCS (WString& newName, WCharCP sourceName) override
    {
    // we want to create a new CS that's a copy of the one we get.
    BaseGCSPtr    newGCS = BaseGCS::CreateGCS (sourceName);

    if (!newGCS->IsValid())
        return GEOCOORDERR_CoordSysNotFound;

    // make up a new name, make sure it is unique across all the libraries we have.
    WChar  proposedName[1024];
    BeStringUtilities::Snwprintf (proposedName, L"Copy-%ls", sourceName);

    // make sure the name is not too long.
    proposedName[23] = 0;

    LibraryP    foundInLibrary;
    int iName;
    for (iName=1; iName<99; iName++)
        {
        if (NULL == LibraryManager::Instance()->GetCS (foundInLibrary, proposedName))
            break;
        BeStringUtilities::Snwprintf (proposedName, L"Copy_%d-%ls", iName, sourceName);
        proposedName[23] = 0;
        }

    if (iName >= 99)
        return GEOCOORDERR_CoordSysNoUniqueName;

    newGCS->SetName (proposedName);

    // add the newGCS to this library.
    StatusInt status;
    if (SUCCESS == (status = AddCS (newGCS.get())))
        {
        newName.assign (proposedName);
        if (NULL != m_csNames)
            {
            delete m_csNames;
            m_csNames = NULL;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       ReplaceCS (BaseGCSP oldGCS, BaseGCSP newGCS) override
    {
    // we need the oldGCS only to get the name, in the case where the user has modified the name.
    // If the name has changed, we need to make sure it's unique.
    StatusInt   status      = SUCCESS;
    bool        nameChanged = false;
    if (0 != BeStringUtilities::Wcsicmp (oldGCS->GetName(), newGCS->GetName()))
        {
        LibraryP    foundInLibrary;

        // name change.
        if (NULL != LibraryManager::Instance()->GetCS (foundInLibrary, newGCS->GetName()))
            return GEOCOORDERR_CoordSysNoUniqueName;
        nameChanged = true;
        }

    // copy the cs definition so we don't have to worry about the original.
    CSParameters*   newCSParams = newGCS->GetCSParameters();
    CSDefinition    newCSDef;
    CSParameters*   oldCSParams = oldGCS->GetCSParameters();
    CSDefinition    oldCSDef;

    memcpy (&newCSDef, &newCSParams->csdef, sizeof (newCSDef));
    memcpy (&oldCSDef, &oldCSParams->csdef, sizeof (oldCSDef));

    // Get the current time.
    cs_Time_    gmtTime;
    gmtTime = CS_time ((cs_Time_ *)0);
    // Compute the CSMap time, as used internally.  This is days since January 1, 1990. If this record does get
    // written, we want it to have the current date in it.
    newCSDef.protect = (short)((gmtTime - 630720000L) / 86400L);

    // Adjust the name and make sure it is all upper case. By convention, coordinate system names are case insensitive.
    if (0 != CS_nampp (newCSDef.key_nm))
        return GEOCOORDERR_CoordSysIllegalName;

    if (0 != CS_nampp (oldCSDef.key_nm))
        return GEOCOORDERR_CoordSysIllegalName;

    csFILE*         stream;
    if (NULL == (stream = OpenLibraryFile (_STRM_BINUP)))
        return GEOCOORDERR_LibraryReadonly;

    // First, we find the coordinate system position in the file (using the old name).
    if (0 >= CS_bins (stream, m_gcsStart, 0L, sizeof (oldCSDef), (char *)&oldCSDef, (CMPFUNC_CAST)CS_cscmp))
        {
        // couldn't find it.
        status = GEOCOORDERR_CoordSysNotFound;
        goto error;
        }

    // Overwrite the old definition.
    if (0 != CS_cswr (stream, &newCSDef, 1))
        {
        status = cs_IOERR;
        goto error;
        }

    // if we changed the coordinate system name, we have to re-sort the file.
    if (nameChanged)
        {
        if (0 != CS_fseek (stream, m_gcsStart, 0))
            {
            CS_erpt (status = cs_IOERR);
            goto error;
            }

        if (0 > CS_ips (stream, sizeof (newCSDef), 0L, (CMPFUNC_CAST)CS_cscmp))
            status = cs_IOERR;
        }

    // The Coordinate System Dictionary entry has been modified, make sure we take it out of our cache.
    LibraryManager::Instance()->RemoveFromCache (oldGCS->GetName());
    if (NULL != m_csNames)
        {
        delete m_csNames;
        m_csNames = NULL;
        }

error:
    CS_fclose (stream);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
public:
virtual StatusInt       GetCSName
(
uint32_t  index,
WStringR        csNameOut
) override
    {
    GetCSNames();
    if (index >= m_csNames->size())
        return ERROR;
    csNameOut.assign ((*m_csNames)[index]);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual size_t          GetCSCount () override
    {
    GetCSNames();
    return m_csNames->size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WCharCP         GetLibraryFileName () override
    {
    return m_libraryPath.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WCharCP         GetOrganizationFileName () override
    {
    if (NULL == m_orgFileName)
        {
        m_orgFileName = new WString (m_libraryPath);
        m_orgFileName->append (L".xml");
        }
    return m_orgFileName->c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool            IsUserLibrary () override
    {
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool            IsReadOnly () override
    {
    return BeFileName::IsFileReadOnly (m_libraryPath.c_str());
    }

private:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       AddCS (BaseGCSP gcs)
    {
    // Note: the coordinate system must be checked for unique name and validity before calling this method!
    // Adapted from CSMaps "cs_csupd" function.
    StatusInt       status = SUCCESS;

    // copy the cs definition so we don't have to worry about the original.
    CSParameters*   csParams = gcs->GetCSParameters();
    CSDefinition    csDef;
    memcpy (&csDef, &csParams->csdef, sizeof (csDef));

    // Get the current time.
    cs_Time_    gmtTime;
    gmtTime = CS_time ((cs_Time_ *)0);
    // Compute the CSMap time, as used internally.  This is days since January 1, 1990. If this record does get
    // written, we want it to have the current date in it.
    csDef.protect = (short)((gmtTime - 630720000L) / 86400L);

    // Adjust the name and make sure it is all upper case. By convention, coordinate system names are case insensitive.
    if (0 != CS_nampp (csDef.key_nm))
        return GEOCOORDERR_CoordSysIllegalName;

    csFILE*         stream;
    if (NULL == (stream = OpenLibraryFile (_STRM_BINUP)))
        return GEOCOORDERR_LibraryReadonly;

    // First we write the new coordinate system to the end of the file.
    if (0 != CS_fseek (stream, 0L, SEEK_END))
        {
        CS_erpt (status = cs_IOERR);
        goto error;
        }

    if (0 != CS_cswr (stream, &csDef, 1))
        {
        status = cs_IOERR;
        goto error;
        }

    // Sort the file into proper order, thereby moving the new coordinate system to its proper place in the file.
    if (0 != CS_fseek (stream, m_gcsStart, 0))
        {
        CS_erpt (status = cs_IOERR);
        goto error;
        }

    if (0 > CS_ips (stream, sizeof (csDef), 0L, (CMPFUNC_CAST)CS_cscmp))
        status = cs_IOERR;

    // The Coordinate System Dictionary has been added.
error:
    CS_fclose (stream);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt               FindCSDef (CSDefinition* csDefP, WCharCP name)
    {
    // This is fairly similar to the CS_csdef function.

    // Make sure the name is not too large.
    AString     mbName (name);
    CS_stncp (csDefP->key_nm, mbName.c_str(), _countof (csDefP->key_nm));
    if (SUCCESS != CS_nampp (csDefP->key_nm))
        return ERROR;

    /* Mark the entry, used for comparison purposes only, as not encrypted. */
    csDefP->fill[0] = '\0';

    /* Search the file for the requested coordinate system definition. */
    csFILE* stream;
    if (NULL == (stream = OpenLibraryFile (_STRM_BINRD)))
        return ERROR;

    /* Locate the coordinate system which we are to return. */
    if (0 >= CS_bins (stream, m_gcsStart, 0L, sizeof (CSDefinition), (char *)csDefP, (CMPFUNC_CAST)CS_cscmp))
        {
        CS_stncp (csErrnam, mbName.c_str(), MAXPATH);
        CS_erpt (cs_CS_NOT_FND);
        CS_fclose (stream);
        return ERROR;
        }

    /* The coordinate system definition exists.  Therefore, we malloc the coordinate system definition structure and read it in. */
    int crypt;
    int readStatus = CS_csrd (stream, csDefP, &crypt);
    CS_fclose (stream);

    if (readStatus <= 0)
        return ERROR;

    return SUCCESS;
    }

// the data following the magic number in a new-format user library.
struct  ConsolidatedFileHeader
    {
    uint32_t m_ellipsoidStart;
    uint32_t m_datumStart;
    uint32_t m_gcsStart;
    uint32_t m_reserved[4];
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
csFILE*                 OpenLibraryFile (CharCP mode)
    {
    char        libraryPath[512];
    m_libraryPath.ConvertToLocaleChars (libraryPath, sizeof (libraryPath));
    csFILE*     stream;
    if (NULL == (stream = CS_fopen (libraryPath, mode)))
        return NULL;

    return stream;
    }

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DiscoverFormat ()
    {
    csFILE*     stream;
    if (NULL == (stream = OpenLibraryFile (_STRM_BINRD)))
        return BSIERROR;

    // check start of file to make sure it's a CS_Map library file.
    FileMagicNumbers    magic = (FileMagicNumbers) 0L;
    if (sizeof (magic) != CS_fread ((char *)&magic, 1, sizeof (magic), stream))
        {
        if (CS_ferror (stream))
            CS_erpt (cs_IOERR);
        else
            CS_erpt (cs_INV_FILE);

        CS_fclose (stream);
        return BSIERROR;
        }

    if (magic == OriginalFormat)
        {
        m_consolidatedFormat    = false;
        m_ellipsoidStart        = 0;
        m_datumStart            = 0;
        m_gcsStart              = sizeof(magic);
        }
    else if (magic == ConsolidatedFormat)
        {
        ConsolidatedFileHeader  header;
        if (sizeof (ConsolidatedFileHeader) != CS_fread ((char *)&header, 1, sizeof (ConsolidatedFileHeader), stream))
            {
            CS_erpt (cs_IOERR);
            CS_fclose (stream);
            return BSIERROR;
            }
        m_consolidatedFormat    = true;
        m_ellipsoidStart        = header.m_ellipsoidStart;
        m_datumStart            = header.m_datumStart;
        m_gcsStart              = header.m_gcsStart;
        }
    else
        {
        CS_fclose (stream);
        // strcpy (csErrnam,cs_Dir);
        CS_erpt (cs_CS_BAD_MAGIC);
        return BSIERROR;
        }

    CS_fclose(stream);
    return BSISUCCESS;
    }

private:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            GetCSNames ()
    {
    // If m_csNames is not NULL, we already have a list, simply return it. */
    if (NULL != m_csNames)
        return;

    m_csNames = new T_WStringVector();

    csFILE *stream;
    if (NULL == (stream = OpenLibraryFile (_STRM_BINRD)))
        return;

    CS_fseek (stream, m_gcsStart, SEEK_SET);

    CSDefinition cs_def;
    int st;
    int crypt;
    while ((st = CS_csrd (stream, &cs_def, &crypt)) > 0)
        {
        // Add the new item to the list.
        m_csNames->push_back (WString(cs_def.key_nm,false));
        }
    CS_fclose (stream);
    }


// -------------------- Datum methods --------------------------
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSDatumDef*     GetDatum (WCharCP datumName) override
    {
    double   cs_DelMax = 50000.0;
    double   cs_RotMax = 50000.0;
    double   cs_SclMax = 2000.0;

    if (!m_consolidatedFormat)
        return NULL;

    AString mbDatumName (datumName);
    // this was adapted from cs_dtdef in CS_dtio.c

    /* Verify the name is OK. */
    CSDatumDef  localDatumDef;
    CS_stncp (localDatumDef.key_nm, mbDatumName.c_str(), sizeof (localDatumDef.key_nm));
    if (0 != CS_nampp (localDatumDef.key_nm))
        return NULL;

    /* Mark this name as unencrypted so that the comaprison function will work. */
    localDatumDef.fill [0] = '\0';

    /* Prepare for an error condition. */
    csFILE*     stream;
    /* Open the Datum Dictionary and test its magic number. */
    if (NULL == (stream = OpenLibraryFile (_STRM_BINRD)))
        return NULL;

    CSDatumDef* dtptr = NULL;

    // Search for the requested datum. The search has to stop at the end of the used datum slots.
    int32_t        sortEndPosition = m_datumStart + ((uint32_t) GetDatumCount() * sizeof (CSDatumDef));
    int flag = CS_bins (stream, m_datumStart, sortEndPosition, sizeof (localDatumDef), &localDatumDef, (CMPFUNC_CAST)CS_dtcmp);
    if (flag < 0)
        goto error;

    /* Tell the user if we didn't find the requested datum. */
    if (!flag)
        {
        CS_stncp (csErrnam, mbDatumName.c_str(), MAXPATH);
        CS_erpt (cs_DT_NOT_FND);
        goto error;
        }
    else
        {
        /* The datum exists, malloc some memory for it. */
        dtptr = (CSDatumDef *)CS_malc (sizeof (*dtptr));
        if (dtptr == NULL)
            {
            CS_erpt (cs_NO_MEM);
            goto error;
            }

        /* Read it in. */
        int     crypt;
        if (!CS_dtrd (stream,dtptr,&crypt))
            {
            goto error;
            }
        }

    /* We don't need the datum dictionary anymore. */
    CS_fclose (stream);
    stream = NULL;

    /* Verify that the values are not completely bogus. Values from CSMap. Comment in cs_dtio.c indicates completely different values, but CSdata.c sets them as shown here. */
    if (fabs (dtptr->delta_X) > cs_DelMax || fabs (dtptr->delta_Y) > cs_DelMax || fabs (dtptr->delta_Z) > cs_DelMax ||
            fabs (dtptr->rot_X) > cs_RotMax    || fabs (dtptr->rot_Y) > cs_RotMax || fabs (dtptr->rot_Z) > cs_RotMax ||
            fabs (dtptr->bwscale) > cs_SclMax)
        {
        CS_stncp (csErrnam, mbDatumName.c_str(), MAXPATH);
        CS_erpt (cs_DTDEF_INV);
        goto error;
        }

    /* Return the initialized datum structure to the user. */
    return dtptr;

error:
    if (NULL != stream)
        CS_fclose (stream);

    CSMAP_FREE_AND_CLEAR (dtptr);

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSDatumDef*     GetDatum (uint32_t index) override
    {
    if (!m_consolidatedFormat)
        return NULL;

    GetDatumNames();
    if (index >= m_datumNames->size())
        return NULL;

    WString     datumName = (*m_datumNames)[index];
    return GetDatum (datumName.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool            DatumInLibrary (WCharCP datumName) override
    {
    GetDatumNames();
    for (WString const& nameString : *m_datumNames)
        {
        if (0 == nameString.CompareToI (datumName))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       DeleteDatum (WCharCP datumName) override
    {
    if (!m_consolidatedFormat)
        return BSIERROR;

    csFILE*     stream;
    if (NULL == (stream = OpenLibraryFile (_STRM_BINUP)))
        return GEOCOORDERR_LibraryReadonly;

    // make sure the datum isn't used by any of the GCS's in this library.
    if (DatumUsedByAnyGCS (stream, datumName))
        return GEOCOORDERR_DatumInUse;

    // step past the ellipsoids
    int     maxDatums       = (m_gcsStart - m_datumStart) / sizeof (CSDatumDef);
    int32_t    readPosition    = m_datumStart;
    int32_t    writePosition   = readPosition;

    CS_fseek (stream, readPosition, SEEK_SET);

    AString mbDatumName (datumName);

    // step through the file record by record until we encounter the one we're deleting.
    for (int iDatum = 0; iDatum < maxDatums; iDatum++)
        {
        int             crypt;
        int             readStatus;
        CSDatumDef      readDatumDef;

        if (IsEmptyDatumRecord (stream, readPosition))
            break;

        if (0 == (readStatus = CS_dtrd (stream, &readDatumDef, &crypt)))
            break;

        // if the one we just read is not the one we're trying to delete, write it.
        if (0 != CS_stricmp (mbDatumName.c_str(), readDatumDef.key_nm))
            {
            // if we haven't yet encountered the one we're trying to delete, then readPosition == writePosition and there's no need to write.
            if (readPosition > writePosition)
                {
                CS_fseek (stream, writePosition, SEEK_SET);
                CS_dtwr (stream, &readDatumDef, crypt);
                }

            // increment write position.
            writePosition += sizeof (CSDatumDef);
            }

        // increment read position
        readPosition += sizeof (CSDatumDef);
        }

    // if readPosition != writePosition, then we found the one we want to delete, and we moved them all down. We have to zero out the current one.
    if (readPosition > writePosition)
        {
        CSDatumDef emptyDatum;
        memset (&emptyDatum, 0, sizeof (emptyDatum));
        CS_fseek (stream, writePosition, SEEK_SET);
        CS_fwrite (&emptyDatum, 1, sizeof (emptyDatum), stream);
        }

    // Close the file. (Don't truncate!)
    CS_fclose(stream);

    // Discard our list of datum names.
    DELETE_AND_CLEAR (m_datumNames);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       CreateNewDatum (DatumP& newDatum, WCharCP seedName) override
    {
    Library*    systemLibrary = LibraryManager::Instance()->GetSystemLibrary();

    // we want to create a new Datum that's a copy of the seed (if supplied).
    WChar  proposedName[1024];
    if ((NULL == seedName) || (0 == seedName[0]))
        seedName = L"NewDatum";

    wcscpy (proposedName, seedName);

    // make sure the name is not too long.
    proposedName[23] = 0;

    int iName;
    for (iName=1; iName<99; iName++)
        {
        if ( (!this->DatumInLibrary (proposedName)) && (!systemLibrary->DatumInLibrary (proposedName)) )
            break;
        BeStringUtilities::Snwprintf (proposedName, L"%ls-%d", seedName, iName);
        proposedName[23] = 0;
        }

    if (iName >= 99)
        return GEOCOORDERR_DatumNoUniqueName;

    CSDatumDef  newDatumDef;
    memset (&newDatumDef, 0, sizeof(newDatumDef));
    CS_stncp (newDatumDef.key_nm, AString(proposedName).c_str(), _countof (newDatumDef.key_nm));

    // make up some defaults.
    CS_stncp (newDatumDef.ell_knm, "INTNL", sizeof (newDatumDef.ell_knm));
    newDatumDef.to84_via = cs_DTCTYP_MOLO;

    newDatum = const_cast <DatumP> (Datum::CreateDatum (newDatumDef, this));

    return newDatum->IsValid() ? BSISUCCESS : newDatum->GetError();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       ReplaceDatum (const CSDatumDef& oldDatum, const CSDatumDef& newDatum) override
    {
    if (!m_consolidatedFormat)
        return BSIERROR;

    // make a copy so we don't have to worry about modifying the original.
    CSDatumDef  newDatumDef;
    CSDatumDef  oldDatumDef;
    memcpy (&newDatumDef, &newDatum, sizeof(newDatumDef));
    memcpy (&oldDatumDef, &oldDatum, sizeof(oldDatumDef));

    // we need the oldDatum only to get the name, in the case where the user has modified the name.
    // If the name has changed, we need to make sure it's unique.
    StatusInt   status      = SUCCESS;
    bool        nameChanged = false;
    uint32_t    numDatums   = 0;
    if (0 != BeStringUtilities::Stricmp (oldDatumDef.key_nm, newDatumDef.key_nm))
        {
        WString newDatumName (newDatumDef.key_nm,false);

        // name change. Make sure the name isn't already in this library.
        if (this->DatumInLibrary (newDatumName.c_str()))
            return GEOCOORDERR_DatumNoUniqueName;

        // and make sure it's not in the system library.
        if (LibraryManager::Instance()->GetSystemLibrary()->DatumInLibrary (newDatumName.c_str()))
            return GEOCOORDERR_DatumNoUniqueName;

        nameChanged = true;
        numDatums = (uint32_t) GetDatumCount();
        }

    // Get the current time.
    cs_Time_    gmtTime;
    gmtTime = CS_time ((cs_Time_ *)0);
    // Compute the CSMap time, as used internally.  This is days since January 1, 1990. If this record does get
    // written, we want it to have the current date in it.
    newDatumDef.protect = (short)((gmtTime - 630720000L) / 86400L);

    // Adjust the name and make sure it is all upper case. By convention, datum names are case insensitive.
    if (0 != CS_nampp (newDatumDef.key_nm))
        return GEOCOORDERR_DatumIllegalName;

    if (0 != CS_nampp (oldDatumDef.key_nm))
        return GEOCOORDERR_DatumIllegalName;

    // find the end position (before opening the file).
    int32_t        sortEndPosition = m_datumStart + ((uint32_t) GetDatumCount() * sizeof (CSDatumDef));

    csFILE*         stream;
    if (NULL == (stream = OpenLibraryFile (_STRM_BINUP)))
        return GEOCOORDERR_LibraryReadonly;

    // First, we find the datum in the file (using the old name).
    if (0 >= CS_bins (stream, m_datumStart, sortEndPosition, sizeof (oldDatumDef), (char *)&oldDatumDef, (CMPFUNC_CAST)CS_dtcmp))
        {
        // couldn't find it.
        status = GEOCOORDERR_DatumNotFound;
        goto error;
        }

    // Overwrite the old definition.
    if (0 != CS_dtwr (stream, &newDatumDef, 0))
        {
        status = cs_IOERR;
        goto error;
        }

    // if we changed the datum name, we have to re-sort the file.
    if (nameChanged)
        {
        // seek to the start of the datum.
        if (0 != CS_fseek (stream, m_datumStart, 0))
            {
            CS_erpt (status = cs_IOERR);
            goto error;
            }

        if (0 > CS_ips (stream, sizeof (newDatumDef), m_datumStart + numDatums * sizeof(CSDatumDef), (CMPFUNC_CAST)CS_dtcmp))
            status = cs_IOERR;

        // If we renamed the datum, we need to change the datum name in every GCS that uses that datum.
        this->DatumRenamed (stream, oldDatumDef.key_nm, newDatumDef.key_nm);
        }

    // The Datum entry has been modified, make sure we remove every GCS in our cache that uses it.
    LibraryManager::Instance()->RemoveAllUsingDatumFromCache (oldDatumDef.key_nm);

    DELETE_AND_CLEAR (m_datumNames);

error:
    CS_fclose (stream);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       GetDatumName (uint32_t index, WStringR datumName) override
    {
    if (!m_consolidatedFormat)
        return BSIERROR;

    GetDatumNames();
    if (index >= m_datumNames->size())
        return ERROR;

    datumName.assign ((*m_datumNames)[index]);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual size_t          GetDatumCount () override
    {
    if (!m_consolidatedFormat)
        return 0;
    GetDatumNames();
    return m_datumNames->size();
    }


private:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool                    DatumUsedByAnyGCS (csFILE* stream, WCharCP datumName)
    {
    // step past the datums and ellipsoids
    int32_t    readPosition    = m_gcsStart;
    CS_fseek (stream, readPosition, SEEK_SET);

    AString searchName (datumName);
    // step through the file record by record until we encounter one that uses the Datum we're looking for.
    for (;;)
        {
        int             crypt;
        int             readStatus;
        CSDefinition    readCsDef;

        CS_fseek (stream, readPosition, SEEK_SET);
        if (0 == (readStatus = CS_csrd (stream, &readCsDef, &crypt)))
            return false;
        else if (readStatus < 0)
            return false;

        // if the one we just read is not the one we're trying to delete, write it.
        if (0 == CS_stricmp (searchName.c_str(), readCsDef.dat_knm))
            return true;

        // increment read position
        readPosition += sizeof (CSDefinition);
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void            DatumRenamed (csFILE* stream, CharCP oldDatumName, CharCP newDatumName)
    {
    // step past the datums and ellipsoids
    int32_t    readPosition    = m_gcsStart;
    CS_fseek (stream, readPosition, SEEK_SET);

    // step through the file record by record until we encounter one that uses the Datum we're looking for.
    for (;;)
        {
        int             crypt;
        int             readStatus;
        CSDefinition    readCsDef;

        CS_fseek (stream, readPosition, SEEK_SET);
        if (0 == (readStatus = CS_csrd (stream, &readCsDef, &crypt)))
            break;
        else if (readStatus < 0)
            break;

        // if the one we just read is not the one we're trying to delete, write it.
        if (0 == CS_stricmp (oldDatumName, readCsDef.dat_knm))
            {
            strcpy (readCsDef.dat_knm, newDatumName);
            CS_fseek (stream, readPosition, SEEK_SET);
            CS_cswr (stream, &readCsDef, crypt);
            }

        // increment read position
        readPosition += sizeof (CSDefinition);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool                    EllipsoidUsedByAnyDatumOrGCS (csFILE* stream, CharCP ellipsoidName)
    {
    // step past the datums and ellipsoids
    int32_t    readPosition    = m_datumStart;
    CS_fseek (stream, readPosition, SEEK_SET);

    // step through the file Datum's record by record until we encounter one that uses the Datum we're looking for.
    for (;;)
        {
        int             crypt;
        int             readStatus;
        CSDatumDef      readDatumDef;

        CS_fseek (stream, readPosition, SEEK_SET);
        if (0 == (readStatus = CS_dtrd (stream, &readDatumDef, &crypt)))
            break;
        else if (readStatus < 0)
            break;

        if (0 == CS_stricmp (ellipsoidName, readDatumDef.ell_knm))
            return true;

        // increment read position
        readPosition += sizeof (CSDatumDef);
        }

    readPosition    = m_gcsStart;
    CS_fseek (stream, readPosition, SEEK_SET);
    // step through the file GCS's record by record until we encounter one that uses the Ellipsoid we're looking for.
    for (;;)
        {
        int             crypt;
        int             readStatus;
        CSDefinition    readCsDef;

        CS_fseek (stream, readPosition, SEEK_SET);
        if (0 == (readStatus = CS_csrd (stream, &readCsDef, &crypt)))
            return false;
        else if (readStatus < 0)
            return false;

        if (0 == CS_stricmp (ellipsoidName, readCsDef.elp_knm))
            return true;

        // increment read position
        readPosition += sizeof (CSDefinition);
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            EllipsoidRenamed (csFILE* stream, CharCP oldEllipsoidName, CharCP newEllipsoidName)
    {
    // step past the datums and ellipsoids
    int32_t    readPosition    = m_datumStart;
    CS_fseek (stream, readPosition, SEEK_SET);

    // step through the file Datum's record by record until we encounter one that uses the Datum we're looking for.
    for (;;)
        {
        int             crypt;
        int             readStatus;
        CSDatumDef      readDatumDef;

        CS_fseek (stream, readPosition, SEEK_SET);
        if (0 == (readStatus = CS_dtrd (stream, &readDatumDef, &crypt)))
            break;
        else if (readStatus < 0)
            break;

        if (0 == CS_stricmp (oldEllipsoidName, readDatumDef.ell_knm))
            {
            strcpy (readDatumDef.ell_knm, newEllipsoidName);
            CS_fseek (stream, readPosition, SEEK_SET);
            CS_dtwr (stream, &readDatumDef, crypt);
            }

        // increment read position
        readPosition += sizeof (CSDatumDef);
        }

    readPosition    = m_gcsStart;
    CS_fseek (stream, readPosition, SEEK_SET);
    // step through the file GCS's record by record until we encounter one that uses the Ellipsoid we're looking for.
    for (;;)
        {
        int             crypt;
        int             readStatus;
        CSDefinition    readCsDef;

        CS_fseek (stream, readPosition, SEEK_SET);
        if (0 == (readStatus = CS_csrd (stream, &readCsDef, &crypt)))
            break;
        else if (readStatus < 0)
            break;

        // if the one we just read is not the one we're trying to delete, write it.
        if (0 == CS_stricmp (oldEllipsoidName, readCsDef.elp_knm))
            {
            strcpy (readCsDef.elp_knm, newEllipsoidName);
            CS_fseek (stream, readPosition, SEEK_SET);
            CS_cswr (stream, &readCsDef, crypt);
            }

        // increment read position
        readPosition += sizeof (CSDefinition);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       AddDatum (const CSDatumDef& datum) override
    {
    if (!m_consolidatedFormat)
        {
        StatusInt   status;
        if (BSISUCCESS != (status = ConvertToConsolidatedFile (NUM_USER_ELLIPSOIDS, NUM_USER_DATUMS)))
            return status;
        }

    // Note: the datum must be checked for unique name and validity before calling this method!
    StatusInt       status = SUCCESS;

    // make a copy so we don't change the original.
    CSDatumDef      datumDef;
    memcpy (&datumDef, &datum, sizeof(datumDef));

    // Get the current time.
    cs_Time_    gmtTime;
    gmtTime = CS_time ((cs_Time_ *)0);
    // Compute the CSMap time, as used internally.  This is days since January 1, 1990. If this record does get
    // written, we want it to have the current date in it.
    datumDef.protect = (short)((gmtTime - 630720000L) / 86400L);

    // Adjust the name and make sure it is all upper case. By convention, coordinate system names are case insensitive.
    if (0 != CS_nampp (datumDef.key_nm))
        return GEOCOORDERR_DatumIllegalName;

    csFILE*         stream;
    if (NULL == (stream = OpenLibraryFile (_STRM_BINUP)))
        return GEOCOORDERR_LibraryReadonly;

    // Step through until we find the end of the datums.
    int32_t    writePosition = FindFirstEmptyDatumRecord (stream);
    if (writePosition < 0)
        {
        assert (false);
        status = GEOCOORDERR_MaxUserLibraryDatums;
        goto error;
        }

    // IsEmptyDatumRecord leaves us positioned at the beginning of the empty datum record.
    if (0 != CS_dtwr (stream, &datumDef, 0))
        {
        assert (false);
        status = GEOCOORDERR_IOError;
        goto error;
        }

    // Sort the file into proper order, thereby moving the new datum to its proper place in the file.
    if (0 != CS_fseek (stream, m_datumStart, 0))
        {
        assert (false);
        CS_erpt (status = cs_IOERR);
        goto error;
        }

    if (0 > CS_ips (stream, sizeof (datumDef), writePosition + sizeof (datumDef), (CMPFUNC_CAST)CS_dtcmp))
        status = cs_IOERR;

error:
    // datumNames is no longer up to date.
    if (NULL != m_datumNames)
        {
        delete m_datumNames;
        m_datumNames = NULL;
        }

    CS_fclose (stream);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       InitializeUserEllipsoidsInFile (csFILE* stream, uint32_t numUserEllipsoids)
    {
    CSEllipsoidDef  zeroEllipsoidDef;
    memset (&zeroEllipsoidDef, 0, sizeof (zeroEllipsoidDef));

    uint32_t writePosition   = m_ellipsoidStart;
    CS_fseek (stream, writePosition, SEEK_SET);
    for (uint32_t iEllipsoid = 0; iEllipsoid < numUserEllipsoids; iEllipsoid++)
        {
        size_t writeCount = CS_fwrite (&zeroEllipsoidDef, 1, sizeof (zeroEllipsoidDef), stream);

        if (sizeof (zeroEllipsoidDef) != writeCount)
            {
            assert (false);
            return BSIERROR;
            }
        writePosition += sizeof (zeroEllipsoidDef);
        }
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       InitializeUserDatumsInFile (csFILE* stream, uint32_t numUserDatums)
    {
    CSDatumDef  zeroDatumDef;
    memset (&zeroDatumDef, 0, sizeof (zeroDatumDef));

    uint32_t writePosition   = m_datumStart;
    CS_fseek (stream, writePosition, SEEK_SET);
    for (uint32_t iDatum = 0; iDatum < numUserDatums; iDatum++)
        {
        size_t writeCount = CS_fwrite (&zeroDatumDef, 1, sizeof (zeroDatumDef), stream);
        if (sizeof (zeroDatumDef) != writeCount)
            {
            assert (false);
            return BSIERROR;
            }

        writePosition += sizeof (zeroDatumDef);
        }
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       WriteHeaderToFile (csFILE* stream)
    {
    CS_fseek (stream, 0, SEEK_SET);

    FileMagicNumbers    magic = ConsolidatedFormat;
    size_t writeCount = CS_fwrite (&magic, 1, sizeof (magic), stream);
    if (sizeof (magic) != writeCount)
        {
        assert (false);
        return BSIERROR;
        }
    ConsolidatedFileHeader header;
    memset (&header, 0, sizeof (header));
    header.m_ellipsoidStart = m_ellipsoidStart;
    header.m_datumStart     = m_datumStart;
    header.m_gcsStart       = m_gcsStart;

    writeCount = CS_fwrite (&header, 1, sizeof (header), stream);
    if (sizeof (header) != writeCount)
        {
        assert (false);
        return BSIERROR;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ConvertToConsolidatedFile (uint32_t numUserEllipsoids, uint32_t numUserDatums)
    {
    // we convert the file in place.
    // The new file format leaves space at the beginning for user defined Datums and user defined ellipsoids.
    // Therefore, we must write the existing CSParameter data from the last one to the first one.

    // first see if we can open the file.
    csFILE*     stream;
    if (NULL == (stream = OpenLibraryFile (_STRM_BINUP)))
        return GEOCOORDERR_LibraryReadonly;

    // get starting read position
    uint32_t gcsCount        = (uint32_t) this->GetCSCount();
    uint32_t oldGCSStart     = m_gcsStart;

    // update the starting positions
    m_ellipsoidStart        = sizeof (FileMagicNumbers) + sizeof (ConsolidatedFileHeader);
    m_datumStart            = m_ellipsoidStart + (numUserEllipsoids * sizeof (CSEllipsoidDef));
    m_gcsStart              = m_datumStart + (numUserDatums * sizeof (CSDatumDef));

    if (0 != gcsCount)
        {
        int32_t    readPosition    = oldGCSStart + ((gcsCount-1) * sizeof(CSDefinition));
        int32_t    writePosition   = m_gcsStart  + ((gcsCount-1) * sizeof(CSDefinition));
        for (unsigned int iGCS = 0; iGCS < gcsCount; iGCS++)
            {
            CS_fseek (stream, readPosition, SEEK_SET);

            int             crypt;
            int             readStatus;
            CSDefinition    readCsDef;
            if (0 == (readStatus = CS_csrd (stream, &readCsDef, &crypt)))
                {
                assert (false);
                break;
                }

            CS_fseek (stream, writePosition, SEEK_SET);
            if (CS_cswr (stream, &readCsDef, crypt))
                {
                // sure hope this doesn't happen!
                assert (false);
                break;
                }
            readPosition  -= sizeof (CSDefinition);
            writePosition -= sizeof (CSDefinition);
            }
        }

    // all GCS's have been transferred to their new positions. Now zero out the datums and ellipsoids.
    InitializeUserEllipsoidsInFile (stream, numUserEllipsoids);
    InitializeUserDatumsInFile (stream, numUserDatums);
    WriteHeaderToFile (stream);

    CS_fclose (stream);

    m_consolidatedFormat = true;

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IsEmptyDatumRecord (csFILE* stream, int32_t readPosition)
    {
    bool        returnVal = false;
    CSDatumDef  datumDef;
    CS_fseek (stream, readPosition, SEEK_SET);

    // see if we can read it.
    if (sizeof (datumDef) != CS_fread ((char *)&datumDef, 1, sizeof (datumDef), stream))
        {
        assert (false);
        returnVal = true;
        }
    // if the name start with 0, this is a blank entry.
    if (0 == datumDef.key_nm[0])
        returnVal = true;

    // seek back to the same place.
    CS_fseek (stream, readPosition, SEEK_SET);
    return returnVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t            FindFirstEmptyDatumRecord (csFILE* stream)
    {
    // Step through until we find the end of the datums.
    int32_t    readPosition    = m_datumStart;
    int     maxDatums       = (m_gcsStart - m_datumStart) / sizeof (CSDatumDef);
    int     iDatum;
    // step through the file record by record until we find the end.
    for (iDatum=0; iDatum < maxDatums; iDatum++)
        {
        if (IsEmptyDatumRecord (stream, readPosition))
            break;

        readPosition += sizeof(CSDatumDef);
        }

    // are we out of room for datums?
    if (iDatum >= maxDatums)
        return -1;

    // the current file position is at the first empty record.
    return readPosition;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t            FindFirstEmptyEllipsoidRecord (csFILE* stream)
    {
    // Step through until we find the end of the ellipsoids.
    int32_t    readPosition    = m_ellipsoidStart;
    int     maxEllipsoids   = (m_datumStart - m_ellipsoidStart) / sizeof (CSEllipsoidDef);
    int     iEllipsoid;
    // step through the file record by record until we find the end.
    for (iEllipsoid=0; iEllipsoid < maxEllipsoids; iEllipsoid++)
        {
        if (IsEmptyEllipsoidRecord (stream, readPosition))
            break;

        readPosition += sizeof(CSEllipsoidDef);
        }

    // are we out of room for ellipsoids ?
    if (iEllipsoid >= maxEllipsoids)
        return -1;

    // the current file position is at the first empty record.
    return readPosition;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            GetDatumNames ()
    {
    // If m_datumNames is not NULL, we already have a list, simply return it. */
    if (NULL != m_datumNames)
        return;

    m_datumNames = new T_WStringVector();

    // no datums, just create empty list.
    if (!m_consolidatedFormat)
        return;

    csFILE *stream;
    if (NULL == (stream = OpenLibraryFile (_STRM_BINRD)))
        return;

    CSDatumDef  datumDef;
    int         maxDatums   = (m_gcsStart - m_datumStart) / sizeof (datumDef);
    int         iDatum      = 0;
    uint32_t    readPosition = m_datumStart;
    for (iDatum = 0; iDatum < maxDatums; iDatum++)
        {
        // we first read to see whether it's a blank datum
        if (IsEmptyDatumRecord (stream, readPosition))
            break;

        // if CS_dtrd returns 0 or negative, we ran out. This happens only if there are no GCS's in the library, which we don't expect to happen.
        int     crypt;
        if (CS_dtrd (stream, &datumDef, &crypt) <= 0)
            {
            assert (false);
            break;
            }

        // Add the new item to the list.
        m_datumNames->push_back (WString(datumDef.key_nm,false));
        readPosition += sizeof (CSDatumDef);
        }
    CS_fclose (stream);
    }

// Ellipsoid methods
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSEllipsoidDef* GetEllipsoid (WCharCP ellipsoidName) override
    {
    double cs_ERadMax = 7.0E+08; // Modified from original value of 7.0E+06 to support solar system objects other than Earth.
    double cs_PRadMax = 7.0E+08; // Modified from original value of 7.0E+06 to support solar system objects other than Earth.
    double cs_ERadMin = 1.0E+02; // Modified from original value of 6.0E+06 to support solar system objects other than Earth.
    double cs_PRadMin = 1.0E+02; // Modified from original value of 6.0E+06 to support solar system objects other than Earth.

    if (!m_consolidatedFormat)
        return NULL;

    AString mbEllipsoidName (ellipsoidName);
    // this was adapted from cs_eldef in CS_elio.c

    /* Verify the name is OK. */
    CSEllipsoidDef  localEllipsoidDef;
    CS_stncp (localEllipsoidDef.key_nm, mbEllipsoidName.c_str(), sizeof (localEllipsoidDef.key_nm));
    if (0 != CS_nampp (localEllipsoidDef.key_nm))
        return NULL;

    /* Mark this name as unencrypted so that the comaprison function will work. */
    localEllipsoidDef.fill [0] = '\0';

    /* Prepare for an error condition. */
    csFILE*     stream;
    /* Open the Dictionary and test its magic number. */
    if (NULL == (stream = OpenLibraryFile (_STRM_BINRD)))
        return NULL;

    CSEllipsoidDef* elptr = NULL;
    double my_flat = 0.0;

    // Search for the requested ellipsoid . The search has to stop at the end of the used ellipsoid slots.
    int32_t        sortEndPosition = m_ellipsoidStart + ((uint32_t) GetEllipsoidCount() * sizeof (CSEllipsoidDef));
    int flag = CS_bins (stream, m_ellipsoidStart, sortEndPosition, sizeof (localEllipsoidDef), &localEllipsoidDef, (CMPFUNC_CAST)CS_elcmp);
    if (flag < 0)
        goto error;

    /* Tell the user if we didn't find the requested ellipsoid. */
    if (!flag)
        {
        CS_stncp (csErrnam, mbEllipsoidName.c_str(), MAXPATH);
        CS_erpt (cs_EL_NOT_FND);
        goto error;
        }
    else
        {
        /* The ellipsoid exists, malloc some memory for it. */
        elptr = (CSEllipsoidDef *)CS_malc (sizeof (*elptr));
        if (elptr == NULL)
            {
            CS_erpt (cs_NO_MEM);
            goto error;
            }

        /* Read it in. */
        int     crypt;
        if (!CS_elrd (stream,elptr,&crypt))
            {
            goto error;
            }
        }

    /* We don't need the dictionary anymore. */
    CS_fclose (stream);
    stream = NULL;

    /* Check the ellipsoid definition for valitity. */
    if (elptr->e_rad < cs_ERadMin || elptr->e_rad > cs_ERadMax ||
        elptr->p_rad < cs_PRadMin || elptr->p_rad > cs_PRadMax)
        {
        CS_stncp (csErrnam, mbEllipsoidName.c_str(), MAXPATH);
        CS_erpt (cs_ELDEF_INV);
        goto error;
        }

    /* e_rad and p_rad are both greater than zero. */
    my_flat = 1.0 - (elptr->p_rad / elptr->e_rad);
    if (my_flat < 0.0)
        {
        CS_stncp (csErrnam,elptr->key_nm,MAXPATH);
        CS_erpt (cs_ELDEF_INV);
        goto error;
        }
    else if (my_flat < 1.0E-07)
        {
        elptr->p_rad = elptr->e_rad;
        elptr->flat  = 0.0;
        elptr->ecent = 0.0;
        }
    else
        {
        double cs_EccentMax = 0.2;
        double my_ecent = sqrt (2.0 * my_flat - (my_flat * my_flat));
        if (my_ecent > cs_EccentMax || fabs (my_ecent - elptr->ecent) > 1.0E-08 || fabs (my_flat  - elptr->flat)   > 1.0E-08)
            {
            CS_stncp (csErrnam, mbEllipsoidName.c_str(), MAXPATH);
            CS_erpt (cs_ELDEF_INV);
            goto error;
            }
        }

    /* Return the initialized ellipsoid structure to the user. */
    return elptr;

error:
    if (NULL != stream)
        CS_fclose (stream);

    CSMAP_FREE_AND_CLEAR (elptr);
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSEllipsoidDef* GetEllipsoid (uint32_t index) override
    {
    if (!m_consolidatedFormat)
        return NULL;

    GetEllipsoidNames();
    if (index >= m_ellipsoidNames->size())
        return NULL;

    WString ellipsoidName = (*m_ellipsoidNames)[index];
    return GetEllipsoid (ellipsoidName.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool            EllipsoidInLibrary (WCharCP ellipsoidName) override
    {
    GetEllipsoidNames();
    for (WString const& nameString : *m_ellipsoidNames)
        {
        if (0 == nameString.CompareToI (ellipsoidName))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       DeleteEllipsoid (WCharCP ellipsoidName) override
    {
    if (!m_consolidatedFormat)
        return BSIERROR;

    AString mbEllipsoidName (ellipsoidName);

    csFILE*     stream;
    if (NULL == (stream = OpenLibraryFile (_STRM_BINUP)))
        return GEOCOORDERR_LibraryReadonly;

    // make sure the ellipsoid isn't used by any of the Datums or GCS's in this library.
    if (EllipsoidUsedByAnyDatumOrGCS (stream, mbEllipsoidName.c_str()))
        return GEOCOORDERR_EllipsoidInUse;

    // step past the header
    int     maxEllipsoids   = (m_datumStart - m_ellipsoidStart) / sizeof (CSEllipsoidDef);
    int32_t    readPosition    = m_ellipsoidStart;
    int32_t    writePosition   = readPosition;

    CS_fseek (stream, readPosition, SEEK_SET);

    // step through the file record by record until we encounter the one we're deleting.
    for (int iEllipsoid = 0; iEllipsoid < maxEllipsoids; iEllipsoid++)
        {
        int             crypt;
        int             readStatus;
        CSEllipsoidDef  readEllipsoidDef;

        if (IsEmptyEllipsoidRecord (stream, readPosition))
            break;

        if (0 == (readStatus = CS_elrd (stream, &readEllipsoidDef, &crypt)))
            break;

        // if the one we just read is not the one we're trying to delete, write it.
        if (0 != CS_stricmp (mbEllipsoidName.c_str(), readEllipsoidDef.key_nm))
            {
            // if we haven't yet encountered the one we're trying to delete, then readPosition == writePosition and there's no need to write.
            if (readPosition > writePosition)
                {
                CS_fseek (stream, writePosition, SEEK_SET);
                CS_elwr (stream, &readEllipsoidDef, crypt);
                }

            // increment write position.
            writePosition += sizeof (CSEllipsoidDef);
            }

        // increment read position
        readPosition += sizeof (CSEllipsoidDef);
        }

    // if readPosition != writePosition, then we found the one we want to delete, and we moved them all down. We have to zero out the current one.
    if (readPosition > writePosition)
        {
        CSEllipsoidDef emptyEllipsoid;
        memset (&emptyEllipsoid, 0, sizeof (emptyEllipsoid));
        CS_fseek (stream, writePosition, SEEK_SET);
        CS_fwrite (&emptyEllipsoid, 1, sizeof (emptyEllipsoid), stream);
        }

    // Close the file. (Don't truncate!)
    CS_fclose(stream);

    // Discard our list of ellipsoid names.
    DELETE_AND_CLEAR (m_ellipsoidNames);

    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       CreateNewEllipsoid (CSEllipsoidDef*& newEllipsoidDef, WCharCP seedName) override
    {
    Library*    systemLibrary = LibraryManager::Instance()->GetSystemLibrary();

    // we want to create a new Ellipsoid that's a copy of the seed (if supplied).
    WChar   proposedName[1024];
    if ((NULL == seedName) || (0 == seedName[0]))
        seedName = L"NewEllipsoid";

    wcscpy (proposedName, seedName);

    // make sure the name is not too long.
    proposedName[23] = 0;

    int iName;
    for (iName=1; iName<99; iName++)
        {
        if ( (NULL == this->GetEllipsoid (proposedName)) && (NULL == systemLibrary->GetEllipsoid (proposedName)) )
            break;
        BeStringUtilities::Snwprintf (proposedName, L"%ls-%d", seedName, iName);
        proposedName[23] = 0;
        }

    if (iName >= 99)
        return GEOCOORDERR_EllipsoidNoUniqueName;

    newEllipsoidDef = (CSEllipsoidDef *)CS_malc (sizeof (CSEllipsoidDef));
    memset (newEllipsoidDef, 0, sizeof(CSEllipsoidDef));
    CS_stncp (newEllipsoidDef->key_nm, AString(proposedName).c_str(), sizeof(newEllipsoidDef->key_nm));

    // make up some defaults.
    newEllipsoidDef->e_rad = 6378137.0;
    newEllipsoidDef->p_rad = 6356752.3142;
    Ellipsoid::CalculateParameters (newEllipsoidDef->flat, newEllipsoidDef->ecent, newEllipsoidDef->e_rad, newEllipsoidDef->p_rad);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       AddEllipsoid (const CSEllipsoidDef& ellipsoid) override
    {
    if (!m_consolidatedFormat)
        {
        StatusInt   status;
        if (BSISUCCESS != (status = ConvertToConsolidatedFile (NUM_USER_ELLIPSOIDS, NUM_USER_DATUMS)))
            return status;
        }

    // Note: the ellipsoid must be checked for unique name and validity before calling this method!
    StatusInt       status = SUCCESS;

    // make a copy so we don't change the original.
    CSEllipsoidDef      ellipsoidDef;
    memcpy (&ellipsoidDef, &ellipsoid, sizeof(ellipsoidDef));

    // Get the current time.
    cs_Time_    gmtTime;
    gmtTime = CS_time ((cs_Time_ *)0);
    // Compute the CSMap time, as used internally.  This is days since January 1, 1990. If this record does get
    // written, we want it to have the current date in it.
    ellipsoidDef.protect = (short)((gmtTime - 630720000L) / 86400L);

    // Adjust the name and make sure it is all upper case. By convention, coordinate system names are case insensitive.
    if (0 != CS_nampp (ellipsoidDef.key_nm))
        return GEOCOORDERR_EllipsoidIllegalName;

    csFILE*         stream;
    if (NULL == (stream = OpenLibraryFile (_STRM_BINUP)))
        return GEOCOORDERR_LibraryReadonly;

    // Step through until we find the end of the ellipsoids.
    int32_t    writePosition = FindFirstEmptyEllipsoidRecord (stream);
    if (writePosition < 0)
        {
        assert (false);
        status = GEOCOORDERR_MaxUserLibraryEllipsoids;
        goto error;
        }

    // IsEmptyEllipsoidRecord leaves us positioned at the beginning of the empty ellipsoid record.
    if (0 != CS_elwr (stream, &ellipsoidDef, 0))
        {
        assert (false);
        status = GEOCOORDERR_IOError;
        goto error;
        }

    // Sort the file into proper order, thereby moving the new ellipsoid to its proper place in the file.
    if (0 != CS_fseek (stream, m_ellipsoidStart, 0))
        {
        assert (false);
        CS_erpt (status = cs_IOERR);
        goto error;
        }

    if (0 > CS_ips (stream, sizeof (ellipsoidDef), writePosition + sizeof (ellipsoidDef), (CMPFUNC_CAST)CS_elcmp))
        status = cs_IOERR;

error:
    // ellipsoidNames is no longer up to date.
    if (NULL != m_ellipsoidNames)
        {
        delete m_ellipsoidNames;
        m_ellipsoidNames = NULL;
        }

    CS_fclose (stream);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       ReplaceEllipsoid (const CSEllipsoidDef& oldEllipsoid, const CSEllipsoidDef& newEllipsoid) override
    {
    if (!m_consolidatedFormat)
        return BSIERROR;

    // make a copy so we don't have to worry about modifying the original.
    CSEllipsoidDef  newEllipsoidDef;
    CSEllipsoidDef  oldEllipsoidDef;
    memcpy (&newEllipsoidDef, &newEllipsoid, sizeof(newEllipsoidDef));
    memcpy (&oldEllipsoidDef, &oldEllipsoid, sizeof(oldEllipsoidDef));

    // we need the oldGCS only to get the name, in the case where the user has modified the name.
    // If the name has changed, we need to make sure it's unique.
    StatusInt   status      = SUCCESS;
    bool        nameChanged = false;
    uint32_t    numEllipsoids   = 0;
    if (0 != BeStringUtilities::Stricmp (oldEllipsoidDef.key_nm, newEllipsoidDef.key_nm))
        {
        WString newEllipsoidName (newEllipsoidDef.key_nm,false);
        // name change. Make sure the name isn't already in this library.
        if (NULL != this->GetEllipsoid (newEllipsoidName.c_str()))
            return GEOCOORDERR_EllipsoidNoUniqueName;

        // and make sure it's not in the system library.
        if (NULL != LibraryManager::Instance()->GetSystemLibrary()->GetEllipsoid (newEllipsoidName.c_str()))
            return GEOCOORDERR_EllipsoidNoUniqueName;

        nameChanged = true;
        numEllipsoids = (uint32_t) GetEllipsoidCount();
        }

    // Get the current time.
    cs_Time_    gmtTime;
    gmtTime = CS_time ((cs_Time_ *)0);
    // Compute the CSMap time, as used internally.  This is days since January 1, 1990. If this record does get
    // written, we want it to have the current date in it.
    newEllipsoidDef.protect = (short)((gmtTime - 630720000L) / 86400L);

    // Adjust the name and make sure it is all upper case. By convention, ellipsoid names are case insensitive.
    if (0 != CS_nampp (newEllipsoidDef.key_nm))
        return GEOCOORDERR_EllipsoidIllegalName;

    if (0 != CS_nampp (oldEllipsoidDef.key_nm))
        return GEOCOORDERR_EllipsoidIllegalName;

    // find the end position (before opening the file).
    int32_t        sortEndPosition = m_ellipsoidStart + ((uint32_t) GetEllipsoidCount() * sizeof (CSEllipsoidDef));

    csFILE*         stream;
    if (NULL == (stream = OpenLibraryFile (_STRM_BINUP)))
        return GEOCOORDERR_LibraryReadonly;

    // First, we find the ellipsoid in the file (using the old name).
    if (0 >= CS_bins (stream, m_ellipsoidStart, sortEndPosition, sizeof (oldEllipsoidDef), (char *)&oldEllipsoidDef, (CMPFUNC_CAST)CS_elcmp))
        {
        // couldn't find it.
        status = GEOCOORDERR_EllipsoidNotFound;
        goto error;
        }

    // Overwrite the old definition.
    if (0 != CS_elwr (stream, &newEllipsoidDef, 0))
        {
        status = cs_IOERR;
        goto error;
        }

    // if we changed the ellipsoid name, we have to re-sort the file.
    if (nameChanged)
        {
        // seek to the start of the ellipsoid.
        if (0 != CS_fseek (stream, m_ellipsoidStart, 0))
            {
            CS_erpt (status = cs_IOERR);
            goto error;
            }

        if (0 > CS_ips (stream, sizeof (newEllipsoidDef), m_ellipsoidStart + numEllipsoids * sizeof(CSEllipsoidDef), (CMPFUNC_CAST)CS_elcmp))
            status = cs_IOERR;

        // we need to change the name of the ellipsoid in every GCS or Datum that uses it.
        this->EllipsoidRenamed (stream, oldEllipsoidDef.key_nm, newEllipsoidDef.key_nm);
        }

    // The Ellipsoid entry has been modified, make sure we remove every GCS in our cache that uses it.
    LibraryManager::Instance()->RemoveAllUsingEllipsoidFromCache (oldEllipsoidDef.key_nm);
    if (NULL != m_ellipsoidNames)
        {
        delete m_ellipsoidNames;
        m_ellipsoidNames = NULL;
        }

error:
    CS_fclose (stream);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       GetEllipsoidName (uint32_t index, WStringR ellipsoidName) override
    {
    if (!m_consolidatedFormat)
        return BSIERROR;
    GetEllipsoidNames();
    if (index >= m_ellipsoidNames->size())
        return BSIERROR;

    ellipsoidName.assign ((*m_ellipsoidNames)[index]);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual size_t          GetEllipsoidCount () override
    {
    if (!m_consolidatedFormat)
        return 0;

    GetEllipsoidNames();
    return m_ellipsoidNames->size();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IsEmptyEllipsoidRecord (csFILE* stream, uint32_t readPosition)
    {
    bool            returnVal = false;
    CSEllipsoidDef  ellipsoidDef;
    CS_fseek (stream, readPosition, SEEK_SET);

    // see if we can read it.
    if (sizeof (ellipsoidDef) != CS_fread ((char *)&ellipsoidDef, 1, sizeof (ellipsoidDef), stream))
        {
        assert (false);
        returnVal = true;
        }
    // if the name start with 0, this is a blank entry.
    if (0 == ellipsoidDef.key_nm[0])
        returnVal = true;

    // seek back to the same place.
    CS_fseek (stream, readPosition, SEEK_SET);
    return returnVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            GetEllipsoidNames ()
    {
    // If m_ellipsoidNames is not NULL, we already have a list, simply return it. */
    if (NULL != m_ellipsoidNames)
        return;

    m_ellipsoidNames = new T_WStringVector();

    // no ellipsoids, just create empty list.
    if (!m_consolidatedFormat)
        return;

    csFILE *stream;
    if (NULL == (stream = OpenLibraryFile (_STRM_BINRD)))
        return;

    CSEllipsoidDef  ellipsoidDef;
    int     maxEllipsoids   = (m_datumStart - m_ellipsoidStart) / sizeof (ellipsoidDef);
    int     iEllipsoid      = 0;
    uint32_t readPosition    = m_ellipsoidStart;
    for (iEllipsoid = 0; iEllipsoid < maxEllipsoids; iEllipsoid++)
        {
        // if CS_elrd returns 0 or negative, we ran out. This happens only if there are no GCS's in the library, which we don't expect to happen.
        int     crypt;

        if (IsEmptyEllipsoidRecord (stream, readPosition))
            break;

        if (CS_elrd (stream, &ellipsoidDef, &crypt) <= 0)
            {
            assert (false);
            break;
            }

        // Add the new item to the list.
        m_ellipsoidNames->push_back (WString(ellipsoidDef.key_nm,false));
        readPosition += sizeof (CSEllipsoidDef);
        }
    CS_fclose (stream);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       CreateNewFile (bool overwriteExisting)
    {
    // if the file exists, only overwrite it if the argument is true.
    // Note: this method is also un-implemented in Vancouver
    return BSIERROR;
    }

};

/*=================================================================================**//**
* CSMap System Library class
+===============+===============+===============+===============+===============+======*/
struct  CSMapLibrary : public Library
{
uint32_t  m_csCount;
WStringP        m_libraryFileName;
WStringP        m_orgFileName;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
CSMapLibrary ()
    {
    m_csCount           = 0;
    m_libraryFileName   = NULL;
    m_orgFileName       = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ~CSMapLibrary() 
    {
    DELETE_AND_CLEAR (m_libraryFileName);
    DELETE_AND_CLEAR (m_orgFileName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSParameters*   GetCS (WCharCP name) override
    {
    AString mbName (name);
    return CS_csloc (mbName.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSParameters*   GetCS (CSDefinition& csDef) override
    {
    return CSMap::CScsloc1 (&csDef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WString     GetGUIName () override
    {
    return L"GCS Library";
    }

/*---------------------------------------------------------------------------------**//**
* NOTE: This always returns either NULL or or a CSParameters that needs to be CSMap::CS_free'd
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSParameters*   GetCS (uint32_t index) override
    {
    char    csNameBuf[128];
    if (CS_csEnum (index, csNameBuf, sizeof(csNameBuf)))
        return CS_csloc (csNameBuf);
    else
        return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool            CSInLibrary (WCharCP csName) override
    {
    const char*   cp = CScsKeyNames ();
    if (NULL == cp)
        return false;

    AString mbName (csName);

    // Look for the last entry
    for (uint32_t index = 0; index < 200000; index++)
        {
        if (0 == CS_stricmp (cp, mbName.c_str()))
            return true;

        // find the end of the current string
        while (*cp++ != '\0');

        // empty string indicates the end.
        if (*cp == '\0')
            break;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       DeleteCS (BaseGCSP gcsToDelete) override
    {
    // we do not allow deletes from the CSMap library.
    return GEOCOORDERR_LibraryReadonly;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       CreateNewCS (WString& newName, WCharCP sourceName) override
    {
    // never modify the CSMap library.
    return GEOCOORDERR_LibraryReadonly;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       ReplaceCS (BaseGCSP oldGCS, BaseGCSP newGCS) override
    {
    // never modify the CSMap library.
    return GEOCOORDERR_LibraryReadonly;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       GetCSName (uint32_t index, WStringR csName) override
    {
    csName.clear();

    char    csNameOut[512];
    if (CS_csEnum (index, csNameOut, _countof(csNameOut)))
        {
        csName.AssignA (csNameOut);
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* NOTE: This always returns either NULL or or a CSParameters that needs to be CSMap::CS_free'd
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual size_t          GetCSCount () override
    {
    if (0 != m_csCount)
        return m_csCount;

    const char*   cp = CScsKeyNames ();
    if (NULL == cp)
        return 0;

    // Look for the last entry
    uint32_t index;
    for (index = 0; ; index++)
        {
        // find the end of the current string
        while (*cp++ != '\0');

        // empty string indicates the end.
        if (*cp == '\0')
            break;
        }
    m_csCount = index;
    return m_csCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WCharCP     GetLibraryFileName () override
    {
    // this is the crazy crap that CS_Map does.
    strcpy (cs_DirP, cs_Csname);

    if (NULL == m_libraryFileName)
        m_libraryFileName = new WString();

    m_libraryFileName->AssignA (cs_Dir);
    return m_libraryFileName->c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WCharCP     GetOrganizationFileName () override
    {
    if (NULL == m_orgFileName)
        {
        m_orgFileName = new WString (GetLibraryFileName());
        m_orgFileName->append (L".xml");
        }
    return m_orgFileName->c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool            IsUserLibrary () override
    {
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool            IsReadOnly () override
    {
    return true;
    }

// Datum methods
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSDatumDef*     GetDatum (WCharCP datumName) override
    {
    // look in CS-Map's library.
    AString mbDatumName (datumName);
    return CSMap::CS_dtdef (mbDatumName.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSDatumDef*     GetDatum (uint32_t index) override
    {
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool            DatumInLibrary (WCharCP datumName) override
    {
    const char*   cp = CSdtKeyNames ();
    if (NULL == cp)
        return false;

    AString mbDatumName (datumName);

    // Look for the last entry
    for (uint32_t index = 0; index < 200000; index++)
        {
        if (0 == CS_stricmp (cp, mbDatumName.c_str()))
            return true;

        // find the end of the current string
        while (*cp++ != '\0');

        // empty string indicates the end.
        if (*cp == '\0')
            break;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       DeleteDatum (WCharCP datumName) override
    {
    return GEOCOORDERR_LibraryReadonly;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       CreateNewDatum (DatumP& newDatum, WCharCP seedName) override
    {
    return GEOCOORDERR_LibraryReadonly;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       AddDatum (const CSDatumDef& newDatum) override
    {
    return GEOCOORDERR_LibraryReadonly;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       ReplaceDatum (const CSDatumDef& oldDatum, const CSDatumDef& newDatum) override
    {
    return GEOCOORDERR_LibraryReadonly;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       GetDatumName (uint32_t index, WStringR datumName) override
    {
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual size_t          GetDatumCount () override
    {
    return 0;
    }

// Ellipsoid methods
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSEllipsoidDef* GetEllipsoid (WCharCP ellipsoidName) override
    {
    AString mbEllipsoidName (ellipsoidName);
    return CSMap::CS_eldef (mbEllipsoidName.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual CSEllipsoidDef* GetEllipsoid (uint32_t index) override
    {
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool            EllipsoidInLibrary (WCharCP ellipsoidName) override
    {
    const char*   cp = CSelKeyNames ();
    if (NULL == cp)
        return false;

    AString mbEllipsoidName (ellipsoidName);

    // Look for the last entry
    for (uint32_t index = 0; index < 200000; index++)
        {
        if (0 == CS_stricmp (cp, mbEllipsoidName.c_str()))
            return true;

        // find the end of the current string
        while (*cp++ != '\0');

        // empty string indicates the end.
        if (*cp == '\0')
            break;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       DeleteEllipsoid (WCharCP datumName) override
    {
    return GEOCOORDERR_LibraryReadonly;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       CreateNewEllipsoid (CSEllipsoidDef*& newEllipsoid, WCharCP seedName) override
    {
    return GEOCOORDERR_LibraryReadonly;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       AddEllipsoid (const CSEllipsoidDef& newEllipsoid) override
    {
    return GEOCOORDERR_LibraryReadonly;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       ReplaceEllipsoid (const CSEllipsoidDef& oldEllipsoid, const CSEllipsoidDef& newEllipsoid) override
    {
    return GEOCOORDERR_LibraryReadonly;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       GetEllipsoidName (uint32_t index, WStringR datumName) override
    {
    datumName.clear();
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual size_t          GetEllipsoidCount () override
    {
    return 0;
    }


};

/*=================================================================================**//**
*
* LibraryManager class
*
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
LibraryManager::LibraryManager    () : m_csParamMap()
    {
    m_userLibraryList = NULL;
    m_csmapLibrary    = new CSMapLibrary;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
LibraryManager*    LibraryManager::Instance ()
    {
    if (NULL == s_instance)
        s_instance = new LibraryManager();
    return s_instance;
    }

/*---------------------------------------------------------------------------------**//**
* NOTE: This always returns either NULL or or a CSParameters that needs to be CSMap::CS_free'd
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
CSParameters*   LibraryManager::GetCS (Library*& outSourceLibrary, WCharCP name)
    {
    // initialize output
    outSourceLibrary = NULL;

    CSParameters*           outParams       = NULL;
    ParamMapEntry*          paramMapEntry   = NULL;

    AString mbName (name);
    T_CSParamMap::iterator  paramsFound = m_csParamMap.find (name);
    if (paramsFound != m_csParamMap.end())
        {
        paramMapEntry = paramsFound->second;
        }
    else
        {
        const CSParameters*     csParams = NULL;
        Library*                sourceLibrary = NULL;
        // first look in the user coordinate system libraries. They're likely to be smaller, and a miss less expensive.
        if (NULL != m_userLibraryList)
            {
            T_UserLibraryList::iterator   listIterator;
            for (listIterator = m_userLibraryList->begin(); listIterator != m_userLibraryList->end(); listIterator++)
                {
                sourceLibrary = *listIterator;
                if (NULL != (csParams = sourceLibrary->GetCS (name)))
                    break;
                }
            }

        // if not in any user library, look in the master library.
        if (NULL == csParams)
            {
            sourceLibrary = m_csmapLibrary;
            csParams = m_csmapLibrary->GetCS (name);
            }

        // if we found the GCS, put it into the map.
        if (NULL != csParams)
            {
            paramMapEntry    = new ParamMapEntry();
            memcpy (paramMapEntry, csParams, sizeof (CSParameters));
            paramMapEntry->m_sourceLibrary = sourceLibrary;
            m_csParamMap[name] = paramMapEntry;

            // we copied the data, so don't need the csParams any more.
            CSMap::CS_free ((void*) csParams);
            csParams = NULL;
            }
        }

    if (NULL != paramMapEntry)
        {
        outParams = (CSParameters *) CS_malc (sizeof(CSParameters));
        memcpy (outParams, paramMapEntry, sizeof(CSParameters));
        outSourceLibrary = paramMapEntry->m_sourceLibrary;
        return outParams;
        }

    // if csParams is NULL, we looked it up and failed, cs_Error is set by CSMap;
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
CSParameters*           LibraryManager::GetCS (Library*& outSourceLibrary, Library* sourceLibrary, CSParameters& sourceCSParameters)
    {
    // initialize output
    outSourceLibrary = sourceLibrary;

    if (NULL == sourceLibrary)
        return m_csmapLibrary->GetCS (sourceCSParameters.csdef);
    else
        return sourceLibrary->GetCS (sourceCSParameters.csdef);
    }

/*---------------------------------------------------------------------------------**//**
* NOTE: This always returns either NULL or or a CSDatumDef that needs to be CSMap::CS_free'd
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
CSDatumDef*             LibraryManager::GetDatumDefFromGCS (BaseGCSCR gcs)
    {
    LibraryP    sourceLibrary = gcs.GetSourceLibrary();

    // find the datum in the same library or the system library (don't look in any user libraries EXCEPT the one we found the GCS in.
    CSDatumDef*  foundDatum = NULL;
    if ( (NULL != sourceLibrary) && (sourceLibrary != m_csmapLibrary) )
        foundDatum = sourceLibrary->GetDatum (gcs.GetDatumName());

    if (NULL == foundDatum)
        foundDatum = m_csmapLibrary->GetDatum (gcs.GetDatumName());

    return foundDatum;
    }

/*---------------------------------------------------------------------------------**//**
* NOTE: This always returns either NULL or or a CSEllipsoidDef that needs to be CSMap::CS_free'd
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
CSEllipsoidDef*             LibraryManager::GetEllipsoidDefFromGCS (BaseGCSCR gcs)
    {
    LibraryP    sourceLibrary = gcs.GetSourceLibrary();

    // find the datum in the same library or the system library (don't look in any user libraries EXCEPT the one we found the GCS in.
    CSEllipsoidDef*  foundEllipsoid = NULL;
    if ( (NULL != sourceLibrary) && (sourceLibrary != m_csmapLibrary) )
        foundEllipsoid = sourceLibrary->GetEllipsoid (gcs.GetEllipsoidName());

    if (NULL == foundEllipsoid)
        foundEllipsoid = m_csmapLibrary->GetEllipsoid (gcs.GetEllipsoidName());

    return foundEllipsoid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
Library*                LibraryManager::GetLibrary (size_t index)
    {
    size_t  numUserLibraries = (NULL != m_userLibraryList) ? m_userLibraryList->size() : 0;

    if (index < numUserLibraries)
        return (*m_userLibraryList)[index];
    else if (index == numUserLibraries)
        return m_csmapLibrary;
    else
        return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
LibraryP                LibraryManager::FindSourceLibrary (WCharCP csName)
    {
    T_CSParamMap::iterator  paramsFound = m_csParamMap.find (csName);
    if (paramsFound != m_csParamMap.end())
        {
        ParamMapEntry*  paramMapEntry = paramsFound->second;
        return paramMapEntry->m_sourceLibrary;
        }
    else
        {
        Library*                sourceLibrary;
        // first look in the user coordinate system libraries. They're likely to be smaller, and a miss less expensive.
        if (NULL != m_userLibraryList)
            {
            T_UserLibraryList::iterator   listIterator;
            for (listIterator = m_userLibraryList->begin(); listIterator != m_userLibraryList->end(); listIterator++)
                {
                sourceLibrary = *listIterator;
                if (sourceLibrary->CSInLibrary (csName))
                    return sourceLibrary;
                }
            }

        if (m_csmapLibrary->CSInLibrary (csName))
            return m_csmapLibrary;
        }

    // if csParams is NULL, we looked it up and failed, cs_Error is set by CSMap;
    return NULL;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
Library*                LibraryManager::GetSystemLibrary ()
    {
    return m_csmapLibrary;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
size_t                  LibraryManager::GetLibraryCount ()
    {
    return 1 + ( (m_userLibraryList == NULL) ? 0 : m_userLibraryList->size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void                    LibraryManager::RemoveFromCache (WCharCP name)
    {
    T_CSParamMap::iterator  paramsFound = m_csParamMap.find (name);
    if (paramsFound != m_csParamMap.end())
        {
        ParamMapEntry*  paramMapEntry = paramsFound->second;
        delete paramMapEntry;
        m_csParamMap.erase (paramsFound);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void                    LibraryManager::RemoveAllUsingDatumFromCache (CharCP datumName)
    {
    T_CSParamMap::iterator  iterator;
    for (iterator = m_csParamMap.begin(); iterator != m_csParamMap.end(); )
        {
        const ParamMapEntry*   paramMapEntry = iterator->second;

        if (0 == CS_stricmp (datumName, paramMapEntry->datum.key_nm))
            {
            delete paramMapEntry;
            iterator = m_csParamMap.erase (iterator);
            }
        else
            ++iterator;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void                    LibraryManager::RemoveAllUsingEllipsoidFromCache (CharCP ellipsoidName)
    {
    T_CSParamMap::iterator  iterator;
    for (iterator = m_csParamMap.begin(); iterator != m_csParamMap.end(); )
        {
        const ParamMapEntry*   paramMapEntry = iterator->second;

        if ( (0 == CS_stricmp (ellipsoidName, paramMapEntry->datum.ell_knm)) || 
             (0 == CS_stricmp (ellipsoidName, paramMapEntry->csdef.elp_knm)) )
            {
            delete paramMapEntry;
            iterator = m_csParamMap.erase (iterator);
            }
        else
            ++iterator;
        }
    }

/*---------------------------------------------------------------------------------**//**
* NOTE: CSParameters passed in must be allocated with CS_malc. If it succeeds in finding
*  the definition, it replaces the contents of existingParameters with the new parameters,
*  avoiding freeing and allocating the coordinates.
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
CSParameters*   LibraryManager::ReplaceCSContents (LibraryP& sourceLibrary, WCharCP name, CSParameters* existingCSParams)
    {
    if (NULL == existingCSParams)
        return GetCS (sourceLibrary, name);

    ParamMapEntry*  paramMapEntry   = NULL;

    T_CSParamMap::iterator  paramsFound = m_csParamMap.find (name);

    if (paramsFound != m_csParamMap.end())
        {
        paramMapEntry = paramsFound->second;
        }
    else
        {
        CSParameters*   csParams;
        // NEEDSWORK_USER_LIBRARY to make it work on user libraries!

        AString mbName (name);

        // if we find the GCS (in the system library), put it into the map.
        if (NULL != (csParams = CS_csloc (mbName.c_str())))
            {
            paramMapEntry = new ParamMapEntry();
            memcpy (paramMapEntry, csParams, sizeof(CSParameters));
            paramMapEntry->m_sourceLibrary = m_csmapLibrary;
            m_csParamMap[name] = paramMapEntry;
            CSMap::CS_free (csParams);
            }
        }

    if (NULL != paramMapEntry)
        {
        memcpy (existingCSParams, paramMapEntry, sizeof(CSParameters));
        sourceLibrary = paramMapEntry->m_sourceLibrary;
        return existingCSParams;
        }
    else
        {
        sourceLibrary = NULL;
        CSMap::CS_free (existingCSParams);
        return NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            LibraryManager::AddUserLibrary (WCharCP libraryPath, WCharCP guiName)
    {
    WString fileName;

    if (NULL == libraryPath)
        return;

    if (NULL == m_userLibraryList)
        m_userLibraryList = new T_UserLibraryList();

    if (NULL == guiName)
        {
        BeFileName libPath(libraryPath);
        libPath.ParseName(NULL, NULL, &fileName, NULL);
        guiName = fileName.c_str();
        }

    T_UserLibraryList::iterator   listIterator;
    for (listIterator = m_userLibraryList->begin(); listIterator != m_userLibraryList->end(); listIterator++)
        {
        Library*  userLib = *listIterator;
        if (0 == BeStringUtilities::Wcsicmp (userLib->GetLibraryFileName(), libraryPath))
            {
            // Remove all entries using this library from the cache.
            T_CSParamMap::iterator  iterator;
            for (iterator = m_csParamMap.begin(); iterator != m_csParamMap.end(); )
                {
                const ParamMapEntry*   paramMapEntry = iterator->second;
                if (userLib = paramMapEntry->m_sourceLibrary)
                    {
                    delete paramMapEntry;
                    iterator = m_csParamMap.erase (iterator);
                    }
                else
                    ++iterator;
                }

            delete userLib;
            m_userLibraryList->erase (listIterator);
            break;
            }
        }

    UserLibrary*    newLibrary = new UserLibrary (libraryPath, guiName);
    if (BSISUCCESS == newLibrary->DiscoverFormat())
        m_userLibraryList->push_back (newLibrary);
    else
        delete newLibrary;
    }


LibraryManager*   LibraryManager::s_instance;

