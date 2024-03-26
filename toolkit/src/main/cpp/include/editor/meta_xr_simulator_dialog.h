// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

#pragma once

#include <godot_cpp/classes/accept_dialog.hpp>

namespace godot {
    class RichTextLabel;
	class LineEdit;
	class EditorFileDialog;
} //namespace godot

using namespace godot;

class MetaXRSimulatorDialog : public AcceptDialog {
	GDCLASS(MetaXRSimulatorDialog, AcceptDialog);

	RichTextLabel *_message = nullptr;
	LineEdit *_path_field = nullptr;
	EditorFileDialog *_file_dialog = nullptr;

	void _update();

	void _on_path_changed(const String &p_path);
	void _on_browse_button_pressed();
	void _on_file_selected(const String &p_path);
	void _on_meta_clicked(const String &p_meta);

	String _get_simulator_path();
	void _set_simulator_path(const String &p_path);

protected:
	static void _bind_methods();

	void _notification(uint32_t p_what);

public:
	void show();

	MetaXRSimulatorDialog();
};
