//===========================================================================
// $Header$
//
//    (C) Copyright 2007 by Autodesk, Inc.
//
// The information contained herein is confidential, proprietary
// to Autodesk, Inc., and considered a trade secret as defined 
// in section 499C of the penal code of the State of California.  
// Use of this information by anyone other than authorized employees
// of Autodesk, Inc. is granted only under a written non-disclosure 
// agreement, expressly prescribing the scope and manner of such use.       
//
// CREATED BY:
//      Norm Olsen
//
// DESCRIPTION:
//

#include "csConsoleUtilities.hpp"
#include "csUsefulRangeReport.hpp"

// This module produces a report concerning the status of the current useful
// range portion of each coordinate system definition.  (Probably should do the
// same for datums and geodetic transformations, but schedule constraints
// limit the resources available to those required to satify the immediate
// need: Coordinate Reference Systems.

// We establish a basic "Useful Range Evaluation" data type (i.e. a class).
// A collection of such records is established; we'll use a vector for now;
// performance is not a big issue here as this utility may never ever be
// run again after its initial use. We then examine each coordinate system
// which has not be deprecated (i.e. not in the LEGACY group).  For each such
// system, a record is added to the above described collection.  The
// resulting collection is sorted and filtered in various ways to produce
// information concerning the useful range of all coordinate reference
// systems in relattion of each other, and possible with regard to an
// equivalent specification in EPSG.

// As this utility has it's own class and supporting structure, it also
// has it's own header file.

bool csUsefulRangeReport (const wchar_t* reportDir,const wchar_t* csDictDir)
{
	bool ok (false);

	char csDictDirC [MAXPATH];
	wchar_t filePath [MAXPATH + MAXPATH];

	struct cs_Prjtab_ *prjTblPtr;  cs_Prjtab;
	std::vector<TcsUsefulRngRpt> usefulRangeVector;

	wcstombs (csDictDirC,csDictDir,sizeof (csDictDirC));
	int status = CS_altdr (csDictDirC);
	ok = (status == 0);
	if (!ok)
	{
		return false;
	}

	const TcsEpsgDataSetV6* epsgPtr = GetEpsgObjectPtr ();
	ok = (epsgPtr != 0);
	if (!ok)
	{
		return ok;
	}

	// Build the report file path.
	wcsncpy (filePath,reportDir,MAXPATH);
	wcscat (filePath,L"\\");
	wcscat (filePath,L"CrsUsefulRangeReport.csv");








	return true;
}
