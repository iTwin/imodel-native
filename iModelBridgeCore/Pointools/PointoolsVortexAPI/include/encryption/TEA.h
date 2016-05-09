
//Rijndael.h

#ifndef __TEA_H__
#define __TEA_H__

//Typical DISCLAIMER:
//The code in this project is Copyright (C) 2003 by George Anescu. You have the right to
//use and distribute the code in any way you see fit as long as this paragraph is included
//with the distribution. No warranties or claims are made as to the validity of the
//information and code contained herein, so use it at your own risk.

#include <encryption/Method.h>
#include <memory>

//The Tiny Encryption Algorithm (TEA)
//
//The Tiny Encryption Algorithm is one of the fastest and most efficient cryptographic algorithms in
//existence. It was developed by David Wheeler and Roger Needham at the Computer Laboratory of Cambridge
//University. It is a Feistel cipher which uses operations from mixed (orthogonal) algebraic groups
//- XORs and additions in this case. It encrypts 64 data bits at a time using a 128-bit key. It seems
//highly resistant to differential cryptanalysis, and achieves complete diffusion (where a one bit
//difference in the plaintext will cause approximately 32 bit differences in the ciphertext) after only
//six rounds. Performance on a modern desktop computer or workstation is very impressive. 
//
//How secure is TEA?
//Very. There have been no known successful cryptanalyses of TEA. It's believed (by James Massey) to be
//as secure as the IDEA algorithm, designed by Massey and Xuejia Lai. It uses the same mixed algebraic
//groups technique as IDEA, but it's very much simpler, hence faster. Also it's public domain, whereas
//IDEA is patented by Ascom-Tech AG in Switzerland. IBM's Don Coppersmith and Massey independently
//showed that mixing operations from orthogonal algebraic groups performs the diffusion and confusion
//functions that a traditional block cipher would implement with P- and S-boxes. As a simple plug-in
//encryption routine, it's great. The code is lightweight and portable enough to be used just about
//anywhere. It even makes a great random number generator for Monte Carlo simulations and the like. The
//minor weaknesses identified by David Wagner at Berkeley are unlikely to have any impact in the real
//world, and you can always implement the new variant TEA which addresses them. If you want a
//low-overhead end-to-end cipher (for real-time data, for example), then TEA fits the bill.
//TEST
//
// KEY       IN                OUT
// aaaabbbb  0000000000000000  23362594D87140F7
// aaaabbbb  0100000000000000  D21CE52C43A62329
// aaaabbbb  aaaaaaaa          CE874919B1DAA54A

class CTEA : public CEncryptMethod
{
private:
	enum { DATA_LEN=384, BUFF_LEN=1024 };
	enum { BLOCK_SIZE=8 };
	enum { KEY_LENGTH=16 };

public:
	//CONSTRUCTOR
	CTEA();
	//Expand a user-supplied key material into a session key.
	// keydata - The key material expanded to 128-bit (16 bytes) user-key to use.
	// keydatalength - how much to take from keydata
	// chain - initial chain block for CBC and CFB modes.
	// iMode=ECB  - Operation Mode
	// iPadding   - Padding Mode
	// key length is fixed to 16
	// blockSize is fixed to 8
	void Initialize(char const* keydata, int keydatalength, char const* chain=sm_chain0,
		int iMode=ECB, int iPadding=ZEROES);

	//Resetting the Initialization Vector
	void ResetChain();
	//Encryption for a string of chars
	void Encrypt(char const* in, char* result, size_t n);
	//Decryption for a string of chars
	void Decrypt(char const* in, char* result, size_t n);
	//Encryption for a File
	void EncryptFile(string const& rostrFileIn, string const& rostrFileOut);
	//Decryption for a File
	void DecryptFile(string const& rostrFileIn, string const& rostrFileOut);

private:
	//Encrypting Blocks of 64 bits (8 bytes)
	void EncryptBlock(unsigned char const* pucIn, unsigned char* pucOut);
	//Decrypting Blocks of 64 bits (8 bytes)
	void DecryptBlock(unsigned char const* pucIn, unsigned char* pucOut);
	//Compute Signature
	void Signature(char* pcSig);
	//Key
    std::unique_ptr<char> m_apKey;
	unsigned int m_auiKey[KEY_LENGTH/4]; //128 bits key (16 bytes)
	//Chain Block
	std::unique_ptr<char> m_apchain0;
    std::unique_ptr<char> m_apchain;

public:
	//Null chain
	static char const sm_chain0[BLOCK_SIZE];
};

#endif // __TEA_H__

