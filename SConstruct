#!/usr/bin/env python
from glob import glob
from pathlib import Path

env = SConscript("thirdparty/godot-cpp/SConstruct")

opts = Variables('custom.py')
opts.Update(env)

# Add code generator for the Platform SDK.
from generate_platform_sdk_bindings import scons_generate_bindings, scons_emit_files
env.Append(
    BUILDERS={
        "MetaPlatformSDK": Builder(action=scons_generate_bindings, emitter=scons_emit_files),
    })
meta_platform_sdk_bindings = env.MetaPlatformSDK(env.Dir('#toolkit/gen/'), source=[
    env.Dir('#thirdparty/ovr_platform_sdk/Include'),
    "generate_platform_sdk_bindings.py",
])

# Add common includes.
env.Append(CPPPATH=[
    "#thirdparty/ovr_platform_sdk/Include/",
    "#toolkit/src/main/cpp/include/",
    "#toolkit/gen/include/",
    ])

sources = []
sources += Glob("#toolkit/src/main/cpp/*.cpp")
sources += Glob("#toolkit/src/main/cpp/editor/*.cpp")
sources += Glob("#toolkit/src/main/cpp/export/*.cpp")
sources += Glob("#toolkit/src/main/cpp/platform_sdk/*.cpp")
sources += Glob("#toolkit/gen/src/*.cpp")

binary_path = '#demo/addons/godot_meta_toolkit/.bin'
project_name = 'godot_meta_toolkit'

# Create the library target
if env["platform"] == "macos":
    library = env.SharedLibrary(
        "{0}/{1}/{2}/lib{3}.{1}.framework/{3}.{1}".format(
            binary_path,
            env["platform"],
            env["target"],
            project_name,
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "{}/{}/{}/{}/lib{}{}".format(
            binary_path,
            env["platform"],
            env["target"],
            env["arch"],
            project_name,
            env["SHLIBSUFFIX"],
        ),
        source=sources,
    )

Default(library)

