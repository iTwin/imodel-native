/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
package com.bentley.android.security;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Build;
import android.security.KeyPairGeneratorSpec;
import android.util.Base64;
import android.util.Log;
import com.bentley.android.security.KeyStorePre18;
import java.lang.String;
import java.math.BigInteger;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.KeyStore;
import java.security.SecureRandom;
import java.util.Calendar;
import java.util.GregorianCalendar;
import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import javax.security.auth.x500.X500Principal;

/**
 * Class for securely encrypting and decrypting vulnerable strings using Android OS capabilities.
 *
 * Decrypt() must be used with same keyAlias as Encrypt() in order to work on encrypted string.
 * Different keyAlias values can be used if different encryption keys are desired for different value domains.
 * 
 * Implementation summary:
 *      Value is encrypted using SecretKey that is generated for each new keyAlias.
 *      SecretKey is encrypted and stored in SharedPreferences for latter value decryption/encryption with same alias.
 *      SecretKey is encrypted and decrypted using KeyPair that is stored in AndroidKeyStore with same keyAlias.
 *      AndroidKeyStore is encrypted by OS, usually using device lock password.
 */

class KeyStoreCipher
    {
    private static SecretKey GetSecretKey (String alias) throws Exception
        {
        if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.JELLY_BEAN_MR2)
            {
            return GetSecretKeyUsingPre18Api(alias);
            }
        return GetSecretKeyUsingAndroidKeyStore(alias);
        }

    private static SecretKey GetSecretKeyUsingPre18Api (String alias) throws Exception
        {
        // AndroidKeyStore provider is not available - use private APIs for system keystore access

        KeyStorePre18 ks = KeyStorePre18.getInstance();

        if (KeyStorePre18.State.UNINITIALIZED == ks.state())
            {
            // Force user to set lock screen password/pin/pattern in order to use system keystore
            Intent intent = new Intent("com.android.credentials.UNLOCK");
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            BeSecurity.getApplicationContext().startActivity(intent);
            System.exit(0);
            }

        byte[] storedKey = ks.get(alias);
        if (null == storedKey)
            {
            SecretKey key = GenerateNewSecretKey();
            if (!ks.put(alias, key.getEncoded()))
                {
                throw new RuntimeException("Failed to store key in KeyStore: " + ks.errorToStr(ks.getLastError()));
                }

            storedKey = ks.get(alias);
            }
            
        return new SecretKeySpec(storedKey, "AES");
        }

    private static SecretKey GetSecretKeyUsingAndroidKeyStore (String alias) throws Exception
        {
        SharedPreferences sp = BeSecurity.getApplicationContext().getSharedPreferences("KeyStoreCipher", Context.MODE_PRIVATE);

        String storedKey = sp.getString(alias, null);
        if (storedKey == null)
            {
            SecretKey key = GenerateNewSecretKey();
            String encryptedKey = EncryptSecretKey(key.getEncoded(), alias);

            SharedPreferences.Editor editor = sp.edit();
            editor.putString(alias, encryptedKey);
            editor.commit();

            storedKey = sp.getString(alias, null);
            }

        return new SecretKeySpec(DecryptSecretKey(storedKey, alias), "AES");
        }

    private static SecretKey GenerateNewSecretKey() throws Exception
        {
        KeyGenerator generator = KeyGenerator.getInstance("AES");
        generator.init(128);
        return generator.generateKey();
        }

    public static String EncryptSecretKey (byte[] keyToEncrypt, String alias) throws Exception
        {
        KeyStore.PrivateKeyEntry keyEntry = GetKeyEntry(alias);

        Cipher cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
        cipher.init(Cipher.ENCRYPT_MODE, keyEntry.getCertificate().getPublicKey());

        byte[] encryptedBytes = cipher.doFinal(keyToEncrypt);
        return Base64.encodeToString(encryptedBytes, Base64.DEFAULT);
        }

    public static byte[] DecryptSecretKey (String keyToDecrypt, String alias) throws Exception
        {
        KeyStore.PrivateKeyEntry keyEntry = GetKeyEntry(alias);

        Cipher cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
        cipher.init(Cipher.DECRYPT_MODE, keyEntry.getPrivateKey());

        byte[] encryptedBytes = Base64.decode(keyToDecrypt, Base64.DEFAULT);
        byte[] decryptedBytes = cipher.doFinal(encryptedBytes);
        return decryptedBytes;
        }

    private static KeyStore.PrivateKeyEntry GetKeyEntry (String alias) throws Exception
        {
        KeyStore keyStore = KeyStore.getInstance("AndroidKeyStore");
        keyStore.load(null);

        // Create the keys if necessary
        if (!keyStore.containsAlias(alias)) 
            {
            Calendar start = new GregorianCalendar();
            Calendar end = new GregorianCalendar();
            end.add(1, Calendar.YEAR);

            KeyPairGeneratorSpec spec = new KeyPairGeneratorSpec.Builder(BeSecurity.getApplicationContext())
                .setAlias(alias)
                .setSubject(new X500Principal("CN=" + alias))
                .setSerialNumber(BigInteger.ONE)
                .setStartDate(start.getTime())
                .setEndDate(end.getTime())
                .build();

            KeyPairGenerator generator = KeyPairGenerator.getInstance("RSA", "AndroidKeyStore");
            generator.initialize(spec);

            KeyPair keyPair = generator.generateKeyPair();
            }

        // Retrieve the keys
        return (KeyStore.PrivateKeyEntry)keyStore.getEntry(alias, null);
        }

    private static byte[] JoinArrays (byte[] one, byte[] two)
        {
        byte[] combined = new byte[one.length + two.length];
        System.arraycopy(one, 0, combined, 0, one.length);
        System.arraycopy(two, 0, combined, one.length, two.length);
        return combined;
        }

    public static String Encrypt(String input, String keyAlias)
        {
        try {
            SecretKey key = GetSecretKey (keyAlias);

            byte[] iv = new byte[16];
            new SecureRandom().nextBytes(iv);

            Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
            cipher.init(Cipher.ENCRYPT_MODE, key, new IvParameterSpec(iv));

            byte[] encryptedBytes = cipher.doFinal(input.getBytes("UTF-8"));
            byte[] dataToStore = JoinArrays(iv, encryptedBytes);
            return Base64.encodeToString(dataToStore, Base64.DEFAULT);
            }
        catch (Exception e)
            {
            Log.e("KeyStoreCipher", Log.getStackTraceString(e));
            return null;
            }
        }

    public static String Decrypt(String input, String keyAlias)
        {
        try {
            SecretKey key = GetSecretKey (keyAlias);

            byte[] storedData = Base64.decode(input, Base64.DEFAULT);

            byte[] iv = new byte[16];
            System.arraycopy(storedData, 0, iv, 0, 16);

            Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
            cipher.init(Cipher.DECRYPT_MODE, key, new IvParameterSpec(iv));

            byte[] decryptedBytes = cipher.doFinal(storedData, 16, storedData.length - 16);
            return new String(decryptedBytes, "UTF-8");
            }
        catch (Exception e)
            {
            Log.e("KeyStoreCipher", Log.getStackTraceString(e));
            return null;
            }
        }
    }
