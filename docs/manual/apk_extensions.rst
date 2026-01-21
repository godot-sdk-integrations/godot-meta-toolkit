.. _manual_apk_extensions:

APK Extensions
==============

Mobile and XR operating systems and app distribution platforms commonly impose strict size limits on apps,
especially compared to desktop platforms. For example, apps on
`Meta Horizon are limited to 1 GB in size <https://developers.meta.com/horizon/resources/vrc-quest-packaging-5/>`_
for the APK file. While not necessarily a problem for some apps, games often require considerable
assets and can easily exceed those limits.

Thankfully, there are ways to distribute additional data beyond the single APK file: APK extensions.
*Meta Horizon* supports two kinds of APK extensions, as outlined
`in the documentation <https://developers.meta.com/horizon/documentation/native/ps-assets/>`_:
**Opaque Binary Blob (OBB)** files and **Required Asset** files.

Opaque Binary Blobs (OBBs) vs. Required Assets
----------------------------------------------

**Opaque Binary Blobs (OBBs)** were supported in Android starting with version
`2.3 Gingerbread (2010) <https://en.wikipedia.org/wiki/Opaque_binary_blob>`_.

**Required Asset** files are specific to *Meta Horizon*.
They are similar, but different (see the
`documentation <https://developers.meta.com/horizon/documentation/native/ps-assets/>`_ for full details).

Both allow distributing arbitrary data of up to 4 GB per file (although 2 GB are recommended),
both are located at a similar path in the system and need to be checked for (and downloaded if missing) by
the app at runtime, and both may be installed automatically when the app is installed.

While both support arbitrary file types, **OBB** files *can* be special ZIP archives (with ``.obb`` extension) that support
signing and encryption (built via Android's ``jobb`` tool).

Note that not all distribution platforms support all formats or impose the same limits. **Required Asset** files are
specific to *Meta Horizon*, and *Google Play* no longer supports OBB expansion files.

As the Godot OBB export functionality is specific to Google Play, it is easiest to use Required Asset files
on Meta Horizon to upload additional Godot PCK files.

General considerations when using APK extensions
------------------------------------------------

Optimize first
~~~~~~~~~~~~~~

Consider whether you can optimize or compress your app and used assets before adding the complexity of APK extensions.
Maybe you can just trim some unused (or rarely) used files or load them on demand? See also:
`Optimizing a build for size <https://docs.godotengine.org/en/stable/engine_details/development/compiling/optimizing_for_size.html>`_
in the Godot documentation.

Permissions
~~~~~~~~~~~

If you go ahead with APK expansions, make sure your app has (or prompts for) the required permissions.
Implementing a comprehensive, reliable downloader and expansion handling
likely requires additional considerations,
`see the Android documentation <https://developer.android.com/google/play/expansion-files#Permissions>`_ for some pointers.

Read the documentation
~~~~~~~~~~~~~~~~~~~~~~

The `Android documentation on expansion files <https://developer.android.com/google/play/expansion-files>`_
is worth a read. Additionally,
`the Meta Horizon documentation on expansion files <https://developers.meta.com/horizon/documentation/native/ps-assets/>`_,
and for Required Assets specifically, the
`Meta blog post announcing the feature <https://developers.meta.com/horizon/blog/introducing-mobile-dlc-support-in-beta/>`_
provide helpful information.

Setup export presets
~~~~~~~~~~~~~~~~~~~~

Consider setting up additional export presets preconfigured to export your expansion files, whether proper
OBB files (exported via the Android exporter, as described below) or PCK files for use as OBB or Required Asset files,
using the normal Godot `exporting facilities <https://docs.godotengine.org/en/stable/tutorials/export/exporting_pcks.html>`_.
You can use export filters and inclusion lists in those export presets to make sure only the intended files are exported
in each extension file, and that those files are excluded from the APK itself.
When first setting this up, make sure to double check whether the configuration works as intended
(potentially using tools such as `GodotPCKExplorer <https://github.com/DmitriySalnikov/GodotPCKExplorer>`_).

App implementation
~~~~~~~~~~~~~~~~~~

After ensuring (and potentially requesting) any needed permissions, the app should check whether the expansion file
exists at the expected path at launch. Some verification may make sense here, like comparing the file size, hash, or signature,
or trying to decrypt the data if encryption was used. The distribution platform may also offer ways to do signature,
vendor or licensing checks specific to that distribution platform.

If any required expansion files are missing, the app needs to check for and download them.

For *Meta Horizon*, the APIs
:ref:`asset_file_get_list_async<class_metaplatformsdk_method_asset_file_get_list_async>`,
:ref:`asset_file_download_by_id_async<class_metaplatformsdk_method_asset_file_download_by_id_async>` and
``MESSAGE_NOTIFICATION_ASSET_FILE_DOWNLOAD_UPDATE`` are a good place to start.

If your app is present on multiple distribution platforms (or planning to be), it may be worth considering
hosting expansion files yourself and implementing the downloader to load them from your own source so they can be used
across platforms. Make sure to check your distribution platforms' terms and guidelines first.

Finally, the app needs to read and use the data. In the case of a PCK,
`ProjectSettings.load_resource_pack() <https://docs.godotengine.org/en/stable/classes/class_projectsettings.html#class-projectsettings-method-load-resource-pack>`_
can be used to load the PCK and access files with standard Godot facilities.

Testing
~~~~~~~

For testing, `ADB <https://developer.android.com/tools/adb>`_ can be used to push the extension files to the expected
paths on testing devices. Make sure your app has the required permissions to read the files. Additionally, take note
that both the extension files and APK are set to the same Android user, or that the APK user at least has the necessary
file permissions on the extension file.

Using Opaque Binary Blobs (OBBs)
--------------------------------

As OBB support was phased out on Google Play and the Godot OBB export functionality was specific to Google Play,
we recommend using Required Assets instead for the Meta Horizon platform.

OBBs can be any arbitrary file type, as outlined in the
`Android documentation <https://developer.android.com/google/play/expansion-files#Filename>`_. Thus, the easiest way
is to use the standard Godot process to
`export a PCK <https://docs.godotengine.org/en/stable/tutorials/export/exporting_pcks.html>`_
to use as an extension file and rename it to match the required scheme, ending in ``.obb``.
As noted under general considerations, consider creating export presets to make this easy in the future,
using filter and inclusion lists to configure what ends up in which expansion file.

Finally, you need to make sure your app implements the required permission checks and code to download and read the
expansion files (see general considerations).

Using Required Assets
---------------------

Use the standard Godot process to
`export one or more PCK files <https://docs.godotengine.org/en/stable/tutorials/export/exporting_pcks.html>`_
to use as extension files and rename them as required. As noted under general considerations,
consider creating export presets to make this easy in the future, using filter and inclusion lists to configure
what ends up in which expansion file.

In addition to the expansion file itself, a
`manifest file needs to be created <https://developers.meta.com/horizon/documentation/native/ps-assets/#upload>`_
in the same directory structure as the expansion file, which is passed as a path to the uploader in the next step.
In general, you will likely want to mark the expansion file as ``required``, indicating it should be automatically
downloaded at install time if possible.

Afterwards upload your expansion file (and manifest) to *Meta Horizon* using the
`ovr-platform-util <https://developers.meta.com/horizon/resources/publish-reference-platform-command-line-utility/>`_
(as described
`in the documentation <https://developers.meta.com/horizon/documentation/native/ps-assets/#upload>`_,
although take note that the exact command may have since changed with updates to the tool).
