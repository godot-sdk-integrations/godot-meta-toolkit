extends XROrigin3D

## Current XR interface
var xr_interface: XRInterface
var hand_tracking_source: Array[OpenXRInterface.HandTrackedSource]

@onready var turkey = $turkey
@onready var left_controller_model: OpenXRFbRenderModel = $LeftController/LeftControllerRenderModel
@onready var right_controller_model: OpenXRFbRenderModel = $RightController/RightControllerRenderModel

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
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

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _physics_process(delta: float) -> void:
	# Make the gltf model slowly rotate
	if turkey:
		turkey.rotate_y(0.001)

	for hand in OpenXRInterface.HAND_MAX:
		var source = xr_interface.get_hand_tracking_source(hand)
		if hand_tracking_source[hand] == source:
			continue

		var controller = left_controller_model if (hand == OpenXRInterface.HAND_LEFT) else right_controller_model
		controller.visible = (source == OpenXRInterface.HAND_TRACKED_SOURCE_CONTROLLER)

		hand_tracking_source[hand] = source

