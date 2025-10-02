// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

#pragma once

#include <godot_cpp/classes/accept_dialog.hpp>
#include <godot_cpp/templates/local_vector.hpp>

namespace godot {
class HBoxContainer;
class OptionButton;
class VBoxContainer;
class Label;
} //namespace godot

namespace godot_meta_toolkit_internal {
class Recommendation;
}

using namespace godot;
using namespace godot_meta_toolkit_internal;

class MetaProjectSetupDialog : public AcceptDialog {
	GDCLASS(MetaProjectSetupDialog, AcceptDialog);

	LocalVector<Recommendation *> recommendations;

	OptionButton *project_type_selector = nullptr;
	HBoxContainer *restart_editor_hbox = nullptr;
	VBoxContainer *scroll_vbox = nullptr;
	Label *rec_list_empty_label = nullptr;

	void add_window_entry(Recommendation *p_recommendation);
	void filter_recommendations();
	void _on_recommendation_button_pressed(uint64_t p_recommendation);
	void _on_project_type_selected(int p_item_index);

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
