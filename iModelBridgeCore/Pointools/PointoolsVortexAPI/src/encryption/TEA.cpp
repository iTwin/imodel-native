#include "PointoolsVortexAPIInternal.h"

//TEA.cpp
#include <encryption/TEA.h>
#include <encryption/SHA.h>
#include <encryption/DoubleBuffering.h>

using namespace std;

//Null chain
char const CTEA::sm_chain0[BLOCK_SIZE] = {0};

//CONSTRUCTOR
CTEA::CTEA()
{
	//Fixed Values
	m_blockSize = BLOCK_SIZE;
	m_keylength = KEY_LENGTH;
}

void CTEA::Initialize(char const* keydata, int keydatalength, char const* chain,
	int iMode, int iPadding)
{
	//Check Initialization Data
	if(NULL == keydata)
		throw runtime_error(string(sm_szErrorMsg4));
	if(keydatalength < 1)
		throw runtime_error(string(sm_szErrorMsg5));
	if(iMode<ECB || iMode>CFB)
		throw runtime_error(string(sm_szErrorMsg2));
	if(iPadding<ZEROES || iPadding>PKCS7)
		throw runtime_error(string(sm_szErrorMsg3));
	m_iMode = iMode;
	m_iPadding = iPadding;
	//Create the Key from Key Data
	int i, j;
	char key[KEY_LENGTH];
	for(i=0,j=0; i<m_keylength; i++,j=(j+1)%keydatalength)
		key[i] = keydata[j];
	bool bSameKey = false;
	bool bSameChain = false;
	if(true == m_bInit)
	{
		//Only if already initialized
		//Check the Chain if is the same
		if(0 == memcmp(m_apchain0.get(), chain, m_blockSize))
			bSameChain = true;
		//Check the Key if is the same
		if(0 == memcmp(m_apKey.get(), key, m_keylength))
			bSameKey = true;
	}
	if(true == bSameChain)
		//Just Reset
		memcpy(m_apchain.get(), m_apchain0.get(), m_blockSize);
	else
	{
		//Initialize the chain
		if(NULL==m_apchain0.get())
			m_apchain0.reset(new char[m_blockSize]);
		if(NULL==m_apchain.get())
			m_apchain.reset(new char[m_blockSize]);
		memcpy(m_apchain0.get(), chain, m_blockSize);
		memcpy(m_apchain.get(), chain, m_blockSize);	
	}
	if(true == bSameKey)
		//Fast Initialization
		return;
	if(NULL == m_apKey.get())
		m_apKey.reset(new char[m_keylength]);
	memcpy(m_apKey.get(), key, m_keylength);
	//Move Key Data into unsigned int array
	BytesToWord(reinterpret_cast<unsigned char const*>(&key[0]), m_auiKey[0]);
	BytesToWord(reinterpret_cast<unsigned char const*>(&key[4]), m_auiKey[1]);
	BytesToWord(reinterpret_cast<unsigned char const*>(&key[8]), m_auiKey[2]);
	BytesToWord(reinterpret_cast<unsigned char const*>(&key[12]), m_auiKey[3]);
	//Initialization Flag
	m_bInit = true;
}

//Resetting the Initialization Vector
void CTEA::ResetChain()
{
	if(false==m_bInit)
		throw runtime_error(string(sm_szErrorMsg1));
	memcpy(m_apchain.get(), m_apchain0.get(), m_blockSize);
}

//Compute Signature
void CTEA::Signature(char* pcSig)
{
	//3+16+1+1+1
	char acSigData[23] = {0};
	strcat(acSigData, "TEA");
	int iLen = static_cast<int>(strlen(acSigData));
	memcpy(acSigData+iLen, m_apKey.get(), m_keylength);
	sprintf(acSigData+iLen+m_keylength, "%d%d", m_iMode, m_iPadding);
	CSHA oSHA;
	oSHA.AddData(acSigData, static_cast<int>(strlen(acSigData)));
	oSHA.FinalDigest(pcSig);
}

//Encrypting Blocks of 64 bits (8 bytes)
void CTEA::EncryptBlock(unsigned char const* pucIn, unsigned char* pucOut)
{
	unsigned int v[2];
	unsigned int w[2];
	BytesToWord(&pucIn[0], v[0]);
	BytesToWord(&pucIn[4], v[1]);
	register unsigned int y=v[0], z=v[1], sum=0, delta=0x9E3779B9, n=32;
	while(n-->0)
	{
		y += (z << 4 ^ z >> 5) + z ^ sum + m_auiKey[sum&3];
		sum += delta;
		z += (y << 4 ^ y >> 5) + y ^ sum + m_auiKey[sum>>11 & 3];
	}
	w[0]=y; w[1]=z;
	WordToBytes(w[0], &pucOut[0]);
	WordToBytes(w[1], &pucOut[4]);
}

//Decrypting Blocks of 64 bits (8 bytes)
void CTEA::DecryptBlock(unsigned char const* pucIn, unsigned char* pucOut)
{
	unsigned int v[2];
	unsigned int w[2];
	BytesToWord(&pucIn[0], v[0]);
	BytesToWord(&pucIn[4], v[1]);
	register unsigned int y=v[0], z=v[1], sum=0xC6EF3720, delta=0x9E3779B9, n=32;
	while(n-->0)
	{
		z -= (y << 4 ^ y >> 5) + y ^ sum + m_auiKey[sum>>11 & 3];
		sum -= delta;
		y -= (z << 4 ^ z >> 5) + z ^ sum + m_auiKey[sum&3];
	}
	w[0]=y; w[1]=z;
	WordToBytes(w[0], &pucOut[0]);
	WordToBytes(w[1], &pucOut[4]);
}

void CTEA::Encrypt(char const* in, char* result, size_t n)
{
	if(false==m_bInit)
		throw runtime_error(string(sm_szErrorMsg1));
	//n should be > 0 and multiple of m_blockSize
	if(n<1 || n%m_blockSize!=0)
		throw runtime_error(string(sm_szErrorMsg6));
	int i;
	char const* pin;
	char* presult;
	if(CBC == m_iMode) //CBC mode, using the Chain
	{
		for(i=0,pin=in,presult=result; i<n/m_blockSize; i++)
		{
			Xor(m_apchain.get(), pin);
			EncryptBlock(reinterpret_cast<unsigned char*>(m_apchain.get()), reinterpret_cast<unsigned char*>(presult));
			memcpy(m_apchain.get(), presult, m_blockSize);
			pin += m_blockSize;
			presult += m_blockSize;
		}
	}
	else if(CFB == m_iMode) //CFB mode, using the Chain
	{
		for(i=0,pin=in,presult=result; i<n/m_blockSize; i++)
		{
			EncryptBlock(reinterpret_cast<unsigned char*>(m_apchain.get()), reinterpret_cast<unsigned char*>(presult));
			Xor(presult, pin);
			memcpy(m_apchain.get(), presult, m_blockSize);
			pin += m_blockSize;
			presult += m_blockSize;
		}
	}
	else //ECB mode, not using the Chain
	{
		for(i=0,pin=in,presult=result; i<n/m_blockSize; i++)
		{
			EncryptBlock(reinterpret_cast<unsigned char const*>(pin), reinterpret_cast<unsigned char*>(presult));
			pin += m_blockSize;
			presult += m_blockSize;
		}
	}
}

void CTEA::Decrypt(char const* in, char* result, size_t n)
{
	if(false==m_bInit)
		throw runtime_error(string(sm_szErrorMsg1));
	//n should be > 0 and multiple of m_blockSize
	if(n<1 || n%m_blockSize!=0)
		throw runtime_error(string(sm_szErrorMsg6));
	int i;
	char const* pin;
	char* presult;
	if(CBC == m_iMode) //CBC mode, using the Chain
	{
		for(i=0,pin=in,presult=result; i<n/m_blockSize; i++)
		{
			DecryptBlock(reinterpret_cast<unsigned char const*>(pin), reinterpret_cast<unsigned char*>(presult));
			Xor(presult, m_apchain.get());
			memcpy(m_apchain.get(), pin, m_blockSize);				
			pin += m_blockSize;
			presult += m_blockSize;
		}
	}
	else if(CFB == m_iMode) //CFB mode, using the Chain, not using Decrypt()
	{
		for(i=0,pin=in,presult=result; i<n/m_blockSize; i++)
		{
			EncryptBlock(reinterpret_cast<unsigned char*>(m_apchain.get()), reinterpret_cast<unsigned char*>(presult));
			//memcpy(presult, pin, m_blockSize);
			Xor(presult, pin);
			memcpy(m_apchain.get(), pin, m_blockSize);
			pin += m_blockSize;
			presult += m_blockSize;
		}
	}
	else //ECB mode, not using the Chain
	{
		for(i=0,pin=in,presult=result; i<n/m_blockSize; i++)
		{
			DecryptBlock(reinterpret_cast<unsigned char const*>(pin), reinterpret_cast<unsigned char*>(presult));
			pin += m_blockSize;
			presult += m_blockSize;
		}
	}
}

void CTEA::EncryptFile(string const& rostrFileIn, string const& rostrFileOut)
{
	if(false==m_bInit)
		throw runtime_error(string(sm_szErrorMsg1));
	//Check if the same file for input and output
	if(rostrFileIn == rostrFileOut)
	    {
        std::string msg(sm_szErrorMsg8);
        msg += rostrFileIn;
        msg += "!\n";
		throw runtime_error(msg);
	    }
	//Open Input File
	ifstream in(rostrFileIn.c_str(), ios::binary);
	if(!in)
	    {
        std::string msg(sm_szErrorMsg7);
        msg += rostrFileIn;
        msg += "!\n";
		throw runtime_error(msg);
	    }
	//Open Output File
	ofstream out(rostrFileOut.c_str(), ios::binary);
	if(!out)
	    {
        std::string msg(sm_szErrorMsg7);
        msg += rostrFileOut;
        msg += "!\n";
        throw runtime_error(msg);
	    }
	//Computing the signature
	char acSig[33] = {0};
	Signature(acSig);
	//Writing the Signature
	out.write(acSig, 32);
	//Resetting the chain
	ResetChain();
	//Reading from file
	char szLargeBuff[BUFF_LEN+1] = {0};
	char szBuffIn[DATA_LEN+1] = {0};
	char szBuffOut[DATA_LEN+1] = {0};
	CDoubleBuffering oDoubleBuffering(in, szLargeBuff, BUFF_LEN, DATA_LEN);
	int iRead;
	while((iRead=oDoubleBuffering.GetData(szBuffIn)) > 0)
	{
		if(iRead < DATA_LEN)
			iRead = Pad(szBuffIn, iRead);
		//Encrypting
		Encrypt(szBuffIn, szBuffOut, iRead);
		out.write(szBuffOut, iRead);
	}
	in.close();
	out.close();
}

void CTEA::DecryptFile(string const& rostrFileIn, string const& rostrFileOut)
{
	if(false==m_bInit)
		throw runtime_error(string(sm_szErrorMsg1));
	//Check if the same file for input and output
	if(rostrFileIn == rostrFileOut)
        {
        std::string msg(sm_szErrorMsg8);
        msg += rostrFileIn;
        msg += "!\n";
        throw runtime_error(msg);
        }
	//Open Input File
	ifstream in(rostrFileIn.c_str(), ios::binary);
	if(!in)
        {
        std::string msg(sm_szErrorMsg7);
        msg += rostrFileIn;
        msg += "!\n";
        throw runtime_error(msg);
        }
	//Open Output File
	ofstream out(rostrFileOut.c_str(), ios::binary);
	if(!out)
        {
        std::string msg(sm_szErrorMsg7);
        msg += rostrFileOut;
        msg += "!\n";
        throw runtime_error(msg);
        }
	//Computing the signature
	char acSig[33] = {0};
	Signature(acSig);
	char acSig1[33] = {0};
	//Reading the Signature
	in.read(acSig1, 32);
	//Compare the signatures
	if(memcmp(acSig1, acSig, 32) != 0)
        {
        std::string msg(sm_szErrorMsg9);
        msg += rostrFileIn;
        msg += sm_szErrorMsg10;
        msg += "\n";
        throw runtime_error(msg);
        }

	//Resetting the chain
	ResetChain();
	//Reading from file
	char szLargeBuff[BUFF_LEN+1] = {0};
	char szBuffIn[DATA_LEN+1] = {0};
	char szBuffOut[DATA_LEN+1] = {0};
	CDoubleBuffering oDoubleBuffering(in, szLargeBuff, BUFF_LEN, DATA_LEN);
	int iRead;
	while((iRead=oDoubleBuffering.GetData(szBuffIn)) > 0)
	{
		//Encrypting
		Decrypt(szBuffIn, szBuffOut, iRead);
		out.write(szBuffOut, iRead);
	}
	in.close();
	out.close();
}


