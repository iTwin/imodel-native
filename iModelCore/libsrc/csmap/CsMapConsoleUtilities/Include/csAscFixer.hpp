/******************************************************************************

	*********************************************************************
	*********************************************************************
	**                                                                 **
	**          Copyright (c) 1997 Mentor Software, Inc.               **
	**                    All Rights Reserved                          **
	**                                                                 **
	** The software and information contained herein are proprietary   **
	** to, and comprise valuable trade secrets of, Mentor Software,    **
	** Inc., which intends to preserve as trade secrets such software  **
	** and information.  This software is furnished pursuant to a      **
	** written license agreement and may be used, copied, transmitted, **
	** and stored only in accordance with the terms of such license    **
	** and with the inclusion of the above copyright notice.  This     **
	** software and information or any other copies thereof may not    **
	** be provided or otherwise made available to any other person.    **
	** Failure to honor the terms of the written license agreement     **
	** could be cause for legal action against you and/or your         **
	** company.                                                        **
	**                                                                 **
	** Notwithstanding any other lease or license that may pertain to, **
	** or accompany the delivery of, this computer software and        **
	** information, the rights of the Government regarding its use,    **
	** reproduction and disclosure are as set forth in Section         **
	** 52.227-19 of the FARS Computer Software-Restricted Rights       **
	** clause.                                                         **
	**                                                                 **
	** Use, duplication, or disclosure by the Government is subject    **
	** to restrictions as set forth in subparagraph (c)(1)(ii) of the  **
	** Rights in Technical Data and Computer Software clause at DFARS  **
	** 252.227-7013. Contractor/Manufacturer is Mentor Software, Inc., **
	** 3907 East 120th Avenue, Suite 200, Thornton, CO  80233.         **
	**                                                                 **
	*********************************************************************  
	*********************************************************************  

			 File Name: $RCSfile$
		   Description:
			   Purpose:

		Revision Level:	$Revision$
		 Check In Date:	$Date$

******************************************************************************/

enum EcsAscLineType {	ascTypNone     = 0,
						ascTypDefName  = 1,
						ascTypLblVal   = 2,
						ascTypComment  = 3,
						ascTypBlank    = 4,
						ascTypUnknown  = 99
					};

enum EcsDictType {	dictTypNone       = 0,
					dictTypCoordsys   = 1,
					dictTypDatum      = 2,
					dictTypEllipsoid  = 4,
					dictTypMreg       = 5,
					dictTypXform      = 6,
					dictTypPath       = 7,
					dictTypUnknown    = 1024
				 };

///////////////////////////////////////////////////////////////////////////////
// TcsDefLine represents a line in a .asc file.  This is maintained in an
// editable form, yet a form which can exactly reproduce the original in all
// ways other than the actual edit.
class TcsDefLine
{
	///////////////////////////////////////////////////////////////////////////
	// Static Constants, Variables, and Member Functions
	static void Pad (char* array,int padCnt,unsigned arraySize);
public:
	static const double InvalidDouble;
	///////////////////////////////////////////////////////////////////////////
	// Construction,  Destruction,  Assignment
	TcsDefLine (unsigned lineNbr,const char* lineText,EcsDictType dictType);
	TcsDefLine (EcsDictType dictType,const char* label,const char* value,
													   const char* comment);
	TcsDefLine (const TcsDefLine& source);
	~TcsDefLine (void);
	TcsDefLine& operator= (const TcsDefLine& rhs);
	///////////////////////////////////////////////////////////////////////////
	// Operator Overloads
	bool operator== (const char* label) const;
	///////////////////////////////////////////////////////////////////////////
	// Public Named Member Functions
	bool IsNameDef (void) const;
	unsigned GetLineNbr (void) const;
	unsigned GetInsertNbr (void) const;
	EcsAscLineType GetType (void) const;
	const char* GetLabel (void) const;
	const char* GetValue (void) const;
	double GetValueAsDouble (long32_t& format) const;
	const char* GetComment (void) const;
	const char* GetLeadWs (void) const;
	const char* GetSepWs (void) const;
	const char* GetCmntWs (void) const;

	unsigned GetLineNbr (void);
	unsigned GetInsertNbr (void);
	EcsAscLineType GetType (void);
	char* GetLabel (void);
	char* GetValue (void);
	char* GetComment (void);
	char* GetLeadWs (void);
	char* GetSepWs (void);
	char* GetCmntWs (void);
	
	bool Matches (const TcsDefLine& thisOne,bool fileScope = false) const;
	bool WriteToStream (std::ostream& outStrm) const;
	void SetLineNbr (unsigned newLineNbr);
	void SetInsertNbr (unsigned newLineNbr);
	void SetType (EcsAscLineType newType);
	void SetLabel (const char* newLabel);
	void SetValue (const char* newValue);
	void SetComment (const char* newComment);
	///////////////////////////////////////////////////////////////////////////
private:
	///////////////////////////////////////////////////////////////////////////
	// Private Support Functions
	void Reset (void);
	EcsAscLineType ParseTextLine (const char* textLine);
	///////////////////////////////////////////////////////////////////////////
	// Private Data Members
	EcsDictType DictType;		// for convenience of edit functions.
	EcsAscLineType Type;
	unsigned LineNbr;			// Line nbr in the original asc file.
	unsigned InsertNbr;			// used to uniquely identify lines after insertion.
	char LeadWs [16];
	char Label [32];
	char SepWs [16];
	char Value [96];
	char CmntWs [64];
	char Comment [256];
};
// A vector of TcsDefLine is used to carry complete definitions and, perhaps,
// a complete file.
typedef std::vector<TcsDefLine> TcsDefLineVctr;
typedef TcsDefLineVctr::iterator TcsDefLnItr;
typedef TcsDefLineVctr::const_iterator TcsDefLnItrK;

//newPage//
///////////////////////////////////////////////////////////////////////////////
// TcsAscDefinition -- A named collection of TcsDefLine objects which
//                     represents in its entirity a single definition.
//
// Comparison operators are provided which compare the name (case insensitive)
// only.  Thus, for example, a vector of this objects is suitable for sorting
// and binary searching.
//
class TcsAscDefinition
{
public:
	static const double InvalidDouble;
	///////////////////////////////////////////////////////////////////////////
	// Construction,  Destruction,  Assignment  
	TcsAscDefinition (EcsDictType type = dictTypNone);
	TcsAscDefinition (EcsDictType type,TcsDefLnItrK begin,TcsDefLnItrK end);
	TcsAscDefinition (const TcsDefLine& firstLine);
	TcsAscDefinition (EcsDictType type,unsigned& lineNbr,std::istream& inStrm);	// unStrm must be seek'able
	TcsAscDefinition (const TcsAscDefinition& source);
	~TcsAscDefinition (void);
	TcsAscDefinition& operator= (const TcsAscDefinition& rhs);
	///////////////////////////////////////////////////////////////////////////
	// Operator Overloads
	// The operator< function is defined specifically for the std::set
	// collection which essentially works on definition names only.  Name
	// comparison is case insensitive.
	bool operator== (const TcsAscDefinition& rhs) const;
	bool operator<  (const TcsAscDefinition& rhs) const;
	bool operator== (const char* defName) const;
	bool operator<  (const char* defName) const;
	// Add operator is used to add lines to a definition object, essentially a
	// push_back on the vector.  The subtract operator will remove the
	// indicated line from the vector.  The argument must be, essentially,
	// an exact image of the line to be removed.
	TcsAscDefinition& operator+ (const TcsDefLine& newLine);
	TcsAscDefinition& operator- (const TcsDefLine& oldLine);
	///////////////////////////////////////////////////////////////////////////
	// Public Named Member Functions
	bool IsDefinition (void) const;
	bool ReadFromStream (unsigned& lineNbr,std::istream& inStrm);	// inStrm must be seek'able
	size_t GetLineCount (void) const {return Definition.size (); }
	const TcsDefLine* GetLine (size_t index) const;
	const TcsDefLine* GetLine (const char* label) const;
	TcsDefLine* GetLine (const char* label);
	const char* GetDefinitionName (void) const;
	bool RenameDef (const char* newName);
	const char* GetValue (const char* label) const;
	double GetValueAsDouble (const char* label,long32_t& format) const;
	const char* GetNextLabel (const char* labelName) const;
	bool SetValue (const char* label,const char* newValue);
	bool PrependLine (const TcsDefLine& newLine);
	bool InsertBefore (const char* label,const TcsDefLine& newLine);
	bool InsertAfter (const char* label,const TcsDefLine& newLine);
	bool Append (const TcsDefLine& newLine);
	bool RemoveLine (const char* label);
	bool WriteToStream (std::ostream& outStrm) const;
private:
	///////////////////////////////////////////////////////////////////////////
	// Private Support Functions
	///////////////////////////////////////////////////////////////////////////
	// Private Data Members
	EcsDictType Type;
	char DefName [64];
	TcsDefLineVctr Definition;
};
bool ValueSwap (TcsAscDefinition& first,TcsAscDefinition& second,const char* grpName);

//newPage//
///////////////////////////////////////////////////////////////////////////////
// TcsDefFile  --  Collection of lines extracted from a .asc file.
//
// This object represents a collection of lines from a .asc file provided to
// the constructor.  Each line is parsed to isolate the label from the
// associated value, and so on in such a way that upon output, the resulting
// text will exactly match the original line (unless changed of course).  This
// is done so that a file produced by creating this object and writing the
// modifed contents will produce an approriate "diff" result showing only
// the changes actually made.
//
class TcsDefFile
{
public:
	///////////////////////////////////////////////////////////////////////////
	// Construction,  Destruction,  Assignment  
	TcsDefFile (EcsDictType dictType);
	TcsDefFile (EcsDictType dictType,const char* filePath);
	TcsDefFile (const TcsDefFile& source);
	~TcsDefFile (void);
	TcsDefFile& operator= (const TcsDefFile& rhs);
	///////////////////////////////////////////////////////////////////////////
	// Operator Overloads
	TcsAscDefinition* operator[] (unsigned index);
	const TcsAscDefinition* operator[] (unsigned index) const;
	///////////////////////////////////////////////////////////////////////////
	// Public Named Member Functions
	EcsDictType GetDictType (void);
	size_t GetDefinitionCount (void) const;
	bool InitializeFromFile (const char* filePath);
	const char* GetDefinitionName (size_t index) const;
	bool GetIndexOf (size_t& index,const char* defName) const;
	const TcsAscDefinition* GetDefinition (size_t index) const;
	const TcsAscDefinition* GetDefinition (const char* defName) const;
	TcsAscDefinition* GetDefinition (const char* defName);
	bool ExtractDefinition (TcsAscDefinition& definition,const char* defName);
	std::vector<TcsAscDefinition>::iterator Locate (const char* defName);
	std::vector<TcsAscDefinition>::const_iterator Locate (const char* defName) const;
	bool RenameDef (const char* oldName,const char* newName);
	bool CopyDef (const char* oldname,const char* newName,const char* insertBefore = 0);
	const char* GetValue (const char* defName,const char* label) const;
	double GetValueAsDouble (const char* defName,const char* label,long32_t& format) const;
	bool SetValue (const char* defName,const char* label,const char* newValue);
	bool DeprecateDef (const char* defName,const char* description,const char* source);
	bool Replace (const TcsAscDefinition& newDef);
	bool ReplaceWith (const char* existingName,const TcsAscDefinition& newDef);
	bool ReplaceAt (size_t index,const TcsAscDefinition& newDef);
	bool InsertBefore (size_t index,const TcsAscDefinition& newDef);
	bool InsertBefore (const char* defName,const TcsAscDefinition& newDef);
	bool Append (const TcsAscDefinition& newDef);
	bool MakeLast (const char* defName);
	bool WriteToStream (std::ostream& outStrm) const;
	bool WriteToFile (const char* pathName) const;
private:
	///////////////////////////////////////////////////////////////////////////
	// Private Support Functions
	bool BuildVector (std::istream& inStrm);
	///////////////////////////////////////////////////////////////////////////
	// Private Data Members
	EcsDictType DictType;
	// Probably should use std::list<>, rather than a std::vector<>, here.
	std::vector<TcsAscDefinition> Definitions;
};

