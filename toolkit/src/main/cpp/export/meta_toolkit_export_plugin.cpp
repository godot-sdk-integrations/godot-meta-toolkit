// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

#include "export/meta_toolkit_export_plugin.h"

#include "util.h"

#include <godot_cpp/classes/editor_export_platform_android.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/xr_interface.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace {
static const char *TOOLKIT_PREBUILT_DEBUG_TEMPLATE_PATH = "res://addons/godot_meta_toolkit/.build_template/prebuilt/debug/android_debug.apk";
static const char *TOOLKIT_PREBUILT_RELEASE_TEMPLATE_PATH = "res://addons/godot_meta_toolkit/.build_template/prebuilt/release/android_release.apk";
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

int MetaToolkitExportPlugin::_get_int_option(const godot::String &p_option, int default_value) const {
	Variant option_value = get_option(p_option);
	if (option_value.get_type() == Variant::Type::INT) {
		return option_value;
	}
	return default_value;
}

bool MetaToolkitExportPlugin::_is_plugin_enabled() const {
	return _get_bool_option("meta_toolkit/enable_meta_toolkit");
}

String MetaToolkitExportPlugin::_bool_to_string(bool p_value) const {
	return p_value ? "true" : "false";
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
	if (!_is_plugin_enabled()) {
		return overrides;
	}

	// Gradle build overrides
	overrides["gradle_build/export_format"] = 0; // apk
	bool is_mobile_editor = OS::get_singleton()->has_feature("mobile");
	if (!is_mobile_editor) {
		overrides["gradle_build/min_sdk"] = "29"; // Android 10
		overrides["gradle_build/target_sdk"] = "34"; // Android 14
		overrides["gradle_build/use_gradle_build"] = true;
	} else {
		overrides["gradle_build/compress_native_libraries"] = false;
	}

	// Check if we have a prebuilt template.
	if (FileAccess::file_exists(TOOLKIT_PREBUILT_DEBUG_TEMPLATE_PATH)) {
		overrides["custom_template/debug"] = TOOLKIT_PREBUILT_DEBUG_TEMPLATE_PATH;
	}
	if (FileAccess::file_exists(TOOLKIT_PREBUILT_RELEASE_TEMPLATE_PATH)) {
		overrides["custom_template/release"] = TOOLKIT_PREBUILT_RELEASE_TEMPLATE_PATH;
	}

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
	overrides["package/show_in_app_library"] = true;
	overrides["package/show_as_launcher_app"] = false;

	// Screen overrides
	overrides["screen/immersive_mode"] = true;
	overrides["screen/support_small"] = true;
	overrides["screen/support_normal"] = true;
	overrides["screen/support_large"] = true;
	overrides["screen/support_xlarge"] = true;

	// Gesture overrides
	overrides["gesture/swipe_to_dismiss"] = false;

	// XR features overrides
	overrides["xr_features/xr_mode"] = 1; // OpenXR mode
	overrides["xr_features/enable_khronos_plugin"] = false;
	overrides["xr_features/enable_lynx_plugin"] = false;
	overrides["xr_features/enable_meta_plugin"] = true;
	overrides["xr_features/enable_pico_plugin"] = false;
	overrides["xr_features/enable_magicleap_plugin"] = false;

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

PackedStringArray MetaToolkitExportPlugin::_get_supported_devices() const {
	PackedStringArray supported_devices;

	if (_get_bool_option("meta_xr_features/quest_1_support")) {
		supported_devices.append("quest");
	}

	if (_get_bool_option("meta_xr_features/quest_2_support")) {
		supported_devices.append("quest2");
	}

	if (_get_bool_option("meta_xr_features/quest_3_support")) {
		supported_devices.append("quest3");
		supported_devices.append("quest3s");
	}

	if (_get_bool_option("meta_xr_features/quest_pro_support")) {
		supported_devices.append("questpro");
	}

	return supported_devices;
}

void MetaToolkitExportPlugin::_get_manifest_entries(Vector<godot::String> &r_permissions, Vector<MetaToolkitExportPlugin::FeatureInfo> &r_features, Vector<MetadataInfo> &r_metadata) const {
	// Supported devices
	const String supported_devices = String("|").join(_get_supported_devices());
	MetadataInfo supported_devices_metadata = {
		"com.oculus.supportedDevices",
		supported_devices
	};
	r_metadata.append(supported_devices_metadata);

	// Check for eye tracking.
	bool eye_tracking_enabled = ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/eye_gaze_interaction");
	if (eye_tracking_enabled) {
		r_permissions.append("com.oculus.permission.EYE_TRACKING");

		FeatureInfo eye_tracking_feature = {
			"oculus.software.eye_tracking",
			_get_int_option("meta_xr_features/eye_tracking", EYE_TRACKING_OPTIONAL_VALUE) == EYE_TRACKING_REQUIRED_VALUE,
		};
		r_features.append(eye_tracking_feature);
	}

	// Check for face tracking.
	bool face_tracking_enabled = ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/meta/face_tracking");
	if (face_tracking_enabled) {
		r_permissions.append("com.oculus.permission.FACE_TRACKING");

		FeatureInfo face_tracking_info = {
			"oculus.software.face_tracking",
			_get_int_option("meta_xr_features/face_tracking", FACE_TRACKING_OPTIONAL_VALUE) == FACE_TRACKING_REQUIRED_VALUE,
		};
		r_features.append(face_tracking_info);
	}

	// Check for body tracking.
	bool body_tracking_enabled = ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/meta/body_tracking");
	if (body_tracking_enabled) {
		r_permissions.append("com.oculus.permission.BODY_TRACKING");

		FeatureInfo body_tracking_info = {
			"com.oculus.software.body_tracking",
			_get_int_option("meta_xr_features/body_tracking", BODY_TRACKING_OPTIONAL_VALUE) == BODY_TRACKING_REQUIRED_VALUE,
		};
		r_features.append(body_tracking_info);
	}

	// Check for hand tracking.
	bool hand_tracking_enabled = ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/hand_tracking");
	if (hand_tracking_enabled) {
		r_permissions.append("com.oculus.permission.HAND_TRACKING");

		FeatureInfo hand_tracking_info = {
			"oculus.software.handtracking",
			_get_int_option("meta_xr_features/hand_tracking", HAND_TRACKING_OPTIONAL_VALUE) == HAND_TRACKING_REQUIRED_VALUE,
		};
		r_features.append(hand_tracking_info);

		MetadataInfo hand_tracking_version_metadata = {
			"com.oculus.handtracking.version",
			"V2.0"
		};
		r_metadata.append(hand_tracking_version_metadata);

		int hand_tracking_frequency = _get_int_option("meta_xr_features/hand_tracking_frequency", HAND_TRACKING_FREQUENCY_LOW_VALUE);
		const String hand_tracking_frequency_label = (hand_tracking_frequency == HAND_TRACKING_FREQUENCY_LOW_VALUE) ? "LOW" : "HIGH";
		MetadataInfo hand_tracking_frequency_metadata = {
			"com.oculus.handtracking.frequency",
			hand_tracking_frequency_label
		};
		r_metadata.append(hand_tracking_frequency_metadata);
	}

	// Check for passthrough.
	bool passthrough_enabled = ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/meta/passthrough");
	if (passthrough_enabled) {
		FeatureInfo passthrough_info = {
			"com.oculus.feature.PASSTHROUGH",
			_get_int_option("meta_xr_features/passthrough", PASSTHROUGH_OPTIONAL_VALUE) == PASSTHROUGH_REQUIRED_VALUE,
		};
		r_features.append(passthrough_info);
	}

	// Check for render model.
	bool render_model_enabled = ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/meta/render_model");
	if (render_model_enabled) {
		r_permissions.append("com.oculus.permission.RENDER_MODEL");

		FeatureInfo render_model_info = {
			"com.oculus.feature.RENDER_MODEL",
			_get_int_option("meta_xr_features/render_model", RENDER_MODEL_OPTIONAL_VALUE) == RENDER_MODEL_REQUIRED_VALUE,
		};
		r_features.append(render_model_info);
	}

	// Check for anchor api.
	bool use_anchor_api = ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/meta/anchor_api");
	if (use_anchor_api) {
		r_permissions.append("com.oculus.permission.USE_ANCHOR_API");
	}

	// Check for anchor sharing.
	bool use_anchor_sharing = ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/meta/anchor_sharing");
	if (use_anchor_sharing) {
		r_permissions.append("com.oculus.permission.IMPORT_EXPORT_IOT_MAP_DATA");
	}

	// Check for scene api.
	bool use_scene_api = ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/meta/scene_api");
	bool use_environment_depth = ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/extensions/meta/environment_depth");
	if (use_scene_api || use_environment_depth) {
		r_permissions.append("com.oculus.permission.USE_SCENE");
	}

	// Check for overlay keyboard.
	bool use_overlay_keyboard_option = _get_bool_option("meta_xr_features/use_overlay_keyboard");
	if (use_overlay_keyboard_option) {
		FeatureInfo overlay_keyboard_info = {
			"oculus.software.overlay_keyboard",
			true
		};
		r_features.append(overlay_keyboard_info);
	}

	// Check for experimental features.
	bool use_experimental_features = _get_bool_option("meta_xr_features/use_experimental_features");
	if (use_experimental_features) {
		FeatureInfo experimental_feature_info = {
			"com.oculus.experimental.enabled",
			true
		};
		r_features.append(experimental_feature_info);
	}

	// Check for boundary mode
	int boundary_mode = _get_int_option("meta_xr_features/boundary_mode", BOUNDARY_ENABLED_VALUE);
	if (boundary_mode == BOUNDARY_DISABLED_VALUE) {
		FeatureInfo boundary_mode_info = {
			"com.oculus.feature.BOUNDARYLESS_APP",
			true
		};
		r_features.append(boundary_mode_info);
	}

	// Check for instant splash screen
	bool instant_splash_screen = _get_bool_option("meta_xr_features/instant_splash_screen");
	if (instant_splash_screen) {
		MetadataInfo instant_splash_screen_metadata = {
			"com.oculus.ossplash",
			"true"
		};
		r_metadata.append(instant_splash_screen_metadata);
	}

	// Check for splash screen in passthrough
	if ((int)ProjectSettings::get_singleton()->get_setting_with_override("xr/openxr/environment_blend_mode") != XRInterface::XR_ENV_BLEND_MODE_OPAQUE) {
		MetadataInfo contextual_passthrough_metadata = {
			"com.oculus.ossplash.background",
			"passthrough-contextual"
		};
		r_metadata.append(contextual_passthrough_metadata);
	}
}

PackedByteArray MetaToolkitExportPlugin::_update_android_prebuilt_manifest(const Ref<godot::EditorExportPlatform> &p_platform, const godot::PackedByteArray &p_manifest_data) const {
	PackedByteArray result;
	if (!_supports_platform(p_platform) || !_is_plugin_enabled() || p_manifest_data.is_empty()) {
		return result;
	}

	Vector<uint8_t> manifest_buffer;
	manifest_buffer.resize(p_manifest_data.size());
	memcpy(manifest_buffer.ptrw(), p_manifest_data.ptr(), p_manifest_data.size());

	// Leaving the unused types commented because looking these constants up
	// again later would be annoying
	// const int CHUNK_AXML_FILE = 0x00080003;
	// const int CHUNK_RESOURCEIDS = 0x00080180;
	const int CHUNK_STRINGS = 0x001C0001;
	// const int CHUNK_XML_END_NAMESPACE = 0x00100101;
	const int CHUNK_XML_END_TAG = 0x00100103;
	// const int CHUNK_XML_START_NAMESPACE = 0x00100100;
	const int CHUNK_XML_START_TAG = 0x00100102;
	// const int CHUNK_XML_TEXT = 0x00100104;
	const int UTF8_FLAG = 0x00000100;

	Vector<String> string_table;

	uint32_t ofs = 8;

	uint32_t string_count = 0;
	uint32_t string_flags = 0;
	uint32_t string_data_offset = 0;

	uint32_t string_table_begins = 0;
	uint32_t string_table_ends = 0;
	Vector<uint8_t> stable_extra;

	Vector<String> permissions;
	Vector<FeatureInfo> features;
	Vector<MetadataInfo> metadata;
	_get_manifest_entries(permissions, features, metadata);

	while (ofs < (uint32_t)manifest_buffer.size()) {
		uint32_t chunk = decode_uint32(&manifest_buffer[ofs]);
		uint32_t size = decode_uint32(&manifest_buffer[ofs + 4]);

		switch (chunk) {
			case CHUNK_STRINGS: {
				int iofs = ofs + 8;

				string_count = decode_uint32(&manifest_buffer[iofs]);
				string_flags = decode_uint32(&manifest_buffer[iofs + 8]);
				string_data_offset = decode_uint32(&manifest_buffer[iofs + 12]);

				uint32_t st_offset = iofs + 20;
				string_table.resize(string_count);
				uint32_t string_end = 0;

				string_table_begins = st_offset;

				for (uint32_t i = 0; i < string_count; i++) {
					uint32_t string_at = decode_uint32(&manifest_buffer[st_offset + i * 4]);
					string_at += st_offset + string_count * 4;

					ERR_FAIL_COND_V_MSG(string_flags & UTF8_FLAG, result, "Unimplemented, can't read UTF-8 string table.");

					if (string_flags & UTF8_FLAG) {
					} else {
						uint32_t len = decode_uint16(&manifest_buffer[string_at]);
						Vector<char32_t> ucstring;
						ucstring.resize(len + 1);
						for (uint32_t j = 0; j < len; j++) {
							uint16_t c = decode_uint16(&manifest_buffer[string_at + 2 + 2 * j]);
							ucstring.write[j] = c;
						}
						string_end = MAX(string_at + 2 + 2 * len, string_end);
						ucstring.write[len] = 0;
						string_table.write[i] = ucstring.ptr();
					}
				}

				for (uint32_t i = string_end; i < (ofs + size); i++) {
					stable_extra.push_back(manifest_buffer[i]);
				}

				string_table_ends = ofs + size;

			} break;
			case CHUNK_XML_END_TAG: {
				int iofs = ofs + 8;
				uint32_t name = decode_uint32(&manifest_buffer[iofs + 12]);
				String tname = string_table[name];

				if (tname == "manifest" || tname == "application") {
					// save manifest ending so we can restore it
					Vector<uint8_t> manifest_end;
					uint32_t manifest_cur_size = manifest_buffer.size();

					manifest_end.resize(manifest_buffer.size() - ofs);
					memcpy(manifest_end.ptrw(), &manifest_buffer[ofs], manifest_end.size());

					int32_t attr_name_string = string_table.find("name");
					ERR_FAIL_COND_V_MSG(attr_name_string == -1, result, "Template does not have 'name' attribute.");

					int32_t ns_android_string = string_table.find("http://schemas.android.com/apk/res/android");
					if (ns_android_string == -1) {
						string_table.push_back("http://schemas.android.com/apk/res/android");
						ns_android_string = string_table.size() - 1;
					}

					if (tname == "manifest") {
						// Updating manifest features
						int32_t attr_uses_feature_string = string_table.find("uses-feature");
						if (attr_uses_feature_string == -1) {
							string_table.push_back("uses-feature");
							attr_uses_feature_string = string_table.size() - 1;
						}

						int32_t attr_required_string = string_table.find("required");
						if (attr_required_string == -1) {
							string_table.push_back("required");
							attr_required_string = string_table.size() - 1;
						}

						for (int i = 0; i < features.size(); i++) {
							const String &feature_name = features[i].name;
							bool feature_required = features[i].required;
							String feature_version = features[i].version;
							bool has_version_attribute = !feature_version.is_empty();

							UtilityFunctions::print_verbose("Adding feature " + feature_name);

							int32_t feature_string = string_table.find(feature_name);
							if (feature_string == -1) {
								string_table.push_back(feature_name);
								feature_string = string_table.size() - 1;
							}

							String required_value_string = feature_required ? "true" : "false";
							int32_t required_value = string_table.find(required_value_string);
							if (required_value == -1) {
								string_table.push_back(required_value_string);
								required_value = string_table.size() - 1;
							}

							int32_t attr_version_string = -1;
							int32_t version_value = -1;
							int tag_size;
							int attr_count;
							if (has_version_attribute) {
								attr_version_string = string_table.find("version");
								if (attr_version_string == -1) {
									string_table.push_back("version");
									attr_version_string = string_table.size() - 1;
								}

								version_value = string_table.find(feature_version);
								if (version_value == -1) {
									string_table.push_back(feature_version);
									version_value = string_table.size() - 1;
								}

								tag_size = 96; // node and three attrs + end node
								attr_count = 3;
							} else {
								tag_size = 76; // node and two attrs + end node
								attr_count = 2;
							}
							manifest_cur_size += tag_size + 24;
							manifest_buffer.resize(manifest_cur_size);

							// start tag
							encode_uint16(0x102, &manifest_buffer.write[ofs]); // type
							encode_uint16(16, &manifest_buffer.write[ofs + 2]); // headersize
							encode_uint32(tag_size, &manifest_buffer.write[ofs + 4]); // size
							encode_uint32(0, &manifest_buffer.write[ofs + 8]); // lineno
							encode_uint32(-1, &manifest_buffer.write[ofs + 12]); // comment
							encode_uint32(-1, &manifest_buffer.write[ofs + 16]); // ns
							encode_uint32(attr_uses_feature_string, &manifest_buffer.write[ofs + 20]); // name
							encode_uint16(20, &manifest_buffer.write[ofs + 24]); // attr_start
							encode_uint16(20, &manifest_buffer.write[ofs + 26]); // attr_size
							encode_uint16(attr_count, &manifest_buffer.write[ofs + 28]); // num_attrs
							encode_uint16(0, &manifest_buffer.write[ofs + 30]); // id_index
							encode_uint16(0, &manifest_buffer.write[ofs + 32]); // class_index
							encode_uint16(0, &manifest_buffer.write[ofs + 34]); // style_index

							// android:name attribute
							encode_uint32(ns_android_string, &manifest_buffer.write[ofs + 36]); // ns
							encode_uint32(attr_name_string, &manifest_buffer.write[ofs + 40]); // 'name'
							encode_uint32(feature_string, &manifest_buffer.write[ofs + 44]); // raw_value
							encode_uint16(8, &manifest_buffer.write[ofs + 48]); // typedvalue_size
							manifest_buffer.write[ofs + 50] = 0; // typedvalue_always0
							manifest_buffer.write[ofs + 51] = 0x03; // typedvalue_type (string)
							encode_uint32(feature_string, &manifest_buffer.write[ofs + 52]); // typedvalue reference

							// android:required attribute
							encode_uint32(ns_android_string, &manifest_buffer.write[ofs + 56]); // ns
							encode_uint32(attr_required_string, &manifest_buffer.write[ofs + 60]); // 'name'
							encode_uint32(required_value, &manifest_buffer.write[ofs + 64]); // raw_value
							encode_uint16(8, &manifest_buffer.write[ofs + 68]); // typedvalue_size
							manifest_buffer.write[ofs + 70] = 0; // typedvalue_always0
							manifest_buffer.write[ofs + 71] = 0x03; // typedvalue_type (string)
							encode_uint32(required_value, &manifest_buffer.write[ofs + 72]); // typedvalue reference

							ofs += 76;

							if (has_version_attribute) {
								// android:version attribute
								encode_uint32(ns_android_string, &manifest_buffer.write[ofs]); // ns
								encode_uint32(attr_version_string, &manifest_buffer.write[ofs + 4]); // 'name'
								encode_uint32(version_value, &manifest_buffer.write[ofs + 8]); // raw_value
								encode_uint16(8, &manifest_buffer.write[ofs + 12]); // typedvalue_size
								manifest_buffer.write[ofs + 14] = 0; // typedvalue_always0
								manifest_buffer.write[ofs + 15] = 0x03; // typedvalue_type (string)
								encode_uint32(version_value, &manifest_buffer.write[ofs + 16]); // typedvalue reference

								ofs += 20;
							}

							// end tag
							encode_uint16(0x103, &manifest_buffer.write[ofs]); // type
							encode_uint16(16, &manifest_buffer.write[ofs + 2]); // headersize
							encode_uint32(24, &manifest_buffer.write[ofs + 4]); // size
							encode_uint32(0, &manifest_buffer.write[ofs + 8]); // lineno
							encode_uint32(-1, &manifest_buffer.write[ofs + 12]); // comment
							encode_uint32(-1, &manifest_buffer.write[ofs + 16]); // ns
							encode_uint32(attr_uses_feature_string, &manifest_buffer.write[ofs + 20]); // name

							ofs += 24;
						}

						// Updating manifest permissions
						int32_t attr_uses_permission_string = string_table.find("uses-permission");
						if (attr_uses_permission_string == -1) {
							string_table.push_back("uses-permission");
							attr_uses_permission_string = string_table.size() - 1;
						}

						for (int i = 0; i < permissions.size(); ++i) {
							UtilityFunctions::print_verbose("Adding permission " + permissions[i]);

							manifest_cur_size += 56 + 24; // node + end node
							manifest_buffer.resize(manifest_cur_size);

							// Add permission to the string pool
							int32_t perm_string = string_table.find(permissions[i]);
							if (perm_string == -1) {
								string_table.push_back(permissions[i]);
								perm_string = string_table.size() - 1;
							}

							// start tag
							encode_uint16(0x102, &manifest_buffer.write[ofs]); // type
							encode_uint16(16, &manifest_buffer.write[ofs + 2]); // headersize
							encode_uint32(56, &manifest_buffer.write[ofs + 4]); // size
							encode_uint32(0, &manifest_buffer.write[ofs + 8]); // lineno
							encode_uint32(-1, &manifest_buffer.write[ofs + 12]); // comment
							encode_uint32(-1, &manifest_buffer.write[ofs + 16]); // ns
							encode_uint32(attr_uses_permission_string, &manifest_buffer.write[ofs + 20]); // name
							encode_uint16(20, &manifest_buffer.write[ofs + 24]); // attr_start
							encode_uint16(20, &manifest_buffer.write[ofs + 26]); // attr_size
							encode_uint16(1, &manifest_buffer.write[ofs + 28]); // num_attrs
							encode_uint16(0, &manifest_buffer.write[ofs + 30]); // id_index
							encode_uint16(0, &manifest_buffer.write[ofs + 32]); // class_index
							encode_uint16(0, &manifest_buffer.write[ofs + 34]); // style_index

							// attribute
							encode_uint32(ns_android_string, &manifest_buffer.write[ofs + 36]); // ns
							encode_uint32(attr_name_string, &manifest_buffer.write[ofs + 40]); // 'name'
							encode_uint32(perm_string, &manifest_buffer.write[ofs + 44]); // raw_value
							encode_uint16(8, &manifest_buffer.write[ofs + 48]); // typedvalue_size
							manifest_buffer.write[ofs + 50] = 0; // typedvalue_always0
							manifest_buffer.write[ofs + 51] = 0x03; // typedvalue_type (string)
							encode_uint32(perm_string, &manifest_buffer.write[ofs + 52]); // typedvalue reference

							ofs += 56;

							// end tag
							encode_uint16(0x103, &manifest_buffer.write[ofs]); // type
							encode_uint16(16, &manifest_buffer.write[ofs + 2]); // headersize
							encode_uint32(24, &manifest_buffer.write[ofs + 4]); // size
							encode_uint32(0, &manifest_buffer.write[ofs + 8]); // lineno
							encode_uint32(-1, &manifest_buffer.write[ofs + 12]); // comment
							encode_uint32(-1, &manifest_buffer.write[ofs + 16]); // ns
							encode_uint32(attr_uses_permission_string, &manifest_buffer.write[ofs + 20]); // name

							ofs += 24;
						}
					}

					if (tname == "application") {
						// Updating application meta-data
						int32_t attr_meta_data_string = string_table.find("meta-data");
						if (attr_meta_data_string == -1) {
							string_table.push_back("meta-data");
							attr_meta_data_string = string_table.size() - 1;
						}

						int32_t attr_value_string = string_table.find("value");
						if (attr_value_string == -1) {
							string_table.push_back("value");
							attr_value_string = string_table.size() - 1;
						}

						for (int i = 0; i < metadata.size(); i++) {
							String meta_data_name = metadata[i].name;
							String meta_data_value = metadata[i].value;

							UtilityFunctions::print_verbose("Adding application metadata " + meta_data_name);

							int32_t meta_data_name_string = string_table.find(meta_data_name);
							if (meta_data_name_string == -1) {
								string_table.push_back(meta_data_name);
								meta_data_name_string = string_table.size() - 1;
							}

							int32_t meta_data_value_string = string_table.find(meta_data_value);
							if (meta_data_value_string == -1) {
								string_table.push_back(meta_data_value);
								meta_data_value_string = string_table.size() - 1;
							}

							int tag_size = 76; // node and two attrs + end node
							int attr_count = 2;
							manifest_cur_size += tag_size + 24;
							manifest_buffer.resize(manifest_cur_size);

							// start tag
							encode_uint16(0x102, &manifest_buffer.write[ofs]); // type
							encode_uint16(16, &manifest_buffer.write[ofs + 2]); // headersize
							encode_uint32(tag_size, &manifest_buffer.write[ofs + 4]); // size
							encode_uint32(0, &manifest_buffer.write[ofs + 8]); // lineno
							encode_uint32(-1, &manifest_buffer.write[ofs + 12]); // comment
							encode_uint32(-1, &manifest_buffer.write[ofs + 16]); // ns
							encode_uint32(attr_meta_data_string, &manifest_buffer.write[ofs + 20]); // name
							encode_uint16(20, &manifest_buffer.write[ofs + 24]); // attr_start
							encode_uint16(20, &manifest_buffer.write[ofs + 26]); // attr_size
							encode_uint16(attr_count, &manifest_buffer.write[ofs + 28]); // num_attrs
							encode_uint16(0, &manifest_buffer.write[ofs + 30]); // id_index
							encode_uint16(0, &manifest_buffer.write[ofs + 32]); // class_index
							encode_uint16(0, &manifest_buffer.write[ofs + 34]); // style_index

							// android:name attribute
							encode_uint32(ns_android_string, &manifest_buffer.write[ofs + 36]); // ns
							encode_uint32(attr_name_string, &manifest_buffer.write[ofs + 40]); // 'name'
							encode_uint32(meta_data_name_string, &manifest_buffer.write[ofs + 44]); // raw_value
							encode_uint16(8, &manifest_buffer.write[ofs + 48]); // typedvalue_size
							manifest_buffer.write[ofs + 50] = 0; // typedvalue_always0
							manifest_buffer.write[ofs + 51] = 0x03; // typedvalue_type (string)
							encode_uint32(meta_data_name_string, &manifest_buffer.write[ofs + 52]); // typedvalue reference

							// android:value attribute
							encode_uint32(ns_android_string, &manifest_buffer.write[ofs + 56]); // ns
							encode_uint32(attr_value_string, &manifest_buffer.write[ofs + 60]); // 'value'
							encode_uint32(meta_data_value_string, &manifest_buffer.write[ofs + 64]); // raw_value
							encode_uint16(8, &manifest_buffer.write[ofs + 68]); // typedvalue_size
							manifest_buffer.write[ofs + 70] = 0; // typedvalue_always0
							manifest_buffer.write[ofs + 71] = 0x03; // typedvalue_type (string)
							encode_uint32(meta_data_value_string, &manifest_buffer.write[ofs + 72]); // typedvalue reference

							ofs += 76;

							// end tag
							encode_uint16(0x103, &manifest_buffer.write[ofs]); // type
							encode_uint16(16, &manifest_buffer.write[ofs + 2]); // headersize
							encode_uint32(24, &manifest_buffer.write[ofs + 4]); // size
							encode_uint32(0, &manifest_buffer.write[ofs + 8]); // lineno
							encode_uint32(-1, &manifest_buffer.write[ofs + 12]); // comment
							encode_uint32(-1, &manifest_buffer.write[ofs + 16]); // ns
							encode_uint32(attr_meta_data_string, &manifest_buffer.write[ofs + 20]); // name

							ofs += 24;
						}
					}

					// copy footer back in
					memcpy(&manifest_buffer.write[ofs], manifest_end.ptr(), manifest_end.size());
				}
			} break;
		}

		ofs += size;
	}

	// Create new android manifest binary.
	Vector<uint8_t> ret;
	ret.resize(string_table_begins + string_table.size() * 4);

	for (uint32_t i = 0; i < string_table_begins; i++) {
		ret.write[i] = manifest_buffer[i];
	}

	ofs = 0;
	for (int i = 0; i < string_table.size(); i++) {
		encode_uint32(ofs, &ret.write[string_table_begins + i * 4]);
		ofs += string_table[i].length() * 2 + 2 + 2;
	}

	ret.resize(ret.size() + ofs);
	string_data_offset = ret.size() - ofs;
	uint8_t *chars = &ret.write[string_data_offset];
	for (int i = 0; i < string_table.size(); i++) {
		String s = string_table[i];
		encode_uint16(s.length(), chars);
		chars += 2;
		for (int j = 0; j < s.length(); j++) {
			encode_uint16(s[j], chars);
			chars += 2;
		}
		encode_uint16(0, chars);
		chars += 2;
	}

	for (int i = 0; i < stable_extra.size(); i++) {
		ret.push_back(stable_extra[i]);
	}

	//pad
	while (ret.size() % 4) {
		ret.push_back(0);
	}

	uint32_t new_stable_end = ret.size();

	uint32_t extra = (manifest_buffer.size() - string_table_ends);
	ret.resize(new_stable_end + extra);
	for (uint32_t i = 0; i < extra; i++) {
		ret.write[new_stable_end + i] = manifest_buffer[string_table_ends + i];
	}

	while (ret.size() % 4) {
		ret.push_back(0);
	}
	encode_uint32(ret.size(), &ret.write[4]); //update new file size

	encode_uint32(new_stable_end - 8, &ret.write[12]); //update new string table size
	encode_uint32(string_table.size(), &ret.write[16]); //update new number of strings
	encode_uint32(string_data_offset - 8, &ret.write[28]); //update new string data offset

	result.resize(ret.size());
	memcpy(result.ptrw(), ret.ptr(), ret.size());

	return result;
}
