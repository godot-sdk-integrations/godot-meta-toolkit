// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

#include "editor/meta_export_template_dialog.h"
#include "version.h"

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/editor_file_dialog.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_paths.hpp>
#include <godot_cpp/classes/editor_settings.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/http_request.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/progress_bar.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/zip_reader.hpp>

static const char *META_EXPORT_TEMPLATE_MISSING = "Meta export template is missing, click the Download button to download and install.";
static const char *META_EXPORT_TEMPLATE_ALREADY_INSTALLED = "Meta export template is already installed in this project and it won't be overwritten.\nRemove the res://addons/godot_meta_toolkit/.build_template directory manually before attempting this operation again.";
static const char *META_EXPORT_TEMPLATE_PREBUILT_DIR = "res://addons/godot_meta_toolkit/.build_template/prebuilt";

void MetaExportTemplateDialog::_bind_methods() {
}

void MetaExportTemplateDialog::_notification(uint32_t p_what) {
	switch (p_what) {
		case NOTIFICATION_POSTINITIALIZE: {
			VBoxContainer *main_vbox = memnew(VBoxContainer);
			main_vbox->set_anchors_preset(Control::LayoutPreset::PRESET_FULL_RECT);
			add_child(main_vbox);

			download_progress_bar = memnew(ProgressBar);
			download_progress_bar->set_h_size_flags(Control::SIZE_EXPAND_FILL);
			download_progress_bar->set_v_size_flags(Control::SIZE_SHRINK_CENTER);
			download_progress_bar->set_min(0);
			download_progress_bar->set_max(1);
			download_progress_bar->set_value(0);
			download_progress_bar->set_step(0.01);
			download_progress_bar->set_editor_preview_indeterminate(true);

			main_vbox->add_child(download_progress_bar);
			message = memnew(Label);
			message->set_v_size_flags(Control::SIZE_EXPAND_FILL);
			message->set_h_size_flags(Control::SIZE_SHRINK_CENTER);
			main_vbox->add_child(message);

			download_button = add_button("Download");
			download_button->connect("pressed", callable_mp(this, &MetaExportTemplateDialog::_download_template));
		} break;

		case NOTIFICATION_VISIBILITY_CHANGED: {
			if (!is_visible()) {
				set_process(false);
				is_downloading_template = false;
			}
		} break;

		case NOTIFICATION_PROCESS: {
			update_countdown -= get_process_delta_time();
			if (update_countdown > 0) {
				return;
			}
			update_countdown = 0.5;

			String status;
			int downloaded_bytes;
			int total_bytes;
			bool success = _humanize_http_status(download_request, status, downloaded_bytes, total_bytes);

			if (downloaded_bytes >= 0) {
				if (total_bytes > 0) {
					_set_current_progress_value(float(downloaded_bytes) / total_bytes, status);
				} else {
					_set_current_progress_value(0, status);
				}
			} else {
				_set_current_progress_status(status);
			}

			if (!success) {
				set_process(false);
				is_downloading_template = false;
			}
		} break;
	}
}

bool MetaExportTemplateDialog::_humanize_http_status(HTTPRequest *p_request, String &r_status, int &r_downloaded_bytes, int &r_total_bytes) {
	r_status = "";
	r_downloaded_bytes = -1;
	r_total_bytes = -1;
	bool success = true;

	switch (p_request->get_http_client_status()) {
		case HTTPClient::STATUS_DISCONNECTED:
			r_status = "Disconnected";
			success = false;
			break;
		case HTTPClient::STATUS_RESOLVING:
			r_status = "Resolving";
			break;
		case HTTPClient::STATUS_CANT_RESOLVE:
			r_status = "Can't Resolve";
			success = false;
			break;
		case HTTPClient::STATUS_CONNECTING:
			r_status = "Connecting...";
			break;
		case HTTPClient::STATUS_CANT_CONNECT:
			r_status = "Can't Connect";
			success = false;
			break;
		case HTTPClient::STATUS_CONNECTED:
			r_status = "Connected";
			break;
		case HTTPClient::STATUS_REQUESTING:
			r_status = "Requesting...";
			break;
		case HTTPClient::STATUS_BODY:
			r_status = "Downloading";
			r_downloaded_bytes = p_request->get_downloaded_bytes();
			r_total_bytes = p_request->get_body_size();

			if (p_request->get_body_size() > 0) {
				r_status += " " + String::humanize_size(p_request->get_downloaded_bytes()) + "/" + String::humanize_size(p_request->get_body_size());
			} else {
				r_status += " " + String::humanize_size(p_request->get_downloaded_bytes());
			}
			break;
		case HTTPClient::STATUS_CONNECTION_ERROR:
			r_status = "Connection Error";
			success = false;
			break;
		case HTTPClient::STATUS_TLS_HANDSHAKE_ERROR:
			r_status = "TLS Handshake Error";
			success = false;
			break;
	}

	return success;
}

void MetaExportTemplateDialog::show() {
	popup_centered();

	download_progress_bar->hide();
	message->add_theme_color_override("font_color", get_theme_color(StringName("font_color"), StringName("Label")));

	if (DirAccess::dir_exists_absolute(META_EXPORT_TEMPLATE_PREBUILT_DIR)) {
		download_button->set_disabled(true);
		message->set_text(META_EXPORT_TEMPLATE_ALREADY_INSTALLED);
	} else {
		download_button->set_disabled(false);
		message->set_text(META_EXPORT_TEMPLATE_MISSING);
	}
}

void MetaExportTemplateDialog::_download_template() {
	if (is_downloading_template) {
		return;
	}
	is_downloading_template = true;

	if (download_request) {
		memfree(download_request);
	}

	download_request = memnew(HTTPRequest);
	add_child(download_request);
	download_request->connect("request_completed", callable_mp(this, &MetaExportTemplateDialog::_download_request_completed));

	download_progress_bar->show();
	download_progress_bar->set_indeterminate(true);
	_set_current_progress_status("Starting the download...");

	download_request->set_download_file(EditorInterface::get_singleton()->get_editor_paths()->get_cache_dir().path_join("tmp-meta-export-template.zip"));
	download_request->set_use_threads(true);

	EditorInterface *editor_interface = EditorInterface::get_singleton();
	if (editor_interface == nullptr) {
		_set_current_progress_status(vformat("Failed to get editor interface."), true);
		return;
	}

	Ref<EditorSettings> editor_settings = EditorInterface::get_singleton()->get_editor_settings();
	if (editor_settings.is_null() || !editor_settings->has_setting("network/http_proxy/host") || !editor_settings->has_setting("network/http_proxy/port")) {
		_set_current_progress_status(vformat("Failed to get editor settings."), true);
		return;
	}

	const String proxy_host = editor_settings->get_setting("network/http_proxy/host");
	const int proxy_port = editor_settings->get_setting("network/http_proxy/port");
	download_request->set_http_proxy(proxy_host, proxy_port);
	download_request->set_https_proxy(proxy_host, proxy_port);

	if (!editor_settings->has_setting("xr/meta_toolkit/base_download_url")) {
		_set_current_progress_status(vformat("No base download URL for Meta toolkit found in editor settings."), true);
		return;
	}

	const GDExtensionGodotVersion &godot_version = godot::internal::godot_version;
	String filename = vformat("meta-export-template-v%d.%d.%d", godot_version.major, godot_version.minor, godot_version.patch);
	String download_url = (String)editor_settings->get_setting("xr/meta_toolkit/base_download_url") + GODOT_META_TOOLKIT_VERSION + "/" + filename;
	Error err = download_request->request(download_url);
	if (err != OK) {
		_set_current_progress_status(vformat("Error requesting URL: %s", download_url), true);
		return;
	}

	set_process(true);
	_set_current_progress_status("Connecting to the mirror...");
}

void MetaExportTemplateDialog::_set_current_progress_value(float p_value, const String &p_status) {
	download_progress_bar->show();
	download_progress_bar->set_indeterminate(false);
	download_progress_bar->set_value(p_value);
	message->set_text(p_status);
}

void MetaExportTemplateDialog::_set_current_progress_status(const String &p_status, bool p_error) {
	message->set_text(p_status);

	if (p_error) {
		download_progress_bar->hide();
		message->add_theme_color_override("font_color", get_theme_color(StringName("error_color"), StringName("Editor")));
	} else {
		message->add_theme_color_override("font_color", get_theme_color(StringName("font_color"), StringName("Label")));
	}
}

void MetaExportTemplateDialog::_download_request_completed(int p_status, int p_code, const PackedStringArray &headers, const PackedByteArray &p_data) {
	switch (p_status) {
		case HTTPRequest::RESULT_CANT_RESOLVE: {
			_set_current_progress_status("Can't resolve the requested address.", true);
		} break;
		case HTTPRequest::RESULT_BODY_SIZE_LIMIT_EXCEEDED:
		case HTTPRequest::RESULT_CONNECTION_ERROR:
		case HTTPRequest::RESULT_CHUNKED_BODY_SIZE_MISMATCH:
		case HTTPRequest::RESULT_TLS_HANDSHAKE_ERROR:
		case HTTPRequest::RESULT_CANT_CONNECT: {
			_set_current_progress_status("Can't connect to the mirror.", true);
		} break;
		case HTTPRequest::RESULT_NO_RESPONSE: {
			_set_current_progress_status("No response from the mirror.", true);
		} break;
		case HTTPRequest::RESULT_REQUEST_FAILED: {
			_set_current_progress_status("Request failed.", true);
		} break;
		case HTTPRequest::RESULT_REDIRECT_LIMIT_REACHED: {
			_set_current_progress_status("Request ended up in a redirect loop.", true);
		} break;
		default: {
			if (p_code != 200) {
				_set_current_progress_status("Request failed: " + itos(p_code), true);
			} else {
				_set_current_progress_status("Download complete; extracting template...");
				String download_path = download_request->get_download_file();
				is_downloading_template = false;

				Ref<ZIPReader> zip_reader;
				zip_reader.instantiate();
				if (!FileAccess::file_exists(download_path)) {
					_set_current_progress_status(vformat("Archive not found at path: %s", download_path), true);
				}
				zip_reader->open(download_path);

				if (DirAccess::dir_exists_absolute(META_EXPORT_TEMPLATE_PREBUILT_DIR)) {
					_set_current_progress_status("Meta toolkit template directory already exists", true);
					break;
				}
				if (DirAccess::make_dir_recursive_absolute(META_EXPORT_TEMPLATE_PREBUILT_DIR) != OK) {
					_set_current_progress_status("Error creating Meta toolkit template directory", true);
					break;
				}

				Ref<DirAccess> root_dir = DirAccess::open(META_EXPORT_TEMPLATE_PREBUILT_DIR);
				if (root_dir.is_null()) {
					_set_current_progress_status("Error opening Meta toolkit template directory", true);
					break;
				}

				bool install_failed = false;
				PackedStringArray files = zip_reader->get_files();
				for (const String &file_path : files) {
					if (root_dir->make_dir_recursive(root_dir->get_current_dir().path_join(file_path).get_base_dir()) != OK) {
						install_failed = true;
						_set_current_progress_status("Error creating Meta toolkit install directory.", true);
						break;
					}

					Ref<FileAccess> file = FileAccess::open(root_dir->get_current_dir().path_join(file_path), FileAccess::WRITE);
					if (file.is_null()) {
						install_failed = true;
						_set_current_progress_status("Error opening Meta toolkit file during installation.", true);
						break;
					}

					PackedByteArray buffer = zip_reader->read_file(file_path);
					file->store_buffer(buffer);
				}

				if (install_failed) {
					break;
				}

				zip_reader->close();
				if (DirAccess::remove_absolute(download_path) != OK) {
					WARN_PRINT("Error cleaning up Meta toolkit template download");
				}

				// Download and install success.
				download_progress_bar->hide();
				_set_current_progress_status("Export template setup complete.");
				download_button->set_disabled(true);
			}
		} break;
	}

	set_process(false);
}

MetaExportTemplateDialog::MetaExportTemplateDialog() {
	set_title("Install Meta Export Template");
	set_ok_button_text("Close");
}
