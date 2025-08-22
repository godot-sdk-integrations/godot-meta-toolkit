// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

#include "editor/meta_xr_simulator_dialog.h"

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/editor_file_dialog.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_settings.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>
#include <godot_cpp/classes/scroll_container.hpp>
#include <godot_cpp/classes/v_box_container.hpp>

static const char *OPENXR_RUNTIME_PATHS_SETTING = "xr/openxr/runtime_paths";
static const char *META_SIMULATOR_RUNTIME_NAME = "Meta XR Simulator";
static const char *META_SIMULATOR_INSTRUCTIONS =
		"In order to allow Godot to run the Meta XR Simulator:\n\n"
		"[ol]\n"
		"  Download the Meta XR Simulator using the [url=https://developer.oculus.com/meta-quest-developer-hub]Meta Quest Developer Hub[/url] or [url=https://developers.meta.com/horizon/documentation/native/xrsim-intro/]download it directly[/url].\n"
		"  Extract the files into a folder on your computer.\n"
		"  Enter the full path to the [code]meta_openxr_simulator.json[/code] file within that directory.\n"
		"[/ol]\n\n"
		"Once correctly configured, a new dropdown will appear in the top-right corner of the Godot Editor, which will allow you to select the \"Meta XR Simulator\" as your OpenXR runtime.\n\n"
		"Running your project with this option selected, will open it in the Meta XR Simulator.\n\n"
		"[b]Note:[/b] The Meta XR Simulator only works with Vulkan, so be sure to select \"Mobile\" or \"Forward+\" from the renderer dropdown in the top-right corner (just to the left of the OpenXR runtime dropdown) when using the XR simulator.\n\n";

void MetaXRSimulatorDialog::_bind_methods() {
}

void MetaXRSimulatorDialog::_notification(uint32_t p_what) {
	switch (p_what) {
		case NOTIFICATION_POSTINITIALIZE: {
			_file_dialog = memnew(EditorFileDialog);
			_file_dialog->connect("file_selected", callable_mp(this, &MetaXRSimulatorDialog::_on_file_selected));
			_file_dialog->set_file_mode(EditorFileDialog::FILE_MODE_OPEN_FILE);
			_file_dialog->set_access(EditorFileDialog::ACCESS_FILESYSTEM);
			PackedStringArray file_filters;
			file_filters.push_back("*.json ; JSON Files");
			_file_dialog->set_filters(file_filters);
			add_child(_file_dialog);

			ScrollContainer *scroll_container = memnew(ScrollContainer);
			scroll_container->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
			scroll_container->set_horizontal_scroll_mode(ScrollContainer::SCROLL_MODE_DISABLED);
			add_child(scroll_container);

			VBoxContainer *main_vbox = memnew(VBoxContainer);
			main_vbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);
			main_vbox->set_v_size_flags(Control::SIZE_EXPAND_FILL);
			scroll_container->add_child(main_vbox);

			_message = memnew(RichTextLabel);
			_message->set_use_bbcode(true);
			_message->set_v_size_flags(Control::SIZE_EXPAND_FILL);
			_message->connect("meta_clicked", callable_mp(this, &MetaXRSimulatorDialog::_on_meta_clicked));
			main_vbox->add_child(_message);

			HBoxContainer *hbox = memnew(HBoxContainer);
			main_vbox->add_child(hbox);

			Label *label = memnew(Label);
			label->set_text("Path to JSON file:");
			hbox->add_child(label);

			_path_field = memnew(LineEdit);
			_path_field->set_h_size_flags(Control::SIZE_EXPAND_FILL);
			_path_field->connect("text_changed", callable_mp(this, &MetaXRSimulatorDialog::_on_path_changed));
			hbox->add_child(_path_field);

			Button *browse_button = memnew(Button);
			browse_button->set_text("Browse");
			browse_button->connect("pressed", callable_mp(this, &MetaXRSimulatorDialog::_on_browse_button_pressed));
			hbox->add_child(browse_button);
		} break;
	}
}

void MetaXRSimulatorDialog::show() {
	popup_centered_ratio(0.4);
	_path_field->set_text(_get_simulator_path());
	_update();
}

void MetaXRSimulatorDialog::_update() {
	String path = _path_field->get_text();

	String note;
	if (path.is_empty()) {
		note = "[color=red]Please enter the path to the [code]meta_openxr_simulator.json[/code] file.[/color]";
	} else if (!FileAccess::file_exists(path)) {
		note = "[color=red]Error: No file exists at the given path.[/color]";
	} else if (path.get_extension() != "json") {
		note = "[color=red]Error: The file must be a [code].json[/code] file.[/color]";
	} else if (path.get_file() != "meta_openxr_simulator.json") {
		note = "[color=yellow]Warning: The file is expected to be named [code]meta_openxr_simulator.json[/code].[/color]";
	} else {
		note = "[color=green]The [code]meta_openxr_simulator.json[/code] file exists at the given path.[/color]";
	}

	_message->set_text(META_SIMULATOR_INSTRUCTIONS + note);
}

void MetaXRSimulatorDialog::_on_path_changed(const String &p_path) {
	_set_simulator_path(p_path);
	_update();
}

void MetaXRSimulatorDialog::_on_browse_button_pressed() {
	_file_dialog->set_current_path(_path_field->get_text());
	// @todo Switch to `popup_file_dialog()` - see PR https://github.com/godotengine/godot/pull/91331
	_file_dialog->popup_centered_clamped(Size2(900, 500), 0.7);
}

void MetaXRSimulatorDialog::_on_file_selected(const String &p_path) {
	String resolved_path = ProjectSettings::get_singleton()->globalize_path(p_path);
	if (OS::get_singleton()->has_feature("windows")) {
		// Godot uses forward slashes on Windows, which is fine if we open the file in Godot,
		// but this value will be put in an environment variable and read by the OpenXR loader,
		// which won't recognize the forward slashes.
		resolved_path = resolved_path.replace("/", "\\");
	}
	_path_field->set_text(resolved_path);
	_on_path_changed(resolved_path);
}

void MetaXRSimulatorDialog::_on_meta_clicked(const String &p_path) {
	OS::get_singleton()->shell_open(p_path);
}

String MetaXRSimulatorDialog::_get_simulator_path() {
	EditorInterface *editor_interface = EditorInterface::get_singleton();
	ERR_FAIL_NULL_V(editor_interface, String());

	Ref<EditorSettings> editor_settings = editor_interface->get_editor_settings();
	if (!editor_settings->has_setting(OPENXR_RUNTIME_PATHS_SETTING)) {
		return String();
	}

	Dictionary runtime_paths = editor_settings->get_setting(OPENXR_RUNTIME_PATHS_SETTING);
	return runtime_paths.get(META_SIMULATOR_RUNTIME_NAME, String());
}

void MetaXRSimulatorDialog::_set_simulator_path(const String &p_path) {
	EditorInterface *editor_interface = EditorInterface::get_singleton();
	ERR_FAIL_NULL(editor_interface);

	Dictionary runtime_paths;

	Ref<EditorSettings> editor_settings = editor_interface->get_editor_settings();
	if (editor_settings->has_setting(OPENXR_RUNTIME_PATHS_SETTING)) {
		runtime_paths = editor_settings->get_setting(OPENXR_RUNTIME_PATHS_SETTING);
	}

	runtime_paths[META_SIMULATOR_RUNTIME_NAME] = p_path;
	editor_settings->set_setting(OPENXR_RUNTIME_PATHS_SETTING, runtime_paths);
}

MetaXRSimulatorDialog::MetaXRSimulatorDialog() {
	set_title("Configure Meta XR Simulator");
}
