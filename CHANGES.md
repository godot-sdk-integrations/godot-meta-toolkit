# Change history for the Godot Meta Toolkit plugin

## 1.0.3
- Modernize class registration
- Fix clang format issues
- Windows: Convert forward slashes from file dialog to backslashes for Meta XR simulator
- Update for Meta Platform SDK v77

## 1.0.2
- Include API documentation in the editor
- Update for Meta Platform SDK v72
- Add demo project demonstrating the Platform SDK

## 1.0.1
- Fix `MetaPlatformSDK_LeaderboardEntry::get_extra_data()` and `MetaPlatformSDK_ChallengeEntry::get_extra_data()` to return `PackedByteArray`
- Change `MetaPlatformSDK_Message.get_error()` method into `error` property
- Encode `PackedStringArray` as UTF-8 before passing to underlying Meta Platform SDK functions
- Get API documentation coverage up to 100% and other small documentation improvements
- Remove `*_long` suffix from `MetaPlatformSDK_AssetFileDownloadUpdate.get_bytes_total_long()` and `.get_bytes_transferred_long()`

## 1.0.0
- Initial release
- Support for Meta Platform SDK v71
- Tool for configuring the Meta XR Simulator
