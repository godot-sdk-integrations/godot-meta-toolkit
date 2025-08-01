// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

#pragma once

#include <godot_cpp/classes/editor_export_platform.hpp>
#include <godot_cpp/classes/editor_export_plugin.hpp>

using namespace godot;

class MetaToolkitExportPlugin : public EditorExportPlugin {
	GDCLASS(MetaToolkitExportPlugin, EditorExportPlugin)

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

protected:
	static void _bind_methods();

private:
	static Dictionary _generate_export_option(const String &p_name, const String &p_class_name,
			Variant::Type p_type,
			PropertyHint p_property_hint,
			const String &p_hint_string,
			PropertyUsageFlags p_property_usage,
			const Variant &p_default_value,
			bool p_update_visibility);

	bool _get_bool_option(const String &p_option) const;

	Dictionary _enable_meta_toolkit_option;
};
