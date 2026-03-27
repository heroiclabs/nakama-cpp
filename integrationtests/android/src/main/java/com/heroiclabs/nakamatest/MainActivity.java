package com.heroiclabs.nakamatest;

import android.app.Activity;
import android.util.Log;
import android.os.Bundle;

public class MainActivity extends Activity {
    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        Log.i("libnakama-test MainActivity", "Starting tests in background thread...");
        
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    Log.i("libnakama-test MainActivity", "Loading libnakama...");
                    System.loadLibrary("nakama-sdk");
                    Log.i("libnakama-test MainActivity", "Loading libnakama-test...");
                    System.loadLibrary("nakama-test");
                } catch (Exception e) {
                    Log.e("libnakama-test MainActivity", "Failed to load libraries: " + e.getMessage());
                }
            }
        }).start();
    }
}