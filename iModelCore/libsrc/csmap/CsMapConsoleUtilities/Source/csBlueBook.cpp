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
#include "csBlueBook.hpp"

//=============================================================================
// TcsBbRec86 --  Implementation
//=============================================================================
// Construction  /  Destruction  /  Assignment
TcsBbRec86::TcsBbRec86 (void) : SequenceNbr           (0UL),
								OrthometricHgt        (0.0),
								EllipsoidalHgt        (0.0),
								GeoidHgt              (0.0),
								StationSerialNbr      (TcsBbRec80::NextStationNumber),
								OrthometricCode	      (L'G'),
								OrthometricNgsidb     (L'N'),
								GeoidHeightCode       (L'4'),
								EllipsoidCode         (L'D'),
								EllipsoidDatum        (L'A')
{
	OrthometricOrderClass [0] = L' ';
	OrthometricOrderClass [1] = L' ';
	OrthometricDatum [0]      = L'8';
	OrthometricDatum [1]      = L'8';
	OrganizationCode [0]      = L'\0';
	EllipsoidOrderClass [0]   = L'5';
	EllipsoidOrderClass [1]   = L'2';
	Comments [0]              = L'\0';
}
// We give preference to ellipsoidal height as that is what is usually
// available from GPS observations.
TcsBbRec86::TcsBbRec86 (double ellipHeight,double orthoHeight) : SequenceNbr           (0UL),
																 OrthometricHgt        (orthoHeight),
																 GeoidHgt              (0.0),
																 EllipsoidalHgt        (ellipHeight),
																 StationSerialNbr      (TcsBbRec80::NextStationNumber),
																 OrthometricCode	   (L'G'),
																 OrthometricNgsidb     (L'N'),
																 GeoidHeightCode       (L'4'),
																 EllipsoidCode         (L'D'),
																 EllipsoidDatum        (L'A')
{
	OrthometricOrderClass [0] = L' ';
	OrthometricOrderClass [1] = L' ';
	OrthometricDatum [0]      = L'8';
	OrthometricDatum [1]      = L'8';
	OrganizationCode [0]      = L'\0';
	EllipsoidOrderClass [0]   = L'5';
	EllipsoidOrderClass [1]   = L'2';
	Comments [0]              = L'\0';
}
TcsBbRec86::TcsBbRec86 (const TcsBbRec86& source) : SequenceNbr          (source.SequenceNbr),
												    OrthometricHgt       (source.OrthometricHgt),
												    GeoidHgt             (source.GeoidHgt),
												    EllipsoidalHgt       (source.EllipsoidalHgt),
												    OrthometricCode	     (source.OrthometricCode),
												    StationSerialNbr     (source.StationSerialNbr),
												    OrthometricNgsidb    (source.OrthometricNgsidb),
												    GeoidHeightCode      (source.GeoidHeightCode),
												    EllipsoidCode        (source.EllipsoidCode),
												    EllipsoidDatum       (source.EllipsoidDatum)
{
	OrthometricOrderClass [0] = source.OrthometricOrderClass [0];
	OrthometricOrderClass [1] = source.OrthometricOrderClass [1];
	OrthometricDatum [0]      = source.OrthometricDatum [0];
	OrthometricDatum [1]      = source.OrthometricDatum [1];
	EllipsoidCode             = source.EllipsoidCode;
	EllipsoidOrderClass [0]   = source.EllipsoidOrderClass [0];
	EllipsoidOrderClass [1]   = source.EllipsoidOrderClass [1];
	EllipsoidDatum            = source.EllipsoidDatum;

	wcsncpy (OrganizationCode,source.OrganizationCode,wcCount (OrganizationCode));
	wcsncpy (Comments,source.Comments,wcCount (Comments));
}
TcsBbRec86& TcsBbRec86::operator= (const TcsBbRec86& rhs)
{
	if (&rhs != this)
	{
		SequenceNbr               = rhs.SequenceNbr;
		OrthometricHgt            = rhs.OrthometricHgt;
		GeoidHgt                  = rhs.GeoidHgt;
		EllipsoidalHgt            = rhs.EllipsoidalHgt;
		OrthometricCode	          = rhs.OrthometricCode;
		StationSerialNbr          = rhs.StationSerialNbr;
		OrthometricNgsidb         = rhs.OrthometricNgsidb;
		GeoidHeightCode           = rhs.GeoidHeightCode;
		EllipsoidCode             = rhs.EllipsoidCode;
		EllipsoidDatum            = rhs.EllipsoidDatum;
		OrthometricOrderClass [0] = rhs.OrthometricOrderClass [0];
		OrthometricOrderClass [1] = rhs.OrthometricOrderClass [1];
		OrthometricDatum [0]      = rhs.OrthometricDatum [0];
		OrthometricDatum [1]      = rhs.OrthometricDatum [1];
		EllipsoidCode             = rhs.EllipsoidCode;
		EllipsoidOrderClass [0]   = rhs.EllipsoidOrderClass [0];
		EllipsoidOrderClass [1]   = rhs.EllipsoidOrderClass [1];
		EllipsoidDatum            = rhs.EllipsoidDatum;

		wcsncpy (OrganizationCode,rhs.OrganizationCode,wcCount (OrganizationCode));
		wcsncpy (Comments,rhs.Comments,wcCount (Comments));
	}
	return *this;
}
TcsBbRec86::~TcsBbRec86 (void)
{
	// Nothing to do here.
}
//=============================================================================
// Operator Overrides
//=============================================================================
// Public Named Member Functions
bool TcsBbRec86::ReadFromStream (std::wistream& inStream)
{
	bool ok (false);
	wchar_t myCardImage [82];

	if (!inStream.eof ())
	{
		inStream.read (myCardImage,80);
		myCardImage [80] = L'\0';
		ok = inStream.good ();
		if (ok)
		{
			// We see if there is a new line waiting, if so we ignore it.
			// This is our attempt at accepting text files with new-lines or
			// simple 80 character record files.
			wchar_t nextChr = inStream.peek ();
			if (nextChr == L'\n')
			{
				inStream.ignore ();
			}
		}
		if (ok)
		{
			ok = FromCardImage (myCardImage);
		}
	}
	return ok;
}
bool TcsBbRec86::GetCardImage (wchar_t* cardImage) const
{
	bool ok (true);
	unsigned long mySeqNbr;
	wchar_t myCardImage [82];

	if (SequenceNbr != 0UL)
	{
		// A sequence number has been assigned, therefore we use it as is.
		mySeqNbr = SequenceNbr;
	}
	else
	{
		// A sequence number has not been assigned, so we assign one for
		// the user.
		mySeqNbr = TcsBbRec80::NextSequenceNumber;
		TcsBbRec80::NextSequenceNumber += 10;
	}

	swprintf (myCardImage,82,L"%06lu"			// Sequence Number               1- 6
							 L"*86*"			// data code	                 7-10
							 L"%04lu"			// Station Serial Number        11-14
							 L"  "				// REQUIRED BLANKs              15-16 
							 L"% 07ld"			// Orthometric Height (+MMMmmm) 17-23
							 L"%c"				// Orthometric Height Code      24
							 L"%c%c"			// Orthometric Order & Class    25-26
							 L"%c"				// NGSIDB Flag                  27
							 L"%c%c"			// OrthometricHeight Datum      28-29
							 L"%-6.6s"			// Organization Code            30-35
							 L"% 07ld"			// Geoid Height                 36-42
							 L"%c"				// Geoid Height Code            43
							 L"  "				// REQUIRED BLANKs (2)          44-45
							 L"% 07ld"			// Ellipsoid Height             46-52
							 L"%c"				// ELlipsoid Height Code        53
							 L"%c%c"			// Ellipsoid Order & Class      54-55
							 L"%c"				// Ellipsoid Datum              56
							 L"%-24.24s",		// Comments                     67-80
							 mySeqNbr,			// Sequence Number
							 StationSerialNbr,	// Station Serial Number
							 static_cast<long>((OrthometricHgt * 1000.0) + 0.5),
							 OrthometricCode,
							 OrthometricOrderClass [0],
							 OrthometricOrderClass [1],
							 OrthometricNgsidb,
							 OrthometricDatum [0],
							 OrthometricDatum [1],
							 OrganizationCode,
							 static_cast<long>((GeoidHgt * 1000.0) + 0.5),
							 GeoidHeightCode,
							 static_cast<long>((EllipsoidalHgt * 1000.0) + 0.5),
							 EllipsoidCode,
							 EllipsoidOrderClass [0],
							 EllipsoidOrderClass [1],
							 EllipsoidDatum,
							 Comments);
	wcsncpy (cardImage,myCardImage,80);
	return ok;
}
bool TcsBbRec86::GetCardImage (char cardImage [80]) const
{
	bool ok;
	wchar_t wcCardImage [82];
	
	ok = GetCardImage (wcCardImage);
	wcstombs (cardImage,wcCardImage,80);
	return ok;
}
void TcsBbRec86::SetStationSerialNbr (unsigned long serialNbr)
{
	if (serialNbr > 9999UL)
	{
		serialNbr = 9999UL;
	}
	StationSerialNbr = serialNbr;
}
void TcsBbRec86::SetOrthometricDatum (const wchar_t* orthoDatum)
{
	wcsncpy (OrthometricDatum,orthoDatum,wcCount (OrthometricDatum));
	OrthometricDatum [wcCount (OrthometricDatum) - 1] = L'\0';
}
void TcsBbRec86::SetOrganizationCode (const wchar_t* orthoOrg)
{
	wcsncpy (OrganizationCode,orthoOrg,wcCount (OrganizationCode));
	OrganizationCode [wcCount (OrganizationCode) - 1] = L'\0';
}
void TcsBbRec86::SetComments (const wchar_t* orthoComment)
{
	wcsncpy (Comments,orthoComment,24);
	Comments [24] = L'\0';
}
bool TcsBbRec86::WriteToStream (std::wostream& outStream,bool newLines) const
{
	bool ok;
	wchar_t cardImage [82];

	ok = GetCardImage (cardImage);
	cardImage [80] = L'\0';
	outStream << cardImage;
	if (newLines)
	{
		outStream << std::endl;
	}
	return ok;
}
bool TcsBbRec86::FromCardImage (const wchar_t* cardImage)
{
	bool ok (false);
	long wrkLong;
	wchar_t wrkBufr [82];

	ok = wcslen (cardImage) >= 80 && 
		 (cardImage [7] == L'8') && (cardImage [8] == L'6');
	if (ok)
	{
		wcsncpy (wrkBufr,cardImage,80);
		wrkBufr [80] = L'\0';
		wcsncpy (Comments,&wrkBufr [56],25);
		EllipsoidDatum = wrkBufr [55];
		EllipsoidOrderClass [1] = wrkBufr [54];
		EllipsoidOrderClass [0] = wrkBufr [53];
		EllipsoidCode = wrkBufr [52];
		wrkBufr [52] = L'\0';
		wrkLong = wcstol (&wrkBufr[45],0,10);
		EllipsoidalHgt = static_cast<double>(wrkLong) / 1000.0;

		GeoidHeightCode = wrkBufr [42];
		wrkBufr [42] = L'\0';
		wrkLong = wcstol (&wrkBufr[35],0,10);
		GeoidHgt = static_cast<double>(wrkLong) / 1000.0;

		wrkBufr [28] = L'\0';
		wcsncpy (OrganizationCode,&wrkBufr [29],7);
		OrganizationCode [6] = L'\0';
		CS_trimWc (OrganizationCode);

		OrthometricDatum [0] = wrkBufr [28];
		OrthometricDatum [1] = wrkBufr [27];
		OrthometricNgsidb = wrkBufr [26];
		OrthometricOrderClass [0] = wrkBufr[24];
		OrthometricOrderClass [1] = wrkBufr[25];
		OrthometricCode = wrkBufr [23];
		wrkBufr [23] = L'\0';
		wrkLong = wcstol (&wrkBufr[16],0,10);
		OrthometricHgt = static_cast<double>(wrkLong) / 1000.0;

		wrkBufr [14] = L'\0';
		StationSerialNbr = wcstoul (&wrkBufr [10],0,10);
		if (StationSerialNbr > 9999UL)
		{
			StationSerialNbr = 9999UL;
		}
		wrkBufr [6] = L'\0';
		SequenceNbr = wcstoul (wrkBufr,0,10);
	}
	return ok;
}
bool TcsBbRec86::FromCardImage (const char* cardImage)
{
	bool ok;
	wchar_t wrkBufr [82];
	
	mbstowcs (wrkBufr,cardImage,wcCount (wrkBufr));
	ok = FromCardImage (wrkBufr);
	return ok;
}
std::wistream& operator>> (std::wistream& inStream,TcsBbRec86& heightRecord)
{
	wchar_t nextChr;
	wchar_t cardImage [82];

	// I am expecting this to read a line or the 80 characters.
	// Currently, we expect 80 characters.  We need to adjust so
	// that less than 80 characters will be acceptable.
	inStream.getline (cardImage,81);
	nextChr = inStream.peek ();
	if (nextChr == L'\n')
	{
		inStream.ignore ();
	}
	heightRecord.FromCardImage (cardImage);

	return inStream;
}
std::wostream& operator<< (std::wostream& outStream,const TcsBbRec86& heightRecord)
{
	heightRecord.WriteToStream (outStream,false);
	return outStream;
}
//=============================================================================
// TcsBbRec80 --  Implementation
unsigned long TcsBbRec80::NextSequenceNumber = 10UL;
unsigned long TcsBbRec80::NextStationNumber = 1UL;
double TcsBbRec80::ParseLatLongStr (const wchar_t* llStr,bool longitude)
{
	unsigned long degrees;
	unsigned long minutes;
	double seconds;
	double result;
	wchar_t wrkBufr [16];

	if (longitude)
	{
		wrkBufr [0] = *llStr++;
		wrkBufr [1] = *llStr++;
		wrkBufr [2] = *llStr++;
		wrkBufr [3] =  L'\0';
	}
	else
	{
		wrkBufr [0] = *llStr++;
		wrkBufr [1] = *llStr++;
		wrkBufr [2] =  L'\0';
	}
	degrees = wcstol (wrkBufr,0,10);
	wrkBufr [0] = *llStr++;
	wrkBufr [1] = *llStr++;
	wrkBufr [2] =  L'\0';
	minutes = wcstol (wrkBufr,0,10);
	wrkBufr [0] = *llStr++;
	wrkBufr [1] = *llStr++;
	wrkBufr [2] =  L'.';
	wrkBufr [3] = *llStr++;
	wrkBufr [4] = *llStr++;
	wrkBufr [5] = *llStr++;
	wrkBufr [6] = *llStr++;
	wrkBufr [7] = *llStr++;
	wrkBufr [8] = L'\0';
	seconds = wcstod (wrkBufr,0);

	result = static_cast<double>((degrees * 60 + minutes) * 60);
	result += seconds;
	result /= 3600;
	if (longitude)
	{
		if (*llStr == L'W' || *llStr == L'w')
		{
			result = -result;
		}
	}
	else
	{
		if (*llStr == L'S' || *llStr == L's')
		{
			result = -result;
		}
	}
	return result;
}
double TcsBbRec80::ParseLatLongStr (const char* llStr,bool longitude)
{
	unsigned long degrees;
	unsigned long minutes;
	double result;
	double seconds;
	char wrkBufr [16];

	if (longitude)
	{
		wrkBufr [0] = *llStr++;
		wrkBufr [1] = *llStr++;
		wrkBufr [2] = *llStr++;
		wrkBufr [3] =  L'\0';
	}
	else
	{
		wrkBufr [0] = *llStr++;
		wrkBufr [1] = *llStr++;
		wrkBufr [2] =  L'\0';
	}
	degrees = strtol (wrkBufr,0,10);
	wrkBufr [0] = *llStr++;
	wrkBufr [1] = *llStr++;
	wrkBufr [2] =  L'\0';
	minutes = strtol (wrkBufr,0,10);
	wrkBufr [0] = *llStr++;
	wrkBufr [1] = *llStr++;
	wrkBufr [2] =  L'.';
	wrkBufr [3] = *llStr++;
	wrkBufr [4] = *llStr++;
	wrkBufr [5] = *llStr++;
	wrkBufr [6] = *llStr++;
	wrkBufr [7] = *llStr++;
	wrkBufr [8] = L'\0';
	seconds = strtod (wrkBufr,0);

	result = static_cast<double>((degrees * 60 + minutes) * 60);
	result += seconds;
	result /= 3600;
	if (longitude)
	{
		if (*llStr == 'W' || *llStr == 'w')
		{
			result = -result;
		}
	}
	else
	{
		if (*llStr == 'S' || *llStr == 's')
		{
			result = -result;
		}
	}
	return result;
}
bool TcsBbRec80::FormatLatLongStr (wchar_t* llStr,size_t strSize,double latLong,bool longitude)
{
	bool ok;
	bool negative (false);
	long degrees;
	long minutes;
	long seconds;
	long fraction;
	wchar_t direction;

	negative = (latLong < 0.0);
	if (negative)
	{
		latLong = -latLong;
		direction = longitude ? L'W' : L'S';
		
	}
	else
	{
		direction = longitude ? L'E' : L'N';
	}

	latLong *= 3600.0;
	seconds = static_cast<long>(latLong);
	latLong -= static_cast<double>(seconds);
	fraction = static_cast<long>((latLong * 100000.0) + 0.5);
	if (fraction >= 100000L)
	{
		fraction = 0L;
		seconds += 1L;
	}
	minutes = seconds / 60L;
	seconds = seconds % 60L;
	degrees = minutes / 60L;
	minutes = minutes % 60L;

	if (longitude)
	{
		ok = (degrees < 180 && minutes < 60 && seconds < 60 && fraction < 100000);
		swprintf (llStr,strSize,L"%03ld%02ld%02ld%05ld%c",degrees,minutes,seconds,fraction,direction);
	}
	else
	{
		ok = (degrees < 90 && minutes < 60 && seconds < 60 && fraction < 100000);
		swprintf (llStr,strSize,L"%02ld%02ld%02ld%05ld%c",degrees,minutes,seconds,fraction,direction);
	}
	return ok;
}
TcsBbRec80::TcsBbRec80 (void) : SequenceNbr  (0UL),
								Latitude     (0.0),
								Longitude    (0.0),
								StationSerialNbr   (TcsBbRec80::NextStationNumber),
								Rec86        ()
{
	StationName         [0] = L'\0';
	StateCountryCode    [0] = L'U';
	StateCountryCode    [1] = L'S';
	StateCountryCode    [2] = L'\0';
	StationOrderAndType [0] = L' ';
	StationOrderAndType [1] = L' ';

	// Force Station Serial Numbers of the two records to be the same.
	// Somewhat redundant, perhaps.
	Rec86.SetStationSerialNbr (StationSerialNbr);
}
TcsBbRec80::TcsBbRec80 (double latitude,double longitude,double ellipsoidHgt)
														 : SequenceNbr        (0UL),
														   Latitude           (latitude),
														   Longitude          (longitude),
														   StationSerialNbr   (TcsBbRec80::NextStationNumber),
														   Rec86              (ellipsoidHgt)
{
	StationName         [0] = L'\0';
	StateCountryCode    [0] = L'U';
	StateCountryCode    [1] = L'S';
	StateCountryCode    [2] = L'\0';
	StationOrderAndType [0] = L'4';
	StationOrderAndType [1] = L'8';
	
	// Force Station Serial Numbers of the two records to be the same.
	// Somewhat redundant, perhaps.
	Rec86.SetStationSerialNbr (StationSerialNbr);
}
TcsBbRec80::TcsBbRec80 (double latitude,double longitude,TcsBbRec86& elevation)
														 : SequenceNbr  (0UL),
														   Latitude     (latitude),
														   Longitude    (longitude),
														   StationSerialNbr   (TcsBbRec80::NextStationNumber),
														   Rec86        (elevation)
{
	StationName         [0] = L'\0';
	StateCountryCode    [0] = L'U';
	StateCountryCode    [1] = L'S';
	StateCountryCode    [2] = L'\0';
	StationOrderAndType [0] = L'4';
	StationOrderAndType [1] = L'8';

	// Force Station Serial Numbers of the two records to be the same.
	// Somewhat redundant, perhaps.
	Rec86.SetStationSerialNbr (StationSerialNbr);
}
TcsBbRec80::TcsBbRec80 (const wchar_t* latitude,const wchar_t* longitude,const wchar_t* ellipsoidHgt)
																		 : SequenceNbr        (0UL),
																		   StationSerialNbr   (TcsBbRec80::NextStationNumber),
																		   Rec86              ()
{
	long wrkLong;
	double ellipHgt;

	Latitude = ParseLatLongStr (latitude,false);
	Longitude = ParseLatLongStr (longitude,true);
	StationName         [0] = L'\0';
	StateCountryCode    [0] = L'U';
	StateCountryCode    [1] = L'S';
	StateCountryCode    [2] = L'\0';
	StationOrderAndType [0] = L'4';
	StationOrderAndType [1] = L'8';

	wrkLong = wcstol (ellipsoidHgt,0,10);
	ellipHgt = static_cast<double>(wrkLong) / 1000.0;
	Rec86.SetEllipsoidalHeight (ellipHgt);

	// Force Station Serial Numbers of the two records to be the same.
	// Somewhat redundant, perhaps.
	Rec86.SetStationSerialNbr (StationSerialNbr);
}
TcsBbRec80::TcsBbRec80 (const char* latitude,const char* longitude,const char*ellipsoidHgt)
																   : SequenceNbr        (0UL),
																	 StationSerialNbr   (TcsBbRec80::NextStationNumber),
																	 Rec86              ()
{
	long wrkLong;
	double ellipHgt;

	Latitude = ParseLatLongStr (latitude,false);
	Longitude = ParseLatLongStr (longitude,true);
	StationName         [0] = L'\0';
	StateCountryCode    [0] = L'U';
	StateCountryCode    [1] = L'S';
	StateCountryCode    [2] = L'\0';
	StationOrderAndType [0] = L'4';
	StationOrderAndType [1] = L'8';

	wrkLong = strtol (ellipsoidHgt,0,10);
	ellipHgt = static_cast<double>(wrkLong) / 1000.0;
	Rec86.SetEllipsoidalHeight (ellipHgt);

	// Force Station Serial Numbers of the two records to be the same.
	// Somewhat redundant, perhaps.
	Rec86.SetStationSerialNbr (StationSerialNbr);
}
TcsBbRec80::TcsBbRec80 (const TcsBbRec80& source) : SequenceNbr      (source.SequenceNbr),
													Latitude         (source.Latitude),
													Longitude        (source.Longitude),
													StationSerialNbr (source.StationSerialNbr),
													Rec86            (source.Rec86)
{
	wcsncpy (StationName,source.StationName,wcCount (StationName));
	StationName [30] = L'\0';
	wcsncpy (StateCountryCode,source.StateCountryCode,wcCount (StateCountryCode));
	StateCountryCode [2] = L'\0';
	StationOrderAndType [0] = source.StationOrderAndType [0];
	StationOrderAndType [1] = source.StationOrderAndType [1];

	// Force Station Serial Numbers of the two records to be the same.
	// Somewhat redundant, perhaps.
	Rec86.SetStationSerialNbr (StationSerialNbr);
}
TcsBbRec80& TcsBbRec80::operator= (const TcsBbRec80& rhs)
{
	if (this != &rhs)
	{
		SequenceNbr             = rhs.SequenceNbr;
		Latitude                = rhs.Latitude;
		Longitude               = rhs.Longitude;
		Rec86                   = rhs.Rec86;
		StationSerialNbr        = rhs. StationSerialNbr;
		StationOrderAndType [0] = rhs.StationOrderAndType [0];
		StationOrderAndType [1] = rhs.StationOrderAndType [1];

		wcsncpy (StationName,rhs.StationName,wcCount (StationName));
		StationName [30] = L'\0';
		wcsncpy (StateCountryCode,rhs.StateCountryCode,wcCount (StateCountryCode));
		StateCountryCode [2] = L'\0';

		// Force Station Serial Numbers of the two records to be the same.
		// Somewhat redundant, perhaps.
		Rec86.SetStationSerialNbr (StationSerialNbr);
	}
	return *this;
}
TcsBbRec80::~TcsBbRec80 (void)
{
	// Nothing to do here.
}
bool TcsBbRec80::ReadFromStream (std::wistream& inStream)
{
	bool ok (false);
	wchar_t myCardImage [82];

	if (!inStream.eof ())
	{
		inStream.read (myCardImage,80);
		myCardImage [80] = L'\0';
		ok = inStream.good ();
		if (ok)
		{
			// We see if there is a new line waiting, if so we ignore it.
			// This is our attempt at accepting text files with new-lines or
			// simple 80 character record files.
			wchar_t nextChr = inStream.peek ();
			if (nextChr == L'\n')
			{
				inStream.ignore ();
			}
		}
		if (ok)
		{
			// This takes care of the *80* record.
			ok = FromCardImage (myCardImage);
			if (ok)
			{
				// That's been successful, now we deal with the associated
				// *86* record.  There always needs to be one.
				ok = Rec86.ReadFromStream (inStream);
			}
		}
	}
	return ok;
}
double TcsBbRec80::GetOrthometricHeight (void) const
{
	return Rec86.GetOrthometricHeight ();
}
double TcsBbRec80::GetGeoidHeight (void) const
{
	return Rec86.GetGeoidHeight ();
}
double TcsBbRec80::GetEllipsoidalHeight (void) const
{
	return Rec86.GetEllipsoidalHeight ();
}
bool TcsBbRec80::GetCardImage (wchar_t* cardImage) const
{
	bool ok (true);
	unsigned long mySeqNbr;
	wchar_t wcLatitudeStr [16];
	wchar_t wcLongitudeStr [16];
	wchar_t myCardImage [82];

	if (SequenceNbr != 0UL)
	{
		// A sequence number has been assigned, therefore we use it as is.
		mySeqNbr = SequenceNbr;
	}
	else
	{
		// A sequence number has not been assigned, so we assign one for
		// the user.
		mySeqNbr = TcsBbRec80::NextSequenceNumber;
		TcsBbRec80::NextSequenceNumber += 10;
	}

	ok  = FormatLatLongStr (wcLatitudeStr,wcCount (wcLatitudeStr),Latitude,false);
	ok &= FormatLatLongStr (wcLongitudeStr,wcCount (wcLongitudeStr),Longitude,true);
	if (ok)
	{
		swprintf (myCardImage,82,L"%06lu"			// Sequence Number               1- 6
								 L"*80*"			// data code	                 7-10
								 L"%04lu"			// Station Serial Number        11-14
								 L"%-30.30s"		// Station Name                 15-44
								 L"%-12.12s"		// Latitude + Dir               45-56
								 L"%-13.13s"		// Longitude + Dir              57-68
								 L"      "			// REQUIRED BLANKs (6)          70-75
								 L" "				// REQUIRED BLANK (1)           76
								 L"%-2.2s"			// State & Country Code         77-78
								 L"%c%c",			// Station Order & Class        79-80
								 mySeqNbr,
								 StationSerialNbr,
								 StationName,
								 wcLatitudeStr,
								 wcLongitudeStr,
								 StateCountryCode,
								 StationOrderAndType [0],
								 StationOrderAndType [1]);
		wcsncpy (cardImage,myCardImage,80);
	}
	else
	{
		cardImage [0] = L'\0';
	}
	return ok;
}
bool TcsBbRec80::GetCardImage (char* cardImage) const
{
	bool ok;
	wchar_t wcCardImage [82];
	
	ok = GetCardImage (wcCardImage);
	wcstombs (cardImage,wcCardImage,80);
	return ok;
}
void TcsBbRec80::SetStationSerialNumber (unsigned long serialNbr)
{
	if (serialNbr > 9999UL)
	{
		serialNbr = 9999UL;
	}
	StationSerialNbr = serialNbr;
}
void TcsBbRec80::SetStationName (const wchar_t* stationName)
{
	wcsncpy (StationName,stationName,wcCount (StationName));
	StationName [30] = L'\0';
}
void TcsBbRec80::SetStateCountryCode (const wchar_t* stateCountryCode)
{
	wcsncpy (StateCountryCode,stateCountryCode,wcCount (StateCountryCode));
	StateCountryCode [2] = L'\0';
}
bool TcsBbRec80::WriteToStream (std::wostream& outStream,bool newLines) const
{
	bool ok;
	size_t wrCount;
	wchar_t cardImage [82];

	ok = GetCardImage (cardImage);
	cardImage [80] = L'\0';
	wrCount = wcslen (cardImage);
	outStream.write (cardImage,wrCount);
	if (newLines)
	{
		outStream.put (L'\n');
	}

	// Now deal with the *86* record which is associated with this horizontal
	// data record.
	Rec86.WriteToStream (outStream,newLines);
	return ok;
}
bool TcsBbRec80::FromCardImage (const wchar_t* cardImage)
{
	bool ok (false);
	unsigned long wrkUlong;
	wchar_t wrkBufr [82];

	ok = wcslen (cardImage) >= 80 && 
		 (cardImage [7] == L'8') && (cardImage [8] == L'0');
	if (ok)
	{
		wcsncpy (wrkBufr,cardImage,80);
		wrkBufr [80] = L'\0';
		StationOrderAndType [1] = wrkBufr [79];
		StationOrderAndType [0] = wrkBufr [78];
		wrkBufr [78] = L'\0';
		wcsncpy (StateCountryCode,&wrkBufr [76],3);
		StateCountryCode [2] = L'\0';
		wrkBufr [69] = L'\0';
		Longitude = ParseLatLongStr (&wrkBufr[56],true);
		wrkBufr [56] = L'\0';
		Latitude = ParseLatLongStr (&wrkBufr[44],false);
		wrkBufr [44] = L'\0';
		wcsncpy (StationName,&wrkBufr [14],30);
		StationName [30] = L'\0';
		CS_trimWc (StationName);
		wrkBufr [14] = L'\0';
		wrkUlong = wcstoul (&wrkBufr [10],0,10);
		if (wrkUlong > 9999UL)		// redundant???
		{
			wrkUlong = 9999UL;
		}
		StationSerialNbr = wrkUlong;
		wrkBufr [6] = L'\0';
		SequenceNbr = wcstoul (wrkBufr,0,10);
	}
	return ok;
}
bool TcsBbRec80::FromCardImage (const char* cardImage)
{
	bool ok;
	wchar_t wrkBufr [82];
	
	mbstowcs (wrkBufr,cardImage,wcCount (wrkBufr));
	ok = FromCardImage (wrkBufr);
	return ok;
}
std::wistream& operator>> (std::wistream& inStream,TcsBbRec80& horzObsCard)
{
	wchar_t nextChr;
	wchar_t cardImage [82];

	// I am expecting this to read a line or the 80 characters.
	// Currently, we expect 80 characters.  We need to adjust so
	// that less than 80 characters will be acceptable.
	inStream.getline (cardImage,81);
	nextChr = inStream.peek ();
	if (nextChr == L'\n')
	{
		inStream.ignore ();
	}
	horzObsCard.FromCardImage (cardImage);
	return inStream;
}
std::wostream& operator<< (std::wostream& outStream,const TcsBbRec80& horzObsCard)
{
	horzObsCard.WriteToStream (outStream,false);
	return outStream;
}
