// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

#pragma once

#ifdef ANDROID_ENABLED
#include <OVR_Types.h>
#endif // ANDROID_ENABLED

#include <godot_cpp/classes/ref.hpp>

using namespace godot;

class MetaPlatformSDK_Request : public RefCounted {
	GDCLASS(MetaPlatformSDK_Request, RefCounted);

	friend class MetaPlatformSDK;

#ifdef ANDROID_ENABLED
	ovrRequest id = 0;
#endif // ANDROID_ENABLED

protected:
	static void _bind_methods();

public:
	inline uint64_t get_id() {
#ifdef ANDROID_ENABLED
		return id;
#else
		return 0;
#endif // ANDROID_ENABLED
	}

	MetaPlatformSDK_Request();
};
