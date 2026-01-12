// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_settings.hpp>

#include "editor/meta_export_template_dialog.h"
#include "editor/meta_toolkit_editor_plugin.h"
#include "editor/meta_xr_simulator_dialog.h"

void MetaToolkitEditorPlugin::_bind_methods() {
}

void MetaToolkitEditorPlugin::_notification(uint32_t p_what) {
	switch (p_what) {
		case NOTIFICATION_POSTINITIALIZE: {
			_add_plugin_editor_settings();

			_meta_xr_simulator_dialog = memnew(MetaXRSimulatorDialog);
			add_child(_meta_xr_simulator_dialog);

			// Only works with Godot 4.5 or later.
			if (godot::internal::godot_version.minor >= 5) {
				_meta_export_templates_dialog = memnew(MetaExportTemplateDialog);
				add_child(_meta_export_templates_dialog);
			}
		} break;

		case NOTIFICATION_ENTER_TREE: {
			add_tool_menu_item("Configure Meta XR Simulator...", callable_mp(this, &MetaToolkitEditorPlugin::_configure_xr_simulator));

			// Only works with Godot 4.5 or later.
			if (godot::internal::godot_version.minor >= 5) {
				add_tool_menu_item("Install Meta Export Template...", callable_mp(this, &MetaToolkitEditorPlugin::_configure_export_template));
			}

			// Initialize the editor export plugin
			_meta_toolkit_export_plugin.instantiate();
			add_export_plugin(_meta_toolkit_export_plugin);
		} break;

		case NOTIFICATION_EXIT_TREE: {
			remove_tool_menu_item("Configure Meta XR Simulator...");

			// Only works with Godot 4.5 or later.
			if (godot::internal::godot_version.minor >= 5) {
				remove_tool_menu_item("Install Meta Export Template...");
			}

			// Clean up the editor export plugin
			remove_export_plugin(_meta_toolkit_export_plugin);
			_meta_toolkit_export_plugin.unref();
		} break;
	}
}

void MetaToolkitEditorPlugin::_configure_xr_simulator() {
	_meta_xr_simulator_dialog->show();
}

void MetaToolkitEditorPlugin::_configure_export_template() {
	_meta_export_templates_dialog->show();
}

void MetaToolkitEditorPlugin::_add_plugin_editor_settings() {
	EditorInterface *editor_interface = EditorInterface::get_singleton();
	ERR_FAIL_NULL(editor_interface);
	Ref<EditorSettings> editor_settings = editor_interface->get_editor_settings();
	ERR_FAIL_COND(editor_settings.is_null());

	String base_download_url_setting = "xr/meta_toolkit/base_download_url";
	String base_download_url_default = "https://github.com/godot-sdk-integrations/godot-meta-toolkit/releases/download/";
	if (!editor_settings->has_setting(base_download_url_setting)) {
		editor_settings->set_setting(base_download_url_setting, base_download_url_default);
	}

	editor_settings->set_initial_value(base_download_url_setting, base_download_url_default, false);
	Dictionary base_download_url_property_info;
	base_download_url_property_info["name"] = base_download_url_setting;
	base_download_url_property_info["type"] = Variant::Type::STRING;
	base_download_url_property_info["hint"] = PROPERTY_HINT_NONE;
	editor_settings->add_property_info(base_download_url_property_info);
}
