// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

#include "editor/meta_project_setup_dialog.h"

#include "raw_headers/start_xr.gd.gen.h"
#include "raw_headers/xr_startup.tscn.gen.h"

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/center_container.hpp>
#include <godot_cpp/classes/config_file.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/editor_file_system.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_settings.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/h_separator.hpp>
#include <godot_cpp/classes/http_request.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/margin_container.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/panel_container.hpp>
#include <godot_cpp/classes/popup_menu.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/scroll_container.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/classes/theme.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

static const char *XR_STARTUP_SCENE_PATH = "res://xr_startup.tscn";
static const char *START_XR_SCRIPT_PATH = "res://start_xr.gd";

void MetaProjectSetupDialog::_bind_methods() {
}

void MetaProjectSetupDialog::_notification(uint32_t p_what) {
	switch (p_what) {
		case NOTIFICATION_POSTINITIALIZE: {
			recommendations.clear();

			// Required XR settings
			recommendations.push_back({ "OpenXR", "xr/openxr/enabled", true, "OpenXR must be enabled", ALERT_TYPE_ERROR, {} });
			recommendations.push_back({ "XR Shaders", "xr/shaders/enabled", true, "XR shaders must be compiled", ALERT_TYPE_ERROR, {} });

			// Other XR setting recommendations
			recommendations.push_back({ "MSAA", "rendering/anti_aliasing/quality/msaa_3d", 1, "Recommended to set MSAA to 2x", ALERT_TYPE_WARNING, {} });
			recommendations.push_back({ "VRS Mode", "rendering/vrs/mode", 2, "Recommended to set VRS mode to VR", ALERT_TYPE_WARNING, {} });
			recommendations.push_back({ "Foveation Level", "xr/openxr/foveation_level", 3, "Recommended to set foveation level to high", ALERT_TYPE_WARNING, {} });
			recommendations.push_back({ "Dynamic Foveation", "xr/openxr/foveation_dynamic", true, "Recommended to enable dynamic foveation", ALERT_TYPE_WARNING, {} });

			ScrollContainer *scroll_container = memnew(ScrollContainer);
			scroll_container->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
			scroll_container->set_horizontal_scroll_mode(ScrollContainer::SCROLL_MODE_DISABLED);
			add_child(scroll_container);

			outer_vbox = memnew(VBoxContainer);
			outer_vbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);
			outer_vbox->set_v_size_flags(Control::SIZE_EXPAND_FILL);
			scroll_container->add_child(outer_vbox);

			EditorInterface *editor_interface = EditorInterface::get_singleton();
			if (editor_interface) {
				Ref<Theme> editor_theme = editor_interface->get_editor_theme();
				if (editor_theme.is_valid()) {
					error_texture = editor_theme->get_icon("StatusError", "EditorIcons");
					warning_texture = editor_theme->get_icon("StatusWarning", "EditorIcons");
					error_color = editor_theme->get_color("error_color", "Editor");
					warning_color = editor_theme->get_color("warning_color", "Editor");
				}
			}

			vendors_plugin_entry = add_window_entry("Vendors Plugin", ALERT_TYPE_ERROR);
			vendors_plugin_entry.description_label->set_text("Please install the Godot OpenXR Vendors Plugin");
			vendors_plugin_entry.button->set_text("Open");
			vendors_plugin_entry.button->connect("pressed", callable_mp(this, &MetaProjectSetupDialog::open_asset_lib));

			export_preset_entry = add_window_entry("Export", ALERT_TYPE_ERROR);
			export_preset_entry.button->set_text("Open");
			export_preset_entry.button->connect("pressed", callable_mp(this, &MetaProjectSetupDialog::open_export_dialog));

			java_sdk_entry = add_window_entry("Java SDK", ALERT_TYPE_ERROR);
			java_sdk_entry.description_label->set_text("Please set a valid Java SDK path in Editor Settings");
			java_sdk_entry.button->set_text("Info");
			java_sdk_entry.button->connect("pressed", callable_mp(this, &MetaProjectSetupDialog::open_android_export_doc));

			android_sdk_entry = add_window_entry("Android SDK", ALERT_TYPE_ERROR);
			android_sdk_entry.description_label->set_text("Please set a valid Android SDK path in Editor Settings");
			android_sdk_entry.button->set_text("Info");
			android_sdk_entry.button->connect("pressed", callable_mp(this, &MetaProjectSetupDialog::open_android_export_doc));

			main_scene_entry = add_window_entry("Main Scene", ALERT_TYPE_NONE);
			main_scene_entry.description_label->set_text("No main scene is set, add XR startup scene?");
			main_scene_entry.button->set_text("Add");
			main_scene_entry.button->connect("pressed", callable_mp(this, &MetaProjectSetupDialog::add_xr_startup_scene));

			int rec_index = 0;
			for (ProjectSettingRecommendation &recommendation : recommendations) {
				recommendation.entry = add_window_entry(recommendation.setting_name, recommendation.alert_type);
				recommendation.entry.button->set_text("Apply");
				recommendation.entry.button->connect("pressed", callable_mp(this, &MetaProjectSetupDialog::apply_recommendation).bind(rec_index));
				rec_index++;
			}

			rec_list_empty_label = memnew(Label);
			rec_list_empty_label->set_h_size_flags(Control::SIZE_EXPAND | Control::SIZE_SHRINK_CENTER);
			rec_list_empty_label->set_v_size_flags(Control::SIZE_EXPAND | Control::SIZE_SHRINK_CENTER);
			rec_list_empty_label->set_text("All recommended settings have been applied!");
			outer_vbox->add_child(rec_list_empty_label);

			set_min_size(Vector2i(600, 300));
		} break;
	}
}

MetaProjectSetupDialog::WindowEntry MetaProjectSetupDialog::add_window_entry(const String &p_entry_name, AlertType p_alert_type) {
	WindowEntry entry;

	VBoxContainer *inner_vbox = memnew(VBoxContainer);
	inner_vbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	outer_vbox->add_child(inner_vbox);
	entry.vbox = inner_vbox;

	MarginContainer *margin_container = memnew(MarginContainer);
	margin_container->add_theme_constant_override("margin_left", 5);
	margin_container->add_theme_constant_override("margin_top", 5);
	margin_container->add_theme_constant_override("margin_right", 5);
	margin_container->add_theme_constant_override("margin_bottom", 5);
	inner_vbox->add_child(margin_container);

	HBoxContainer *hbox = memnew(HBoxContainer);
	margin_container->add_child(hbox);

	Label *name_label = memnew(Label);
	name_label->set_text(p_entry_name);
	name_label->set_custom_minimum_size(Vector2(150.0, 0.0));
	hbox->add_child(name_label);

	HBoxContainer *message_hbox = memnew(HBoxContainer);
	message_hbox->set_h_size_flags(Control::SIZE_EXPAND | Control::SIZE_SHRINK_BEGIN);
	hbox->add_child(message_hbox);

	CenterContainer *icon_container = memnew(CenterContainer);
	message_hbox->add_child(icon_container);
	entry.icon_container = icon_container;

	TextureRect *icon = memnew(TextureRect);
	icon->set_expand_mode(TextureRect::EXPAND_IGNORE_SIZE);
	icon_container->add_child(icon);

	Label *description_label = memnew(Label);
	message_hbox->add_child(description_label);
	entry.description_label = description_label;

	switch (p_alert_type) {
		case ALERT_TYPE_ERROR: {
			icon->set_texture(error_texture);
			icon->set_custom_minimum_size(Vector2(15.0, 15.0));
			description_label->add_theme_color_override("font_color", error_color);
		} break;
		case ALERT_TYPE_WARNING: {
			icon->set_texture(warning_texture);
			icon->set_custom_minimum_size(Vector2(15.0, 15.0));
			description_label->add_theme_color_override("font_color", warning_color);
		} break;
		case ALERT_TYPE_NONE: {
		} break;
	}

	Button *button = memnew(Button);
	hbox->add_child(button);
	entry.button = button;

	HSeparator *separator = memnew(HSeparator);
	inner_vbox->add_child(separator);

	return entry;
}

void MetaProjectSetupDialog::open() {
	if (!asset_lib_nodes_populated) {
		populate_asset_lib_nodes();
		if (!asset_lib || !asset_lib_button || !asset_lib_filter) {
			UtilityFunctions::print_verbose("Failed to find Asset Library nodes for MetaProjectSetupDialog");
			vendors_plugin_entry.button->hide();
		}
	}

	ProjectSettings *project_settings = ProjectSettings::get_singleton();
	ERR_FAIL_NULL(project_settings);

	bool is_rec_list_empty = true;

	// Check vendor extension.
	if (FileAccess::file_exists("res://addons/godotopenxrvendors/plugin.gdextension")) {
		vendors_plugin_entry.vbox->hide();
	} else {
		vendors_plugin_entry.vbox->show();
		is_rec_list_empty = false;
	}

	// Check export preset.
	Ref<ConfigFile> config;
	config.instantiate();
	config->load("res://export_presets.cfg");
	PackedStringArray sections = config->get_sections();

	bool has_android_export = false;
	for (const String &section : sections) {
		if (!config->has_section_key(section, "platform")) {
			continue;
		}

		if (config->get_value(section, "platform") == "Android") {
			has_android_export = true;

			String options_section = section + String(".options");
			if (!sections.has(options_section)) {
				continue;
			}

			if (!config->has_section_key(options_section, "meta_toolkit/enable_meta_toolkit")) {
				continue;
			}

			if (config->get_value(options_section, "meta_toolkit/enable_meta_toolkit")) {
				export_preset_entry.vbox->hide();
				break;
			} else {
				export_preset_entry.vbox->show();
				export_preset_entry.description_label->set_text("Meta Toolkit is not enabled for any Android export preset");
				is_rec_list_empty = false;
			}
		}
	}

	if (!has_android_export) {
		export_preset_entry.vbox->show();
		export_preset_entry.description_label->set_text("No Android export preset was found");
		is_rec_list_empty = false;
	}

	// Check Java and Android SDK
	EditorInterface *editor_interface = EditorInterface::get_singleton();
	if (editor_interface) {
		Ref<EditorSettings> editor_settings = editor_interface->get_editor_settings();
		if (editor_settings.is_valid()) {
			String java_sdk_path = editor_settings->get_setting("export/android/java_sdk_path");
			if (!java_sdk_path.is_empty() && DirAccess::dir_exists_absolute(java_sdk_path)) {
				java_sdk_entry.vbox->hide();
			} else {
				java_sdk_entry.vbox->show();
				is_rec_list_empty = false;
			}

			String android_sdk_path = editor_settings->get_setting("export/android/android_sdk_path");
			if (!android_sdk_path.is_empty() && DirAccess::dir_exists_absolute(android_sdk_path)) {
				android_sdk_entry.vbox->hide();
			} else {
				android_sdk_entry.vbox->show();
				is_rec_list_empty = false;
			}
		}
	}

	// Check for main scene.
	if (project_settings->get("application/run/main_scene") == String("")) {
		main_scene_entry.vbox->show();
		is_rec_list_empty = false;
	} else {
		main_scene_entry.vbox->hide();
	}

	// Check project setting recommendations.
	for (const ProjectSettingRecommendation &recommendation : recommendations) {
		if (project_settings->get(recommendation.setting_path) == recommendation.recommended_value) {
			recommendation.entry.vbox->hide();
		} else {
			recommendation.entry.vbox->show();
			recommendation.entry.button->set_disabled(false);
			recommendation.entry.description_label->set_text(recommendation.description);

			switch (recommendation.alert_type) {
				case ALERT_TYPE_ERROR: {
					recommendation.entry.description_label->add_theme_color_override("font_color", error_color);
				} break;
				case ALERT_TYPE_WARNING: {
					recommendation.entry.description_label->add_theme_color_override("font_color", warning_color);
				} break;
				case ALERT_TYPE_NONE: {
				} break;
			}

			if (recommendation.entry.icon_container != nullptr) {
				recommendation.entry.icon_container->show();
			}

			is_rec_list_empty = false;
		}
	}

	if (is_rec_list_empty) {
		rec_list_empty_label->show();
	} else {
		rec_list_empty_label->hide();
	}

	popup_centered_ratio(0.35);
}

void MetaProjectSetupDialog::populate_asset_lib_nodes() {
	asset_lib_nodes_populated = true;

	SceneTree *tree = get_tree();
	if (tree == nullptr) {
		return;
	}

	Window *root = tree->get_root();
	if (root == nullptr) {
		return;
	}

	asset_lib = root->find_child("@EditorAssetLibrary@*", true, false);
	if (asset_lib == nullptr) {
		return;
	}

	Node *lib_main_vbox = asset_lib->find_child("@VBoxContainer@*", false, false);
	if (lib_main_vbox == nullptr) {
		return;
	}

	// Asset Library node has multiple HBoxContainer children, but the search container is added first.
	Node *search_hb = lib_main_vbox->get_child(0);
	if (search_hb == nullptr) {
		return;
	}

	asset_lib_filter = Object::cast_to<LineEdit>(search_hb->find_child("@LineEdit@*", false, false));
	if (asset_lib_filter == nullptr) {
		return;
	}

	asset_lib_button = Object::cast_to<Button>(root->find_child("AssetLib", true, false));
	if (asset_lib_button == nullptr) {
		return;
	}
}

void MetaProjectSetupDialog::open_asset_lib() {
	hide();

	asset_lib_button->emit_signal("pressed");

	if (!asset_lib_filter->is_editable()) {
		asset_lib_request = Object::cast_to<HTTPRequest>(asset_lib->find_child("*HTTPRequest*", true, false));
		asset_lib_request->connect("request_completed", callable_mp(this, &MetaProjectSetupDialog::on_asset_lib_request_completed), CONNECT_ONE_SHOT);
	} else {
		asset_lib_filter->set_text("Godot OpenXR Vendors plugin v4");
		asset_lib_filter->emit_signal("text_changed", asset_lib_filter->get_text());
	}
}

void MetaProjectSetupDialog::on_asset_lib_request_completed(int p_result, int p_response_code, const PackedStringArray &p_headers, const PackedByteArray &p_body) {
	if (p_response_code != 200) {
		UtilityFunctions::print_verbose(vformat("Asset Library HTTPRequest returned with response code %s", p_response_code));
		return;
	}

	if (asset_lib_filter == nullptr) {
		UtilityFunctions::print_verbose("No Asset Library LineEdit node found");
		return;
	}

	asset_lib_filter->set_text("Godot OpenXR Vendors plugin v4");
	asset_lib_filter->emit_signal("text_changed", asset_lib_filter->get_text());
}

void MetaProjectSetupDialog::open_export_dialog() {
	SceneTree *tree = get_tree();
	if (tree == nullptr) {
		return;
	}

	Window *root = tree->get_root();
	if (root == nullptr) {
		return;
	}

	Node *menu_bar = root->find_child("*MenuBar*", true, false);
	if (menu_bar == nullptr) {
		return;
	}

	PopupMenu *project_menu = Object::cast_to<PopupMenu>(menu_bar->find_child("*Project*", true, false));
	if (project_menu == nullptr) {
		return;
	}

	int item_count = project_menu->get_item_count();
	int item_id = -1;
	for (int i = 0; i < item_count; i++) {
		if (project_menu->get_item_text(i) == "Export...") {
			item_id = project_menu->get_item_id(i);
		}
	}

	// Hacky fallback for non-English editor usecase.
	if (item_id < 0 && item_count > 5) {
		item_id = project_menu->get_item_id(5);
	}

	if (item_id > 0) {
		hide();
		project_menu->emit_signal("id_pressed", item_id);
	} else {
		UtilityFunctions::print_verbose("MetaProjectSetupDialog failed to find export dialog menu entry");
	}
}

void MetaProjectSetupDialog::open_android_export_doc() {
	OS *os = OS::get_singleton();
	if (os == nullptr) {
		return;
	}

	os->shell_open("https://docs.godotengine.org/en/stable/tutorials/export/exporting_for_android.html");
}

void MetaProjectSetupDialog::add_xr_startup_scene() {
	ProjectSettings *project_settings = ProjectSettings::get_singleton();
	ERR_FAIL_NULL(project_settings);

	EditorInterface *editor_interface = EditorInterface::get_singleton();
	ERR_FAIL_NULL(editor_interface);

	if (FileAccess::file_exists(START_XR_SCRIPT_PATH)) {
		ERR_FAIL_EDMSG("XR startup script start_xr.gd already exists");
	}

	if (FileAccess::file_exists(XR_STARTUP_SCENE_PATH)) {
		ERR_FAIL_EDMSG("XR startup scene xr_startup.tscn already exists");
	}

	Ref<FileAccess> start_xr = FileAccess::open(START_XR_SCRIPT_PATH, FileAccess::WRITE);
	start_xr->store_string(String(start_xr_gd));
	start_xr->close();

	Ref<FileAccess> startup_scene = FileAccess::open(XR_STARTUP_SCENE_PATH, FileAccess::WRITE);
	startup_scene->store_string(String(xr_startup_tscn));
	startup_scene->close();

	project_settings->set_setting("application/run/main_scene", XR_STARTUP_SCENE_PATH);
	editor_interface->get_resource_filesystem()->scan();
	editor_interface->open_scene_from_path(XR_STARTUP_SCENE_PATH);

	hide();
}

void MetaProjectSetupDialog::apply_recommendation(int p_rec_index) {
	ProjectSettings *project_settings = ProjectSettings::get_singleton();
	ERR_FAIL_NULL(project_settings);

	ProjectSettingRecommendation recommendation = recommendations[p_rec_index];
	project_settings->set_setting(recommendation.setting_path, recommendation.recommended_value);
	project_settings->save();

	recommendation.entry.button->set_disabled(true);

	if (recommendation.entry.icon_container != nullptr) {
		recommendation.entry.icon_container->hide();
	}

	if (recommendation.alert_type == ALERT_TYPE_ERROR) {
		recommendation.entry.description_label->set_text("Restart editor to apply updated setting");
		recommendation.entry.description_label->add_theme_color_override("font_color", warning_color);
	} else {
		recommendation.entry.description_label->set_text("Setting successfully updated");
		recommendation.entry.description_label->remove_theme_color_override("font_color");
	}
}

MetaProjectSetupDialog::MetaProjectSetupDialog() {
	set_title("XR Project Setup");
}

MetaProjectSetupDialog::~MetaProjectSetupDialog() {
}
