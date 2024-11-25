class_name MetaHybridApp
extends RefCounted

enum HybridAppMode {
	NONE = -1,
	IMMERSIVE = 0,
	PANEL = 1
}

static func is_hybrid_app() -> bool:
	return OS.has_feature("meta_hybrid_app")

static func get_mode() -> HybridAppMode:
	if not is_hybrid_app():
		return HybridAppMode.NONE

	if OS.has_feature("meta_panel_app"):
		return HybridAppMode.PANEL

	return HybridAppMode.IMMERSIVE

static func switch_mode(p_mode: HybridAppMode, p_data: String = "") -> bool:
	if p_mode == HybridAppMode.NONE:
		return false

	if not is_hybrid_app():
		return false

	var GodotMetaToolkit = Engine.get_singleton("GodotMetaToolkit")
	if not GodotMetaToolkit:
		return false

	return GodotMetaToolkit.hybridAppSwitchTo(p_mode, p_data)

static func get_launch_data() -> String:
	if not is_hybrid_app():
		return ""

	var GodotMetaToolkit = Engine.get_singleton("GodotMetaToolkit")
	if not GodotMetaToolkit:
		return ""

	return GodotMetaToolkit.getHybridAppLaunchData()
