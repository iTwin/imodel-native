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

struct TcsCrsRange
{
	TcsCrsRange (void);
	TcsCrsRange (double swLng,double swLat,double neLng,double neLat);
	TcsCrsRange (const double southwest [2],const double northeast [2]);
	TcsCrsRange (const TcsCrsRange& source);
	TcsCrsRange& operator= (const TcsCrsRange& rhs);
	TcsCrsRange& operator+= (const TcsCrsRange& rhs);
	short LngRange (void) const;
	short LatRange (void) const;
public:				// Redundant, for documentation purposes
	double RangeSW [2];
	double RangeNE [2];
};
std::wostream& operator<< (std::wostream& oStrm,const TcsCrsRange& subject);

class TcsUsefulRngRpt
{
public:
	// Construction / Destruction / Assignment
	TcsUsefulRngRpt (void);
	TcsUsefulRngRpt (const char* csKeyName);
	TcsUsefulRngRpt (const TcsUsefulRngRpt& source);
	virtual ~TcsUsefulRngRpt (void);
	TcsUsefulRngRpt& operator= (const TcsUsefulRngRpt& rhs);
	// Operator Overrides / Virtual Functions
	// This is the sort comparison function.  The code for this function will
	// be adjusted to produce the various reports.
	bool operator< (TcsUsefulRngRpt& comparedTo);
	// Public Named Member Functions
	bool IsValid (void) const {return Valid; }
	void SetKeyName (const char* csKeyName);
	void SetPrjName (const char* prjName);
	void SetAreaName (const std::wstring& areaName);
	short SetCrsPrjCode (short csPrjCode);
	long SetRangeEvaluation (long rangeEvaluation);
	TcsEpsgCode SetCrsEpsgCode (TcsEpsgCode crsEpsgCode);
	TcsEpsgCode SetAreaEpsgCode (TcsEpsgCode areaEpsgCode);
	void SetCsMapLimits (const TcsCrsRange& csRange);
	void SetEpsgLimits (const TcsCrsRange& csRange);

	short CsMapLngRange (void) const;
	short CsMapLatRange (void) const;
	short EpsgLngRange (void) const;
	short EpsgLatRange (void) const;
	void WriteToStream (std::wostream& oStrm,bool wrtHeader = false);
	bool Validate (void);
protected:
	// Protected Named Member Functions
	// Protected Data Members
	bool Valid;
	wchar_t CsKeyName [24];
	wchar_t CsPrjName [64];
	wchar_t AreaName [84];
	short CsPrjCode;
	long RngEvaluation;
	TcsEpsgCode CrsEpsgCode;
	TcsEpsgCode AreaEpsgCode;
	TcsCrsRange CsMapRange;
	TcsCrsRange EpsgRange;
private:
	// Private Named Member Functions
	// Private Data Members
};
