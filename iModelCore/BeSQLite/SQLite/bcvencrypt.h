/*
** 2023-06-20
**
******************************************************************************
**
** This file contains declarations for the public interface to code in
** bcvencrypt.c. Alternative encryption schemes may be supported by 
** replacing bcvencrypt.c with an alternative implementation of this
** interface. The four interface functions are:
**
**   bcvEncryptionKeyNew:
**     Convert a 128 bit (16-byte) binary key to a compiled key object of
**     type BcvEncryptionKey.
**
**   bcvEncryptionKeyFree:
**     Free a compiled key object created by bcvEncryptionKeyNew().
**
**   bcvEncrypt:
**     Encrypt a buffer using the supplied compiled key object.
**   
**   bcvDecrypt:
**     Decrypt a buffer using the supplied compiled key object.
**
** See below for futher details.
*/

/*
** Encryption uses 16 byte (128-bit) keys. This is part of the interface
** assumed by other modules in this project. It is not possible to change
** this simply by replacing the contents of bcvencrypt.c.
*/
#define BCV_KEY_SIZE 16

/*
** BcvEncryptionKey is an opaque type used to represent a compiled 
** BCV_KEY_SIZE byte binary encryption key. BcvEncryptionKey objects
** are allocated using bcvEncryptionKeyNew() and freed using
** bcvEncryptionKeyFree().
*/
typedef struct BcvEncryptionKey BcvEncryptionKey;

/*
** The argument passed to this function points to a buffer containing
** a BCV_KEY_SIZE byte binary key. This function allocates and returns a
** BcvEncryptionKey object that may be used to encrypt or decrypt buffers 
** using that key. If an out-of-memory or other error occurs, NULL is 
** returned.
*/
BcvEncryptionKey *bcvEncryptionKeyNew(const unsigned char *aKey);

/*
** Free an object returned by an earlier call to bcvEncryptionKeyNew().
*/
void bcvEncryptionKeyFree(BcvEncryptionKey *pKey);

/*
** Parameter aData points to a buffer nData bytes in size. This function
** encrypts the buffer in place. SQLITE_OK is returned if successful,
** or an SQLite error code otherwise.
**
** The buffer is encrypted according to the key passed as the first 
** argument. Parameter aNonce points to a BCV_KEY_SIZE buffer containing 
** a nonce, or initialization vector (IV), to use for the encryption.
*/
int bcvEncrypt(
  BcvEncryptionKey *pKey, 
  const unsigned char *aNonce, 
  unsigned char *aData, int nData
);

/*
** Parameter aData points to a buffer nData bytes in size. This function
** decrypts the buffer in place. SQLITE_OK is returned if successful,
** or an SQLite error code otherwise.
**
** The buffer is encrypted according to the key passed as the first 
** argument. Parameter aNonce points to a BCV_KEY_SIZE buffer containing 
** the nonce, or initialization vector (IV), that was passed to the 
** bcvEncrypt() function when the buffer was first encrypted.
*/
int bcvDecrypt(
  BcvEncryptionKey *pKey, 
  const unsigned char *aNonce, 
  unsigned char *aData, int nData
);


