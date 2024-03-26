// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

#pragma once

#include <godot_cpp/classes/editor_plugin.hpp>

#include "export/meta_toolkit_export_plugin.h"

using namespace godot;
class MetaXRSimulatorDialog;

class MetaToolkitEditorPlugin : public EditorPlugin {
	GDCLASS(MetaToolkitEditorPlugin, EditorPlugin);

	Ref<MetaToolkitExportPlugin> _meta_toolkit_export_plugin;
	MetaXRSimulatorDialog *_meta_xr_simulator_dialog = nullptr;

	void _configure_xr_simulator();

protected:
	static void _bind_methods();

	void _notification(uint32_t p_what);

public:
	virtual String _get_plugin_name() const override {
		return "MetaToolkitEditorPlugin";
	}
};
