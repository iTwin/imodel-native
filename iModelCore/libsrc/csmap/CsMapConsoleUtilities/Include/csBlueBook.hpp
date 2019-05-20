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

//=============================================================================
// NGS Blue Book Horizontal Observation
//
// Represents a Horizontal Observation in internal format.
//

enum EcsBbRecordType {	bbTypNone  = 0,
						bbTypHorzObs,
						bbTypUnknown = 999
					 };

class TcsBbRec86
{
public:
	///////////////////////////////////////////////////////////////////////////
	// Construction  /  Destruction  /  Assignment
	TcsBbRec86 (void);
	TcsBbRec86 (double ellipHeight,double orthoHeight = 0.0);
	TcsBbRec86 (wchar_t* cardImage);		// expects 80 characters min
	TcsBbRec86 (char* cardImage);			// expects 80 characters min
	TcsBbRec86 (const TcsBbRec86& source);
	TcsBbRec86& operator= (const TcsBbRec86& rhs);
	~TcsBbRec86 (void);
	///////////////////////////////////////////////////////////////////////////
	// Operator Overrides
	///////////////////////////////////////////////////////////////////////////
	// Public Named Member Functions
	bool ReadFromStream (std::wistream& instream);
	unsigned long GetSequenceNbr (void) const {return SequenceNbr; };
	double GetOrthometricHeight (void) const {return OrthometricHgt; };
	double GetGeoidHeight (void) const {return GeoidHgt; };
	double GetEllipsoidalHeight (void) const {return EllipsoidalHgt; };
	unsigned long GetStationSerialNbr (void) {return StationSerialNbr; };
	wchar_t GetOrthometricCode (void) const {return OrthometricCode; }; 
	wchar_t GetOrthometricOrder (void) const {return OrthometricOrderClass[0]; }; 
	wchar_t GetOrthometricClass (void) const {return OrthometricOrderClass[1]; }; 
	wchar_t GetNgsidbFlag (void) const {return OrthometricNgsidb; }; 
	const wchar_t* GetOrthometricDatum (void) const {return OrthometricDatum; }; 
	const wchar_t* GetOrganizationCode (void) const {return OrganizationCode; };
	wchar_t GetGeoidCode (void) const {return GeoidHeightCode; }; 
	wchar_t GetEllipsoidCode (void) const {return EllipsoidCode; }; 
	wchar_t GetEllipsoidOrder (void) const {return EllipsoidOrderClass[0]; }; 
	wchar_t GetEllipsoidClass (void) const {return EllipsoidOrderClass[1]; }; 
	wchar_t GetEllipsoidDatum (void) const {return EllipsoidDatum; };
	const wchar_t* GetComments (void) const {return Comments; };
	bool GetCardImage (wchar_t* cardImage) const;
	bool GetCardImage (char* cardImage) const;

	void SetSequenceNbr (unsigned long seqNbr) {SequenceNbr = seqNbr; };
	void SetOrthometricHeight (double orthoHgt) {OrthometricHgt = orthoHgt;};
	void SetGeoidHeight (double geoidHgt) {GeoidHgt = geoidHgt;};
	void SetEllipsoidalHeight (double ellipHgt) {EllipsoidalHgt = ellipHgt;};
	void SetStationSerialNbr (unsigned long serialNbr);
	void SetOrthometricCode (wchar_t orthoCode) {OrthometricCode = orthoCode; }; 
	void SetOrthometricOrder (wchar_t orthoOrder) {OrthometricOrderClass[0] = orthoOrder; }; 
	void SetOrthometricClass (wchar_t orthoClass) {OrthometricOrderClass[1] = orthoClass; }; 
	void SetNgsidbFlag (wchar_t ngsidbFlag) {OrthometricNgsidb = ngsidbFlag; }; 
	void SetOrthometricDatum (const wchar_t* orthoDatum); 
	void SetOrganizationCode (const wchar_t* orthoOrg); 
	void SetGeoidCode (wchar_t geoidCode) {GeoidHeightCode = geoidCode; }; 
	void SetEllipsoidCode (wchar_t ellipCode) {EllipsoidCode = ellipCode; }; 
	void SetEllipsoidOrder (wchar_t ellipOrder) {EllipsoidOrderClass[0] = ellipOrder; }; 
	void SetEllipsoidClass (wchar_t ellipClass) {EllipsoidOrderClass[1] = ellipClass; }; 
	void SetEllipsoidDatum (wchar_t ellipDatum) {EllipsoidDatum = ellipDatum; };
	void SetComments (const wchar_t* orthoComment); 
	bool WriteToStream (std::wostream& outStream,bool newLines = false) const;
	bool FromCardImage (const wchar_t* cardImage);
	bool FromCardImage (const char* cardImage);
protected:
private:
	///////////////////////////////////////////////////////////////////////////
	// Private Data Members
	unsigned long SequenceNbr;			// Not used to much, set when read from stream
	double OrthometricHgt;				// Meters, positive means above geoid
	double GeoidHgt;					// Meters,positive means above ellipsoid
	double EllipsoidalHgt;				// Meters, positive means above ellipsoid
	unsigned long StationSerialNbr;		// Station Serial Number, 0001 - 9999.
	wchar_t OrthometricCode;			// Single character code
	wchar_t OrthometricOrderClass [2];	// Two character code, NOT NULL TERMINATED
	wchar_t OrthometricNgsidb;			// 'Y' or 'N'
	wchar_t OrthometricDatum [2];		// Two character code, NOT NULL TERMINATED
	wchar_t OrganizationCode [8];		// Orthometric Maintenance organiztion, six
										// character code, null terminated.
	wchar_t GeoidHeightCode;			// Single character code
	wchar_t EllipsoidCode;				// Single character code
	wchar_t EllipsoidOrderClass [2];	// Two character code, NOT NULL TERMINATED
	wchar_t EllipsoidDatum;				// Once character code
	wchar_t Comments [32];				// 24 character comment field, null terminated.
};
std::wistream& operator>> (std::wistream& inStream,TcsBbRec86& heightRecord);
std::wostream& operator<< (std::wostream& outStream,const TcsBbRec86& heightRecord);

// Blue Book Record 80 -- aka Horizontal Observation
class TcsBbRec80
{
public:
	static unsigned long NextSequenceNumber;
	static unsigned long NextStationNumber;
	static double ParseLatLongStr (const wchar_t* llStr,bool longitude);
	static double ParseLatLongStr (const char* llStr,bool longitude);
	static bool FormatLatLongStr (wchar_t* llStr,size_t strSize,double latLong,
																bool longitude);
	static void BumpStationNumber (void) {NextStationNumber += 1;};
	///////////////////////////////////////////////////////////////////////////
	// Construction  /  Destruction  /  Assignment
	TcsBbRec80 (void);
	TcsBbRec80 (double latitude,double longitude,double ellipsoidHeight);
	TcsBbRec80 (double latitude,double longitude,TcsBbRec86& elevation);
	TcsBbRec80 (const wchar_t* latitude,const wchar_t* longitude,const wchar_t*ellipsoidHgt);
	TcsBbRec80 (const char* latitude,const char* longitude,const char*ellipsoidHgt);
	TcsBbRec80 (const TcsBbRec80& source);
	TcsBbRec80& operator= (const TcsBbRec80& rhs);
	~TcsBbRec80 (void);
	///////////////////////////////////////////////////////////////////////////
	// Operator Overrides
	///////////////////////////////////////////////////////////////////////////
	// Public Named Member Functions
	bool ReadFromStream (std::wistream& instream);
	unsigned long GetSequenceNbr (void) const {return SequenceNbr; };
	double GetLatitude (void) const {return Latitude; };
	double GetLongitude (void) const {return Longitude; };
	unsigned long GetStationSerialNumber (void) const {return StationSerialNbr; };
	const wchar_t* GetStationName (void) const {return StationName; };
	const wchar_t* GetStateCountryCode (void) const {return StateCountryCode; };
	wchar_t GetStationOrder (void) const {return StationOrderAndType [0]; };
	wchar_t GetStationType (void) const {return StationOrderAndType [1]; };
	const TcsBbRec86& GetHeightRecord (void) const {return Rec86; };
	double GetOrthometricHeight (void) const;
	double GetGeoidHeight (void) const;
	double GetEllipsoidalHeight (void) const;
	bool GetCardImage (wchar_t* cardImage) const;
	bool GetCardImage (char* cardImage) const;

	void SetSequenceNbr (unsigned long seqNbr) {SequenceNbr = seqNbr; };
	void SetLatitude (double latitude) {Latitude = latitude; };
	void SetLongitude (double longitude) {Longitude = longitude; };
	void SetStationSerialNumber (unsigned long serialNbr);
	void SetStationName (const wchar_t* stationName);
	void SetStateCountryCode (const wchar_t* stateCountryCode);
	void SetStationOrder (wchar_t order) {StationOrderAndType [0] = order; };
	void SetStationType (wchar_t type) {StationOrderAndType [0] = type; };
	void SetHeightRecord (const TcsBbRec86& rec86) {Rec86 = rec86; };
	void SetOrthometricHeight (double orthoHgt) {Rec86.SetOrthometricHeight (orthoHgt); };
	void SetGeoidHeight (double geoidHgt) {Rec86.SetGeoidHeight (geoidHgt); };
	void SetEllipsoidalHeight (double ellipHgt) {Rec86.SetEllipsoidalHeight (ellipHgt); };
	bool WriteToStream (std::wostream& outStream,bool newLines = false) const;
	bool FromCardImage (const wchar_t* cardImage);
	bool FromCardImage (const char* cardImage);
protected:
private:
	///////////////////////////////////////////////////////////////////////////
	// Private Data Members
	unsigned long SequenceNbr;			// Present only if read from a stream
	double Latitude;					// Degrees, negative means South
	double Longitude;					// Degrees, negative means West
	unsigned long StationSerialNbr;		// Staion Serial number, 0001 - 9999.
	wchar_t StationName [32];			// 30 characters max, carried in null terminated form.
	wchar_t StateCountryCode [4];		// Two character code, carried in null terminated form.
	wchar_t StationOrderAndType [2];	// Two character code, NOT NULL TERMINATED
	TcsBbRec86 Rec86;					// Carries the height/elevation information concerning
										// this point.
};
std::wistream& operator>> (std::wistream& inStream,TcsBbRec80& horzObsCard);
std::wostream& operator<< (std::wostream& outStream,const TcsBbRec80& horzObsCard);
