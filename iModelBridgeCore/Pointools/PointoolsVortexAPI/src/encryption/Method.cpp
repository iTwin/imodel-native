#include "PointoolsVortexAPIInternal.h"

//Method.cpp
#include <encryption/Method.h>

using namespace std;

//Error Messages
char const* CEncryptMethod::sm_szErrorMsg1 = "FileCrypt ERROR: Encryption/Decryption Object not Initialized!";
char const* CEncryptMethod::sm_szErrorMsg2 = "FileCrypt ERROR: Illegal Operation Mode!";
char const* CEncryptMethod::sm_szErrorMsg3 = "FileCrypt ERROR: Illegal Padding Mode!";
char const* CEncryptMethod::sm_szErrorMsg4 = "FileCrypt ERROR: No Key DataSpecified!";
char const* CEncryptMethod::sm_szErrorMsg5 = "FileCrypt ERROR: Key Data Length should be > 0!";
char const* CEncryptMethod::sm_szErrorMsg6 = "Illegal Block Size!";
char const* CEncryptMethod::sm_szErrorMsg7 = "FileCrypt ERROR: Cannot open File ";
char const* CEncryptMethod::sm_szErrorMsg8 = "FileCrypt ERROR: The same File for Input and Output ";
char const* CEncryptMethod::sm_szErrorMsg9 = "FileCrypt ERROR: File ";
char const* CEncryptMethod::sm_szErrorMsg10 = " cannot be Correctly Decrypted!";

//CONSTRUCTOR
CEncryptMethod::CEncryptMethod() : m_bInit(false)
{
}

//DESTRUCTOR
CEncryptMethod::~CEncryptMethod()
{
}

//Auxiliary Function
void CEncryptMethod::Xor(char* buff, char const* chain)
{
	if(false==m_bInit)
		throw runtime_error(string(sm_szErrorMsg1));
	for(int i=0; i<m_blockSize; i++)
		*(buff++) ^= *(chain++);	
}

//Setting the Operation Mode
void CEncryptMethod::SetMode(int iMode)
{
	if(false==m_bInit)
		throw runtime_error(string(sm_szErrorMsg1));
	if(iMode<ECB || iMode>CFB)
		throw runtime_error(string(sm_szErrorMsg2));
	m_iMode = iMode;
}

//Setting the Padding Mode
void CEncryptMethod::SetPadding(int iPadding)
{
	if(false==m_bInit)
		throw runtime_error(string(sm_szErrorMsg1));
	if(iPadding<ZEROES || iPadding>PKCS7)
		throw runtime_error(string(sm_szErrorMsg3));
	m_iPadding = iPadding;
}

//Getters
int CEncryptMethod::GetKeyLength()
{
	if(false==m_bInit)
		throw runtime_error(string(sm_szErrorMsg1));
	return m_keylength;
}

int CEncryptMethod::GetBlockSize()
{
	if(false==m_bInit)
		throw runtime_error(string(sm_szErrorMsg1));
	return m_blockSize;
}

int CEncryptMethod::GetMode()
{
	if(false==m_bInit)
		throw runtime_error(string(sm_szErrorMsg1));
	return m_iMode;
}

int CEncryptMethod::GetPadding()
{
	if(false==m_bInit)
		throw runtime_error(string(sm_szErrorMsg1));
	return m_iPadding;
}

//Padding the input string before encryption
int CEncryptMethod::Pad(char* in, int iLength)
{
	if(false==m_bInit)
		throw runtime_error(string(sm_szErrorMsg1));
	int iRes = iLength%m_blockSize;
	if(iRes != 0)
	{
		//Is the caller's responsability to ensure that in buffer is large enough
		int iPadded = m_blockSize - iRes;
		char* pin = in+iLength;
		switch(m_iPadding)
		{
			case ZEROES:
			{
				for(int i=0; i<iPadded; i++, pin++)
					*pin = 0;
				break;
			}

			case BLANKS:
			{
				for(int i=0; i<iPadded; i++, pin++)
					*pin = 0x20;
				break;
			}

			case PKCS7:
			{
				for(int i=0; i<iPadded; i++, pin++)
					*pin = char(iPadded);
				break;
			}
		}
		return iLength + iPadded;
	}
	return iLength;
}

void CEncryptMethod::HelpThrow(string const& rostrFileIn)
{
    pt::String str;
    str.format("FileCrypt ERROR: Not an FileCrypt Encrypted File %s!/n", rostrFileIn.c_str());
    std::string stdStr(str.c_str());
	throw runtime_error(stdStr);
}

void CEncryptMethod::BytesToWord(unsigned char const* pucBytes, unsigned int& ruiWord)
{
	ruiWord = 0;
	ruiWord |= (*(pucBytes++) & 0xFF) << 24;
	ruiWord |= (*(pucBytes++) & 0xFF) << 16;
	ruiWord |= (*(pucBytes++) & 0xFF) << 8;
	ruiWord |= *pucBytes & 0xFF;
}

void CEncryptMethod::WordToBytes(unsigned int uiWord, unsigned char* pucBytes)
{
	*(pucBytes++) = (uiWord>>24) & 0xFF;
	*(pucBytes++) = (uiWord>>16) & 0xFF;
	*(pucBytes++) = (uiWord>>8) & 0xFF;
	*pucBytes = uiWord & 0xFF;
}


