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

if env["target"] in ["editor", "template_debug"]:
  doc_data = env.GodotCPPDocData("#toolkit/src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
  sources.append(doc_data)

binary_path = '#demo/addons/godot_meta_toolkit/.bin'
android_src_path = '#toolkit/src'
project_name = 'godot_meta_toolkit'

if env['platform'] == "android" and env["target"] != "editor":
    env.Append(LIBPATH=['thirdparty/ovr_platform_sdk/Android/libs/arm64-v8a'])
    env.Append(LIBS=['ovrplatformloader'])

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

if env["platform"] == "android" and env["target"] != "editor":
    # Copy the libovrplatformloader.so files to the addon
    ovrplatformloader_copy_path = "#thirdparty/ovr_platform_sdk/Android/libs/arm64-v8a/libovrplatformloader.so"
    ovrplatformloader_copy_dest = "{}/{}/{}/{}/libovrplatformloader.so".format(
                                              binary_path,
                                              env["platform"],
                                              env["target"],
                                              env["arch"])
    ovrplatformloader_copy = env.Command(ovrplatformloader_copy_dest, ovrplatformloader_copy_path, Copy('$TARGET', '$SOURCE'))
    Default(ovrplatformloader_copy)

    # Copy the libgodot_meta_toolkit.so files to the project libs directory
    android_target = "release" if env["target"] == "template_release" else "debug"
    android_arch = ""
    if env["arch"] == "arm64":
        android_arch = "arm64-v8a"
    elif env["arch"] == "x86_64":
        android_arch = "x86_64"
    else:
        raise Exception("Unable to map %s to Android architecture name" % env["arch"])

    library_copy_path = "{}/main/libs/{}/{}/{}/lib{}{}".format(
        android_src_path,
        android_target,
        android_arch,
        android_arch,
        project_name,
        env["SHLIBSUFFIX"])

    library_copy = env.Command(library_copy_path, library, Copy('$TARGET', '$SOURCE'))

    Default(library_copy)
