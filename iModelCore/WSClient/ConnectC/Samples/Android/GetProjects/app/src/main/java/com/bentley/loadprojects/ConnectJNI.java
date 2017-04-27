package com.bentley.loadprojects;

import java.util.HashMap;

/**
 * Created by Robert.Priest on 3/7/2017.
 */

public final class ConnectJNI {

    public static native String GetConnectUser();
    public static native HashMap<String, String> GetProjects();
    public static native HashMap<Integer, String> GetProjectProperties(String projectId);
    public static native void Initialize(String homeDir, String tempDir, String externalStorageDir, String deviceId);

}
