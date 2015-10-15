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

HRESULT aecGuid_generate   ( BeSQLite::BeGuid *newGuid );
int     aecGuid_compare    ( const BeSQLite::BeGuid *g1, const BeSQLite::BeGuid *g2 );    // designed for use with qsort
BOOL    aecGuid_equal      ( const BeSQLite::BeGuid *g1, const BeSQLite::BeGuid *g2 );
HRESULT aecGuid_fromString ( BeSQLite::BeGuid *guid, LPCWSTR str );
HRESULT aecGuid_toString   ( LPWSTR str, const BeSQLite::BeGuid *guid );
HRESULT aecGuid_clear      ( BeSQLite::BeGuid *g );
HRESULT aecGuid_copy       ( BeSQLite::BeGuid *g1, const BeSQLite::BeGuid *g2 );
BOOL    aecGuid_isClear    ( const BeSQLite::BeGuid *g );
