//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
#pragma once

//---------------------------------------------------------------------------
// Macros and Constants
//---------------------------------------------------------------------------

#define GUID_STRING_MAX_SIZE 40

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

HRESULT aecGuid_generate   ( BeGuid *newGuid );
int     aecGuid_compare    ( const BeGuid *g1, const BeGuid *g2 );    // designed for use with qsort
BOOL    aecGuid_equal      ( const BeGuid *g1, const BeGuid *g2 );
HRESULT aecGuid_fromString ( BeGuid *guid, LPCWSTR str );
HRESULT aecGuid_toString   ( LPWSTR str, const BeGuid *guid );
HRESULT aecGuid_clear      ( BeGuid *g );
HRESULT aecGuid_copy       ( BeGuid *g1, const BeGuid *g2 );
BOOL    aecGuid_isClear    ( const BeGuid *g );
