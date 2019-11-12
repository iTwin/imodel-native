/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
package com.bentley.android.http;

import android.content.Context;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.X509TrustManager;
import java.security.cert.X509Certificate;
import java.security.KeyStore;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.Collections;
import java.util.Enumeration;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
public final class BeHttp
    {
    public static byte[][] ReadSystemTrustedCertificatesBinary()
        {
        try
            {
            ArrayList<X509Certificate> allCerts = new ArrayList<X509Certificate>();

            // Get system + user certificates
            KeyStore ks = KeyStore.getInstance("AndroidCAStore");
            if (ks == null)
                return null;

            ks.load(null, null);
            Enumeration aliases = ks.aliases();
            while (aliases.hasMoreElements())
                {
                String alias = (String) aliases.nextElement();
                X509Certificate cert = (X509Certificate) ks.getCertificate(alias);
                allCerts.add(cert);
                }

            byte[][] binaryCerts = new byte[allCerts.size()][0];

            int index = 0;
            for (X509Certificate cert : allCerts)
                binaryCerts[index++] = cert.getEncoded();

            return binaryCerts;
            }
        catch (Exception e)
            {
            // Ignore any exceptions to avoid handling them in C++, just return null as error indication
            }
        return null;
        }
    }