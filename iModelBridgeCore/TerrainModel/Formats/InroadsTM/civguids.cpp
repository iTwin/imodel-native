//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+

//---------------------------------------------------------------------------
// Include files
//---------------------------------------------------------------------------
#include <Bentley/Bentley.h>
#include <BeSQLite/BeSQLite.h>
#include <Bentley/BeStringUtilities.h>
//#include <Rpc.h>
#if defined (BENTLEY_WIN32)
#include <Windows.h>
#else
#define HRESULT long
#define S_OK 0
typedef bool BOOL;
#endif

typedef BENTLEY_NAMESPACE_NAME::BeSQLite::BeGuid BeGuid;
#include "civguids.h"

#pragma comment (lib, "rpcrt4.lib")

//union BeGuid { struct _GUID g; uint64_t u[2]; uint32_t i[4]; char b[16]; };

//---------------------------------------------------------------------------
// DESC: Generates a new unique id for civil data
// HIST: Original - dakloske - Oct 22, 1998
// MISC: 
//---------------------------------------------------------------------------
HRESULT aecGuid_generate (BeGuid* newGuid)
    {
    //UUID* uidP = (UUID*)newGuid;
    //return UuidCreate (uidP);
    newGuid->Create ();
    return(S_OK);
    }


//---------------------------------------------------------------------------
// DESC: Returns 0 if equal, -1 or 1 if not equal.
// HIST: Original - dakloske - Oct 22, 1998
// MISC: Designed for use with qsort
//---------------------------------------------------------------------------
int aecGuid_compare (const BeGuid *g1, const BeGuid *g2)
    {
    return memcmp (g1, g2, sizeof (BeGuid));
    }


//---------------------------------------------------------------------------
// DESC: Returns TRUE if equal, FALSE if not equal.
// HIST: Original - dakloske - Oct 22, 1998
// MISC: 
//---------------------------------------------------------------------------
BOOL aecGuid_equal (const BeGuid *g1, const BeGuid *g2)
{
    //return IsEqualGUID(*g1, *g2);
    return (*g1) == (*g2);
}

/*---------------------------------------------------------------------------------*
 * Copied from apr_guid.c
 * convert a pair of hex digits to an integer value [0,255]
 * @bsimethod                                                    Sam.Wilson      04/13
 +---------------+---------------+---------------+---------------+---------------+------*/
static unsigned char parse_hexpair (LPCWSTR s)
    {
    int result = s[0] - '0';
    if (result > 48)
        result = (result - 39) << 4;
    else if (result > 16)
        result = (result - 7) << 4;
    else
        result = result << 4;

    int temp = s[1] - '0';
    if (temp > 48)
        result |= temp - 39;
    else if (temp > 16)
        result |= temp - 7;
    else
        result |= temp;

    return (unsigned char)result;
    }

//---------------------------------------------------------------------------
// DESC: Given a string, convert it to a GUID.
// HIST: Original - dakloske - Oct 22, 1998
// MISC: 
//---------------------------------------------------------------------------
HRESULT aecGuid_fromString (BeGuid *guid, LPCWSTR pStr)
    {
    BeGuid* beGuid = guid;
    for (int i = 0; i < 36; ++i)
        {
        wchar_t c = pStr[i];
        if (!isxdigit (c) && !(c == '-' && (i == 8 || i == 13 || i == 18 || i == 23)))
            return ERROR;       /* ### need a better value */
        }

    if (pStr[36] != '\0')
        return ERROR; /* ### need a better value */

    beGuid->m_guid.b[0] = parse_hexpair (&pStr[0]);
    beGuid->m_guid.b[1] = parse_hexpair (&pStr[2]);
    beGuid->m_guid.b[2] = parse_hexpair (&pStr[4]);
    beGuid->m_guid.b[3] = parse_hexpair (&pStr[6]);
    beGuid->m_guid.b[4] = parse_hexpair (&pStr[9]);
    beGuid->m_guid.b[5] = parse_hexpair (&pStr[11]);
    beGuid->m_guid.b[6] = parse_hexpair (&pStr[14]);
    beGuid->m_guid.b[7] = parse_hexpair (&pStr[16]);
    beGuid->m_guid.b[8] = parse_hexpair (&pStr[19]);
    beGuid->m_guid.b[9] = parse_hexpair (&pStr[21]);

    for (int i = 6; i--;)
        beGuid->m_guid.b[10 + i] = parse_hexpair (&pStr[i * 2 + 24]);

    return S_OK;
    }

//---------------------------------------------------------------------------
// DESC: Given a GUID, convert it to a readable string
// HIST: Original - dakloske - Oct 22, 1998
// MISC: 
//---------------------------------------------------------------------------
HRESULT aecGuid_toString (LPWSTR pStr, const BeGuid *guid)
{
BeGuid* beGuid = const_cast<BeGuid*> (guid);
BeStringUtilities::Snwprintf (pStr, 36, L"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
    beGuid->m_guid.b[0], beGuid->m_guid.b[1], beGuid->m_guid.b[2], beGuid->m_guid.b[3], beGuid->m_guid.b[4], beGuid->m_guid.b[5], beGuid->m_guid.b[6], beGuid->m_guid.b[7],
    beGuid->m_guid.b[8], beGuid->m_guid.b[9], beGuid->m_guid.b[10], beGuid->m_guid.b[11], beGuid->m_guid.b[12], beGuid->m_guid.b[13], beGuid->m_guid.b[14], beGuid->m_guid.b[15]);

    return S_OK;
}

//---------------------------------------------------------------------------
// DESC: Given a GUID, clear it (set to all 0's)
// HIST: Original - dakloske - Oct 22, 1998
// MISC: 
//---------------------------------------------------------------------------
HRESULT aecGuid_clear (BeGuid *g)
{
	if (g)
		memset(g, 0, sizeof(GUID));
	return(S_OK);
}

//---------------------------------------------------------------------------
// DESC: Is the given GUID empty?  Sometimes we clear a GUID to indicate that
//       it hasn't been set.  This is to check for those instances.
// HIST: jmw 3/12/99 - Richard's 50th
// MISC: 
//---------------------------------------------------------------------------
BOOL aecGuid_isClear
(
const BeGuid *g
)
{
BeGuid clear;

    memset(&clear, 0, sizeof(GUID));

    if (aecGuid_compare(g, &clear) == 0)
        return(TRUE);

    return(FALSE);
}

//---------------------------------------------------------------------------
// DESC: Given 2 GUIDs, copy g2 to g1
// HIST: Original - dakloske - Oct 22, 1998
// MISC: 
//---------------------------------------------------------------------------
HRESULT aecGuid_copy (BeGuid *g1, const BeGuid *g2)
{
	if (g1 && g2)
	{
        memcpy (g1, g2, sizeof (BeGuid));
	}
	return(S_OK);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeGetUserName (WStringR s)
    {
#if defined (BENTLEY_WIN32)
    wchar_t     userName[256];
    unsigned long len = _countof(userName);
    if (!::GetUserNameW (userName, &len))
        return ERROR;
    s.assign (userName);
#elif defined (__unix__)
    AString aname;
    if (BeGetUserNameA (aname) != SUCCESS)
        return ERROR;
    BeStringUtilities::Utf8ToWChar (s, aname.c_str());
#endif
    return SUCCESS;    
    }