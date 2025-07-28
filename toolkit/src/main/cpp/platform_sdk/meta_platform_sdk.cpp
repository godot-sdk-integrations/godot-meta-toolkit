// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

/*
 * NOTE:
 *
 *       This is where we put hand-written code for MetaPlatformSDK and other generated classes.
 *       There is another 'meta_platform_sdk.cpp' which is automatically generated.
 *
 */

#include "platform_sdk/meta_platform_sdk.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/main_loop.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/local_vector.hpp>

#include "platform_sdk/meta_platform_sdk_http_transfer_update.h"
#include "platform_sdk/meta_platform_sdk_message.h"
#include "platform_sdk/meta_platform_sdk_packet.h"

#ifdef ANDROID_ENABLED
#include <OVR_Platform.h>
#include <jni.h>

static JNIEnv *jni_env = nullptr;
static jobject jactivity = nullptr;
#endif

MetaPlatformSDK::PlatformInitializeResult MetaPlatformSDK::initialize_platform(const String &p_app_id, const Dictionary &p_options) {
#ifdef ANDROID_ENABLED
	ERR_FAIL_NULL_V(jni_env, PLATFORM_INITIALIZE_UNINITIALIZED);
	ERR_FAIL_NULL_V(jactivity, PLATFORM_INITIALIZE_UNINITIALIZED);

	LocalVector<ovrKeyValuePair> key_value_pairs;
	if (p_options.size() > 0) {
		if (p_options.has("disable_p2p_networking")) {
			key_value_pairs.push_back(ovr_InitConfigOption_CreateBool(ovrInitConfigOption_DisableP2pNetworking, (bool)p_options["disable_p2p_networking"]));
		}
		if (p_options.has("enable_cowatching")) {
			key_value_pairs.push_back(ovr_InitConfigOption_CreateBool(ovrInitConfigOption_EnableCowatching, (bool)p_options["enable_cowatching"]));
		}
	}

	ovrPlatformInitializeResult result;
	if (key_value_pairs.size() > 0) {
		result = ovr_PlatformInitializeAndroidWithOptions(p_app_id.ascii().ptr(), jactivity, jni_env, key_value_pairs.ptr(), key_value_pairs.size());
	} else {
		result = ovr_PlatformInitializeAndroid(p_app_id.ascii().ptr(), jactivity, jni_env);
	}

	if (result != ovrPlatformInitialize_Success) {
		const char *error = ovrPlatformInitializeResult_ToString(result);
		ERR_PRINT(vformat("Unable to initialize Meta Platform SDK with error: %s", error));
	}

	_initialize_platform();

	return (PlatformInitializeResult)result;
#else
	ERR_PRINT("Can only initialize Meta Platform SDK when running on Android");
	return PLATFORM_INITIALIZE_UNINITIALIZED;
#endif
}

Ref<MetaPlatformSDK_Request> MetaPlatformSDK::initialize_platform_async(const String &p_app_id) {
#ifdef ANDROID_ENABLED
	ERR_FAIL_NULL_V(jni_env, Ref<MetaPlatformSDK_Request>());
	ERR_FAIL_NULL_V(jactivity, Ref<MetaPlatformSDK_Request>());

	Ref<MetaPlatformSDK_Request> request = MetaPlatformSDK::_create_request(ovr_PlatformInitializeAndroidAsynchronous(p_app_id.ascii().ptr(), jactivity, jni_env));
	request->connect("completed", callable_mp(this, &MetaPlatformSDK::_initialize_platform_async));

	// Need to initialize so that async requests will be handled.
	_initialize_platform();

	return request;
#else
	ERR_PRINT("Can only initialize Meta Platform SDK when running on Android");
	return Ref<MetaPlatformSDK_Request>();
#endif
}

#ifdef ANDROID_ENABLED
extern "C" {
JNIEXPORT void JNICALL Java_com_meta_w4_godot_toolkit_GodotMetaToolkit_initPlatformSDK(JNIEnv *p_env, jobject p_obj, jobject p_activity) {
	jni_env = p_env;
	jactivity = reinterpret_cast<jobject>(jni_env->NewGlobalRef(p_activity));
}
}

void MetaPlatformSDK::_initialize_platform() {
	if (!_platform_initialized) {
		_platform_initialized = true;

		MainLoop *main_loop = Engine::get_singleton()->get_main_loop();
		main_loop->connect("process_frame", callable_mp(this, &MetaPlatformSDK::_process_messages));
	}
}

void MetaPlatformSDK::_initialize_platform_async(const Ref<MetaPlatformSDK_Message> &p_message) {
	Ref<MetaPlatformSDK_PlatformInitialize> pi = p_message->get_platform_initialize();
	ovrPlatformInitializeResult result = (ovrPlatformInitializeResult)pi->get_result();
	if (result != ovrPlatformInitialize_Success) {
		const char *error = ovrPlatformInitializeResult_ToString(result);
		ERR_PRINT(vformat("Unable to initialize Meta Platform SDK with error: %s", error));
	}
}

Ref<MetaPlatformSDK_Request> MetaPlatformSDK::_create_request(ovrRequest p_request) {
	Ref<MetaPlatformSDK_Request> request;
	request.instantiate();
	request->id = p_request;
	requests[p_request] = request;
	return request;
}

void MetaPlatformSDK::_process_messages() {
	while (ovrMessageHandle message_handle = ovr_PopMessage()) {
		Ref<MetaPlatformSDK_Message> message = MetaPlatformSDK_Message::_create_with_ovr_handle(message_handle);
		if (message->is_notification()) {
			emit_signal("notification_received", message);
		} else {
			ovrRequest request_id = (ovrRequest)message->get_request_id();
			if (request_id != 0) {
				Ref<MetaPlatformSDK_Request> *request = requests.getptr(request_id);
				if (request) {
					(*request)->emit_signal("completed", message);
					requests.erase(request_id);
				} else {
					ERR_PRINT(vformat("MetaPlatformSDK: Received message %s with unknown request id %s", ovrMessageType_ToString((ovrMessageType)message->get_type()), request_id));
				}
			}
		}
	}
}
#endif

/*
 * Next, hand-written functions for other generated classes.
 */

uint64_t MetaPlatformSDK_Message::get_request_id() const {
#ifdef ANDROID_ENABLED
	return ovr_Message_GetRequestID(handle);
#else
	return 0;
#endif
}

bool MetaPlatformSDK_Message::is_notification() const {
#ifdef ANDROID_ENABLED
	return ovrMessageType_IsNotification((ovrMessageType)type);
#else
	return 0;
#endif
}

uint64_t MetaPlatformSDK_HttpTransferUpdate::get_id() const {
#ifdef ANDROID_ENABLED
	return ovr_HttpTransferUpdate_GetID(handle);
#else
	return 0;
#endif
}

PackedByteArray MetaPlatformSDK_ChallengeEntry::get_extra_data() const {
#ifdef ANDROID_ENABLED
	PackedByteArray result;

	const char *data = ovr_ChallengeEntry_GetExtraData(handle);
	if (data) {
		size_t size = ovr_ChallengeEntry_GetExtraDataLength(handle);
		if (size > 0) {
			result.resize(size);
			memcpy(result.ptrw(), data, size);
		}
	}

	return result;
#else
	return PackedByteArray();
#endif
}

PackedByteArray MetaPlatformSDK_LeaderboardEntry::get_extra_data() const {
#ifdef ANDROID_ENABLED
	PackedByteArray result;

	const char *data = ovr_LeaderboardEntry_GetExtraData(handle);
	if (data) {
		size_t size = ovr_LeaderboardEntry_GetExtraDataLength(handle);
		if (size > 0) {
			result.resize(size);
			memcpy(result.ptrw(), data, size);
		}
	}

	return result;
#else
	return PackedByteArray();
#endif
}

PackedByteArray MetaPlatformSDK_HttpTransferUpdate::get_bytes() const {
#ifdef ANDROID_ENABLED
	PackedByteArray result;

	const void *data = ovr_HttpTransferUpdate_GetBytes(handle);
	if (data) {
		size_t size = ovr_HttpTransferUpdate_GetSize(handle);
		if (size > 0) {
			result.resize(size);
			memcpy(result.ptrw(), data, size);
		}
	}

	return result;
#else
	return PackedByteArray();
#endif
}

PackedByteArray MetaPlatformSDK_Packet::get_bytes() const {
#ifdef ANDROID_ENABLED
	PackedByteArray result;

	const void *data = ovr_Packet_GetBytes(handle);
	if (data) {
		size_t size = ovr_Packet_GetSize(handle);
		if (size > 0) {
			result.resize(size);
			memcpy(result.ptrw(), data, size);
		}
	}

	return result;
#else
	return PackedByteArray();
#endif
}
