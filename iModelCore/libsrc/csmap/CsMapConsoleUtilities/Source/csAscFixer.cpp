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
//lint -esym(534,TcsDefLine::ParseTextLine)
//lint -esym(534,TcsDefIndex::ConstructFromFile)

#include "csConsoleUtilities.hpp"

///////////////////////////////////////////////////////////////////////////////
// TcsDefLine:: An object which represents a line of a .ASC file in editable
//				form.
///////////////////////////////////////////////////////////////////////////////
// Static Constants, Variables, and Member Functions
const double TcsDefLine::InvalidDouble = -1.0E+64;
void TcsDefLine::Pad (char* array,int padCnt,unsigned arraySize)
{
	if (padCnt > 0 && static_cast<unsigned>(padCnt) < (arraySize - 1))
	{
		while (padCnt-- > 0)
		{
			*array++ = ' ';
		}
		*array = '\0';
	}
}
///////////////////////////////////////////////////////////////////////////////
// Construction,  Destruction,  Assignment
TcsDefLine::TcsDefLine (unsigned lineNbr,const char* lineText,EcsDictType dictType)
																:
															  DictType  (dictType),
															  Type      (ascTypNone),
															  LineNbr   (lineNbr),
															  InsertNbr (0U)
{
	Reset ();
	ParseTextLine (lineText);
	if (IsNameDef ())
	{
		Type = ascTypDefName;
	}
}
TcsDefLine::TcsDefLine (EcsDictType dictType,const char* label,const char* value,
															   const char* comment)
																:
															   DictType  (dictType),
															   Type      (ascTypNone),
															   LineNbr   (0U),
															   InsertNbr (0U)
{
	int wsCnt;
	int wsBase;
	int lblValCnt;

	switch (dictType) {
	case dictTypCoordsys:  wsBase = 16;   break;
	case dictTypDatum:     wsBase = 14;   break;
	case dictTypEllipsoid: wsBase = 16;   break;
	case dictTypMreg:      wsBase = 13;   break;
	case dictTypXform:     wsBase =  8;   break;
	case dictTypPath:      wsBase = 13;   break;
	default:               wsBase = 15;   break;
	}
	
	Reset ();
	if (label == 0 || *label == '\0')
	{
		if (comment == 0 || *comment == '\0')
		{
			Type = ascTypBlank;
		}
		else
		{
			Type = ascTypComment;
			CS_stncp (Comment,comment,sizeof (Comment));
		}
	}
	else if (value != 0 && *value != '\0')
	{
		Type = ascTypLblVal;
		CS_stncp (Label,label,sizeof (Label));
		CS_stncp (Value,value,sizeof (Value));
		if (comment != 0 && *comment != '\0')
		{
			CS_stncp (Comment,comment,sizeof (Comment));
		}
		wsCnt = wsBase - static_cast<int>(strlen (Label));
		if (wsCnt > 0)
		{
			Pad (LeadWs,wsCnt,sizeof (LeadWs));
		}
		Pad (SepWs,1,sizeof (SepWs));
		if (Comment [0] != '\0')
		{
			lblValCnt = static_cast<int>(strlen (LeadWs) + strlen (Label) + strlen (SepWs) + strlen (Value));
			wsCnt = 40 - lblValCnt;
			if (wsCnt < 4)
			{
				wsCnt = 4;
			}
			Pad (CmntWs,wsCnt,sizeof (CmntWs));
		}
		if (IsNameDef ())
		{
			Type = ascTypDefName;
		}
	}
	else
	{
		Type = ascTypUnknown;
	}
}
TcsDefLine::TcsDefLine (const TcsDefLine& source) : DictType  (source.DictType),
													Type      (source.Type),
													LineNbr   (source.LineNbr),
													InsertNbr (source.InsertNbr)
{
	CS_stncp (LeadWs,source.LeadWs,sizeof (LeadWs));
	CS_stncp (Label,source.Label,sizeof (Label));
	CS_stncp (SepWs,source.SepWs,sizeof (SepWs));
	CS_stncp (Value,source.Value,sizeof (Value));
	CS_stncp (CmntWs,source.CmntWs,sizeof (CmntWs));
	CS_stncp (Comment,source.Comment,sizeof (Comment));
}
TcsDefLine::~TcsDefLine (void)
{
}
TcsDefLine& TcsDefLine::operator= (const TcsDefLine& rhs)
{
	if (&rhs != this)
	{
		DictType  = rhs.DictType;
		Type      = rhs.Type;
		LineNbr   = rhs.LineNbr;
		InsertNbr = rhs.InsertNbr;
		CS_stncp (LeadWs,rhs.LeadWs,sizeof (LeadWs));
		CS_stncp (Label,rhs.Label,sizeof (Label));
		CS_stncp (SepWs,rhs.SepWs,sizeof (SepWs));
		CS_stncp (Value,rhs.Value,sizeof (Value));
		CS_stncp (CmntWs,rhs.CmntWs,sizeof (CmntWs));
		CS_stncp (Comment,rhs.Comment,sizeof (Comment));
	}
	return *this;
}
///////////////////////////////////////////////////////////////////////////////
// Operator Overloads
bool TcsDefLine::operator== (const char* label) const
{
	bool equal (false);
	
	if (Type == ascTypLblVal || Type == ascTypDefName)
	{
		equal = !CS_stricmp (Label,label);
	}
	return equal;	
}
///////////////////////////////////////////////////////////////////////////////
// Public Named Member Functions
bool TcsDefLine::IsNameDef () const
{
	bool isNameDef (false);

	if (Type == ascTypLblVal)
	{
		switch (DictType) {
		case dictTypCoordsys:
			isNameDef = (stricmp (Label,"CS_NAME:") == 0);
			break;
		case dictTypDatum:
			isNameDef = (stricmp (Label,"DT_NAME:") == 0);
			break;
		case dictTypEllipsoid:
			isNameDef = (stricmp (Label,"EL_NAME:") == 0);
			break;
		case dictTypMreg:
			isNameDef = (stricmp (Label,"DATUM_NAME:") == 0);
			break;
		case dictTypXform:
			isNameDef = (stricmp (Label,"GX_NAME:") == 0);
			break;
		case dictTypPath:
			isNameDef = (stricmp (Label,"GP_NAME:") == 0);
			break;
		case dictTypNone:
		case dictTypUnknown:
		default:
			break;
		}
	}
	return isNameDef;
}
unsigned TcsDefLine::GetLineNbr (void) const
{
	return LineNbr;
}
unsigned TcsDefLine::GetInsertNbr (void) const
{
	return InsertNbr;
}
EcsAscLineType TcsDefLine::GetType (void) const
{
	return Type;
}
const char* TcsDefLine::GetLabel (void) const
{
	return Label;
}
const char* TcsDefLine::GetValue (void) const
{
	return Value;
}
double TcsDefLine::GetValueAsDouble (long32_t& format) const
{
	double rtnValue;

	format = CS_atof (&rtnValue,Value);
	if (format < 0)
	{
		rtnValue = InvalidDouble;
	}
	return rtnValue;
}
const char* TcsDefLine::GetComment (void) const
{
	return Comment;
}
const char* TcsDefLine::GetLeadWs (void) const
{
	return LeadWs;
}
const char* TcsDefLine::GetSepWs (void) const
{
	return SepWs;
}
const char* TcsDefLine::GetCmntWs (void) const
{
	return CmntWs;
}

unsigned TcsDefLine::GetLineNbr (void)
{
	return LineNbr;
}
unsigned TcsDefLine::GetInsertNbr (void)
{
	return InsertNbr;
}
EcsAscLineType TcsDefLine::GetType (void)
{
	return Type;
}
char* TcsDefLine::GetLabel (void)
{
	return Label;
}
char* TcsDefLine::GetValue (void)
{
	return Value;
}
char* TcsDefLine::GetComment (void)
{
	return Comment;
}
char* TcsDefLine::GetLeadWs (void)
{
	return LeadWs;
}
char* TcsDefLine::GetSepWs (void)
{
	return SepWs;
}
char* TcsDefLine::GetCmntWs (void)
{
	return CmntWs;
}

bool TcsDefLine::Matches (const TcsDefLine& thisOne,bool fileScope) const
{
	bool matches;

	matches = (Type == thisOne.Type);
	if (matches)
	{
		if (fileScope)
		{
			matches = (LineNbr == thisOne.LineNbr);
		}
		if (matches)
		{
			switch (Type) {
			case ascTypBlank:
				break;
			case ascTypDefName:
				matches = (stricmp (Value,thisOne.Value) == 0);
				break;
			case ascTypLblVal:
				matches = (stricmp (Label,thisOne.Label) == 0);
				matches &= (stricmp (Value,thisOne.Value) == 0);
				break;
			case ascTypComment:
				matches = (stricmp (Comment,thisOne.Comment) == 0);
				break;
			case ascTypNone:
			case ascTypUnknown:
			default:
				matches = false;
				break;
			}
		}
	}
	return matches;
}
bool TcsDefLine::WriteToStream (std::ostream& outStrm) const
{
	if (Type == ascTypLblVal || Type == ascTypDefName)
	{
		outStrm << LeadWs
				<< Label
				<< SepWs
				<< Value
				<< CmntWs
				<< Comment
				<< std::endl;
	}
	else if (Type == ascTypComment)
	{
		outStrm << Comment
				<< std::endl;
	}
	else if (Type == ascTypBlank)
	{
		outStrm << Comment
				<< std::endl;
	}
	return outStrm.good ();
}
void TcsDefLine::SetLineNbr (unsigned newLineNbr)
{
	LineNbr = newLineNbr;
}
void TcsDefLine::SetInsertNbr (unsigned newInsertNbr)
{
	InsertNbr = newInsertNbr;
}
void TcsDefLine::SetType (EcsAscLineType newType)
{
	Type = newType;
}
void TcsDefLine::SetLabel (const char* newLabel)
{
	CS_stncp (Label,newLabel,sizeof (Label));
}
void TcsDefLine::SetValue (const char* newValue)
{
	CS_stncp (Value,newValue,sizeof (Value));
}
void TcsDefLine::SetComment (const char* newComment)
{
	CS_stncp (Comment,newComment,sizeof (Comment));
}
///////////////////////////////////////////////////////////////////////////////
// Private Support Functions
void TcsDefLine::Reset (void)
{
	Type = ascTypNone;
	memset (LeadWs,'\0',sizeof (LeadWs));
	memset (Label,'\0',sizeof (Label));
	memset (SepWs,'\0',sizeof (SepWs));
	memset (Value,'\0',sizeof (Value));
	memset (CmntWs,'\0',sizeof (CmntWs));
	memset (Comment,'\0',sizeof (Comment));
}
EcsAscLineType TcsDefLine::ParseTextLine (const char* textLine)
{
	char cc;
	char* vCp;
	const char* kCp;
	
	unsigned leadWsCnt (0);
	unsigned labelCnt (0);
	unsigned sepWsCnt (0);
	unsigned valueCnt (0);
	unsigned cmntWsCnt (0);
	unsigned cmntCnt (0);

	enum { leadWs, label, sepWs, value, comment, done, bogus } state;
	
	Type = ascTypNone;
	Reset ();

	kCp = textLine;
	state = leadWs;
	while (state != done)
	{
		cc = *kCp;
		switch (state) {
		case bogus:
			Reset ();
			Type = ascTypUnknown;
			CS_stncp (Comment,textLine,sizeof (Comment));
			state = done;
			break;

		case leadWs:
			if (cc == '\0' || cc == '\n' || cc == '\r')
			{
				Type = ascTypBlank;
				CS_stncp (Comment,textLine,sizeof (Comment));
				state = done;
			}
			else if (cc == ' ' || cc == '\t')
			{
				kCp += 1;			// accept character
				if (leadWsCnt < (sizeof (LeadWs) - 1))
				{
					LeadWs [leadWsCnt++] = cc;
					LeadWs [leadWsCnt] = '\0';
				}
				else
				{
					leadWsCnt += 1;
				}
			}
			else if (cc == '#')
			{
				Type = ascTypComment;
				CS_stncp (Comment,textLine,sizeof (Comment));
				state = done;
			}
			else
			{
				state = label;
			}
			break;

		case label:
			if (cc == ':')
			{
				if (labelCnt == 0)
				{
					state = bogus;
				}
				else
				{
					Type = ascTypLblVal;	// Looks like a valid Label/Value pair.
					
					kCp += 1;				// accept character
					// Add it to the 'Value' we are capturing.
					if (labelCnt < (sizeof (Label) - 1))
					{
						Label [labelCnt++] = cc;
						Label [labelCnt] = '\0';
					}
					state = sepWs;
				}
			}
			else if (isalnum (cc) || cc == '_' || cc == ' ')
			{
				kCp += 1;			// accept character
				if (labelCnt < (sizeof (Label) - 1))
				{
					Label [labelCnt++] = cc;
					Label [labelCnt] = '\0';
				}
				else
				{
					labelCnt += 1;
				}
			}
			else
			{
				state = bogus;
			}
			break;

		case sepWs:
			if (cc == ' ' || cc == '\t')
			{
				kCp += 1;			// accept character
				if (sepWsCnt < (sizeof (SepWs) - 1))
				{
					SepWs [sepWsCnt++] = cc;
					SepWs [sepWsCnt] = '\0';
				}
				else
				{
					sepWsCnt += 1;
				}
			}
			else
			{
				state = value;
			}
			break;

		case value:
			// Careful here, sometimes the value is blank.  Note that the only
			// place state can be set to 'value' is in the 'sepWs' code.  The
			// only place state can be set to 'sepWs' is when a valid label
			// has been sensed.  Thus, if we encounter the end of the text
			// line here, is simply means no value and no comment.
			if (cc == '\0' || cc == '\n' || cc == '\r')
			{
				state = done;
			}
			else if (cc == '#')
			{
				state = comment;	
			}
			else
			{
				kCp += 1;				// accept character
				if (valueCnt < (sizeof (Value) - 1))
				{
					Value [valueCnt++] = cc;
					Value [valueCnt] = '\0';
				}
				else
				{
					valueCnt += 1;
				}
			}
			break;
		case comment:
			if (cc == '\0' || cc == '\n' || cc == '\r')
			{
				state = done;
			}
			else
			{
				kCp += 1;			// accept character
				if (cmntCnt < (sizeof (Comment) - 1))
				{
					Comment [cmntCnt++] = cc;
					Comment [cmntCnt] = '\0';
				}
				else
				{
					cmntCnt += 1;
				}
			}
			break;
		case done:					// :>)
		default:
			state = bogus;
			break;
		}
	}
	
	// If Type is that of a valid Label/Value pair, we copy any trailing white
	// space on the value and transfer it to CmntWs.
	if (Type == ascTypLblVal)
	{
		cmntWsCnt = 0;
		vCp = Value + strlen (Value) - 1;
		while (*vCp == ' ' || *vCp == '\t')
		{
			cmntWsCnt += 1;
			vCp -= 1;
		}
		if (cmntWsCnt > 0)
		{
			vCp++;
			if (cmntWsCnt >= sizeof (CmntWs))
			{
				cmntWsCnt = sizeof (CmntWs) - 1;
			}
			CS_stncp (CmntWs,vCp,cmntWsCnt + 1);
			*vCp = '\0';
		}
	}
	return Type;
}
//newPage//
///////////////////////////////////////////////////////////////////////////////
// TcsAscDefinition -- A collection of TcsDefLine objects which represents
//					   a complete definition.
//
// A complete definition is defined, in order, as:
// 1> all blank and comment lines preceeding the definition.
// 2> The line which defines the name of the definition.
// 3> All non-blank line lines (values and comments) which completes the definition.
//
// Note that a blank line is considered to terminate the definition in order to
// get comments which preceed the definition to be a part of the definition.
//
///////////////////////////////////////////////////////////////////////////////
// Static constants, Variables, and Member Functions
//
// FUnctions which return a double value will return the following value in the
// event of an error.
const double TcsAscDefinition::InvalidDouble = -1.0E+64;

///////////////////////////////////////////////////////////////////////////////
// Construction,  Destruction,  Assignment  
TcsAscDefinition::TcsAscDefinition (EcsDictType type) : Type       (type),
														Definition () 
{
	memset (DefName,'\0',sizeof (DefName));
}
TcsAscDefinition::TcsAscDefinition (EcsDictType type,TcsDefLnItrK begin,TcsDefLnItrK end)
																			:
																		Type        (type),
																		Definition  (begin,end)
{
	TcsDefLnItr lineItr;
	
	for (lineItr = Definition.begin ();lineItr != Definition.end ();lineItr++)
	{
		if (lineItr->IsNameDef ())
		{
			const char* defNamePtr = lineItr->GetValue ();
			strncpy (DefName,defNamePtr,sizeof (DefName));
			DefName [sizeof (DefName) - 1] = '\0';
			break;
		}
	}
}
TcsAscDefinition::TcsAscDefinition (EcsDictType type,unsigned& lineNbr,std::istream& inStrm)
																		:
																	   Type       (type),
																	   Definition ()
{
	bool ok;

	memset (DefName,0,sizeof (DefName));
	ok = ReadFromStream (lineNbr,inStrm);
}
TcsAscDefinition::TcsAscDefinition (const TcsAscDefinition& source) : Type        (source.Type),
																	  Definition  (source.Definition)
{
	strncpy (DefName,source.DefName,sizeof (DefName));
	DefName [sizeof (DefName) - 1] = '\0';
}
TcsAscDefinition::~TcsAscDefinition (void)
{
	// Nothing to do here, yet.
}
TcsAscDefinition& TcsAscDefinition::operator= (const TcsAscDefinition& rhs)
{
	if (&rhs != this)
	{
		Type       = rhs.Type;
		Definition = rhs.Definition;
		strncpy (DefName,rhs.DefName,sizeof (DefName));
		DefName [sizeof (DefName) - 1] = '\0';
	}
	return *this;
}
///////////////////////////////////////////////////////////////////////////////
// Operator Overloads
bool TcsAscDefinition::operator== (const TcsAscDefinition& rhs) const
{
	bool equal = (stricmp (DefName,rhs.DefName) == 0);
	return equal;
}
bool TcsAscDefinition::operator<  (const TcsAscDefinition& rhs) const
{
	bool lessThan = (stricmp (DefName,rhs.DefName) < 0);
	return lessThan;
}
bool TcsAscDefinition::operator== (const char* defName) const
{
	bool equal = (stricmp (DefName,defName) == 0);
	return equal;
}
bool TcsAscDefinition::operator<  (const char* defName) const
{
	bool lessThan = (stricmp (DefName,defName) < 0);
	return lessThan;
}
TcsAscDefinition& TcsAscDefinition::operator+ (const TcsDefLine& newLine)
{
	Definition.push_back (newLine);
	return *this;
}
TcsAscDefinition& TcsAscDefinition::operator- (const TcsDefLine& oldLine)
{
	bool match;
	TcsDefLnItr lineItr;
	
	for (lineItr = Definition.begin ();lineItr != Definition.end ();lineItr++)
	{
		match = lineItr->Matches (oldLine);
		if (match)
		{
			break;
		}
	}
	if (lineItr != Definition.end ())
	{
		Definition.erase (lineItr);
	}
	return *this;
}
bool TcsAscDefinition::IsDefinition (void) const
{
	bool isDef;

	isDef = (DefName [0] != '\0');
	return isDef;
}
bool TcsAscDefinition::ReadFromStream (unsigned& lineNbr,std::istream& inStrm)
{
	bool ok (true);
	bool haveName (false);
	const char* nmPtr;
	EcsAscLineType lineType;
	std::istream::pos_type strmPos;
	char lineBufr [1024];

	while (inStrm.good ())
	{
		strmPos = inStrm.tellg ();
		inStrm.getline (lineBufr,sizeof (lineBufr),'\n');	//lint !e534
		lineNbr += 1;

		TcsDefLine newLine (lineNbr,lineBufr,Type);
		lineType = newLine.GetType ();
		if (!haveName)
		{
			if (lineType == ascTypDefName)
			{
				haveName = true;
				nmPtr = newLine.GetValue ();
				strncpy (DefName,nmPtr,sizeof (DefName));
				DefName [sizeof (DefName) - 1] = '\0';
				Definition.push_back (newLine);
			}
			else
			{
				Definition.push_back (newLine);
			}
		}
		else
		{
			if (lineType == ascTypDefName || lineType == ascTypBlank)
			{
				lineNbr -= 1;
				inStrm.seekg (strmPos);
				break;
			}
			else
			{
				Definition.push_back (newLine);
			}
		}
	}
	return ok;
}
const TcsDefLine* TcsAscDefinition::GetLine (size_t index) const
{
	const TcsDefLine* rtnValue = 0;

	if (index < Definition.size ())
	{
		rtnValue = &(Definition [index]);
	}
	return rtnValue;
}
const TcsDefLine* TcsAscDefinition::GetLine (const char* label) const
{
	EcsAscLineType lineType;
	const char* lblPtr;
	const TcsDefLine* rtnValue = 0;
	TcsDefLnItrK lineItr;
	
	for (lineItr = Definition.begin ();lineItr != Definition.end ();lineItr++)
	{
		lineType = lineItr->GetType ();
		if (lineType == ascTypLblVal || lineType == ascTypDefName)
		{
			lblPtr = lineItr->GetLabel ();
			if (!stricmp (lblPtr,label))
			{
				rtnValue = &(*lineItr);
				break;
			}
		}
	}
	return rtnValue;
}
TcsDefLine* TcsAscDefinition::GetLine (const char* label)
{
	EcsAscLineType lineType;
	const char* lblPtr;
	TcsDefLine* rtnValue = 0;
	TcsDefLnItr lineItr;
	
	for (lineItr = Definition.begin ();lineItr != Definition.end ();lineItr++)
	{
		lineType = lineItr->GetType ();
		if (lineType == ascTypLblVal || lineType == ascTypDefName)
		{
			lblPtr = lineItr->GetLabel ();
			if (!stricmp (lblPtr,label))
			{
				rtnValue = &(*lineItr);
				break;
			}
		}
	}
	return rtnValue;
}
const char* TcsAscDefinition::GetDefinitionName (void) const
{
	return DefName;
}
bool TcsAscDefinition::RenameDef (const char* newName)
{
	bool ok (false);
	EcsAscLineType lineType;
	TcsDefLnItr lineItr;

	for (lineItr = Definition.begin ();lineItr != Definition.end ();lineItr++)
	{
		lineType = lineItr->GetType ();
		if (lineType == ascTypDefName)
		{
			lineItr->SetValue (newName);
			ok = true;
		}
	}
	if (ok)
	{
		strncpy (DefName,newName,sizeof (DefName));
		DefName [sizeof (DefName) - 1] = '\0';
	}
	return ok;
}
const char* TcsAscDefinition::GetValue (const char* label) const
{
	const char* valPtr = 0;

	const TcsDefLine* linePtr = GetLine (label);
	if (linePtr != 0)
	{
		valPtr = linePtr->GetValue ();
	}
	return valPtr;
}
double TcsAscDefinition::GetValueAsDouble (const char* label,long32_t& format) const
{
	double rtnValue = InvalidDouble;

	const char* valPtr = GetValue (label);
	if (valPtr != 0)
	{
		format = CS_atof (&rtnValue,valPtr);
	}
	return rtnValue;
}
// The following is used to determine the location of a specific
// label/value pair within a definition so that it can be inserted
// into a new definition in the same place.
const char* TcsAscDefinition::GetNextLabel (const char* labelName) const
{
	EcsAscLineType lineType;
	const char* lblPtr;
	const char* rtnValue = 0;
	TcsDefLnItrK lineItr;

	// Locate the label/value line.
	for (lineItr = Definition.begin ();lineItr != Definition.end ();lineItr++)
	{
		lineType = lineItr->GetType ();
		if (lineType == ascTypLblVal || lineType == ascTypDefName)
		{
			lblPtr = lineItr->GetLabel ();
			if (!stricmp (lblPtr,labelName))
			{
				break;
			}
		}
	}

	// We've located the search line.  Skip to the next label/value
	// line (i.e. skip comments and blank lines.
	for (lineItr++;lineItr != Definition.end ();lineItr++)
	{
		lineType = lineItr->GetType ();
		if (lineType == ascTypLblVal || lineType == ascTypDefName)
		{
			rtnValue = lineItr->GetLabel ();
			break;
		}
	}
	return rtnValue;
}
bool TcsAscDefinition::SetValue (const char* label,const char* newValue)
{
	bool ok (false);
	EcsAscLineType lineType;
	const char* lblPtr;
	TcsDefLnItr lineItr;

	for (lineItr = Definition.begin ();lineItr != Definition.end ();lineItr++)
	{
		lineType = lineItr->GetType ();
		if (lineType == ascTypLblVal || lineType == ascTypDefName)
		{
			lblPtr = lineItr->GetLabel ();
			if (!stricmp (lblPtr,label))
			{
				lineItr->SetValue (newValue);
				ok = true;
				break;
			}
		}
	}
	return ok;
}
bool TcsAscDefinition::PrependLine (const TcsDefLine& newLine)
{
	bool ok (false);

	if (newLine.GetType () == Type)
	{
		Definition.insert (Definition.begin (),newLine);
		ok = true;
	}
	return ok;
}
bool TcsAscDefinition::InsertBefore (const char* label,const TcsDefLine& newLine)
{
	bool ok (false);
	unsigned lineNbr;
	unsigned insertNbr;
	EcsAscLineType lineType;
	const char* lblPtr;
	TcsDefLnItr lineItr;
	TcsDefLnItr newItr;
	
	for (lineItr = Definition.begin ();lineItr != Definition.end ();lineItr++)
	{
		lineType = lineItr->GetType ();
		if (lineType == ascTypLblVal || lineType == ascTypDefName)
		{
			lblPtr = lineItr->GetLabel ();
			if (!stricmp (lblPtr,label))
			{
				lineNbr = lineItr->GetLineNbr ();
				insertNbr = lineItr->GetInsertNbr ();
				newItr = Definition.insert (lineItr,newLine);
				newItr->SetLineNbr (lineNbr);
				newItr->SetInsertNbr (insertNbr + 1);
				ok = true;
				break;
			}
		}
	}
	return ok;
}
bool TcsAscDefinition::InsertAfter (const char* label,const TcsDefLine& newLine)
{
	bool ok (false);
	unsigned lineNbr;
	unsigned insertNbr;
	EcsAscLineType lineType;
	const char* lblPtr;
	TcsDefLnItr lineItr;
	TcsDefLnItr newItr;
	
	for (lineItr = Definition.begin ();lineItr != Definition.end ();lineItr++)
	{
		lineType = lineItr->GetType ();
		if (lineType == ascTypLblVal || lineType == ascTypDefName)
		{
			lblPtr = lineItr->GetLabel ();
			if (!stricmp (lblPtr,label))
			{
				// The current iterator position is the insert after location.
				// std::vector::insert is an insert before operation.
				lineNbr = lineItr->GetLineNbr ();
				insertNbr = lineItr->GetInsertNbr ();
				lineItr++;
				newItr = Definition.insert (lineItr,newLine);
				newItr->SetLineNbr (lineNbr);
				newItr->SetInsertNbr (insertNbr + 1);
				ok = true;
				break;
			}
		}
	}
	return ok;
}
bool TcsAscDefinition::Append (const TcsDefLine& newLine)
{
	bool ok (false);
	unsigned lineNbr (0);
	unsigned insertNbr (0);
	EcsAscLineType lineType;
	TcsDefLnItr lineItr;
	TcsDefLnItr newItr;

	// We do this to make sure this is not an empty definition.
	for (lineItr = Definition.begin ();lineItr != Definition.end ();lineItr++)
	{
		lineType = lineItr->GetType ();
		if (lineType == ascTypLblVal || lineType == ascTypDefName)
		{
			lineNbr = lineItr->GetLineNbr ();
			insertNbr = lineItr->GetInsertNbr ();
			ok = true;
		}
	}
	if (ok)
	{
		Definition.push_back (newLine);
		lineItr = Definition.end () - 1;
		lineItr->SetLineNbr (lineNbr);
		lineItr->SetInsertNbr (insertNbr + 1);
	}
	return ok;
}
bool TcsAscDefinition::RemoveLine (const char* label)
{
	bool ok (false);
	EcsAscLineType lineType;
	const char* lblPtr;
	TcsDefLnItr lineItr;

	for (lineItr = Definition.begin ();lineItr != Definition.end ();lineItr++)
	{
		lineType = lineItr->GetType ();
		if (lineType == ascTypLblVal)
		{
			lblPtr = lineItr->GetLabel ();
			if (!stricmp (lblPtr,label))
			{
				Definition.erase (lineItr);
				ok = true;
				break;
			}
		}
	}
	return ok;
}
bool TcsAscDefinition::WriteToStream (std::ostream& outStrm) const
{
	bool ok (true);
	TcsDefLnItrK lineItr;
	
	for (lineItr = Definition.begin ();ok && lineItr != Definition.end ();lineItr++)
	{
		ok = lineItr->WriteToStream (outStrm);
	}
	return ok;	
}
// Swap a specific value element between two definitions.  Elementary logic, but
// implementation details are laborious.
bool ValueSwap (TcsAscDefinition& first,TcsAscDefinition& second,const char* lblName)
{
	bool ok (false);

	TcsDefLine* firstLinePtr;
	TcsDefLine* secondLinePtr;

	const char* tmpNamePtr;

	char swpBufr [512];
	
	firstLinePtr = first.GetLine (lblName);
	secondLinePtr = second.GetLine (lblName);

	if (firstLinePtr == 0 && secondLinePtr == 0)
	{
		// Neither definition has a line with the supplied label.
		// Nothing to do really, don't think this should be
		// considered failure.
		ok = true;
	}
	else if (firstLinePtr != 0 && secondLinePtr == 0)
	{
		// The second definition does not have a line with the
		// supplied label.  Extract the value from the first
		// group and insert it into the second group at a
		// similar location.

		// Decide where to insert the line into the second definition.
		TcsDefLine insertLine (*firstLinePtr);
		tmpNamePtr = first.GetNextLabel (lblName);
		if (tmpNamePtr != 0)
		{
			ok = second.InsertBefore (tmpNamePtr,insertLine);
		}
		if (tmpNamePtr == 0 || !ok)
		{
			ok = second.Append (insertLine);
		}
		if (ok)
		{
			ok = first.RemoveLine (lblName);
		}
	}
	else if (firstLinePtr == 0 && secondLinePtr != 0)
	{
		// The first definition does not have a line with the
		// supplied label.
		TcsDefLine insertLine (*secondLinePtr);
		tmpNamePtr = second.GetNextLabel (lblName);
		if (tmpNamePtr != 0)
		{
			ok = first.InsertBefore (tmpNamePtr,insertLine);
		}
		if (tmpNamePtr == 0 || !ok)
		{
			ok = first.Append (insertLine);
		}
		if (ok)
		{
			ok = second.RemoveLine (lblName);
		}
	}
	else
	{
		// Both definitions have a line with the supplied label.
		if (firstLinePtr->GetType () == ascTypDefName)
		{
			// Special processing if we are swapping the name of the definition.
			tmpNamePtr = first.GetDefinitionName ();
			CS_stncp (swpBufr,tmpNamePtr,sizeof (swpBufr));
			ok = first.RenameDef (second.GetDefinitionName ());
			if (ok)
			{
				second.RenameDef (swpBufr);
			}
		}
		else
		{
			char* firstValuePtr;
			char* secondValuePtr;

			// Not the definition name, normal swap processing.
			firstValuePtr = firstLinePtr->GetValue ();
			secondValuePtr = secondLinePtr->GetValue ();

			CS_stncp (swpBufr,firstValuePtr,sizeof (swpBufr));
			firstLinePtr->SetValue (secondValuePtr);
			secondLinePtr->SetValue (swpBufr);
			ok = true;
		}
	}
	return ok;
}

//newPage//
///////////////////////////////////////////////////////////////////////////////
// TcsDefFile  -- An object which represents a complete .ASC definition file
//				  as a collection of TcsAscDefinition objects.
//
// May need to tweak this so that if separates the initial comment from the
// initial definition.  Currently, the initial comment is considered part
// of the initial definition.
//
///////////////////////////////////////////////////////////////////////////////
// Construction,  Destruction,  Assignment
TcsDefFile::TcsDefFile (EcsDictType dictType) : DictType    (dictType),
												Definitions ()
{
	switch (DictType) {
	case dictTypCoordsys:
		Definitions.reserve (1000U);
		break;
	case dictTypDatum:
		Definitions.reserve (600U);
		break;
	case dictTypEllipsoid:
		Definitions.reserve (30U);
		break;
	case dictTypMreg:
		Definitions.reserve (48U);
		break;
	case dictTypXform:
		Definitions.reserve (600U);
		break;
	case dictTypPath:
		Definitions.reserve (60U);
		break;
	case dictTypNone:
	case dictTypUnknown:
	default:
		break;
	}
}
TcsDefFile::TcsDefFile (EcsDictType dictType,const char* filePath) : DictType    (dictType),
																	 Definitions ()
{
	unsigned lineNbr (0U);

	std::ifstream inStrm (filePath,std::ios_base::in);
	if (inStrm.is_open ())
	{
		// Should add a loop here which parses lines into a Header collection,
		// until the first non-comment line is encountered.  This would separate
		// the initial comment from the initial definition.  The WriteToStream
		// member function would then need to be modified to write the Header
		// collection prior to the following.
		while (inStrm.good ())
		{
			TcsAscDefinition newDef (DictType,lineNbr,inStrm);
			Definitions.push_back (newDef);
		}
		// Do we have a problem with stuff on the end of the file???
		// WHat we need to do here is modify the TcsAscDefinition
		// constructor so that it reseeks the file back to where it
		// started in the event that it doesn't find a definition name
		// line in the stream.  This implies we have some way ofo checking
		// the results of the construction; i.e. a status value in the
		// the object itself.
	}
}
TcsDefFile::TcsDefFile (const TcsDefFile& source) : DictType    (source.DictType),
													Definitions (source.Definitions)
{
}
TcsDefFile::~TcsDefFile (void)
{
}
TcsDefFile& TcsDefFile::operator= (const TcsDefFile& rhs)
{
	if (&rhs != this)
	{
		Definitions.clear ();
		DictType    = rhs.DictType;
		Definitions = rhs.Definitions;
	}
	return *this;
}
///////////////////////////////////////////////////////////////////////////////
// Operator Overloads
TcsAscDefinition* TcsDefFile::operator[] (unsigned index)
{
	TcsAscDefinition* rtnValue (0);
	if (index < Definitions.size ())
	{
		rtnValue = &Definitions [index];
	}
	return rtnValue;
}
const TcsAscDefinition* TcsDefFile::operator[] (unsigned index) const
{
	const TcsAscDefinition* rtnValue (0);
	if (index < Definitions.size ())
	{
		rtnValue = &Definitions [index];
	}
	return rtnValue;
}
///////////////////////////////////////////////////////////////////////////////
// Public Named Member Functions
EcsDictType TcsDefFile::GetDictType (void)
{
	return DictType;
}
size_t TcsDefFile::GetDefinitionCount (void) const
{
	size_t rtnValue = Definitions.size ();
	return rtnValue;
}
bool TcsDefFile::InitializeFromFile (const char* filePath)
{
	bool ok (false);

	unsigned lineNbr (0U);

	Definitions.clear ();

	std::ifstream inStrm (filePath,std::ios_base::in);
	ok = inStrm.is_open ();
	if (ok)
	{
		// Should add a loop here which parses lines into a Header collection,
		// until the first non-comment line is encountered.  This would separate
		// the initial comment from the initial definition.  The WriteToStream
		// member function would then need to be modified to write the Header
		// collection prior to the following.
		while (inStrm.good ())
		{
			TcsAscDefinition newDef (DictType,lineNbr,inStrm);
			Definitions.push_back (newDef);
		}
		// Do we have a problem with stuff on the end of the file???
		// What we need to do here is modify the TcsAscDefinition
		// constructor so that it reseeks the file back to where it
		// started in the event that it doesn't find a definition name
		// line in the stream.  This implies we have some way of checking
		// the results of the construction; i.e. a status value in the
		// the object itself.
	}
	return ok;
}
const char* TcsDefFile::GetDefinitionName (size_t index) const
{
	const char* defName = 0;
	const TcsAscDefinition* defPtr;

	if (index < Definitions.size ())
	{
		defPtr = &Definitions [index];
		defName = defPtr->GetDefinitionName ();
	}
	return defName;
}
bool TcsDefFile::GetIndexOf (size_t& index,const char* defName) const
{
	bool ok (false);

	const char* defNamePtr;
	std::vector<TcsAscDefinition>::const_iterator itr;

	index = 0;
	for (itr = Definitions.begin ();itr != Definitions.end ();itr++)
	{
		defNamePtr = itr->GetDefinitionName ();
		if (!stricmp (defNamePtr,defName))
		{
			index = itr - Definitions.begin();
			ok = true;
			break;
		}
	}
	return ok;
}
const TcsAscDefinition* TcsDefFile::GetDefinition (size_t index) const
{
	const TcsAscDefinition* defPtr;

	defPtr = 0;
	if (index < Definitions.size ())
	{
		defPtr = &Definitions [index];
	}
	return defPtr;
}

TcsAscDefinition* TcsDefFile::GetDefinition (const char* defName)
{
	const char* defNamePtr;
	TcsAscDefinition* rtnValue (0);
	std::vector<TcsAscDefinition>::iterator itr;

	for (itr = Definitions.begin ();itr != Definitions.end ();itr++)
	{
		defNamePtr = itr->GetDefinitionName ();
		if (!stricmp (defNamePtr,defName))
		{
			rtnValue = &(*itr);
			break;
		}
	}
	return rtnValue;
}
bool TcsDefFile::ExtractDefinition (TcsAscDefinition& definition,const char* defName)
{
	bool ok (false);

	const char* defNamePtr;
	std::vector<TcsAscDefinition>::iterator itr;

	for (itr = Definitions.begin ();itr != Definitions.end ();itr++)
	{
		defNamePtr = itr->GetDefinitionName ();
		if (!stricmp (defNamePtr,defName))
		{
			definition = *itr;
			ok = true;
			break;
		}
	}
	return ok;
}
const TcsAscDefinition* TcsDefFile::GetDefinition (const char* defName) const
{
	const char* defNamePtr;
	const TcsAscDefinition* rtnValue (0);
	std::vector<TcsAscDefinition>::const_iterator itr;

	for (itr = Definitions.begin ();itr != Definitions.end ();itr++)
	{
		defNamePtr = itr->GetDefinitionName ();
		if (!stricmp (defNamePtr,defName))
		{
			rtnValue = &(*itr);
			break;
		}
	}
	return rtnValue;
}
std::vector<TcsAscDefinition>::iterator TcsDefFile::Locate (const char* defName)
{
	const char* defNamePtr;
	std::vector<TcsAscDefinition>::iterator itr;
	std::vector<TcsAscDefinition>::iterator rtnValue;

	rtnValue = Definitions.end ();
	for (itr = Definitions.begin ();itr != Definitions.end ();itr++)
	{
		defNamePtr = itr->GetDefinitionName ();
		if (!stricmp (defNamePtr,defName))
		{
			rtnValue = itr;
			break;
		}
	}
	return rtnValue;
}
std::vector<TcsAscDefinition>::const_iterator TcsDefFile::Locate (const char* defName) const
{
	const char* defNamePtr;
	std::vector<TcsAscDefinition>::const_iterator itr;
	std::vector<TcsAscDefinition>::const_iterator rtnValue;

	rtnValue = Definitions.end ();
	for (itr = Definitions.begin ();itr != Definitions.end ();itr++)
	{
		defNamePtr = itr->GetDefinitionName ();
		if (!stricmp (defNamePtr,defName))
		{
			rtnValue = itr;
			break;
		}
	}
	return rtnValue;
}
bool TcsDefFile::RenameDef (const char* oldName,const char* newName)
{
	bool ok (false);
	TcsAscDefinition* defPtr;

	defPtr = GetDefinition (oldName);
	if (defPtr != 0)
	{
		defPtr->RenameDef (newName);
		ok = true;
	}
	return ok;
}
bool TcsDefFile::CopyDef (const char* oldName,const char* newName,const char* insertBefore)
{
	bool ok (false);

	const TcsAscDefinition* oldDefPtr;
	TcsAscDefinition* newDefPtr = 0;
	
	oldDefPtr = GetDefinition (oldName);
	ok = (oldDefPtr != 0);
	if (ok)
	{
		newDefPtr = new TcsAscDefinition (*oldDefPtr);
		ok = (newDefPtr != 0);
		if (ok)
		{
			ok = newDefPtr->RenameDef (newName);		
		}
	}
	if (ok)
	{
		if (insertBefore == 0)
		{
			ok = Append (*newDefPtr);
		}
		else
		{
			ok = InsertBefore (insertBefore,*newDefPtr);
		}
	}
	if (newDefPtr != 0)
	{
		delete newDefPtr;
	}
	return ok;
}
const char* TcsDefFile::GetValue (const char* defName,const char* label) const
{
	const char* rtnValue (0);
	const TcsAscDefinition* defPtr;

	defPtr = GetDefinition (defName);
	if (defPtr != 0)
	{
		rtnValue = defPtr->GetValue (label);
	}
	return rtnValue;
}
double TcsDefFile::GetValueAsDouble (const char* defName,const char* label,long32_t& format) const
{
	double rtnValue (TcsAscDefinition::InvalidDouble);
	const TcsAscDefinition* defPtr;

	defPtr = GetDefinition (defName);
	if (defPtr != 0)
	{
		rtnValue = defPtr->GetValueAsDouble (label,format);
	}
	return rtnValue;
}
bool TcsDefFile::SetValue (const char* defName,const char* label,const char* newValue)
{
	bool ok (false);
	TcsAscDefinition* defPtr;

	defPtr = GetDefinition (defName);
	if (defPtr != 0)
	{
		ok = defPtr->SetValue (label,newValue);
	}
	return ok;
}
bool TcsDefFile::DeprecateDef (const char* defName,const char* description,const char* source)
{
	bool ok (false);
	TcsAscDefinition* defPtr;

	const char* legacyValue = (DictType == dictTypEllipsoid) ? "LGACY" : "LEGACY";

	defPtr = GetDefinition (defName);
	if (defPtr != 0)
	{
		defPtr->SetValue ("GROUP:",legacyValue);
		if (description != 0)
		{
			defPtr->SetValue ("DESC_NM:",description);
		}
		if (source != 0)
		{
			defPtr->SetValue ("SOURCE:",source);
		}
		ok = true;
	}
	return ok;
}
bool TcsDefFile::Replace (const TcsAscDefinition& newDef)
{
	bool ok (false);
	const char* newNamePtr;
	const char* itrNamePtr;
	std::vector<TcsAscDefinition>::iterator rplItr;
	std::vector<TcsAscDefinition>::iterator loopItr;

	rplItr = Definitions.end ();
	newNamePtr = newDef.GetDefinitionName ();
	for (loopItr = Definitions.begin ();loopItr != Definitions.end ();loopItr++)
	{
		itrNamePtr = loopItr->GetDefinitionName ();
		if (!stricmp (newNamePtr,itrNamePtr))
		{
			rplItr = loopItr;
			break;
		}
	}
	if (rplItr != Definitions.end ())
	{
		*rplItr = newDef;
		ok = true;
	}
	return ok;
}
bool TcsDefFile::ReplaceWith (const char* existingName,const TcsAscDefinition& newDef)
{
	bool ok (false);
	const char* itrNamePtr;
	std::vector<TcsAscDefinition>::iterator rplItr;
	std::vector<TcsAscDefinition>::iterator loopItr;

	rplItr = Definitions.end ();
	for (loopItr = Definitions.begin ();loopItr != Definitions.end ();loopItr++)
	{
		itrNamePtr = loopItr->GetDefinitionName ();
		if (!stricmp (existingName,itrNamePtr))
		{
			rplItr = loopItr;
			break;
		}
	}
	if (rplItr != Definitions.end ())
	{
		*rplItr = newDef;		// Presumably this calls the copy constructor, nicht whar???
		ok = true;
	}
	return ok;
}
bool TcsDefFile::ReplaceAt (size_t index,const TcsAscDefinition& newDef)
{
	bool ok;

	std::vector<TcsAscDefinition>::iterator rplItr;
	std::vector<TcsAscDefinition>::iterator loopItr;

	ok = (index < Definitions.size());
	if (ok)
	{
		rplItr = Definitions.begin () + index;
		*rplItr = newDef;
	}
	return ok;
}
bool TcsDefFile::InsertBefore (size_t index,const TcsAscDefinition& newDef)
{
	bool ok (false);
	std::vector<TcsAscDefinition>::iterator itr;
	std::vector<TcsAscDefinition>::iterator newItr;

	if (index < Definitions.size ())
	{
		itr = Definitions.begin () + index;	
		Definitions.insert (itr,newDef);
		ok = true;
	}
	return ok;
}
bool TcsDefFile::InsertBefore (const char* defName,const TcsAscDefinition& newDef)
{
	bool ok (false);
	std::vector<TcsAscDefinition>::iterator itr;
	std::vector<TcsAscDefinition>::iterator newItr;
	
	itr = Locate (defName);
	if (itr != Definitions.end ())
	{
		// Don't know that there is any possible indication of failure.  I
		// suppose an exception of some sort is thrown.
		Definitions.insert (itr,newDef);
		ok = true;
	}
	return ok;
}
bool TcsDefFile::Append (const TcsAscDefinition& newDef)
{
	Definitions.push_back (newDef);
	return true;
}
bool TcsDefFile::MakeLast (const char* defName)
{
	bool ok (false);
	const char* defNamePtr;
	std::vector<TcsAscDefinition>::const_iterator itr;

	for (itr = Definitions.begin ();itr != Definitions.end ();itr++)
	{
		defNamePtr = itr->GetDefinitionName ();
		if (!stricmp (defNamePtr,defName))
		{
			TcsAscDefinition moveBuffer (*itr);
			Definitions.erase (itr);
			Definitions.push_back (moveBuffer);
			ok = true;
			break;
		}
	}
	return ok;
}
// Write the possibly modified content to the provided stream.
bool TcsDefFile::WriteToStream (std::ostream& outStrm) const
{
	bool ok (true);
	std::vector<TcsAscDefinition>::const_iterator itr;

	for (itr = Definitions.begin ();ok && itr != Definitions.end ();itr++)
	{
		ok = itr->WriteToStream (outStrm);
	}
	return ok;
}
// Write the possibly modified content to a file with the given name and
// location.
bool TcsDefFile::WriteToFile (const char* pathName) const
{
	bool ok;
	
	std::ofstream outStrm;

	outStrm.open (pathName,std::ios_base::out | std::ios_base::trunc);
	ok = outStrm.is_open ();
	if (ok)
	{
		ok = WriteToStream (outStrm);
		outStrm.close ();
	}
	return ok;
}
//newPage//
///////////////////////////////////////////////////////////////////////////////
// Private Support Functions
