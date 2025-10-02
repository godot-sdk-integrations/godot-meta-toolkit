// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

#include "editor/meta_toolkit_editor_plugin.h"
#include "editor/meta_project_setup_dialog.h"
#include "editor/meta_xr_simulator_dialog.h"

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/http_request.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/popup_menu.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

MetaToolkitEditorPlugin *MetaToolkitEditorPlugin::singleton = nullptr;

MetaToolkitEditorPlugin *MetaToolkitEditorPlugin::get_singleton() {
	return singleton;
}

MetaToolkitEditorPlugin::MetaToolkitEditorPlugin() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An MetaToolkitEditorPlugin singleton already exists");
	singleton = this;
}

MetaToolkitEditorPlugin::~MetaToolkitEditorPlugin() {
	singleton = nullptr;
}

void MetaToolkitEditorPlugin::_bind_methods() {
}

void MetaToolkitEditorPlugin::_notification(uint32_t p_what) {
	switch (p_what) {
		case NOTIFICATION_POSTINITIALIZE: {
			_meta_xr_simulator_dialog = memnew(MetaXRSimulatorDialog);
			add_child(_meta_xr_simulator_dialog);

			_meta_project_setup_dialog = memnew(MetaProjectSetupDialog);
			add_child(_meta_project_setup_dialog);
		} break;

		case NOTIFICATION_ENTER_TREE: {
			add_tool_menu_item("Configure Meta XR Simulator...", callable_mp(this, &MetaToolkitEditorPlugin::_configure_xr_simulator));
			add_tool_menu_item("XR Project Setup Wizard...", callable_mp(this, &MetaToolkitEditorPlugin::_open_project_setup));

			// Initialize the editor export plugin
			_meta_toolkit_export_plugin.instantiate();
			add_export_plugin(_meta_toolkit_export_plugin);
		} break;

		case NOTIFICATION_EXIT_TREE: {
			remove_tool_menu_item("Configure Meta XR Simulator...");
			remove_tool_menu_item("XR Project Setup Wizard...");

			// Clean up the editor export plugin
			remove_export_plugin(_meta_toolkit_export_plugin);
			_meta_toolkit_export_plugin.unref();
		} break;
	}
}

void MetaToolkitEditorPlugin::_configure_xr_simulator() {
	_meta_xr_simulator_dialog->show();
}

void MetaToolkitEditorPlugin::_open_project_setup() {
	_meta_project_setup_dialog->open();
}

void MetaToolkitEditorPlugin::open_asset_library(const String &p_search_string) {
	SceneTree *tree = get_tree();
	if (tree == nullptr) {
		return;
	}

	Window *root = tree->get_root();
	if (root == nullptr) {
		return;
	}

	Node *asset_lib = root->find_child("@EditorAssetLibrary@*", true, false);
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

	LineEdit *asset_library_filter = Object::cast_to<LineEdit>(search_hb->find_child("@LineEdit@*", false, false));
	if (asset_library_filter == nullptr) {
		return;
	}

	Button *asset_lib_button = Object::cast_to<Button>(root->find_child("AssetLib", true, false));
	if (asset_lib_button == nullptr) {
		return;
	}

	asset_lib_button->emit_signal("pressed");

	if (!asset_library_filter->is_editable()) {
		HTTPRequest *asset_lib_request = Object::cast_to<HTTPRequest>(asset_lib->find_child("*HTTPRequest*", true, false));
		asset_lib_request->connect("request_completed", callable_mp(this, &MetaToolkitEditorPlugin::_on_asset_library_request_completed).bind(asset_library_filter, p_search_string), CONNECT_ONE_SHOT);
	} else {
		asset_library_filter->set_text(p_search_string);
		asset_library_filter->emit_signal("text_changed", asset_library_filter->get_text());
	}
}

void MetaToolkitEditorPlugin::_on_asset_library_request_completed(int p_result, int p_response_code, const PackedStringArray &p_headers, const PackedByteArray &p_body, LineEdit *p_asset_library_filter, String p_search_string) {
	if (p_response_code != 200) {
		UtilityFunctions::print_verbose(vformat("Asset Library HTTPRequest returned with response code %s", p_response_code));
		return;
	}

	if (p_asset_library_filter == nullptr) {
		UtilityFunctions::print_verbose("No Asset Library LineEdit node found");
		return;
	}

	p_asset_library_filter->set_text(p_search_string);
	p_asset_library_filter->emit_signal("text_changed", p_asset_library_filter->get_text());
}

void MetaToolkitEditorPlugin::open_project_settings(const String &p_filter_string) {
	SceneTree *tree = get_tree();
	if (tree == nullptr) {
		return;
	}

	Window *root = tree->get_root();
	if (root == nullptr) {
		return;
	}

	Node *project_settings_editor = root->find_child("@ProjectSettingsEditor@*", true, false);
	if (project_settings_editor == nullptr) {
		return;
	}

	// Desired tab container should be first child of the project settings editor.
	Node *tab_container = project_settings_editor->get_child(0);
	if (tab_container == nullptr) {
		return;
	}

	// Desired VBoxContainer should be first child of the tab container.
	Node *general_editor = tab_container->get_child(0);
	if (general_editor == nullptr) {
		return;
	}

	Node *search_bar = general_editor->find_child("@HBoxContainer@*", false, false);
	if (search_bar == nullptr) {
		return;
	}

	LineEdit *search_box = Object::cast_to<LineEdit>(search_bar->find_child("@LineEdit@*", false, false));
	if (search_box == nullptr) {
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
		if (project_menu->get_item_text(i) == "Project Settings...") {
			item_id = project_menu->get_item_id(i);
		}
	}

	// Hacky fallback for non-English editor usecase.
	if (item_id < 0) {
		item_id = project_menu->get_item_id(0);
	}

	if (item_id < 0) {
		UtilityFunctions::print_verbose("MetaToolkitEditorPlugin failed to find project settings menu entry");
		return;
	}

	project_menu->emit_signal("id_pressed", item_id);
	search_box->set_text(p_filter_string);
	search_box->emit_signal("text_changed", search_box->get_text());
}

void MetaToolkitEditorPlugin::open_export_dialog() {
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

	if (item_id < 0) {
		UtilityFunctions::print_verbose("MetaToolkitEditorPlugin failed to find export dialog menu entry");
		return;
	}

	project_menu->emit_signal("id_pressed", item_id);
}
