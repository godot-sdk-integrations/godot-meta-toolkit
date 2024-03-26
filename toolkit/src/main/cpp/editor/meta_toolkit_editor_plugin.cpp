// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

#include "editor/meta_toolkit_editor_plugin.h"
#include "editor/meta_xr_simulator_dialog.h"

void MetaToolkitEditorPlugin::_bind_methods() {
}

void MetaToolkitEditorPlugin::_notification(uint32_t p_what) {
    switch (p_what) {
        case NOTIFICATION_POSTINITIALIZE: {
			_meta_xr_simulator_dialog = memnew(MetaXRSimulatorDialog);
			add_child(_meta_xr_simulator_dialog);
		} break;

		case NOTIFICATION_ENTER_TREE: {
			add_tool_menu_item("Configure Meta XR Simulator...", callable_mp(this, &MetaToolkitEditorPlugin::_configure_xr_simulator));

			// Initialize the editor export plugin
			_meta_toolkit_export_plugin.instantiate();
			add_export_plugin(_meta_toolkit_export_plugin);
		} break;

        case NOTIFICATION_EXIT_TREE: {
			remove_tool_menu_item("Configure Meta XR Simulator...");

			// Clean up the editor export plugin
			remove_export_plugin(_meta_toolkit_export_plugin);
			_meta_toolkit_export_plugin.unref();
		} break;
	}
}

void MetaToolkitEditorPlugin::_configure_xr_simulator() {
	_meta_xr_simulator_dialog->show();
}
