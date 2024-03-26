// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.

#include "register_types.h"

#include <godot_cpp/classes/engine.hpp>

#include "editor/meta_toolkit_editor_plugin.h"
#include "editor/meta_xr_simulator_dialog.h"
#include "export/meta_toolkit_export_plugin.h"
#include "platform_sdk/meta_platform_sdk.h"

using namespace godot;

void initialize_toolkit_module(ModuleInitializationLevel p_level) {
    switch (p_level) {
        case godot::MODULE_INITIALIZATION_LEVEL_SCENE: {
            ClassDB::register_class<MetaPlatformSDK_Request>();

            // Register generated classes last, because they may use the hand-written ones.
            MetaPlatformSDK::_register_generated_classes();

            // Now that everything is registered, we can safely create our singleton.
            Engine::get_singleton()->register_singleton("MetaPlatformSDK", MetaPlatformSDK::get_singleton());
        } break;
        case godot::MODULE_INITIALIZATION_LEVEL_EDITOR: {
            ClassDB::register_class<MetaToolkitExportPlugin>();
            ClassDB::register_class<MetaXRSimulatorDialog>();
            ClassDB::register_class<MetaToolkitEditorPlugin>();
            EditorPlugins::add_by_type<MetaToolkitEditorPlugin>();
        } break;
    }
}

void terminate_toolkit_module(ModuleInitializationLevel p_level) {}

extern "C" {
GDExtensionBool GDE_EXPORT
toolkit_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
                     GDExtensionClassLibraryPtr p_library,
                     GDExtensionInitialization *r_initialization) {
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_toolkit_module);
    init_obj.register_terminator(terminate_toolkit_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
}
