// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

#pragma once

#include <godot_cpp/classes/accept_dialog.hpp>
#include <godot_cpp/templates/local_vector.hpp>

namespace godot {
class Button;
class CenterContainer;
class VBoxContainer;
class Label;
class LineEdit;
class HTTPRequest;
} //namespace godot

using namespace godot;

class MetaProjectSetupDialog : public AcceptDialog {
	GDCLASS(MetaProjectSetupDialog, AcceptDialog);

	enum AlertType {
		ALERT_TYPE_ERROR,
		ALERT_TYPE_WARNING,
		ALERT_TYPE_NONE,
	};

	struct WindowEntry {
		VBoxContainer *vbox = nullptr;
		CenterContainer *icon_container = nullptr;
		Label *description_label = nullptr;
		Button *button = nullptr;
	};

	struct ProjectSettingRecommendation {
		String setting_name;
		String setting_path;
		Variant recommended_value;
		String description;
		AlertType alert_type;
		WindowEntry entry;
	};

	WindowEntry vendors_plugin_entry;
	WindowEntry export_preset_entry;
	WindowEntry java_sdk_entry;
	WindowEntry android_sdk_entry;
	WindowEntry main_scene_entry;

	LocalVector<ProjectSettingRecommendation> recommendations;

	VBoxContainer *outer_vbox = nullptr;
	Label *rec_list_empty_label = nullptr;

	bool asset_lib_nodes_populated = false;
	Node *asset_lib = nullptr;
	Button *asset_lib_button = nullptr;
	LineEdit *asset_lib_filter = nullptr;
	HTTPRequest *asset_lib_request = nullptr;

	WindowEntry add_window_entry(const String &p_entry_name, AlertType p_alert_type);

	void populate_asset_lib_nodes();
	void open_asset_lib();
	void on_asset_lib_request_completed(int p_result, int p_response_code, const PackedStringArray &p_headers, const PackedByteArray &p_body);

	void open_export_dialog();

	void open_android_export_doc();

	void add_xr_startup_scene();

	void apply_recommendation(int p_rec_index);

	Ref<Texture2D> error_texture;
	Ref<Texture2D> warning_texture;
	Color error_color = Color(1.0, 0.0, 0.0);
	Color warning_color = Color(1.0, 1.0, 0.0);

protected:
	static void _bind_methods();

	void _notification(uint32_t p_what);

public:
	void open();

	MetaProjectSetupDialog();
	~MetaProjectSetupDialog();
};
