package com.meta.w4.godot.toolkit

import android.app.Activity
import android.content.ComponentName
import android.content.Intent
import android.util.Log
import android.view.View;
import org.godotengine.godot.Godot
import org.godotengine.godot.plugin.GodotPlugin
import org.godotengine.godot.plugin.UsedByGodot

/**
 * Godot plugin for the Godot Meta Toolkit
 */
class GodotMetaToolkit(godot: Godot?) : GodotPlugin(godot) {
    companion object {
        private val TAG = GodotMetaToolkit::class.java.simpleName

        private const val IMMERSIVE_ACTIVITY = "com.godot.game.GodotApp"
        private const val PANEL_ACTIVITY = "com.meta.w4.godot.toolkit.GodotPanelApp"

        private const val IMMERSIVE_MODE = 0
        private const val PANEL_MODE = 1

        private const val INTENT_EXTRA_DATA = "godot_meta_toolkit_data"

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

    private var hybridMode: Int = IMMERSIVE_MODE
    private var hybridLaunchData: String? = null

    override fun onMainCreate(activity: Activity): View? {
        if (activity::class.qualifiedName == PANEL_ACTIVITY) {
            hybridMode = PANEL_MODE
        } else {
            hybridMode = IMMERSIVE_MODE
        }

        hybridLaunchData = activity.intent.getStringExtra(INTENT_EXTRA_DATA)

        return null
    }

    @UsedByGodot
    fun hybridAppSwitchTo(mode: Int, data: String = ""): Boolean {
        if (hybridMode == mode) return false

        val context = getActivity()
        if (context == null) return false

        val activityName = if (mode == 0) IMMERSIVE_ACTIVITY else PANEL_ACTIVITY
        val newInstance = Intent()
            .setComponent(ComponentName(context, activityName))
            .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
            .putExtra(INTENT_EXTRA_DATA, data)

        val godot = godot
        if (godot != null) {
            godot.destroyAndKillProcess {
                context.startActivity(newInstance)
            }
        } else {
            context.startActivity(newInstance)
            context.finish()
        }

        return true
    }

    @UsedByGodot
    fun getHybridAppLaunchData(): String {
        val data = hybridLaunchData
        return data ?: ""
    }
}
