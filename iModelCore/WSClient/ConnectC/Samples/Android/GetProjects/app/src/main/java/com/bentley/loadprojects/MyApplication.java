package com.bentley.loadprojects;

import android.app.Application;
import android.content.res.Configuration;

import java.util.HashMap;

public class MyApplication extends Application {
    private static MyApplication instance;
    private HashMap<String, String> m_projects = new HashMap<>();

    public void SetProjects(HashMap<String, String> projects){
        m_projects = projects;
    }

    public HashMap<String, String> GetProjects () {
        return m_projects;
    }

    public static MyApplication getInstance() {
        return instance;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        instance = this;
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    @Override
    public void onLowMemory() {
        super.onLowMemory();
    }

    @Override
    public void onTerminate() {
        super.onTerminate();
    }
}
