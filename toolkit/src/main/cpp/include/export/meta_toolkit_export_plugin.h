// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

#pragma once

#include <godot_cpp/classes/editor_export_platform.hpp>
#include <godot_cpp/classes/editor_export_plugin.hpp>

using namespace godot;

class MetaToolkitExportPlugin : public EditorExportPlugin {
    GDCLASS(MetaToolkitExportPlugin, EditorExportPlugin)

	static const int EYE_TRACKING_NONE_VALUE = 0;
	static const int EYE_TRACKING_OPTIONAL_VALUE = 1;
	static const int EYE_TRACKING_REQUIRED_VALUE = 2;

	static const int FACE_TRACKING_NONE_VALUE = 0;
	static const int FACE_TRACKING_OPTIONAL_VALUE = 1;
	static const int FACE_TRACKING_REQUIRED_VALUE = 2;

	static const int BODY_TRACKING_NONE_VALUE = 0;
	static const int BODY_TRACKING_OPTIONAL_VALUE = 1;
	static const int BODY_TRACKING_REQUIRED_VALUE = 2;

	static const int PASSTHROUGH_NONE_VALUE = 0;
	static const int PASSTHROUGH_OPTIONAL_VALUE = 1;
	static const int PASSTHROUGH_REQUIRED_VALUE = 2;

	static const int RENDER_MODEL_NONE_VALUE = 0;
	static const int RENDER_MODEL_OPTIONAL_VALUE = 1;
	static const int RENDER_MODEL_REQUIRED_VALUE = 2;

	static const int HAND_TRACKING_NONE_VALUE = 0;
	static const int HAND_TRACKING_OPTIONAL_VALUE = 1;
	static const int HAND_TRACKING_REQUIRED_VALUE = 2;

	static const int HAND_TRACKING_FREQUENCY_LOW_VALUE = 0;
	static const int HAND_TRACKING_FREQUENCY_HIGH_VALUE = 1;

	static const int BOUNDARY_ENABLED_VALUE = 0;
	static const int BOUNDARY_DISABLED_VALUE = 1;

public:
    MetaToolkitExportPlugin();

    String _get_name() const override {
        return "Godot Meta Toolkit";
    }

    PackedStringArray _get_android_libraries(const Ref<EditorExportPlatform> &p_platform, bool p_debug) const override;

    String _get_export_option_warning(const Ref<EditorExportPlatform> &p_platform, const String &p_option) const override;

    TypedArray<Dictionary> _get_export_options(const Ref<EditorExportPlatform> &p_platform) const override;

    Dictionary _get_export_options_overrides(const Ref<EditorExportPlatform> &p_platform) const override;

    bool _supports_platform(const Ref<EditorExportPlatform> &p_platform) const override;

	PackedByteArray _update_android_prebuilt_manifest(const Ref<EditorExportPlatform> &p_platform, const PackedByteArray &updated_manifest) const override;

protected:
    static void _bind_methods();

private:
	struct FeatureInfo {
		String name;
		bool required;
		String version = "";
	};

	struct MetadataInfo {
		String name;
		String value;
	};

    static Dictionary _generate_export_option(const String &p_name, const String &p_class_name,
                                              Variant::Type p_type,
                                              PropertyHint p_property_hint,
                                              const String &p_hint_string,
                                              PropertyUsageFlags p_property_usage,
                                              const Variant &p_default_value,
                                              bool p_update_visibility);

    bool _get_bool_option(const String &p_option) const;

	int _get_int_option(const String &p_option, int default_value) const;

	String _bool_to_string(bool p_value) const;

	bool _is_plugin_enabled() const;

	bool _is_eye_tracking_enabled() const;

	void _get_manifest_entries(Vector<String> &r_permissions, Vector<FeatureInfo> &r_features, Vector<MetadataInfo> &r_metadata) const;

	PackedStringArray _get_supported_devices() const;

    Dictionary _enable_meta_toolkit_option;
};
