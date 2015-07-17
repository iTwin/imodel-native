//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
#pragma once

//---------------------------------------------------------------------------
// Macros and Constants
//---------------------------------------------------------------------------

#define GUID_STRING_MAX_SIZE 40

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

HRESULT aecGuid_generate   ( GUID *newGuid );
int     aecGuid_compare    ( const GUID *g1, const GUID *g2 );    // designed for use with qsort
BOOL    aecGuid_equal      ( const GUID *g1, const GUID *g2 );
HRESULT aecGuid_fromString ( GUID *guid, LPCWSTR str );
HRESULT aecGuid_toString   ( LPWSTR str, const GUID *guid );
HRESULT aecGuid_clear      ( GUID *g );
HRESULT aecGuid_copy       ( GUID *g1, const GUID *g2 );
BOOL    aecGuid_isClear    ( const GUID *g );
