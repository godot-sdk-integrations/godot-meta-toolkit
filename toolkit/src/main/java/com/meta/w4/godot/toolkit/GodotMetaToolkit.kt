package com.meta.w4.godot.toolkit

import android.app.Activity
import android.util.Log
import org.godotengine.godot.Godot
import org.godotengine.godot.plugin.GodotPlugin

/**
 * Godot plugin for the Godot Meta Toolkit
 */
class GodotMetaToolkit(godot: Godot?) : GodotPlugin(godot) {
    companion object {
        private val TAG = GodotMetaToolkit::class.java.simpleName

        init {
            try {
                Log.v(TAG, "Loading godot_meta_toolkit library")
                System.loadLibrary("godot_meta_toolkit")
            } catch (e: UnsatisfiedLinkError) {
                Log.e(TAG, "Unable to load godot_meta_toolkit shared library")
            }
        }
    }

    override fun getPluginName() = "GodotMetaToolkit"

    override fun getPluginGDExtensionLibrariesPaths() = setOf("res://addons/godot_meta_toolkit/toolkit.gdextension")

    override fun onGodotSetupCompleted() {
        initPlatformSDK(getActivity());
    }

    private external fun initPlatformSDK(activity: Activity?);
}
