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

extern "C" struct cs_Prjtab_ cs_Prjtab [];
extern "C" unsigned long KcsNmInvNumber;
extern "C" unsigned long KcsNmMapNoNumber;
extern "C" double cs_Huge;
extern "C" double cs_Mhuge;

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
// systems in relation of each other, and possible with regard to an
// equivalent specification in EPSG.

// As this utility has it's own class and supporting structure, it also
// has it's own header file.

struct csUsefulRangeXfer_
{
	char csMapName [32];
	unsigned long epsgCode;
	char epsgName [128];			// Defensive purposes.
};

const csUsefulRangeXfer_ KcsUsefulRangeXfer [] =
{
	// The following systems were listed in Trac Ticket #   .
	{  "ETRS89.UTM-28N",           25828UL,    "ETRS89 / UTM zone 28N"                                      },
	{  "ETRS89.UTM-29N",           25829UL,    "ETRS89 / UTM zone 29N"                                      },
	{  "ETRS89.UTM-30N",           25830UL,    "ETRS89 / UTM zone 30N"                                      },
	{  "ETRS89.UTM-31N",           25831UL,    "ETRS89 / UTM zone 31N"                                      },
	{  "ETRS89.UTM-32N",           25832UL,    "ETRS89 / UTM zone 32N"                                      },
	{  "ETRS89.UTM-33N",           25833UL,    "ETRS89 / UTM zone 33N"                                      },
	{  "ETRS89.UTM-34N",           25834UL,    "ETRS89 / UTM zone 34N"                                      },
	{  "ETRS89.UTM-35N",           25835UL,    "ETRS89 / UTM zone 35N"                                      },
	{  "ETRS89.UTM-36N",           25836UL,    "ETRS89 / UTM zone 36N"                                      },
	{  "ETRS89.UTM-37N",           25837UL,    "ETRS89 / UTM zone 37N"                                      },
	{  "ETRS89.UTM-38N",           25838UL,    "ETRS89 / UTM zone 38N"                                      },
	// The following systems are added as a result of the review of the report
	// produced by the csUsefulRangeReport console utiltiy.  These are systems
	// where the CS-MAP and EPSG disagreement is severe.
	{    "WGS84.PseudoMercator",    3857UL,    ""                                                           },
	{    "LL-CSRS",                 4617UL,    ""                                                           },
	{    "WORLD-MERCATOR",          3395UL,    ""                                                           },
	{    "LL83",                    4269UL,    ""                                                           },
	{    "LL27",                    4267UL,    ""                                                           },
	{    "LL-HPGN",                 4152UL,    ""                                                           },
	{    "NSRS07.LL",               4759UL,    ""                                                           },
	{    "NSRS11.LL",               6318UL,    ""                                                           },
	{    "Pulkovo42.LL",            4284UL,    ""                                                           },
	{    "LL-ETRF89",               4258UL,    ""                                                           },
	{    "EUROP79.LL",              4668UL,    ""                                                           },
	{    "LL-JGD2011-ITRF08",       6668UL,    ""                                                           },
	{    "LL-JGD2K-7P",             4612UL,    ""                                                           },
	{    "LL-GD1949-Grid",          4272UL,    ""                                                           },
	{    "ROME1940.LL",             4265UL,    ""                                                           },
	{    "NTF.LL",                  4275UL,    ""                                                           },
	{    "OSGB.LL",                 4277UL,    ""                                                           },
	{    "CANAVRL.LL",              4717UL,    ""                                                           },
	{    "TIMBALAI.LL",             4298UL,    ""                                                           },
	{    "MICHIGAN.LL",             4268UL,    ""                                                           },
	{    "HJORSEY.LL",              4658UL,    ""                                                           },
	{    "LL-ATS77",                4122UL,    ""                                                           },
	{    "CzechJTSK/5b.LL",         4156UL,    ""                                                           },
	{    "HD72/7Pa.LL",             4237UL,    ""                                                           },
	{    "Amersfoort/a.LL",         4289UL,    ""                                                           },
	{    "CANARY.LL",               4728UL,    ""                                                           },
	{    "CH1903/GSB.LL",           4149UL,    ""                                                           },
	{    "LLCH1903",                4149UL,    ""                                                           },
	{    "TRISTAN.LL",              4734UL,    ""                                                           },
	{    "BELLEVUE.LL",             4714UL,    ""                                                           },
	{    "KERGUELN.LL",             4698UL,    ""                                                           },
	{    "CHATHAM.LL",              4672UL,    ""                                                           },
	{    "MADEIRA.LL",              4615UL,    ""                                                           },
	{    "HONGKONG.LL",             4738UL,    ""                                                           },
	{    "GUAM63.LL",               4675UL,    ""                                                           },
	{    "OBSRV66.LL",              4182UL,    ""                                                           },
	{    "EASTER.LL",               4719UL,    ""                                                           },
	{    "ASCENSN.LL",              4712UL,    ""                                                           },
	{    "MIDWAY.LL",               4727UL,    ""                                                           },
	{    "PITCAIRN.LL",             4729UL,    ""                                                           },
	{    "JHNSTN.LL",               4725UL,    ""                                                           },
	{    "Kusaie51.LL",             4735UL,    ""                                                           },
	{    "NZGD2000.LL",             4167UL,    ""                                                           },
	{    "ETRF89.Europe/EqArea",    3035UL,    ""                                                           },
	{    "ETRF89.Europe/Lambert",   3034UL,    ""                                                           },
	{    "WGS84.PlateCarree",       4087UL,    ""                                                           },
	{    "WGS84.UPSNorth",         32661UL,    ""                                                           },
	{    "WGS84.UPSSouth",         32761UL,    ""                                                           },
	// End of Table marker.
	{                "",       0UL,    ""                                                                   }
};

bool csUsefulRangeXfer (const wchar_t* csDictDir,const wchar_t* csDataDir,int ticketNbr)
{
	bool ok (false);

	const struct csUsefulRangeXfer_* tblPtr;
	const wchar_t* epsgRevision;

	char filePathC [MAXPATH + MAXPATH];

	wcstombs (filePathC,csDictDir,sizeof (filePathC));
	int status = CS_altdr (filePathC);
	ok = (status == 0);
	if (!ok)
	{
		return false;
	}

	// Preclude a careless crash.
	if (wcslen (csDataDir) > (MAXPATH - 10))
	{
		return ok;		// ok == false
	}

	// We'll be extract useful ranges from this database.
	const TcsEpsgDataSetV6* epsgPtr = GetEpsgObjectPtr ();
	ok = (epsgPtr != 0);
	if (!ok)
	{
		return ok;
	}
	epsgRevision = epsgPtr->GetRevisionLevel ();
	std::wcout << L"Using EPSG database version "
			   << epsgRevision
			   << L"."
			   << std::endl;

	// We will be transferring useful ranges to this dictionary.
	wcstombs (filePathC,csDictDir,sizeof (filePathC));
	CS_stncat (filePathC,"\\coordsys.asc",sizeof (filePathC));
	TcsDefFile coordsysAsc (dictTypCoordsys,filePathC);
	ok = (coordsysAsc.GetDictType () == dictTypCoordsys);  // Valid dictionary type indicates success
	if (!ok)
	{
		return ok;
	}
	
	// Loop through the table defined above and for each system specified
	// there, extract the "Area of Use" from the EPSG database, agument the
	// region by a certain amount depending upon the range, and enter the new
	// values in the Coordinate System Dictionary ASCII file.
	for (tblPtr = KcsUsefulRangeXfer;ok && tblPtr->csMapName [0] != '\0';tblPtr += 1)
	{
		bool epsgOk;

		TcsEpsgCode crsEpsgCode;
		TcsEpsgCode epsgAreaCode;

		double usefulRngLngW(0.0);
		double usefulRngLngE(0.0);
		double usefulRngLatS(0.0);
		double usefulRngLatN(0.0);
		double deltaLng;
		double deltaLat;

		TcsAscDefinition* trgDefPtr;
		TcsDefLine* defLinePtr;

		char minLng [32];
		char minLat [32];
		char maxLng [32];
		char maxLat [32];
		char comment [64];

		trgDefPtr = coordsysAsc.GetDefinition (tblPtr->csMapName);
		ok = (trgDefPtr != 0);
		if (ok)
		{
			// Get the useful range from EPSG.
			epsgOk = epsgPtr->GetFieldByCode (epsgAreaCode,epsgTblReferenceSystem,epsgFldAreaOfUseCode,tblPtr->epsgCode);
			if (epsgAreaCode == KcsNmInvNumber || epsgAreaCode == KcsNmMapNoNumber)
			{
				epsgAreaCode.Invalidate ();
			}
			epsgOk = epsgOk && epsgAreaCode.IsValid ();
			if (epsgOk)
			{
				std::wstring areaName;

				// Get the EPSG extrema for the Area.  These values are latitude and longitude in decimal degrees.
				// Generally west/left longitude should be less than the east/right longitude but for areas
				// crossing the 180 degree meridian the west/left value will be greater.
				epsgOk  = epsgPtr->GetFieldByCode (usefulRngLngW,epsgTblArea,epsgFldAreaWestBoundLng,epsgAreaCode);
				epsgOk &= epsgPtr->GetFieldByCode (usefulRngLngE,epsgTblArea,epsgFldAreaEastBoundLng,epsgAreaCode);
				epsgOk &= epsgPtr->GetFieldByCode (usefulRngLatS,epsgTblArea,epsgFldAreaSouthBoundLat,epsgAreaCode);
				epsgOk &= epsgPtr->GetFieldByCode (usefulRngLatN,epsgTblArea,epsgFldAreaNorthBoundLat,epsgAreaCode);
			}
			ok = epsgOk;
		}

		if (ok)
		{
			// Adjust for the 180 degree crack.
			if (usefulRngLngW > usefulRngLngE)
			{
				// The useful range crosses the 180 degree meridian.  Note, that
				// by convention, we always cross from west to east.  That is,
				// our convention is that the useful range is defined by a
				// "southwest corner" (which we call min_ll) and a
				// northeast corner (which we call max_ll).  Thus, the
				// convention is that when crossing the 180 degree crack,
				// we always cross in the west to east direction.  The following
				// enforces that convention, although it does produce reults
				// which are not entirely desirable despite being numerically
				// totally correct.
				usefulRngLngW -= 360;
				
				// We know have a legitmate range, which crosses the 180 degree
				// crack west to east.  However, if the western edge of the
				// area of use is actually in the eastern hemisphere, we want
				// to shift the coordinates to the east by 360 degrees, thus
				// giving the more expected range.  Since this is totally
				// cosmetic, the choice of a constant here is rather arbitrary.

				// DISCLOSURE:  The choice used here tends to keep all US geography
				// in the Pacific Ocean, including all of Alaska, in terms of
				// negative (i.e. western) longitude.  Thus the choice.
				if (usefulRngLngW < -220.0)
				{
					usefulRngLngW += 360;
					usefulRngLngE += 360;
				}
			}

			// Expand the values.  EPSG likes to use tight values for the Area of use,
			// For example, a UTM zone is always 6 degrees wide.  CsMap recognizes that
			// a CRS is generally quite "useful" to a small extent outside the design
			// range.
			double areaWidth = fabs (usefulRngLngE - usefulRngLngW);
			double areaHeight = fabs (usefulRngLatN - usefulRngLatS);
			deltaLng = (areaWidth > 15.0) ? 0.75 : 0.5;
			deltaLat = (areaHeight > 15.0) ? 0.75 : 0.5;
			usefulRngLngW -= deltaLng;
			usefulRngLatS -= deltaLat;
			usefulRngLngE += deltaLng;
			usefulRngLatN += deltaLat;

			// Round to minutes fo arc, always expanding the area.
			usefulRngLngW = floor (usefulRngLngW * 60.0) / 60.0;
			usefulRngLatS = floor (usefulRngLatS * 60.0) / 60.0;
			usefulRngLngE = ceil (usefulRngLngE * 60.0) / 60.0;
			usefulRngLatN = ceil (usefulRngLatN * 60.0) / 60.0;

			// Can't have latitudes greater than 90.
			if (usefulRngLatN > 90.0)
			{
				usefulRngLatN = 90.0;
			}
			if (usefulRngLatS < -90.0)
			{
				usefulRngLatS = -90.0;
			}

			// Format them as we would like them to appear in the .ASC file.
			long32_t status;
			status = CS_ftoa (minLng,sizeof (minLng),usefulRngLngW,cs_ATOF_LNGDFLT-2);
			ok &= (status > 0);
			status = CS_ftoa (minLat,sizeof (minLat),usefulRngLatS,cs_ATOF_LATDFLT-2);
			ok &= (status > 0);
			status = CS_ftoa (maxLng,sizeof (maxLng),usefulRngLngE,cs_ATOF_LNGDFLT-2);
			ok &= (status > 0);
			status = CS_ftoa (maxLat,sizeof (maxLat),usefulRngLatN,cs_ATOF_LATDFLT-2);
			ok &= (status > 0);
		}

		if (ok)
		{
			sprintf (comment,"# Ticket %d: Useful range extracted from EPSG(%S)::%u.",ticketNbr,epsgRevision,tblPtr->epsgCode);

			defLinePtr = trgDefPtr->GetLine ("MIN_LNG:");
			if (defLinePtr == 0)
			{
				TcsDefLine newLine (dictTypCoordsys,"MIN_LNG:",minLng,comment);
				ok = trgDefPtr->InsertAfter ("Y_OFF:",newLine);
				if (!ok)
				{
					ok = trgDefPtr->Append (newLine);
				}
			}
			else
			{
				defLinePtr->SetValue (minLng);
				defLinePtr->SetComment (comment);
			}
		}
		if (ok)
		{
			defLinePtr = trgDefPtr->GetLine ("MIN_LAT:");
			if (defLinePtr == 0)
			{
				TcsDefLine newLine (dictTypCoordsys,"MIN_LAT:",minLat,0);
				ok = trgDefPtr->InsertAfter ("MIN_LNG:",newLine);
				if (!ok)
				{
					ok = trgDefPtr->Append (newLine);
				}
			}
			else
			{
				defLinePtr->SetValue (minLat);
			}
		}
		if (ok)
		{
			defLinePtr = trgDefPtr->GetLine ("MAX_LNG:");
			if (defLinePtr == 0)
			{
				TcsDefLine newLine (dictTypCoordsys,"MAX_LNG:",maxLng,0);
				ok = trgDefPtr->InsertAfter ("MIN_LAT:",newLine);
				if (!ok)
				{
					ok = trgDefPtr->Append (newLine);
				}
			}
			else
			{
				defLinePtr->SetValue (maxLng);
			}
		}
		if (ok)
		{
			defLinePtr = trgDefPtr->GetLine ("MAX_LAT:");
			if (defLinePtr == 0)
			{
				TcsDefLine newLine (dictTypCoordsys,"MAX_LAT:",maxLat,0);
				ok = trgDefPtr->InsertAfter ("MAX_LNG:",newLine);
				if (!ok)
				{
					ok = trgDefPtr->Append (newLine);
				}
			}
			else
			{
				defLinePtr->SetValue (maxLat);
			}
		}
	}
	if (ok)
	{
		wcstombs (filePathC,csDataDir,sizeof (filePathC));
		CS_stncat (filePathC,"\\coordsys.asc",sizeof (filePathC));
		ok = coordsysAsc.WriteToFile (filePathC);
	}
	delete epsgPtr;
	return ok;
}
