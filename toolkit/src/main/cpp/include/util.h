// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

#pragma once

#include <godot_cpp/templates/local_vector.hpp>

struct CharStringList {
	LocalVector<CharString> list;
	LocalVector<const char *> pointers;

	CharStringList(const PackedStringArray &p_array) {
		list.resize(p_array.size());
		pointers.resize(p_array.size());
		for (int i = 0; i < p_array.size(); i++) {
			list[i] = p_array[i].utf8();
			pointers[i] = list[i].get_data();
		}
	}
};
