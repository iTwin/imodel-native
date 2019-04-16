/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
package com.bentley.android.security;

import android.content.Context;

public final class BeSecurity
    {
    private static boolean s_initialized       = false;
    private static Context s_appContext        = null;
    /**
     * Must be called (once at startup) before it is valid to use the BeSecurity application framework.
     * @param context   The context, typically the Activity.  Cannot be null.
     */
    public static void initialize (Context context)
        {
        if (!s_initialized)
            {
            s_initialized = true;
            s_appContext = context.getApplicationContext();
            }
        }
    public static Context getApplicationContext()
        {
            return s_appContext;
        }
    }