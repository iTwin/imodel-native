
//Method.h

#ifndef __METHOD_H__
#define __METHOD_H__

//Typical DISCLAIMER:
//The code in this project is Copyright (C) 2003 by George Anescu. You have the right to
//use and distribute the code in any way you see fit as long as this paragraph is included
//with the distribution. No warranties or claims are made as to the validity of the
//information and code contained herein, so use it at your own risk.

#include <string>
#include <fstream>

using namespace std;

//Abstract Encryption/Decryption Method Interface
class CEncryptMethod
{
public:
	//Operation Modes
	//The Electronic Code Book (ECB), Cipher Block Chaining (CBC) and Cipher Feedback Block (CFB) modes
	//are implemented.
	//In ECB mode if the same block is encrypted twice with the same key, the resulting
	//ciphertext blocks are the same.
	//In CBC Mode a ciphertext block is obtained by first xoring the
	//plaintext block with the previous ciphertext block, and encrypting the resulting value.
	//In CFB mode a ciphertext block is obtained by encrypting the previous ciphertext block
	//and xoring the resulting value with the plaintext.
	enum { ECB=0, CBC=1, CFB=2 };

	//Padding Modes
	//ZEROES - The padding string consists of bytes set to zero.
	//BLANKS - The padding string consists of bytes set to blank.
	//PKCS7 - The Public-Key Cryptography Standards version 7 (PKCS7) padding string
	//consists of a sequence of bytes, each of which is equal to the total number of
	//padding bytes added. For example, if 24 bits (3 bytes) of padding need to be
	//added, the padding string is "03 03 03".
	enum { ZEROES=0, BLANKS=1, PKCS7=2 };

	//CONSTRUCTOR
	CEncryptMethod();

	//DESTRUCTOR
	virtual ~CEncryptMethod();

protected:
	//Compute Signature
	virtual void Signature(char* pcSig) = 0;
	//Auxiliary Functions
	void Xor(char* buff, char const* chain);
	static void HelpThrow(string const& rostrFileIn);
	static void BytesToWord(unsigned char const* pucBytes, unsigned int& ruiWord);
	static void WordToBytes(unsigned int uiWord, unsigned char* pucBytes);

public:
	//Encryption for a string of chars
	virtual void Encrypt(char const* in, char* result, size_t n) = 0;
	//Decryption for a string of chars
	virtual void Decrypt(char const* in, char* result, size_t n) = 0;
	//Encryption for a File
	virtual void EncryptFile(string const& rostrFileIn, string const& rostrFileOut) = 0;
	//Decryption for a File
	virtual void DecryptFile(string const& rostrFileIn, string const& rostrFileOut) = 0;
	//Setting the Operation Mode
	void SetMode(int iMode);
	//Setting the Padding Mode
	void SetPadding(int iPadding);
	//Getters
	int GetKeyLength();
	int GetBlockSize();	
	int GetMode();
	int GetPadding();
	//Padding the input string before encryption
	int Pad(char* in, int iLength);
	//Resetting the Initialization Vector
	virtual void ResetChain() = 0;

protected:
	//Error Messages
	static char const* sm_szErrorMsg1;
	static char const* sm_szErrorMsg2;
	static char const* sm_szErrorMsg3;
	static char const* sm_szErrorMsg4;
	static char const* sm_szErrorMsg5;
	static char const* sm_szErrorMsg6;
	static char const* sm_szErrorMsg7;
	static char const* sm_szErrorMsg8;
	static char const* sm_szErrorMsg9;
	static char const* sm_szErrorMsg10;
	//Key Initialization Flag
	bool m_bInit;
	//Block Size
	int	m_blockSize;
	//Key Length
	int m_keylength;
	//Operation Mode
	int m_iMode;
	//Padding Mode
	int m_iPadding;
};

#endif // __METHOD_H__

