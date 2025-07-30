// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

#pragma once

#include <godot_cpp/classes/accept_dialog.hpp>

namespace godot {
class Label;
class ProgressBar;
class Button;
class HTTPRequest;
} //namespace godot

using namespace godot;

class MetaExportTemplateDialog : public AcceptDialog {
	GDCLASS(MetaExportTemplateDialog, AcceptDialog);

	Label *message = nullptr;
	ProgressBar *download_progress_bar = nullptr;
	Button *download_button = nullptr;
	HTTPRequest *download_request = nullptr;

	float update_countdown = 0.0f;

	bool is_downloading_template = false;

	void _update();

	void _download_template();
	void _set_current_progress_value(float p_value, const String &p_status);
	void _set_current_progress_status(const String &p_status, bool p_error = false);
	void _download_request_completed(int p_status, int p_code, const PackedStringArray &headers, const PackedByteArray &p_data);
	bool _humanize_http_status(HTTPRequest *p_request, String &r_status, int &r_downloaded_bytes, int &r_total_bytes);

protected:
	static void _bind_methods();

	void _notification(uint32_t p_what);

public:
	void show();

	MetaExportTemplateDialog();
};
