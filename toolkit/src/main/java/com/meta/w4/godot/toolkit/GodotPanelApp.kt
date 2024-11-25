package com.meta.w4.godot.toolkit

import android.os.Bundle
import android.util.Log
import androidx.core.splashscreen.SplashScreen.Companion.installSplashScreen
import org.godotengine.godot.GodotActivity
import org.godotengine.godot.BuildConfig

/**
 * Activity for the Panel app mode in a hybrid app.
 */
class GodotPanelApp : GodotActivity() {
    companion object {
        private val TAG = GodotPanelApp::class.java.simpleName

        // .NET libraries.
        init {
            if (BuildConfig.FLAVOR == "mono") {
                try {
                    Log.v(TAG, "Loading System.Security.Cryptography.Native.Android library")
                    System.loadLibrary("System.Security.Cryptography.Native.Android")
                } catch (e: UnsatisfiedLinkError) {
                    Log.e(TAG, "Unable to load System.Security.Cryptography.Native.Android library")
                }
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        installSplashScreen()
        super.onCreate(savedInstanceState)
    }

    override fun getCommandLine(): MutableList<String> {
        // Force XR to be turned off.
        return mutableListOf("--xr_mode_regular", "--xr-mode", "off")
    }

    override fun supportsFeature(featureTag: String): Boolean {
        if ("meta_panel_app" == featureTag) {
            return true
        }

        return false
    }
}
