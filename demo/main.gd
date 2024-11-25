extends XROrigin3D

const PanelSwitcherLayerScene = preload("res://panel_switcher_layer.tscn")
const PanelSwitcherLayer = preload("res://panel_switcher_layer.gd")
const PanelSwitcherScene = preload("res://panel_switcher.tscn")
const PanelSwitcher = preload("res://panel_switcher.gd")

var panel_switcher: PanelSwitcher
var panel_switcher_layer: PanelSwitcherLayer
var pointer_pressed := false

## Current XR interface
var xr_interface: XRInterface
var hand_tracking_source: Array[OpenXRInterface.HandTrackedSource]

@onready var turkey = $turkey
@onready var left_controller_model: OpenXRFbRenderModel = $LeftController/LeftControllerRenderModel
@onready var right_controller_model: OpenXRFbRenderModel = $RightController/RightControllerRenderModel
@onready var right_controller_pointer: XRController3D = $RightControllerPointer

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	var data_string: String = MetaHybridApp.get_launch_data()
	if data_string != "":
		var data: Dictionary = JSON.parse_string(data_string)

		# Restore the turkey's rotation.
		var turkey_rotation := Vector3(0.0, data.get('turkey_y_rotation', 0.0), 0.0)
		turkey.transform.basis = Basis.from_euler(turkey_rotation)

	xr_interface = XRServer.find_interface("OpenXR")
	if xr_interface and xr_interface.is_initialized():
		var vp: Viewport = get_viewport()
		vp.use_xr = true

		# Give the viewport a 'transparent_bg' so we can see our passthrough
		vp.transparent_bg = true

		# Disable vsync
		DisplayServer.window_set_vsync_mode(DisplayServer.VSYNC_DISABLED)

		# Enable passthrough
		xr_interface.environment_blend_mode = XRInterface.XR_ENV_BLEND_MODE_ALPHA_BLEND

		hand_tracking_source.resize(OpenXRInterface.HAND_MAX)
		for hand in OpenXRInterface.HAND_MAX:
			hand_tracking_source[hand] = xr_interface.get_hand_tracking_source(hand)

		panel_switcher_layer = PanelSwitcherLayerScene.instantiate()
		add_child(panel_switcher_layer)
		panel_switcher_layer.global_transform = %PanelSwitcherMarker.global_transform
		panel_switcher = panel_switcher_layer.get_panel_switcher()

	else:
		panel_switcher = PanelSwitcherScene.instantiate()
		add_child(panel_switcher)

	print("Hybrid App Mode: ", MetaHybridApp.get_mode())

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _physics_process(delta: float) -> void:
	# Make the gltf model slowly rotate
	if turkey:
		turkey.rotate_y(0.001)

		# Store the data in the panel switcher, so the turkey rotation is maintained in the other mode.
		var turkey_rotation: Vector3 = turkey.transform.basis.get_euler()
		panel_switcher.data['turkey_y_rotation'] = turkey_rotation.y

	if xr_interface and xr_interface.is_initialized():
		for hand in OpenXRInterface.HAND_MAX:
			var source = xr_interface.get_hand_tracking_source(hand)
			if hand_tracking_source[hand] == source:
				continue

			var controller = left_controller_model if (hand == OpenXRInterface.HAND_LEFT) else right_controller_model
			controller.visible = (source == OpenXRInterface.HAND_TRACKED_SOURCE_CONTROLLER)

			hand_tracking_source[hand] = source

func _process(p_delta: float) -> void:
	if panel_switcher_layer:
		var t: Transform3D = right_controller_pointer.global_transform
		panel_switcher_layer.update_pointer(t.origin, -t.basis.z, pointer_pressed)

func _on_right_controller_pointer_button_pressed(p_name: String) -> void:
	if p_name == 'trigger_click':
		pointer_pressed = true

func _on_right_controller_pointer_button_released(p_name: String) -> void:
	if p_name == 'trigger_click':
		pointer_pressed = false
