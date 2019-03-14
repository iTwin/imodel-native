/* Copyright (c) 2008, Autodesk, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Autodesk, Inc. nor the names of its
 *       contr butors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Autodesk, Inc. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Autodesk, Inc. OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


// The following table associates a boundary with a ??hpgn.L?s file pair.
// Given that we initially create teo TcsLasLosFile objects as a 48hpgn.l?s
// file set, for each entry in the table immediately defined below, we:
//	1> activate the indicated boundary as a fence,
//	2> use 2 TcsLasLosFile objects to activate the ??hpgn.l?s fo;e pair, 
//	3> Iteracte through the longitude and latitude pairs for which a grid
//	   value exists in the ??hpgn.l?s pair,
//	4> determine, based on the longitude and latitude of the grid value point,
//	   if the point is within the boundary established in #1 above, and if so,
//	5> extract the grid values from the ??hpgn.l?s objects and insert these
//	   values into the 48Hpgn.L?s file pair.
// After all entries in the table have been processed, we write the 48hpgn.l?s
// file pair out to the dictionary directory and we're done.

// The above described process is straight forward and shold produce the
// desired ersult.  A potential problem remains, and for two distinct
// reasons. As of this writing, we only document the problem, plan to insert
// code to notify us if the problem actually arises, and plan to consider
// possible solutions only in the case where the problem does indeed arise.
//
// The problem is that at step 5, we could very likely be replacing an hpgn.l?s
// grid value extracted from a precious grid.  We will report this.  This is
// possible as the construction of a TcsLasLosFile object initializes the grid
// to an absured value.  Before inserting a value into the target grid, we will
// extract and evaluate the existing value.  If it is not the absurd
// initialization value, and the value is significantly different from the
// value that is toi be inserted, a detailed information message will be
// issued and the replacement skipped.

// There are two reasons why this situation can occur.
//	1> A boundary definition segment (implying its neighboring boundary as
//	   well) exactly traces or passes through a grid point.  Thus, a point
//	   in the target grid file set would be considered to exist within both
//	   boundaries.
//	2> In several cases (CA, TX, MT), there exists two sets of grid files
//	   associated with a single boundry, and which will overlap by as much as a
//	   full degree (i.e. four cells).  In these cases, the data points will be
//	   certainly be duplicated.  Not a problem if the grid cell values are
//	   essentially the same, a rather sticky wicket if they differ
//	   significantly.

// This program is unlikely to be used, officially, more than once.  So obvious
// situations where performance could be improved are totally ignored in favor
// of accuracy and reliability of the reult.

// Note: There are 50 sets of ??hpgn.l?s data files.  We are not concerned here
// with four of those sets:
//	hihpgn.l?s   -- Hawaii
//	pvhpgn.l?s   -- Puerto Rico and Virgin Islands
//	wshpgn.l?s   -- Western Samoa (actually Western American Samoa)
//	wshpgn.l?s   -- Eastern Samoa (actually Eastern American Samoa)
//
// Thus, to build a complete 48hpgn.l?s, we need 46 entries.
//

#include "csConsoleUtilities.hpp"


struct TcsHpgn48Generate
{
	char BoundaryFile [80];
	char HpgnFilePair [80];
	char RptLabel     [48];
} KcsHpgn48Generate [] =
{
	{ "EPSGPolygon_1372_2013-06-15.xml", "alhpgn.l?s", "AL"                                      },
	{ "EPSGPolygon_1373_2013-06-15.xml", "azhpgn.l?s", "AZ"                                      },
	{ "EPSGPolygon_1374_2013-06-15.xml", "arhpgn.l?s", "AR"                                      },
	{ "EPSGPolygon_2297_2013-06-15.xml", "cnhpgn.l?s", "CA North of 36.5"                        },
	{ "EPSGPolygon_2298_2013-06-15.xml", "cshpgn.l?s", "CA South of 36.5"                        },
	{ "EPSGPolygon_1376_2013-06-15.xml", "cohpgn.l?s", "CO"                                      },
	{ "EPSGPolygon_2378_2013-06-15.xml", "nehpgn.l?s", "CT, MA, NH, RI, VT"                      },
	{ "EPSGPolygon_2377_2013-06-15.xml", "mdhpgn.l?s", "DE, MD"                                  },
	{ "EPSGPolygon_1379_2013-06-15.xml", "flhpgn.l?s", "FL"                                      },
	{ "EPSGPolygon_1380_2013-06-15.xml", "gahpgn.l?s", "GA"                                      },
	{ "EPSGPolygon_1381_2013-06-15.xml", "wmhpgn.l?s", "ID"                                      },
	{ "EPSGPolygon_1382_2013-06-15.xml", "ilhpgn.l?s", "IL"                                      },
	{ "EPSGPolygon_1383_2013-06-15.xml", "inhpgn.l?s", "IN"                                      },
	{ "EPSGPolygon_1384_2013-06-15.xml", "iahpgn.l?s", "IA"                                      },
	{ "EPSGPolygon_1385_2013-06-15.xml", "kshpgn.l?s", "KS"                                      },
	{ "EPSGPolygon_1386_2013-06-15.xml", "kyhpgn.l?s", "KY"                                      },
	{ "EPSGPolygon_1387_2013-06-15.xml", "lahpgn.l?s", "LA"                                      },
	{ "EPSGPolygon_1388_2013-06-15.xml", "mehpgn.l?s", "ME (Maine)"                              },
	{ "EPSGPolygon_1391_2013-06-15.xml", "mihpgn.l?s", "MI"                                      },
	{ "EPSGPolygon_1392_2013-06-15.xml", "mnhpgn.l?s", "MN"                                      },
	{ "EPSGPolygon_1393_2013-06-15.xml", "mshpgn.l?s", "MS"                                      },
	{ "EPSGPolygon_1394_2013-06-15.xml", "mohpgn.l?s", "MO"                                      },
	{ "EPSGPolygon_1395_2013-06-15.xml", "emhpgn.l?s", "MT East (113W -> 103W)"                  },
	{ "EPSGPolygon_1395_2013-06-15.xml", "wmhpgn.l?s", "MT West (119W -> 109W)"                  },
	{ "EPSGPolygon_1396_2013-06-15.xml", "nbhpgn.l?s", "Nebraska (NE? -- nehpgn is New England)" },
	{ "EPSGPolygon_1397_2013-06-15.xml", "nvhpgn.l?s", "NV"                                      },
	{ "EPSGPolygon_1399_2013-06-15.xml", "njhpgn.l?s", "NJ"                                      },
	{ "EPSGPolygon_1400_2013-06-15.xml", "nmhpgn.l?s", "NM"                                      },
	{ "EPSGPolygon_1401_2013-06-15.xml", "nyhpgn.l?s", "NY"                                      },
	{ "EPSGPolygon_1402_2013-06-15.xml", "nchpgn.l?s", "NC"                                      },
	{ "EPSGPolygon_1403_2013-06-15.xml", "ndhpgn.l?s", "ND"                                      },
	{ "EPSGPolygon_1404_2013-06-15.xml", "ohhpgn.l?s", "OH"                                      },
	{ "EPSGPolygon_1405_2013-06-15.xml", "okhpgn.l?s", "OK"                                      },
	{ "EPSGPolygon_2381_2013-06-15.xml", "wohpgn.l?s", "WA, OR"                                  },
	{ "EPSGPolygon_1407_2013-06-15.xml", "pahpgn.l?s", "PA"                                      },
	{ "EPSGPolygon_1409_2013-06-15.xml", "schpgn.l?s", "SC"                                      },
	{ "EPSGPolygon_1410_2013-06-15.xml", "sdhpgn.l?s", "SD"                                      },
	{ "EPSGPolygon_1411_2013-06-15.xml", "tnhpgn.l?s", "TN"                                      },
	{ "EPSGPolygon_2379_2013-06-15.xml", "ethpgn.l?s", "TX east of 100W"                         },
	{ "EPSGPolygon_2380_2013-06-15.xml", "wthpgn.l?s", "TX west of 100W"                         },
	{ "EPSGPolygon_1413_2013-06-15.xml", "uthpgn.l?s", "UT"                                      },
	{ "EPSGPolygon_1415_2013-06-15.xml", "vahpgn.l?s", "VA"                                      },
	{ "EPSGPolygon_1417_2013-06-15.xml", "wvhpgn.l?s", "WV"                                      },
	{ "EPSGPolygon_1418_2013-06-15.xml", "wihpgn.l?s", "WI"                                      },
	{ "EPSGPolygon_1419_2013-06-15.xml", "wyhpgn.l?s", "WY"                                      },
	{                                "", "",           "??"                                      }
};

// The basic algorithm, using polygons obtained from EPSG, leaves a grid with
// some 10 holes involving 37 grid cell points.  These are grid cell points
// which are generally offshore and outside the boundary of any given state,
// but are required in order to give appropriate results given that the
// adjacent cells are on shore and thus the are covered by a 4 cell point
// rectangle will generally include some onshore geography.
//
// These holes were located by an earlier version of this program, manually
// examined, and the determination made as to which hpgn file(s) the resulting
// grid cell point values should be obtained from.
//
// Note that the last point in this table was a toss up. A cell point in the
// Long Island Sound, halfway between NY and CT.  The values of the two grid
// files were non-trivially different. We chose NY for this table.  We may
// later decide to set this particular point manually to be the average of the
// grid cell values for this point.
struct TcsHpgn48Holes
{
	double Longitude;
	double Latitude;
	char HpgnFilePair [80];
} KcsHpgn48Holes [] =
{
	// Hole # 1,  2 cell points  --  Cape Cod Bay
	{ -70.50,  42.00, "nehpgn.l?s"    },
	{ -70.25,  42.00, "nehpgn.l?s"    },
	// Hole # 2,  3 cell points  -- Chesapeak Bay
	{ -76.25,  38.00, "mdhpgn.l?s"    },
	{ -76.00,  38.00, "mdhpgn.l?s"    },
	{ -76.25,  38.25, "mdhpgn.l?s"    },
	// Hole # 3,  1 cell point   -- Delaware Bay
	{ -75.25,  39.00, "mdhpgn.l?s"    },
	// Hole # 4, 15 cell points  -- Canada, Lake Erie, & Lake Huron; east of Detroit
	{ -83.00,  42.00, "mihpgn.l?s"    },
	{ -83.00,  42.25, "mihpgn.l?s"    },
	{ -82.75,  41.75, "mihpgn.l?s"    },
	{ -82.75,  42.00, "mihpgn.l?s"    },
	{ -82.75,  42.25, "mihpgn.l?s"    },
	{ -82.50,  41.75, "mihpgn.l?s"    },
	{ -82.50,  42.00, "mihpgn.l?s"    },
	{ -82.50,  42.25, "mihpgn.l?s"    },
	{ -82.50,  42.50, "mihpgn.l?s"    },
	{ -82.25,  42.00, "mihpgn.l?s"    },
	{ -82.25,  42.25, "mihpgn.l?s"    },
	{ -82.25,  42.50, "mihpgn.l?s"    },
	{ -82.25,  42.75, "mihpgn.l?s"    },
	{ -82.25,  43.00, "mihpgn.l?s"    },
	{ -82.25,  43.25, "mihpgn.l?s"    },
	// Hole # 5,  2 cell points, -- West coast of FL, offshore 
	{ -83.00,  28.50, "flhpgn.l?s"    },
	{ -83.00,  28.75, "flhpgn.l?s"    },
	// Hole # 6,  4 cell points, -- West coast of FL, offshore, west of Ft Meyers 
	{ -82.25,  26.00, "flhpgn.l?s"    },
	{ -82.25,  26.25, "flhpgn.l?s"    },
	{ -82.00,  26.00, "flhpgn.l?s"    },
	{ -82.00,  26.25, "flhpgn.l?s"    },
	// Hole # 7,  1 cell point,  -- Gulf of Mexico, offshore, SE LA
	{ -89.00,  29.50, "lahpgn.l?s"    },
	// Hole # 8,  6 cell points, -- Gulf of Mexico, offshore, east of SE Texas coast
	{ -97.00,  26.25, "ethpgn.l?s"    },
	{ -97.00,  26.50, "ethpgn.l?s"    },
	{ -97.00,  26.75, "ethpgn.l?s"    },
	{ -97.00,  27.00, "ethpgn.l?s"    },
	{ -97.00,  27.25, "ethpgn.l?s"    },
	{ -97.00,  27.50, "ethpgn.l?s"    },
	// Hole # 9,  1 cell point,  -- offshore, halfway between WA and BC Canada
	{-123.00,  48.25, "wohpgn.l?s"    },
	// Hole #10,  1 cell point,  -- Offshore Maine, SE of Brunswick
	{ -70.00,  43.25, "mehpgn.l?s"    },
	// Hole #11,  1 cell point,  -- Long Island Sound, halfway between NY and CT
	{ -73.25,  41.00, "nyhpgn.l?s"    },
	// End of table marker.
	{   0.00,  00.00, ""              } 
};

// We use the polygons associated with each state to select which set of hpgn
// files we use to populate specific regions of geography.  At this point,
// 48Hpgn as all the specific values which are rather meticulously chosen.
// We use the following table to fill in around the edges of the 48Hpgn grid.
// Most of these values are actually geographically located offshore, Canada,
// and/or Mexico.  We need these values reasonablly accurate to appropriately
// balance grid cells which are onshore, but whose neighbors may be offshore
// and not included in the grid set so far.
//
// Per the purpose described above, we don't need all 40+ grid file sets in
// this table, only those which have a non-trivial shoreline or border with
// Canada and/or Mexico.
//
// To preserve our sanity, we progress clockwise from the SW.
struct TcsHpgn48EdgeFill
{
	char HpgnFilePair [80];		// the HPGN grid file set to use for this entry.
	double Longitude [2];		// longitude range of this entry.
	double Latitude [2];		// latitude range of this entry.
} KcsHpgn48EdgeFill [] =
{
	{	"cshpgn.l?s", -122.00, -116.00, 32.00, 36.50   },
	{	"cnhpgn.l?s", -125.00, -122.00, 36.50, 42.00   },
	{	"wohpgn.l?s", -125.00, -117.00, 42.00, 50.00   },
	{	"wmhpgn.l?s", -117.00, -111.00, 49.00, 50.00   },
	{	"emhpgn.l?s", -111.00, -104.00, 49.00, 50.00   },
	{	"ndhpgn.l?s", -104.00,  -97.75, 49.00, 50.00   },
	{	"mnhpgn.l?s",  -97.75,  -89.75, 46.25, 50.00   },
	{	"mihpgn.l?s",  -89.75,  -82.00, 42.00, 48.00   },
	{	"ohhpgn.l?s",  -83.50,  -80.50, 41.00, 43.00   },
	{	"pahpgn.l?s",  -80.50,  -79.75, 42.00, 44.00   },
	{	"nyhpgn.l?s",  -79.75,  -73.00, 42.75, 46.00   },
	{	"nehpgn.l?s",  -73.00,  -71.00, 45.00, 46.00   },
	{	"mehpgn.l?s",  -71.25,  -66.00, 43.00, 48.00   },
	{	"nehpgn.l?s",  -74.50,  -69.75, 41.00, 43.25   },
	{	"nyhpgn.l?s",  -73.50,  -72.50, 40.50, 41.25   },
	{	"njhpgn.l?s",  -75.00,  -72.50, 38.75, 40.50   },
	{	"mdhpgn.l?s",  -75.25,  -74.00, 38.25, 38.50   },
	{	"vahpgn.l?s",  -76.50,  -75.00, 36.50, 38.50   },
	{	"nchpgn.l?s",  -78.25,  -75.00, 33.75, 36.50   },
	{	"schpgn.l?s",  -81.75,  -78.00, 32.00, 34.00   },
	{	"gahpgn.l?s",  -82.00,  -80.00, 30.35, 32.00   },
	{	"flhpgn.l?s",  -87.75,  -80.00, 24.00, 31.25   },
	{	"alhpgn.l?s",  -88.25,  -86.25, 30.00, 31.25   },
	{	"mshpgn.l?s",  -89.50,  -88.50, 30.00, 30.75   },
	{	"lahpgn.l?s",  -94.00,  -88.00, 28.75, 30.25   },
	{	"ethpgn.l?s", -100.00,  -94.00, 25.00, 30.00   },
	{	"wthpgn.l?s", -106.50, -100.00, 25.00, 32.00   },
	{	"nmhpgn.l?s", -109.00, -106.50, 31.00, 32.00   },
	{	"wthpgn.l?s", -107.00, -106.50, 25.00, 31.00   },
	{	"azhpgn.l?s", -115.75, -109.00, 30.00, 33.00   },
	{	          "",   -0.00,   -0.00,  0.00,  0.00   }
};
bool csGenerate48Hpgn (const wchar_t* csDictDir,const wchar_t* epsgPolygonDir,bool verbose)
{
	bool ok (false);
	int subCnt;

	char hpgnDirPath [MAXPATH];
	char fenceDirPath [MAXPATH];

	// Note, the status checking here is primarily to make sure that success is
	// truly success.  That is, when we get through this whole thing with the
	// ok variable remaining true, we will feel confident that the result is
	// valid.

	wcstombs (hpgnDirPath,csDictDir,sizeof (hpgnDirPath));
	do
	{
		subCnt = CS_envsub (hpgnDirPath,sizeof (hpgnDirPath));
	} while (subCnt > 0);
	CS_stncat (hpgnDirPath,"\\USA\\HARN",sizeof (hpgnDirPath));

	wcstombs  (fenceDirPath,epsgPolygonDir,sizeof (fenceDirPath));
	do
	{
		subCnt = CS_envsub (fenceDirPath,sizeof (fenceDirPath));
	} while (subCnt > 0);

	// Create two objects in which we will build the results.  Note that upon
	// construction, all grid values are set to a specific value which is
	// easy to identify as being "not set"; i.e. a hole in the grid which was
	// not set by all the code below.
	
	// In each case, there exist a set of parameters commented out.  These are
	// the parameters which match the existing set of ??hpgn.l?s files.  The
	// parameters which are actually used match the coverage of the NSRS2007
	// and NSRS2011 grid data files.  The NSRS grid files cover an additional
	// 3 degrees of longitude for some reason.  Since these two datums are
	// directly related, I thought it best that the coverages be the same.
	
	// Regardles of the above differences, both files must have the exact
	// same coverages if the NODCON/HPGN/HARN code is to work.
	TcsLasLosFile Hpgn48Los (-125.0,24.0,0.25,105,237);		// ala NSRS
//	TcsLasLosFile Hpgn48Los (-125.0,24.0,0.25,105,225);		// ala composite HPGN
	Hpgn48Los.SetFileIdent ("NADCON: Unofficial 48 state HARN Grid by Open Source");
	Hpgn48Los.SetProgram ("CS-MAP");
	Hpgn48Los.SetZeeCount (0L);
	Hpgn48Los.SetAngle (0.0);

	TcsLasLosFile Hpgn48Las (-125.0,24.0,0.25,105,237);		// ala NSRS
//	TcsLasLosFile Hpgn48Las (-125.0,24.0,0.25,105,225);		// ala composite HPGN
	Hpgn48Las.SetFileIdent ("NADCON: Unofficial 48 state HARN Grid by Open Source");
	Hpgn48Las.SetProgram ("CS-MAP");
	Hpgn48Las.SetZeeCount (0L);
	Hpgn48Las.SetAngle (0.0);

	// Start a loop, once for each entry in the above table.
	ok = true;						// To get the loop started.
	TcsHpgn48Generate* tblPtr;
	for (tblPtr = KcsHpgn48Generate;ok && tblPtr->BoundaryFile [0] != '\0';tblPtr += 1)
	{
		// Here once for each entry in the table.  Construct two TcsLasLosFile
		// objects; one for the source los file, the second for the source las
		// file.
		long32_t recCount (0L);
		long32_t eleCount (0L);

		char fenceSourceFilePath [MAXPATH];
		char losSourceFilePath [MAXPATH];
		char lasSourceFilePath [MAXPATH];

		if (verbose)
		{
			std::cout << "Processing "
					  << tblPtr->RptLabel
					  << std::endl;
		}

		CS_stncp  (fenceSourceFilePath,fenceDirPath,sizeof (fenceSourceFilePath));
		CS_stncat (fenceSourceFilePath,"\\",sizeof (fenceSourceFilePath));
		CS_stncat (fenceSourceFilePath,tblPtr->BoundaryFile,sizeof (fenceSourceFilePath));

		TcsFence* fencePtr = new TcsFence (fenceSourceFilePath);
		ok = fencePtr->IsOk ();

		CS_stncp  (losSourceFilePath,hpgnDirPath,sizeof (losSourceFilePath));
		CS_stncat (losSourceFilePath,"\\",sizeof (losSourceFilePath));
		CS_stncat (losSourceFilePath,tblPtr->HpgnFilePair,sizeof (losSourceFilePath));
		CS_strrpl (losSourceFilePath,sizeof (losSourceFilePath),"l?s","los");

		CS_stncp  (lasSourceFilePath,hpgnDirPath,sizeof (lasSourceFilePath));
		CS_stncat (lasSourceFilePath,"\\",sizeof (lasSourceFilePath));
		CS_stncat (lasSourceFilePath,tblPtr->HpgnFilePair,sizeof (lasSourceFilePath));
		CS_strrpl (lasSourceFilePath,sizeof (lasSourceFilePath),"l?s","las");

		TcsLasLosFile* sourceLos = new TcsLasLosFile ();
		TcsLasLosFile* sourceLas = new TcsLasLosFile ();
		ok  = sourceLos->ReadFromFile (losSourceFilePath);
		ok &= sourceLas->ReadFromFile (lasSourceFilePath);

#ifdef __SKIP__
		// This code segment is used in testing and evaluation only.  Used with
		// TcsLasLosFile::WriteToGrid () to view grids in Excel.
		char losCsvFilePath [MAXPATH];
		char lasCsvFilePath [MAXPATH];
		CS_stncp  (losCsvFilePath,hpgnDirPath,sizeof (losCsvFilePath));
		CS_stncat (losCsvFilePath,"\\",sizeof (losCsvFilePath));
		CS_stncat (losCsvFilePath,tblPtr->HpgnFilePair,sizeof (losCsvFilePath));
		CS_strrpl (losCsvFilePath,sizeof (losCsvFilePath),".l?s","Los.csv");

		CS_stncp  (lasCsvFilePath,hpgnDirPath,sizeof (lasCsvFilePath));
		CS_stncat (lasCsvFilePath,"\\",sizeof (lasCsvFilePath));
		CS_stncat (lasCsvFilePath,tblPtr->HpgnFilePair,sizeof (lasCsvFilePath));
		CS_strrpl (lasCsvFilePath,sizeof (lasCsvFilePath),".l?s","Las.csv");

		sourceLos->WriteGridToCsv (losCsvFilePath);
		sourceLas->WriteGridToCsv (lasCsvFilePath);
#endif

		if (ok)
		{
			// Extract the dimensions of the source HPGN grid.
			recCount = sourceLos->GetRecordCount ();
			eleCount = sourceLos->GetElementCount ();
		}

		// Outer loop, once for each record in the hpdgn source file.
		for (long32_t recIdx = 0;ok && recIdx < recCount;recIdx += 1)
		{
			// Inner Loop, once for each element in each record.
			for (long32_t eleIdx = 0;ok && eleIdx < eleCount;eleIdx += 1)
			{
				double gridValueLos   (0.0);
				double gridValueLas   (0.0);
				double gridValueLos48 (0.0);
				double gridValueLas48 (0.0);
				double gridLL [2];

				// Get the grid value and the latitude and longitude associated
				// with the current index values.
				ok  = sourceLos->GetGridLocation (gridLL,recIdx,eleIdx);
				ok &= sourceLos->GetGridValue (gridValueLos,recIdx,eleIdx);
				ok &= sourceLas->GetGridValue (gridValueLas,recIdx,eleIdx);
				if (ok)
				{
					// See if the location of this grid point is within, or on,
					// the boundary defined by the fence.  If not, we just skip
					// this particular point.
					bool isIn = fencePtr->IsInside (gridLL);
					if (!isIn) continue;

					// See if the LL is within the coverage of the 48hpgn file.
					isIn = Hpgn48Los.IsCovered (gridLL);
					if (!isIn) continue;

					// Bit of a kludge here.  There are three states which have
					// multiple hpgn files which implies there is substantial
					// overlap.  Of course, the shoft values are not the same
					// in the areas of overlap; that would make too much sense.
					// For these cases, we some selective processing per
					// manual investigation into the overlap and the geography
					// involved.
					if (!CS_strnicmp (tblPtr->HpgnFilePair,"cs",2) &&
						(gridLL [1] >= 36.0))
					{
						// For California, we choose to use the northern file
						// for all points at or above 36 degrees latitude.
						continue;
					}
					if (!CS_strnicmp (tblPtr->HpgnFilePair,"em",2) &&
						(gridLL [0] < -111.0))
					{
						// For Montana, we choose to use the eastern file
						// for all points at or east of 111.0 degrees of
						// longitude.
						continue;
					}
					if (!CS_strnicmp (tblPtr->HpgnFilePair,"wm",2) &&
						(gridLL [0] >= -111.0))
					{
						// For Montana, we choose to use the western file
						// for all points west of 111.0 degrees of longitude.
						continue;
					}
					if (!CS_strnicmp (tblPtr->HpgnFilePair,"et",2) &&
						(gridLL [0] <= -100.0))
					{
						// For Texas, we choose to use the eastern file
						// for all points east of 100.0 degrees of longitude.
						continue;
					}
					if (!CS_strnicmp (tblPtr->HpgnFilePair,"wt",2) &&
						(gridLL [0] > -100.0))
					{
						// For Texas, we choose to use the western file
						// for all points at or west of 100.0 degrees of
						// longitude.
						continue;
					}

					// Get the grid values from 48hpgn los.
					ok  = Hpgn48Los.GetGridValue (gridValueLos48,gridLL [0],gridLL [1]);
					ok &= Hpgn48Las.GetGridValue (gridValueLas48,gridLL [0],gridLL [1]);
				}
				if (ok)
				{
					// See if the new 48hpgn data set already has a value for
					// this grid point.
					if (gridValueLos48 > Hpgn48Los.NoValueTest)
					{
						double deltaLatMM = fabs (gridValueLas - gridValueLas48) * (111000000.0 / 3600.0);
						double deltaLngMM = fabs (gridValueLos - gridValueLos48) * cos (gridLL [1] * ONE_DEGREE) * (111000000.0 / 3600.0);
						double deltaMaxMM = sqrt (deltaLatMM * deltaLatMM + deltaLngMM * deltaLngMM);
						// A value for this specific point has already been
						// set.  We have duplicate HPGN data points.  Not a
						// problem if the grid values are essentially the
						// same.
						if (deltaMaxMM > 1.0)
						{
							// OK, we have a problem.  We have multiple values
							// for this grid point, and the values are
							// sufficiently different for us to be concerned.
							std::wcout << L"Discrepancy:,"
									   << tblPtr->HpgnFilePair
									   << L","
									   << gridLL [0]
									   << L","
									   << gridLL [1]
									   << L","
									   << deltaMaxMM
									   << std::endl;
							continue;
						}
					}
					// If we are still here, all is OK.  We set the
					// grid values as appropriate.
					ok  = Hpgn48Los.SetGridValue (gridLL [0],gridLL [1],gridValueLos);
					ok &= Hpgn48Las.SetGridValue (gridLL [0],gridLL [1],gridValueLas);
				}
			}
		}
		
		// Do any holes which can be satisfied with the current las/los file
		// pair.
		struct TcsHpgn48Holes* holePtr;
		for (holePtr = KcsHpgn48Holes;ok && holePtr->HpgnFilePair[0] != '\0';holePtr += 1)
		{
			if (!CS_stricmp (tblPtr->HpgnFilePair,holePtr->HpgnFilePair))
			{
				double gridValueLos   (0.0);
				double gridValueLas   (0.0);
				// OK, we have a hole which we can deal with now.
				ok  = sourceLos->GetGridValue (gridValueLos,holePtr->Longitude,holePtr->Latitude);
				ok &= sourceLas->GetGridValue (gridValueLas,holePtr->Longitude,holePtr->Latitude);
				if (ok)
				{
					ok  = Hpgn48Los.SetGridValue (holePtr->Longitude,holePtr->Latitude,gridValueLos);
					ok &= Hpgn48Las.SetGridValue (holePtr->Longitude,holePtr->Latitude,gridValueLas);
				}
			}
		}
		// On to next table entry.  sourceLas and sourceLos should have
		// go out of scope and therefore will be deleted before the next
		// iteration starts.
	}

	// Fill in result edges.  This is clearly an after thought. If performance
	// were to be crucial, we would rewrite so that we do not repeatedly
	// recoontruct the HPGN grid file pairs.  However, all of the code above
	// works well and we choose not to mess with it.
	//
	// Getiing at least one cell of accurate edge fill is important as all
	// HPGN grid shifts are based on a four point grid cell. In areas close
	// to shor lines and/or national borders, one or more of these four points
	// may be offshore and outside the boundary of the specific state for which
	// the particular HPGN grid covers.  Thus, we need to build a buffer of
	// edge cells which accurately (accurate as possible) replicate the edge
	// that would be encountered in the specific HPGN grid file.
	TcsHpgn48EdgeFill* fillPtr;
	for (fillPtr = KcsHpgn48EdgeFill;ok && fillPtr->HpgnFilePair[0] != '\0';fillPtr += 1)
	{
		double longitude;
		double latitude;
		double losValue;
		double lasValue;
		double trgValue;
		char losSourceFilePath [MAXPATH];
		char lasSourceFilePath [MAXPATH];
	
		// Here once for each entry in the table.  We need to:
		//	1> Construct the grid file pair (again :<( )
		//	2> Loop through the longitude and latitude extents in the table
		//	   entry.
		//	3> If the target 48HPGN.l?s file already has a legitimate value
		//	   from whatever source, we go back to 2, and skip this particular
		//	   LL pair.
		//	4> If, and only if, the target 48HPGN.l?s grid file set does not
		//	   a legimate value for the current LL pair, we extract values from
		//	   the indicated ??hpgn.l?s file pair and insert the value into
		//	   the target 48HPGN.l?s file.
	
		CS_stncp  (losSourceFilePath,hpgnDirPath,sizeof (losSourceFilePath));
		CS_stncat (losSourceFilePath,"\\",sizeof (losSourceFilePath));
		CS_stncat (losSourceFilePath,fillPtr->HpgnFilePair,sizeof (losSourceFilePath));
		CS_strrpl (losSourceFilePath,sizeof (losSourceFilePath),"l?s","los");

		CS_stncp  (lasSourceFilePath,hpgnDirPath,sizeof (lasSourceFilePath));
		CS_stncat (lasSourceFilePath,"\\",sizeof (lasSourceFilePath));
		CS_stncat (lasSourceFilePath,fillPtr->HpgnFilePair,sizeof (lasSourceFilePath));
		CS_strrpl (lasSourceFilePath,sizeof (lasSourceFilePath),"l?s","las");

		TcsLasLosFile* sourceLos = new TcsLasLosFile ();
		TcsLasLosFile* sourceLas = new TcsLasLosFile ();
		ok  = sourceLos->ReadFromFile (losSourceFilePath);
		ok &= sourceLas->ReadFromFile (lasSourceFilePath);
	
		for (longitude = fillPtr->Longitude[0];ok && longitude <= fillPtr->Longitude[1];longitude += sourceLos->GetDeltaLongitude())
		{
			for (latitude = fillPtr->Latitude[0];ok && latitude <= fillPtr->Latitude[1];latitude += sourceLos->GetDeltaLatitude())
			{
				ok = Hpgn48Los.GetGridValue (trgValue,longitude,latitude);
				if (ok && trgValue < sourceLos->NoValueTest)
				{
					// This particular celll value has not been set by any of
					// the algorithms above.  So we'll set it from the the
					// currently active ??hpgn.l?s grid sets.
					ok  = sourceLos->GetGridValue (losValue,longitude,latitude);
					ok &= sourceLas->GetGridValue (lasValue,longitude,latitude);
					if (ok)
					{
						ok  = Hpgn48Los.SetGridValue (longitude,latitude,losValue);
						ok &= Hpgn48Las.SetGridValue (longitude,latitude,lasValue);
					}
				}
			}
		}
	}

	// Fill in remaining edges.
	if (ok)
	{
		// This EdgeFillDelta function is commented out as it will introduce
		// holes into a grid which previously did not have holes. The idea
		// behond the function was to produce a gradual slope to zero shift
		// at the edges.  A new and better algorithm will need to be found to
		// achieve this objective it it turns out to be required. 
		//ok  = Hpgn48Los.EdgeFillDelta (0.8);
		//ok &= Hpgn48Las.EdgeFillDelta (0.8);
		//ok &= Hpgn48Los.EdgeFillDelta (0.8);
		//ok &= Hpgn48Las.EdgeFillDelta (0.8);
		//ok &= Hpgn48Los.EdgeFillDelta (0.8);
		//ok &= Hpgn48Las.EdgeFillDelta (0.8);
		//ok &= Hpgn48Los.EdgeFillDelta (0.8);
		//ok &= Hpgn48Las.EdgeFillDelta (0.8);
		//ok &= Hpgn48Los.EdgeFillDelta (0.8);
		//ok &= Hpgn48Las.EdgeFillDelta (0.8);
		ok  = Hpgn48Los.EdgeFill  (0.0);
		ok &= Hpgn48Las.EdgeFill  (0.0);
	}

	// Check the result for holes.
	if (ok)
	{
		long32_t holeCountLos;
		long32_t holeCountLas;

		holeCountLos = Hpgn48Los.HoleCheck (std::wcerr,true);
		holeCountLas = Hpgn48Las.HoleCheck (std::wcerr,true);
		if (holeCountLos != 0 || holeCountLas != 0)
		{
			std::wcout << "Los Hole Count = "
					   << holeCountLos
					   << "; LasHoleCount = "
					   << holeCountLas
					   << L"."
					   << std::endl;
			ok = false;
		}
	}
	if (verbose)
	{
		bool csvOk;
		csvOk = Hpgn48Los.WriteGridToCsv ("C:\\Temp\\48HpgnLos.csv");
		csvOk = Hpgn48Las.WriteGridToCsv ("C:\\Temp\\48HpgnLas.csv");
	}

	// Write out the 48hpgn.l?s files.
	if (ok)
	{
		char hpgn48LosPath [MAXPATH];
		char hpgn48LasPath [MAXPATH];

		// Construct the file name path for the resulting los file.
		CS_stncp  (hpgn48LosPath,hpgnDirPath,sizeof (hpgn48LosPath));
		CS_stncat (hpgn48LosPath,"\\",sizeof (hpgn48LosPath));
		CS_stncat (hpgn48LosPath,"48hpgn.los",sizeof (hpgn48LosPath));
		ok = Hpgn48Los.WriteToFile (hpgn48LosPath);

		CS_stncp  (hpgn48LasPath,hpgnDirPath,sizeof (hpgn48LasPath));
		CS_stncat (hpgn48LasPath,"\\",sizeof (hpgn48LasPath));
		CS_stncat (hpgn48LasPath,"48hpgn.las",sizeof (hpgn48LasPath));
		ok &= Hpgn48Las.WriteToFile (hpgn48LasPath);
	}

	// Return the status of this entire result.;
	return ok;
}

//
// Be careful here with points in Texas, Montana, and California where there
// are multiple ??hpgn.l?s file sets for the given state.  These states
// have a single HARN/?? datum and thus a single HARN/??.LL coordinate
// system.  If the point you chose for testing is in one of these states,
// make sure it is not in the region where the two ??hpgn.l?s file
// systems overlap. If you don't do this, your likely to get results which
// will drive you crazy trying to figure out why the tests are failing.
//
struct TcsHpn48TestSuite
{
	char TestLng [32];
	char TestLat [32];
	char SrcCRS  [32];
	char TrgCRS1 [32];
	char TrgCRS2 [32];
	double Tolerance;
	char Comment [80];
} KcsHpgn48TestSuite [] =
{
	// The following points are central to the various regions indicated by the TrgCRS1 coordinate system.
	// Thus, these should convert fairly precisely and the tolerance should be rather tight.
	{  "69 30 20.0000000W","45 05 40.0000000N", "LL83",    "HARN/ME.LL",    "NAD83/HARN.LL",    5.0E-10, "Central Maine"                },
	{  "72 30 40.0000000W","42 47 30.0000000N", "LL83",    "HARN/NE.LL",    "NAD83/HARN.LL",    5.0E-10, "VT/NH/MA join point"          },
	{  "74 56 40.0000000W","42 52 35.0000000N", "LL83",    "HARN/NY.LL",    "NAD83/HARN.LL",    5.0E-10, "Central New York"             },
	{  "75 03 30.0000000W","39 38 20.0000000N", "LL83",    "HARN/NJ.LL",    "NAD83/HARN.LL",    5.0E-10, "Central New Jersey"           },
	{  "75 41 39.0000000W","38 27 42.0000000N", "LL83",    "HARN/MD.LL",    "NAD83/HARN.LL",    5.0E-10, "Southwest corner of Delaware" },
	{  "77 30 30.0000000W","40 45 30.0000000N", "LL83",    "HARN/PA.LL",    "NAD83/HARN.LL",    5.0E-10, "Central Pennsylvania"         },
	{  "80 56 30.0000000W","38 31 30.0000000N", "LL83",    "HARN/WV.LL",    "NAD83/HARN.LL",    5.0E-10, "Central West Virginia"        },
	{  "78 33 30.0000000W","37 32 30.0000000N", "LL83",    "HARN/VA.LL",    "NAD83/HARN.LL",    5.0E-10, "Central Virgina"              },
	{  "78 52 36.0000000W","35 28 42.0000000N", "LL83",    "HARN/NC.LL",    "NAD83/HARN.LL",    5.0E-10, "Central North Carolina"       },
	{ "120 57 00.0000000W","44 20 00.0000000N", "LL83",    "HARN/WO.LL",    "NAD83/HARN.LL",    5.0E-10, "South Central Washington"     },
	{ "107 18 05.0000000W","43 10 00.0000000N", "LL83",    "HARN/WY.LL",    "NAD83/HARN.LL",    5.0E-10, "Central Wyoming"              },

	// The following points are right on the border of at least two states. Thus, each point gets converted
	// two, three, or four different HPGN regions.  We know that these conversions will be less accurate.  The
	// primary purpose of the test is to make sure that the difference is within tolerance.
	{ "109 02 31.7700000W","36 59 52.4300000N", "LL83",    "HARN/CO.LL",    "NAD83/HARN.LL",    5.0E-08, "Four Corners - AZ/CO/NM/UT"   },
	{ "109 02 31.7700000W","36 59 52.4300000N", "LL83",    "HARN/UT.LL",    "NAD83/HARN.LL",    5.0E-08, "Four Corners - AZ/CO/NM/UT"   },
	{ "109 02 31.7700000W","36 59 52.4300000N", "LL83",    "HARN/AZ.LL",    "NAD83/HARN.LL",    5.0E-08, "Four Corners - AZ/CO/NM/UT"   },
	{ "109 02 31.7700000W","36 59 52.4300000N", "LL83",    "HARN/NM.LL",    "NAD83/HARN.LL",    5.0E-08, "Four Corners - AZ/CO/NM/UT"   },
	{ "103 00 13.9800000W","36 28 48.8200000N", "LL83",    "HARN/OK.LL",    "NAD83/HARN.LL",    5.0E-08, "OK/TX/NM"                     },
	{ "103 00 13.9800000W","36 28 48.8200000N", "LL83",    "HARN/TX.LL",    "NAD83/HARN.LL",    5.0E-08, "OK/TX/NM"                     },
	{ "103 00 13.9800000W","36 28 48.8200000N", "LL83",    "HARN/NM.LL",    "NAD83/HARN.LL",    5.0E-08, "OK/TX/NM"                     },
	{ "102 03 29.0800000W","36 59 57.6900000N", "LL83",    "HARN/CO.LL",    "NAD83/HARN.LL",    5.0E-08, "CO/OK/KS"                     },
	{ "102 03 29.0800000W","36 59 57.6900000N", "LL83",    "HARN/OK.LL",    "NAD83/HARN.LL",    5.0E-08, "CO/OK/KS"                     },
	{ "102 03 29.0800000W","36 59 57.6900000N", "LL83",    "HARN/KS.LL",    "NAD83/HARN.LL",    5.0E-08, "CO/OK/KS"                     },
	{  "84 49 20.1800000W","39 07 00.6300000N", "LL83",    "HARN/IN.LL",    "NAD83/HARN.LL",    5.0E-08, "IN/OH/KY"                     },
	{  "84 49 20.1800000W","39 07 00.6300000N", "LL83",    "HARN/OH.LL",    "NAD83/HARN.LL",    5.0E-08, "IN/OH/KY"                     },
	{  "84 49 20.1800000W","39 07 00.6300000N", "LL83",    "HARN/KY.LL",    "NAD83/HARN.LL",    5.0E-08, "IN/OH/KY"                     },
	{  "74 43 37.9500000W","41 21 55.4200000N", "LL83",    "HARN/NY.LL",    "NAD83/HARN.LL",    5.0E-08, "NY/NJ/PA"                     },
	{  "74 43 37.9500000W","41 21 55.4200000N", "LL83",    "HARN/NJ.LL",    "NAD83/HARN.LL",    5.0E-08, "NY/NJ/PA"                     },
	{  "74 43 37.9500000W","41 21 55.4200000N", "LL83",    "HARN/PA.LL",    "NAD83/HARN.LL",    5.0E-08, "NY/NJ/PA"                     },
	{ "116 55 48.7400000W","46 00 22.1900000N", "LL83",    "HARN/WO.LL",    "NAD83/HARN.LL",    5.0E-08, "WA/OR/ID"                     },
	{ "116 55 48.7400000W","46 00 22.1900000N", "LL83",    "HARN/MT.LL",    "NAD83/HARN.LL",    5.0E-08, "WA/OR/ID (ID==west Montana)"  },

	// Mark the end of the table.
	{              "",            "",         "",              "",                 "",        0.0, "End of table marker"          },
};
// A function to write a series of test points for the generated 48hpgn.l?s
// files using the above table.  After this is all tested and debugged, we
// may add a large number of points to the above table.
bool csGenerate48HpgnTest (const wchar_t* csDictDir)
{
	bool ok;
	int st;
	int subCnt;

	char ccCsMapDir [MAXPATH];
	struct TcsHpn48TestSuite* tblPtr;

	// Loop through the table and in test file format, write each test.  Note
	// that we use CS-MAP to compute the results.  In the conversion process
	// we use TrgCRS1 for the calculation, but use TrgCRS2 when we write the
	// test out.  Thus, when the test data file is processed, the calculation
	// will be use 48hpgn.l?s and the results will be compared to results
	// computed using a HPGN??.LL; i.e. using the hpgn??.l?s files
	// individually.
	wcstombs (ccCsMapDir,csDictDir,sizeof (ccCsMapDir));
	do
	{
		subCnt = CS_envsub (ccCsMapDir,sizeof (ccCsMapDir));
	} while (subCnt > 0);	
	st = CS_altdr (ccCsMapDir);
	ok = (st == 0);
	for (tblPtr = KcsHpgn48TestSuite;ok && tblPtr->TestLng [0] != '\0';tblPtr += 1)
	{
		int st;

		long32_t lngFormat;
		long32_t latFormat;
		double longitude;
		double latitude;
		double wrkLL [2];
		char rsltLng [32];
		char rsltLat [32];
		
		lngFormat = CS_atof (&longitude,tblPtr->TestLng);
		latFormat = CS_atof (&latitude,tblPtr->TestLat);
		wrkLL [0] = longitude;
		wrkLL [1] = latitude;
		st = CS_cnvrt (tblPtr->SrcCRS,tblPtr->TrgCRS1,wrkLL);
		ok = (st == 0);
		if (ok)
		{
			// Calculate the ??hpgn.l?s result.
			CS_ftoa (rsltLng,sizeof (rsltLng),wrkLL [0],lngFormat);
			CS_ftoa (rsltLat,sizeof (rsltLat),wrkLL [1],latFormat);

			// Write a test case where the 48hpgn.l?s file will be used and
			// the same results =/-tolerance required for the test to pass.
			printf ("%s,%s,%s,%s,%s,%s,%7.1E,%7.1E\n",tblPtr->SrcCRS,
													  tblPtr->TestLng,
						 							  tblPtr->TestLat,
													  tblPtr->TrgCRS2,
													  rsltLng,
													  rsltLat,
													  tblPtr->Tolerance,
													  tblPtr->Tolerance);
			// The inverse of the above test; using the same tolerance???
			printf ("%s,%s,%s,%s,%s,%s,%7.1E,%7.1E\n",tblPtr->TrgCRS2,
													  rsltLng,
													  rsltLat,
													  tblPtr->SrcCRS,
													  tblPtr->TestLng,
						 							  tblPtr->TestLat,
													  tblPtr->Tolerance,
													  tblPtr->Tolerance);
		}
		else
		{
			char errMsg [MAXPATH];
			CS_errmsg (errMsg,sizeof (errMsg));
			printf ("Failure!!! Reason = %s\n",errMsg);
			ok = true;
		}
	}
	return ok;
}
