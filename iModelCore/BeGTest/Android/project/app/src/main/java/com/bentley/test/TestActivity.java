package com.bentley.test;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONTokener;

import android.app.Activity;
import android.app.KeyguardManager;
import android.app.KeyguardManager.KeyguardLock;
import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.os.Bundle;
import android.preference.*;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.util.Log;

public class TestActivity extends Activity implements SurfaceHolder.Callback 
{
    private Surface     m_surface = null;
    private WakeLock    m_wakeLock = null;
    private static boolean s_didExtractAssets;
    private static boolean s_didInitialize;

    private static final String     ASSETS_MANIFEST = "BeTestAndroid.manifest";
    private static final String     ASSETS_HASH_KEY = "BeTestAndroidAssetsHashes";
    private static final String     LOG_TAG = "TestRunner";

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate (savedInstanceState);
        setContentView (R.layout.main);
        }

    /** Called by unit tests that require a surface */
    public void getSurfaceInternal () 
        {
        if (m_surface != null)
            return;

        Log.d ("BeTestX", "getSurfaceInternal");
        
        KeyguardManager keyGuardManager = (KeyguardManager) getSystemService (KEYGUARD_SERVICE);
        KeyguardLock    lock = keyGuardManager.newKeyguardLock (getClass ().getName ());
        lock.disableKeyguard ();
        
        PowerManager pm = (PowerManager) getSystemService (POWER_SERVICE);
        m_wakeLock = pm.newWakeLock (PowerManager.SCREEN_DIM_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP, "TestActivity.m_wakeLock");
        m_wakeLock.acquire();
        
        SurfaceView   view   = (SurfaceView) findViewById (R.id.surfaceView);
        SurfaceHolder holder = view.getHolder ();
        holder.addCallback (this);
        m_surface = holder.getSurface ();
        }

    /** Called by test setUp methods to make sure assets are extracted. */
    public void extractAssets ()
    	{
    	if (s_didExtractAssets)
            return;

    	extractAssets(this, getApplicationInfo().dataDir, ASSETS_MANIFEST);
    	s_didExtractAssets = true;
    	}
    
    /**
     * Extracts assets from the APK to the "{applicationDirectory}" directory.
     * @param context           The context, typically the Activity. Cannot be null.
     * @param manifestFileName  Name of the assets' manifest file. The first line in the file
     *                          should contain identification string (usually a hash value) that's used
     *                          to check if the assets have changed. All other lines list paths to the assets.
     */
    private static void extractAssets (TestActivity context, String destDir, String manifestFileName)
        {
        SharedPreferences preferences   = PreferenceManager.getDefaultSharedPreferences (context);
        
        // get already extracted resources' hash values
        JSONObject hashes = null;
        try 
            {
            String serializedSavedHashes = preferences.getString (ASSETS_HASH_KEY, "");
            JSONTokener tokener = new JSONTokener (serializedSavedHashes);
            hashes = (JSONObject) (tokener.more () ? tokener.nextValue () : new JSONObject ());
            }
        catch (Exception e)
            {
            Log.w (LOG_TAG, "JSON of manifests' hashes is malformed. Starting from scratch.");
            hashes = new JSONObject ();
            }
                
        // read manifest file
        String       assetsHash = "";
        List<String> assets     = new ArrayList<String> ();
        try 
            {
            BufferedReader reader = new BufferedReader (new InputStreamReader (context.getAssets ().open (manifestFileName)));
            
            // the first line contains combined hash value of all assets
            assetsHash = reader.readLine ();
            if (hashes.has (manifestFileName) && hashes.getString (manifestFileName).equals (assetsHash))
                {
                Log.d (LOG_TAG, "Assets from manifest [" + manifestFileName + "] match, skipping extraction.");
                reader.close ();
                return;
                }
            
            // if hash values don't match, extract all assets;
            // the following lines contain paths to assets 
            Log.i (LOG_TAG, "Assets from manifest [" + manifestFileName + "] mismatch, extracting resource files...");
            String line;
            while (null != (line = reader.readLine ()))
                assets.add (line.replace ("\\", File.separator));
            reader.close ();
            }
        catch (IOException e)
            {
            Log.e (LOG_TAG, "Failed to open [" + manifestFileName + "] manifest file.");
            return;
            }
        catch (JSONException e)
            {
            // this should never happen
            }
        
        int numberOfAssetsExtracted = 0;
        for (String assetPath : assets)
            {
            // create destination directory structure
            File destAsset = new File (destDir + File.separator + assetPath);
            if (!destAsset.getParentFile ().exists ())
                {
                if (!destAsset.getParentFile ().mkdirs ())
                    {
                    Log.e (LOG_TAG, "Unable to create directory structure for resource file \"" + destAsset.getAbsolutePath() + "\".");
                    continue;
                    }
                }
            
            try
                {
                // extract the file
                InputStream      iStream = context.getAssets ().open (assetPath);
                FileOutputStream oStream = new FileOutputStream (destAsset);
                
                int bytesRead;
                byte[] buffer = new byte[1024];
                while (-1 != (bytesRead = iStream.read (buffer)))
                    oStream.write (buffer, 0, bytesRead);
                
                iStream.close ();
                oStream.close ();
                
                numberOfAssetsExtracted++;
                Log.i (LOG_TAG, "Resource file \"" + destAsset.getAbsolutePath() + "\" extracted.");
                }
            catch (IOException e)
                {
                Log.e (LOG_TAG, "Failed to extract resource file \"" + destAsset.getAbsolutePath() + "\": " + e.toString ());
                }
            }
            
        if (numberOfAssetsExtracted == assets.size ())
            {
            try
                {
                hashes.put (manifestFileName, assetsHash);
                preferences.edit ().putString (ASSETS_HASH_KEY, hashes.toString ()).commit ();
                }
            catch (JSONException e)
                {
                Log.e (LOG_TAG, "Failed to save manifest's hash value.");
                }
            Log.i (LOG_TAG, "All resources extracted.");
            }
        else
            Log.e (LOG_TAG, "Not all resources were extracted.");
        }

    public static void initialize(Context context)
        {
        if (s_didInitialize)
            {
            File dataDir = new File (context.getApplicationInfo ().dataDir);
            if (!dataDir.isDirectory())
                Log.e (LOG_TAG, "dataDir does not exist!");
            return;
            }

        String appDataDir = context.getApplicationInfo().dataDir;
        String cacheDir = context.getCacheDir().getAbsolutePath();

        try {
            File fileInDataDir = new File (appDataDir, "test_test");
            fileInDataDir.createNewFile();
            if (fileInDataDir.exists())
                Log.d (LOG_TAG, "Created file in dataDir");
            }
        catch (IOException e)
            {
            Log.d (LOG_TAG, "Failed to create file in dataDir");
            }

        try {
            File fileInCacheDir = new File (context.getCacheDir(), "test_test");
            fileInCacheDir.createNewFile();
            if (fileInCacheDir.exists())
                Log.d (LOG_TAG, "Created file in cacheDir");
            }
        catch (IOException e)
            {
            Log.d (LOG_TAG, "Failed to create file in cacheDir");
            }

                    /* assets       docs                    temp      localState */
        initializeJni (appDataDir, appDataDir+"/Documents", cacheDir, appDataDir);
        s_didInitialize = true;
        }
    private static native void initializeJni (String appDir, String docsDir, String tempDir, String localStateDir);
    
    public static native void initializeSqlang ();
    public static native void uninitializeSqlang ();
    
    @Override
    protected void onDestroy() 
        {
        if (m_wakeLock != null)
            m_wakeLock.release ();
        m_wakeLock = null;
        m_surface = null;
        super.onDestroy ();
        }
    
    public Surface getSurface ()
        {
        getSurfaceInternal ();

        if (!m_surface.isValid ())
            {
            synchronized (this) 
                { 
                    try {
                        Log.w ("TestActivity", "Surface is not yet valid. Maybe the device is locked? Waiting..");
                        wait (); 
                        } 
                    catch (InterruptedException e) {} 
                }
            }
    	return m_surface;
        }

    @Override
    public void surfaceChanged (SurfaceHolder holder, int format, int width, int height) { synchronized (this) { notify (); } }
    @Override
    public void surfaceCreated (SurfaceHolder holder) { synchronized (this) { notify (); } }
    @Override
    public void surfaceDestroyed (SurfaceHolder holder) { }
}
