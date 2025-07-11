// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

#include "export/meta_toolkit_export_plugin.h"

#include <godot_cpp/classes/editor_export_platform_android.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/project_settings.hpp>

using namespace godot;

namespace {
static const char *TOOLKIT_BUILD_TEMPLATE_ZIP_PATH = "res://addons/godot_meta_toolkit/.build_template/quest_build_template.zip";
static const char *TOOLKIT_DEBUG_KEYSTORE_PATH = "res://addons/godot_meta_toolkit/.build_template/keystore/debug.keystore";
static const char *TOOLKIT_DEBUG_KEYSTORE_PROPERTIES_PATH = "res://addons/godot_meta_toolkit/.build_template/keystore/debug.keystore.properties";
} // namespace

MetaToolkitExportPlugin::MetaToolkitExportPlugin() {
	_enable_meta_toolkit_option = _generate_export_option(
			"meta_toolkit/enable_meta_toolkit",
			"",
			Variant::Type::BOOL,
			PROPERTY_HINT_NONE,
			"",
			PROPERTY_USAGE_DEFAULT,
			false,
			true);
}

void MetaToolkitExportPlugin::_bind_methods() {}

Dictionary MetaToolkitExportPlugin::_generate_export_option(const godot::String &p_name,
		const godot::String &p_class_name,
		Variant::Type p_type,
		godot::PropertyHint p_property_hint,
		const godot::String &p_hint_string,
		godot::PropertyUsageFlags p_property_usage,
		const godot::Variant &p_default_value,
		bool p_update_visibility) {
	Dictionary option_info;
	option_info["name"] = p_name;
	option_info["class_name"] = p_class_name;
	option_info["type"] = p_type;
	option_info["hint"] = p_property_hint;
	option_info["hint_string"] = p_hint_string;
	option_info["usage"] = p_property_usage;

	Dictionary export_option;
	export_option["option"] = option_info;
	export_option["default_value"] = p_default_value;
	export_option["update_visibility"] = p_update_visibility;

	return export_option;
}

bool MetaToolkitExportPlugin::_supports_platform(const Ref<godot::EditorExportPlatform> &p_platform) const {
	return p_platform->is_class(EditorExportPlatformAndroid::get_class_static());
}

bool MetaToolkitExportPlugin::_get_bool_option(const godot::String &p_option) const {
	Variant option_enabled = get_option(p_option);
	if (option_enabled.get_type() == Variant::Type::BOOL) {
		return option_enabled;
	}
	return false;
}

TypedArray<Dictionary> MetaToolkitExportPlugin::_get_export_options(const Ref<godot::EditorExportPlatform> &p_platform) const {
	TypedArray<Dictionary> export_options;
	if (!_supports_platform(p_platform)) {
		return export_options;
	}

	export_options.append(_enable_meta_toolkit_option);

	return export_options;
}

String MetaToolkitExportPlugin::_get_export_option_warning(
		const Ref<godot::EditorExportPlatform> &p_platform, const godot::String &p_option) const {
	return "";
}

Dictionary MetaToolkitExportPlugin::_get_export_options_overrides(
		const Ref<godot::EditorExportPlatform> &p_platform) const {
	Dictionary overrides;
	if (!_supports_platform(p_platform)) {
		return overrides;
	}

	// Check if this plugin is enabled
	if (!_get_bool_option("meta_toolkit/enable_meta_toolkit")) {
		return overrides;
	}

	// Gradle build overrides
	overrides["gradle_build/use_gradle_build"] = true;
	overrides["gradle_build/export_format"] = 0; // apk
	overrides["gradle_build/min_sdk"] = "29"; // Android 10
	overrides["gradle_build/target_sdk"] = "32"; // Android 12

	// Check if we have an alternate build template
	if (FileAccess::file_exists(TOOLKIT_BUILD_TEMPLATE_ZIP_PATH)) {
		overrides["gradle_build/gradle_build_directory"] = "res://addons/godot_meta_toolkit/build";
		overrides["gradle_build/android_source_template"] = TOOLKIT_BUILD_TEMPLATE_ZIP_PATH;
	}

	// Check if we have a debug keystore available
	if (FileAccess::file_exists(TOOLKIT_DEBUG_KEYSTORE_PATH) && FileAccess::file_exists(TOOLKIT_DEBUG_KEYSTORE_PROPERTIES_PATH)) {
		// Read the debug keystore user and password from the properties file
		// The file should contain the following lines:
		//    key.alias=platformkeystore
		//    key.alias.password=android
		Ref<FileAccess> file_access = FileAccess::open(TOOLKIT_DEBUG_KEYSTORE_PROPERTIES_PATH, FileAccess::ModeFlags::READ);
		if (file_access.is_valid()) {
			String debug_user;
			String debug_password;

			while (file_access->get_position() < file_access->get_length() && (debug_user.is_empty() || debug_password.is_empty())) {
				String current_line = file_access->get_line();
				PackedStringArray current_line_splits = current_line.split("=", false);
				if (current_line_splits.size() == 2) {
					if (current_line_splits[0] == "key.alias") {
						debug_user = current_line_splits[1];
					} else if (current_line_splits[0] == "key.alias.password") {
						debug_password = current_line_splits[1];
					}
				}
			}

			if (!debug_user.is_empty() && !debug_password.is_empty()) {
				overrides["keystore/debug"] = ProjectSettings::get_singleton()->globalize_path(TOOLKIT_DEBUG_KEYSTORE_PATH);
				overrides["keystore/debug_user"] = debug_user;
				overrides["keystore/debug_password"] = debug_password;
			}
		}
	}

	// Architectures overrides
	overrides["architectures/armeabi-v7a"] = false;
	overrides["architectures/arm64-v8a"] = true;
	overrides["architectures/x86"] = false;
	overrides["architectures/x86_64"] = false;

	// Package overrides
	overrides["package/show_in_android_tv"] = false;

	// Screen overrides
	overrides["screen/immersive_mode"] = true;
	overrides["screen/support_small"] = true;
	overrides["screen/support_normal"] = true;
	overrides["screen/support_large"] = true;
	overrides["screen/support_xlarge"] = true;

	// XR features overrides
	overrides["xr_features/xr_mode"] = 1; // OpenXR mode
	overrides["xr_features/enable_khronos_plugin"] = false;
	overrides["xr_features/enable_lynx_plugin"] = false;
	overrides["xr_features/enable_meta_plugin"] = true;
	overrides["xr_features/enable_pico_plugin"] = false;

	return overrides;
}

PackedStringArray MetaToolkitExportPlugin::_get_android_libraries(const Ref<godot::EditorExportPlatform> &p_platform, bool p_debug) const {
	PackedStringArray dependencies;
	if (!_supports_platform(p_platform)) {
		return dependencies;
	}

	// Check if the Godot Meta toolkit aar dependency is available
	const String debug_label = p_debug ? "debug" : "release";
	const String toolkit_aar_file_path = "res://addons/godot_meta_toolkit/.bin/android/" + debug_label + "/godot_meta_toolkit-" + debug_label + ".aar";
	if (FileAccess::file_exists(toolkit_aar_file_path)) {
		dependencies.append(toolkit_aar_file_path);
	}

	return dependencies;
}
