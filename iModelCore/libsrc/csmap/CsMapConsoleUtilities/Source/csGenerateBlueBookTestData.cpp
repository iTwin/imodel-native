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
#include "csBlueBook.hpp"

struct TcsBlueBookTestDataTable
{
	wchar_t station [3];
	double srcLongitude;
	double srcLatitude;
	double srcEllipsoidHgt;
	double trgLatitude;
	double trgLongitude;
	double trgEllipsoidHgt;
} KcsBlueBookTestDataTable [] =
{
	// The following tests exact cell corners.
	{L"AA",    -102.00000000000000,  35.00000000000000,   100.000,       0.0,0.0,0.0  },		//Hard core cell corner
	{L"AB",    -102.01666666666667,  35.00000000000000,   100.000,       0.0,0.0,0.0  },		//Hard core cell corner, west one cell.
	{L"AC",    -102.00000000000000,  35.01666666666667,   100.000,       0.0,0.0,0.0  },		//Hard core cell corner, north one cell.
	{L"AD",    -102.01666666666667,  35.01666666666667,   100.000,       0.0,0.0,0.0  },		//Hard core cell corner, north west one cell.

	// The following test mid-points on the cell sides.
	{L"BA",    -102.00000000000000,  35.00833333333333,   100.000,       0.0,0.0,0.0  },
	{L"BB",    -102.00833333333333,  35.00000000000000,   100.000,       0.0,0.0,0.0  },
	{L"BC",    -102.01666666666667,  35.00833333333333,   100.000,       0.0,0.0,0.0  },
	{L"BD",    -102.00833333333333,  35.01666666666667,   100.000,       0.0,0.0,0.0  },

	// The following tests the center of several different cells.
	{L"CA",    -102.00833333333333,  35.00833333333333,   100.000,       0.0,0.0,0.0  },
	{L"CB",    -102.00833333333333,  45.00833333333333,   100.000,       0.0,0.0,0.0  },
	{L"CC",    -105.00833333333333,  35.00833333333333,   100.000,       0.0,0.0,0.0  },
	{L"CD",    -105.00833333333333,  35.00833333333333,   100.000,       0.0,0.0,0.0  },

	// The following tests point about one quarter into the cell from all sides.
	{L"DA",    -102.00416666666667,  35.00416666666667,   100.000,       0.0,0.0,0.0  },
	{L"DB",    -102.00416666666667,  35.01250000000000,   100.000,       0.0,0.0,0.0  },
	{L"DC",    -102.01250000000000,  35.01250000000000,   100.000,       0.0,0.0,0.0  },
	{L"DD",    -102.01250000000000,  35.00416666666667,   100.000,       0.0,0.0,0.0  },
	{L"DE",    - 82.00416666666667,  41.00416666666667,   100.000,       0.0,0.0,0.0  },
	{L"DF",     -82.00416666666667,  41.01250000000000,   100.000,       0.0,0.0,0.0  },
	{L"DG",     -82.01250000000000,  41.01250000000000,   100.000,       0.0,0.0,0.0  },
	{L"DH",     -82.01250000000000,  41.00416666666667,   100.000,       0.0,0.0,0.0  },

	// The following tests random points, forcing an update of buffer contents.
	{L"EA",    -102.11223344556677,  35.77665544332211,   100.000,       0.0,0.0,0.0  },
	{L"EB",     -87.11223344556677,  43.77665544332211,   100.000,       0.0,0.0,0.0  },
	{L"EC",    -115.11223344556677,  39.77665544332211,   100.000,       0.0,0.0,0.0  },
	{L"ED",    -101.11223344556677,  28.77665544332211,   100.000,       0.0,0.0,0.0  },
	{L"EE",     -70.33333333333333,  45.22222222222222,   100.000,       0.0,0.0,0.0  },
	{L"EF",     -82.50012312432524,  26.36239487134892,   100.000,       0.0,0.0,0.0  },
	{L"EG",    -122.91827364546372,  27.19283746547382,   100.000,       0.0,0.0,0.0  },	


	{L"",         0.0,               0.0,                   0.0,         0.0,0.0,0.0  }		// End of Table;
};

bool csGenerateBlueBookTestData (const wchar_t* testDataDir,bool phaseTwo)
{
	bool ok (false);

	TcsBlueBookTestDataTable* tblPtr;
	wchar_t filePath [MAXPATH];
	wchar_t workBufr [256];

	// There are two phases. In Phase One, we use the above hard coded table to
	// generate the original test data file in blue book format which is to be
	// processed by the geocon.exe program. This file we arbitrarily name
	// "BlueBookDTestDataIn.txt".
	
	// We thought about actually running geocon.exe from this module, but that
	// would be painful as geocon.exe does not work off command line
	// parameters; it issues prompts for all operating parameters.
	
	// Once this file is produced, we run it through geocon.exe and produce
	// a file which is arbitrarily named "BlueBookTestDataOut.txt".  In Phase
	// Two we/ use the information in the table above and the information in
	// the processed data file to produce a third file which we arbitrarily
	// name "CS_TEST_BB.dat".

	// Using these hard coded file names we can avoid a lotof coding.  This
	// code will probably not be used more than two or three times.

	if (!phaseTwo)
	{
		// If we not doing Phase Two, we assume that we a re doing Phase One.
		// Create the output stream.
		wcsncpy (filePath,testDataDir,wcCount (filePath));
		CS_envsubWc (filePath,wcCount (filePath));
		wcsncat (filePath,L"\\BlueBookTestDataIn.txt",wcCount (filePath));
		std::wofstream bbTestStrmAA (filePath,std::ios_base::out | std::ios_base::trunc);
		ok = bbTestStrmAA.is_open ();
		if (ok)
		{
			// Note, sequence number is a card thing, station number is a point thing.
			for (tblPtr = KcsBlueBookTestDataTable;ok && tblPtr->station [0] != '\0';tblPtr+= 1)
			{
				TcsBbRec80 newTest (tblPtr->srcLatitude,tblPtr->srcLongitude,tblPtr->srcEllipsoidHgt);
				// By leaving sequence number set at the default zero value,
				// we instruct the TcsBbRec80 object to supply sequence numbers
				// before output.
				newTest.SetStationName (tblPtr->station);
				ok = newTest.WriteToStream (bbTestStrmAA,true);
				TcsBbRec80::BumpStationNumber ();
			}
			bbTestStrmAA.close ();
		}
	}
	else
	{
		// We're doing Phase Two.
		wcsncpy (filePath,testDataDir,wcCount (filePath));
		CS_envsubWc (filePath,wcCount (filePath));
		wcsncat (filePath,L"\\BlueBookTestDataOut.txt",wcCount (filePath));
		std::wifstream bbStrmTestBB (filePath,std::ios_base::in);
		ok = bbStrmTestBB.is_open ();
		if (ok)
		{
			wcsncpy (filePath,testDataDir,wcCount (filePath));
			CS_envsubWc (filePath,wcCount (filePath));
			wcsncat (filePath,L"\\CS_TEST_BB.dat",wcCount (filePath));
			std::wofstream bbStrmTestCC (filePath,std::ios_base::out | std::ios_base::trunc);
			ok = bbStrmTestCC.is_open ();
			if (ok)
			{
				tblPtr = KcsBlueBookTestDataTable;
				while (ok && bbStrmTestBB.good ())
				{
					TcsBbRec80 newTest;
					ok = newTest.ReadFromStream (bbStrmTestBB);
					// We assume that the converted points will be in the same order
					// as the table above which was used to produce them.
					// We compare the station names just to make sure.
					if (ok)
					{
						int status = wcscmp (tblPtr->station,newTest.GetStationName ());
						ok = (status == 0);
					}
					if (ok)
					{
						// OK, we have the original point from the table and the
						// converted point from the geocon processed file.  Write
						// a test line for the ConsoleTestCpp program.  Ooops!
						// I'm also going to have to hard code the test coordinate
						// system names into this module.
						swprintf (workBufr,wcCount (workBufr),
											L"%s,%.12f,%.12f,%s,%.12f,%.12f,1.5E-09,1.5E-09",
											L"NSRS07.LL",
											tblPtr->srcLongitude,
											tblPtr->srcLatitude,
											L"NSRS11.LL",
											newTest.GetLongitude (),
											newTest.GetLatitude ());
						bbStrmTestCC << workBufr << std::endl;
						swprintf (workBufr,wcCount (workBufr),
											L"%s,%.12f,%.12f,%s,%.12f,%.12f,1.5E-09,1.5E-09",
											L"NSRS11.LL",
											newTest.GetLongitude (),
											newTest.GetLatitude (),
											L"NSRS07.LL",
											tblPtr->srcLongitude,
											tblPtr->srcLatitude);
						bbStrmTestCC << workBufr << std::endl;
						tblPtr++;
					}
				}
				bbStrmTestCC.close ();
			}
			bbStrmTestBB.close ();
		}
	}
	return ok;
}
