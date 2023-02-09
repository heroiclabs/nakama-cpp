package com.nakamatest;

import android.app.Activity;

public class MainActivity extends Activity {
    static {
        // load sdk first so that it may initialize before the test application
        System.loadLibrary("libnakama-sdk.so");

        System.loadLibrary("libnakama-test.so");
    }
}