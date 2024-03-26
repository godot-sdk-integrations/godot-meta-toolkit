// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

#include "platform_sdk/meta_platform_sdk_request.h"

#include <godot_cpp/core/class_db.hpp>

void MetaPlatformSDK_Request::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_id"), &MetaPlatformSDK_Request::get_id);
	ADD_SIGNAL(MethodInfo("completed", PropertyInfo(Variant::OBJECT, "message", PROPERTY_HINT_RESOURCE_TYPE, "MetaPlatformSDK_Message")));
}

MetaPlatformSDK_Request::MetaPlatformSDK_Request() {
}
