package com.bentley.loadprojects;

/**
 * Created by Robert.Priest on 3/7/2017.
 */

public final class ConnectJNI {

    public static native String GetConnectUser();
    public static native String[] GetProjects();
    public static native void Initialize(String homeDir, String tempDir, String externalStorageDir, String deviceId);

}
