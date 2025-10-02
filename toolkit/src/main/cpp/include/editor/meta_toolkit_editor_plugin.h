// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

#pragma once

#include <godot_cpp/classes/editor_plugin.hpp>

#include "export/meta_toolkit_export_plugin.h"

using namespace godot;
class MetaXRSimulatorDialog;
class MetaProjectSetupDialog;

namespace godot {
class LineEdit;
}

class MetaToolkitEditorPlugin : public EditorPlugin {
	GDCLASS(MetaToolkitEditorPlugin, EditorPlugin);

	static MetaToolkitEditorPlugin *singleton;

	Ref<MetaToolkitExportPlugin> _meta_toolkit_export_plugin;
	MetaXRSimulatorDialog *_meta_xr_simulator_dialog = nullptr;
	MetaProjectSetupDialog *_meta_project_setup_dialog = nullptr;

	void _configure_xr_simulator();
	void _open_project_setup();

protected:
	static void _bind_methods();

	void _notification(uint32_t p_what);

public:
	static MetaToolkitEditorPlugin *get_singleton();

	virtual String _get_plugin_name() const override {
		return "MetaToolkitEditorPlugin";
	}

	void open_asset_library(const String &p_filter_string);
	void _on_asset_library_request_completed(int p_result, int p_response_code, const PackedStringArray &p_headers, const PackedByteArray &p_body, LineEdit *p_asset_library_filter, String p_search_string);

	void open_project_settings(const String &p_filter_string);

	void open_export_dialog();

	MetaToolkitEditorPlugin();
	~MetaToolkitEditorPlugin();
};
