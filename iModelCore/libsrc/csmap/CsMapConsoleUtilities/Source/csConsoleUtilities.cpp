/*
 * Copyright (c) 2008, Autodesk, Inc.
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
 *       contributors may be used to endorse or promote products derived
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

#include "csConsoleUtilities.hpp"

#if (_RUN_TIME < _rt_UNIXPCC)
wchar_t csDataDir [MAXPATH] = L"%OPEN_SOURCE%\\MetaCrs\\CsMap\\trunk\\CsMapDev\\Data";
wchar_t csDictDir [MAXPATH] = L"%OPEN_SOURCE%\\MetaCrs\\CsMap\\trunk\\CsMapDev\\Dictionaries";
wchar_t csDictSrc [MAXPATH] = L"%OPEN_SOURCE%\\MetaCrs\\CsMap\\trunk\\CsMapDev\\Dictionaries";
wchar_t csEpsgDir [MAXPATH] = L"%GEODETIC_DATA%\\EPSG\\CSV";
wchar_t csEpsgPolyDir []    = L"%GEODETIC_DATA%\\EPSG\\EPSG-Polygon-package-20130626";
wchar_t csTempDir [MAXPATH] = L"C:\\TEMP";
#else
const wchar_t csDataDir [] = L"$OSGEO/CsMap/MetaCrs/CsMap/trunk/CsMapDev/Data";
const char csDictDir []    = "$OSGEO/CsMap/MetaCrs/CsMap/trunk/CsMapDev/Dictionaries";
wchar_t csEpsgDir []       = L"${GeodeticData}/Epsg/CSV";
const wchar_t csDataDir [] = L"$OSGEO/CsMap/MetaCrs/CsMap/trunk/CsMapDev/Data";
const wchar_t csTempDir [] = L"/usr/tmp";
#endif

int main (int argc,char* argv [])
{
	bool ok (false);
	int envStatus;

#if defined (_MSC_VER) && _MSC_VER >= 1400 && _MSC_VER < 1900
	// This is a Microsoft specific function call.  It forces the exponential
	// printf format to two digits, which I prefer.  Maybe there is a more
	// generic form of this, but I don't know about it.
	_set_output_format(_TWO_DIGIT_EXPONENT);
#	ifdef _DEBUG
		_CrtSetDbgFlag (_CRTDBG_CHECK_DEFAULT_DF);
#	endif
#endif

	// Perform environmental variable substitution on the global variables
	// defined above which specify the location of stuff on the host system.
	// The loops are required as the CS_envsubWc function only replaces a
	// single environmental variable per call.  Looping ensures multiple
	// references are replaced.
	for (envStatus = 1;envStatus != 0;)
	{
		envStatus = CS_envsubWc (csDataDir,wcCount (csDataDir));
	}
	for (envStatus = 1;envStatus != 0;)
	{
		envStatus = CS_envsubWc (csDictDir,wcCount (csDictDir));
	}
	for (envStatus = 1;envStatus != 0;)
	{
		envStatus = CS_envsubWc (csDictSrc,wcCount (csDictSrc));
	}
	for (envStatus = 1;envStatus != 0;)
	{
		envStatus = CS_envsubWc (csEpsgDir,wcCount (csEpsgDir));
	}
	for (envStatus = 1;envStatus != 0;)
	{
		envStatus = CS_envsubWc (csTempDir,wcCount (csTempDir));
	}

#ifdef __SKIP__
	// The following untility is a frequently used one.  We leave here,
	// but comment out so it can be used with ease.
	//
	// Resort a manually edited NameMapper.csv file to standard order.  Also,
	// this utility will match the quoting in the sorted  data file as
	// maintained in SVN.  This feature is often required so that a "diff"
	// between old and manually edited (especially if you use Excel to do the
	// editing) will produce usable results.
	//
	// ok = ResortNameMapperCsv (csTempDir,csDictSrc,true);
	//
	// Note that the Resort utility will overwrite the source file if the same
	// directory is used for the first two parameters.  Thus, to avoid losing
	// the results of a painful editing session, we leave the controlled source
	// to point to the temporary directory as the result directory.  It is
	// suggested that this only be changed on a temporary basis.
	return ok;
#endif

#ifdef __SKIP__
	// The following utility is a frequently used one.  We leave here,
	// but comment out so it can be used with ease.
	//
	// Produce a list of EPSG CRS's in the current version of EPSG which
	// are not referenced in the NameMapper.  Entries in the report thus
	// produced may be ommissions in the NameMapper, or systems in EPSG
	// which are not contained in the CS-MAP coordsys dictionary.

	ok = ListUnmappedEpsgCodes (csEpsgDir,csDictDir);
	return ok;
#endif

#ifdef __SKIP__
	// The following can be useful at times.  It produces .csv format files for
	// the various dictionary files.  Oops!!!  Haven't figured out how to
	// put the Geodetic Transformation Dictionary into a CSV format, YET!  There
	// is the issue of a huge union which complicates things.
	ok = csCsdToCsvCT (csDictDir,true);
	ok = csCsdToCsvEL (csDictDir,true);
	ok = csCsdToCsvDT (csDictDir,true);
	ok = csCsdToCsvCS (csDictDir,true);
	ok = csCsdToCsvGX (csDictDir,true);
	ok = csCsdToCsvGP (csDictDir,true);
#endif

#ifdef __SKIP__
	// The following will deprecate 221 coordinate systems.  These are the
	// Wisonsin County specific systems referenced to the old HPGN datum
	// definition.  The new dictionaries are written to the csTempDir for
	// inspection prior to manual replacement in the SVN repository.
	ok = csDeprecateWiHpgn (csTempDir,csDictSrc);
#endif

	return ok?0:-1;
}
