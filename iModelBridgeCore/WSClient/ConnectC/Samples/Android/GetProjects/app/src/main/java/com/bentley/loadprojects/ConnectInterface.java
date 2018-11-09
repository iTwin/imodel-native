package com.bentley.loadprojects;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.provider.Settings.Secure;
import android.util.Log;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Properties;
import java.util.List;
import java.util.ArrayList;

import static java.lang.System.in;

/**
 * Created by Robert.Priest on 3/7/2017.
 */

public final class ConnectInterface {
    private static final String TAG = "ConnectInterface";
    private static boolean s_initialized       = false;
    private static Context s_appContext        = null;

/* Do not log potentially sensitive information.
    private static void GetVariableValue(String args[]) {
        if( args.length == 0 ) {
            Properties p = System.getProperties();
            Enumeration keys = p.keys();
            while (keys.hasMoreElements()) {
                String key = (String)keys.nextElement();
                String value = (String)p.get(key);
                Log.d(TAG, key + " : " + value);
            }
        }
        else {
            for (String key: args) {
                Log.d(TAG, System.getProperty( key ));
            }
        }
    }
*/

    private static void loadLibraries () {
            // insert "marker" into log
            Log.i (TAG, "===================================================================");
            Log.i (TAG, "*******************************************************************");
            Log.i (TAG,"===================================================================");
            try
            {
                // shared libraries must be loaded in reverse dependency order
                System.loadLibrary ("c++_shared");
                try
                {
                    System.loadLibrary ("android");
                }
                catch (Throwable t)
                {
                }

                System.loadLibrary ("native-lib");
            }
            catch (UnsatisfiedLinkError e)
            {
                Log.e(TAG, e.toString ());
                Log.e(TAG, e.getMessage ());
                throw e;
            }
        }

    // Used to load the 'native-lib' library on application startup.
    static {
        Log.d(TAG, "About to load the native library...");
        // Do not log potentially sensitive information.
        // GetVariableValue(new String[] {"java.library.path"});
        loadLibraries();
    }

    public static String GetConnectUser() {
        return ConnectJNI.GetConnectUser();
    }

    public static HashMap<String, String> GetProjects() {
        return ConnectJNI.GetProjects();
    }

    public static HashMap<Integer, String> GetProjectProperties(String projectId) {
        return ConnectJNI.GetProjectProperties(projectId);
    }


    public static void Initialize (Context context) {
        if (!s_initialized) {
            s_initialized = true;
            s_appContext = context.getApplicationContext();

            // set standard directories based on where the application was installed
            String homeDirName = s_appContext.getApplicationInfo().dataDir;
            String tempDirName = s_appContext.getCacheDir().getAbsolutePath();
            String externalStorageDirName = Environment.getExternalStorageDirectory().getAbsolutePath();
            String deviceId = Secure.getString(s_appContext.getContentResolver(), Secure.ANDROID_ID);
            ConnectJNI.Initialize(homeDirName, tempDirName, externalStorageDirName, deviceId);
        }
    }
}
