package com.heroiclabs.nakamatest;

import android.app.Activity;
import android.util.Log;
import android.os.Bundle;

public class MainActivity extends Activity {
    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        // load sdk first so that it may initialize before the test application
        Log.i("libnakama-test MainActivity", "Loading libnakama...");
        System.loadLibrary("nakama-sdk");
        Log.i("libnakama-test MainActivity", "Loading libnakama-test...");
        System.loadLibrary("nakama-test");
    }
}