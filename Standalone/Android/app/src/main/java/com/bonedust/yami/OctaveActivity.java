package com.bonedust.standalone;

import android.app.NativeActivity;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.MulticastLock;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import com.bonedust.standalone.databinding.ActivityMainBinding;

public class OctaveActivity extends NativeActivity {

    // Used to load the 'standalone' library on application startup.
    static {
        System.loadLibrary("standalone");
    }

    private ActivityMainBinding binding;

    private WifiManager wifiManager;
    private MulticastLock multicastLock;

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        int SDK_INT = android.os.Build.VERSION.SDK_INT;
        if (SDK_INT >= 19) {
            setImmersiveSticky();

            View decorView = getWindow().getDecorView();
            decorView.setOnSystemUiVisibilityChangeListener
                    (new View.OnSystemUiVisibilityChangeListener() {
                        @Override
                        public void onSystemUiVisibilityChange(int visibility) {
                            setImmersiveSticky();
                        }
                    });
        }

        super.onCreate(savedInstanceState);
    }

    @Override
    protected void onResume() {
        //Hide toolbar
        int SDK_INT = android.os.Build.VERSION.SDK_INT;
        if (SDK_INT >= 11 && SDK_INT < 14) {
            getWindow().getDecorView().setSystemUiVisibility(View.STATUS_BAR_HIDDEN);
        } else if (SDK_INT >= 14 && SDK_INT < 19) {
            getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_LOW_PROFILE);
        } else if (SDK_INT >= 19) {
            setImmersiveSticky();
        }
        super.onResume();

        // Acquire multicast lock. In the future, maybe only do this when searching for LAN sessions.
        // If we don't acquire a multicast lock then we won't be able to receive LAN session broadcasts.
        if (wifiManager == null)
        {
            wifiManager = (WifiManager) getSystemService(Context.WIFI_SERVICE);
        }

        if (multicastLock == null)
        {
            multicastLock = wifiManager.createMulticastLock("Octave");
            multicastLock.setReferenceCounted(true);
        }

        if (multicastLock != null && !multicastLock.isHeld())
        {
            multicastLock.acquire();
        }
    }

    @Override
    protected void onPause()
    {
        super.onPause();

        if (multicastLock != null && multicastLock.isHeld())
        {
            multicastLock.release();
        }
    }

    void setImmersiveSticky() {
        View decorView = getWindow().getDecorView();
        decorView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
    }

    void setSystemOrientation(int orientation)
    {
        if (orientation == 0) {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }
        else if (orientation == 1)
        {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        }
        else {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_USER);
        }
    }
}