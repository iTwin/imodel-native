//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/hfcfileattributes.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCFileAttributes
//-----------------------------------------------------------------------------
// Definition of file attributes.
//-----------------------------------------------------------------------------

#pragma once


// STANDARD FILE ATTRIBUTES

// Archive. Set whenever the file is changed,
// and cleared by the BACKUP command.
const uint32_t HFC_FILE_ATTRIBUTES_ARCHIVE = 0x20;

// Hidden file. Not normally seen with the DIR command,
// unless the /AH option is used. Returns information about
// normal files as well as files with this attribute.
const uint32_t HFC_FILE_ATTRIBUTES_HIDDEN = 0x02;

// Normal. File can be read or written
// to without restriction.
const uint32_t HFC_FILE_ATTRIBUTES_NORMAL = 0x00;

// Read-only. File cannot be opened for writing,
// and a file with the same name cannot be created.
const uint32_t HFC_FILE_ATTRIBUTES_RDONLY = 0x01;

// Subdirectory.
const uint32_t HFC_FILE_ATTRIBUTES_SUBDIR = 0x10;

// System file. Not normally seen with the DIR command,
// unless the /AS option is used.
const uint32_t HFC_FILE_ATTRIBUTES_SYSTEM = 0x04;


// HMR SPECIFIC FILE ATTRIBUTES

// Logical path. This attribute represents a logical path
// from the hmr server.
const uint32_t HFC_FILE_ATTRIBUTES_LOGICAL_PATH = 0x80;

// Database Object. This attribute represents a database object
// from the hmr server.
const uint32_t HFC_FILE_ATTRIBUTES_DATABASE_OBJECT = 0x100;
