// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

#pragma once

#if defined(ANDROID_ENABLED) && !defined(TOOLS_ENABLED)
#include <OVR_Types.h>
#endif // defined(ANDROID_ENABLED) && !defined(TOOLS_ENABLED)

#include <godot_cpp/classes/ref.hpp>

using namespace godot;

class MetaPlatformSDK_Request : public RefCounted {
	GDCLASS(MetaPlatformSDK_Request, RefCounted);

	friend class MetaPlatformSDK;

#if defined(ANDROID_ENABLED) && !defined(TOOLS_ENABLED)
	ovrRequest id = 0;
#endif // defined(ANDROID_ENABLED) && !defined(TOOLS_ENABLED)

protected:
	static void _bind_methods();

public:
	inline uint64_t get_id() {
#if defined(ANDROID_ENABLED) && !defined(TOOLS_ENABLED)
		return id;
#else
		return 0;
#endif // defined(ANDROID_ENABLED) && !defined(TOOLS_ENABLED)
	}

	MetaPlatformSDK_Request();
};
