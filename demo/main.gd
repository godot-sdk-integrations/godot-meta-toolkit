extends XROrigin3D

const MAX_DISPLAY_FRIENDS := 5
const PRODUCT_SKU := "00"
const SIMPLE_ACHIEVEMENT_NAME := "simple-achievement-example"
const COUNT_ACHIEVEMENT_NAME := "count-achievement-example"
const BITFIELD_ACHIEVEMENT_NAME := "bitfield-achievement-example"
const BITFIELD_ACHIEVEMENT_LENGTH := 5

# You need to supply your own application ID from https://developers.meta.com/ in order to test this app.
var APPLICATION_ID = ""

var platform_sdk_initialized := false

var purchase_processing := false
var product_purchased := false

var simple_achievement_processing := true
var count_achievement_processing := true
var bitfield_achievement_processing := true
var simple_achievement_unlocked := false
var count_achievement_unlocked := false
var bitfield_achievement_unlocked := false


@onready var initialization_info: Node3D = $InitializationInfo
@onready var user_info: Node3D = $UserInfo
@onready var achievement_info: Node3D = $AchievementInfo
@onready var iap_info: Node3D = $IAPInfo
@onready var friend_info: Node3D = $FriendInfo

@onready var left_controller_ray_cast: RayCast3D = $LeftController/LeftControllerRayCast
@onready var right_controller_ray_cast: RayCast3D = $RightController/RightControllerRayCast
@onready var initialization_label: Label3D = $InitializationInfo/InitializationLabel
@onready var entitled_label: Label3D = $UserInfo/EntitledLabel
@onready var oculus_id_label: Label3D = $UserInfo/OculusIDLabel
@onready var user_image: Sprite3D = $UserInfo/UserImage
@onready var friend_names_label: Label3D = $FriendInfo/FriendNamesLabel
@onready var iap_label: Label3D = $IAPInfo/IAPLabel
@onready var simple_achievement_label: Label3D = $AchievementInfo/SimpleAchievementInfo/SimpleAchievementLabel
@onready var count_achievement_label: Label3D = $AchievementInfo/CountAchievementInfo/CountAchievementLabel
@onready var bitfield_achievement_label: Label3D = $AchievementInfo/BitfieldAchievementInfo/BitfieldAchievementLabel

func _ready() -> void:
	var xr_interface = XRServer.find_interface("OpenXR")
	if xr_interface and xr_interface.is_initialized():
		var vp: Viewport = get_viewport()
		vp.use_xr = true

		vp.transparent_bg = true
		DisplayServer.window_set_vsync_mode(DisplayServer.VSYNC_DISABLED)
		xr_interface.environment_blend_mode = XRInterface.XR_ENV_BLEND_MODE_ALPHA_BLEND

	if ResourceLoader.exists("res://local.gd"):
		var local = load('res://local.gd')
		if local and "APPLICATION_ID" in local:
			APPLICATION_ID = local.APPLICATION_ID

	if APPLICATION_ID == "":
		initialization_label.text += "No app ID provided!"
		hide_non_initialization_info()
		return

	initialize_platform_sdk()


func initialize_platform_sdk():
	var result: MetaPlatformSDK_Message

	result = await MetaPlatformSDK.initialize_platform_async(APPLICATION_ID).completed
	if result.is_error():
		initialization_label.text += "FAILED"
		hide_non_initialization_info()
		return

	var platform_initialize := result.get_platform_initialize()
	if platform_initialize.result != MetaPlatformSDK.PLATFORM_INITIALIZE_SUCCESS:
		initialization_label.text += "FAILED"
		hide_non_initialization_info()
		return

	platform_sdk_initialized = true
	initialization_label.text += "SUCCESS"

	update_user_info()
	update_friend_info()
	update_iap_info()
	update_achievement_info()


func update_user_info():
	var result: MetaPlatformSDK_Message

	result = await MetaPlatformSDK.entitlement_get_is_viewer_entitled_async().completed
	if result.is_success():
		entitled_label.text += "TRUE"
	else:
		entitled_label.text += "FALSE"

	result = await MetaPlatformSDK.user_get_logged_in_user_async().completed
	if result.is_error():
		oculus_id_label.text = "Failed to get user data!"
		push_error("Failed to get user data: ", result.error)
		return

	var user: MetaPlatformSDK_User = result.get_user()
	oculus_id_label.text += user.oculus_id

	var image_request = HTTPRequest.new()
	add_child(image_request)
	image_request.request_completed.connect(self._image_request_completed.bind(image_request))

	var error = image_request.request(user.image_url)
	if error != OK:
		push_error("There was an error with the image request.")


func update_friend_info():
	var result: MetaPlatformSDK_Message = await MetaPlatformSDK.user_get_logged_in_user_friends_async().completed
	if result.is_error():
		friend_names_label.text = "Error retrieving friends!"
		push_error("Error retrieving friends: ", result.error)
		return


	var friend_array := result.get_user_array()
	if friend_array.size() == 0:
		return

	var friend_count = 0
	friend_names_label.text = ""
	for friend in friend_array:
		if friend_count >= MAX_DISPLAY_FRIENDS:
			break
		var friend_name = friend.display_name if friend.display_name != "" else friend.oculus_id
		friend_names_label.text += friend_name + "\n"
		friend_count += 1


func update_iap_info():
	var result: MetaPlatformSDK_Message = await MetaPlatformSDK.iap_get_viewer_purchases_async().completed
	if result.is_error():
		iap_label.text = "Error getting\nuser purchases!"
		purchase_processing = false
		return

	var purchase_array := result.get_purchase_array()
	for purchase: MetaPlatformSDK_Purchase in purchase_array:
		if purchase.sku == PRODUCT_SKU:
			product_purchased = true
			break

	if product_purchased:
		iap_label.text = "You have purchased\nthe in-app product!"

	purchase_processing = false

func update_achievement_info():
	var result: MetaPlatformSDK_Message
	result = await MetaPlatformSDK.achievements_get_all_progress_async().completed
	if result.is_error():
		simple_achievement_label.text = "Error getting\nachievement progress!"
		count_achievement_label.text = "Error getting\nachievement progress!"
		bitfield_achievement_label.text = "Error getting\nachievement progress!"
		simple_achievement_processing = false
		count_achievement_processing = false
		bitfield_achievement_processing = false
		push_error("Couldn't get achievement progress: ", result.error)
		return

	var achievement_progress_array := result.get_achievement_progress_array()
	for achievement_progress: MetaPlatformSDK_AchievementProgress in achievement_progress_array:
		match achievement_progress.name:
			SIMPLE_ACHIEVEMENT_NAME:
				if achievement_progress.is_unlocked:
					simple_achievement_label.text = "Simple Achievement\nUnlocked!"
					simple_achievement_unlocked = true
			COUNT_ACHIEVEMENT_NAME:
				if achievement_progress.is_unlocked:
					count_achievement_label.text = "Count Achievement\nUnlocked!"
					count_achievement_unlocked = true
				else:
					count_achievement_label.text = "Count is %s\nout of 3" % achievement_progress.count
			BITFIELD_ACHIEVEMENT_NAME:
				if achievement_progress.is_unlocked:
					bitfield_achievement_label.text = "Bitfield Achievement\nUnlocked!"
					bitfield_achievement_unlocked = true
				else:
					var active_bits = achievement_progress.bitfield.split().count("1")
					bitfield_achievement_label.text = "%s of 5 bits active\nActivate 3 bits to unlock" % active_bits

	simple_achievement_processing = false
	count_achievement_processing = false
	bitfield_achievement_processing = false


func hide_non_initialization_info():
	user_info.hide()
	achievement_info.hide()
	iap_info.hide()
	friend_info.hide()


func update(collider_name):
	if not platform_sdk_initialized:
		return

	match collider_name:
		"IAPButton":
			purchase_product()
		"SimpleAchievementButton":
			unlock_simple_achievement()
		"CountAchievementButton":
			increment_count_achievement()
		"BitfieldAchievementButton1":
			add_field_bitfield_achievement(0)
		"BitfieldAchievementButton2":
			add_field_bitfield_achievement(1)
		"BitfieldAchievementButton3":
			add_field_bitfield_achievement(2)
		"BitfieldAchievementButton4":
			add_field_bitfield_achievement(3)
		"BitfieldAchievementButton5":
			add_field_bitfield_achievement(4)


func purchase_product():
	if purchase_processing or product_purchased:
		return

	iap_label.text = "Processing..."
	purchase_processing = true

	# This App's example product is listed as free in the Meta Quest Developer Dashboard.
	# Launching checkout flow for a free product like this will cause a panel to appear momentarily.
	var result: MetaPlatformSDK_Message = await MetaPlatformSDK.iap_launch_checkout_flow_async(PRODUCT_SKU).completed
	if result.is_error():
		iap_label.text = "Error launching\nproduct checkout flow!"
		purchase_processing = false;
		return

	update_iap_info()


func unlock_simple_achievement():
	if simple_achievement_processing or simple_achievement_unlocked:
		return

	simple_achievement_label.text = "Processing..."
	simple_achievement_processing = true

	var result: MetaPlatformSDK_Message = await MetaPlatformSDK.achievements_unlock_async(SIMPLE_ACHIEVEMENT_NAME).completed
	if result.is_error():
		simple_achievement_label.text = "Error unlocking\nSimple Achievement"
	else:
		var achievement_update := result.get_achievement_update()
		if achievement_update.just_unlocked:
			simple_achievement_label.text = "Simple Achievement\njust unlocked!"
			simple_achievement_unlocked = true

	simple_achievement_processing = false


func increment_count_achievement():
	if count_achievement_processing or count_achievement_unlocked:
		return

	count_achievement_label.text = "Processing..."
	count_achievement_processing = true

	var result: MetaPlatformSDK_Message = await MetaPlatformSDK.achievements_add_count_async(COUNT_ACHIEVEMENT_NAME, 1).completed
	if result.is_error():
		count_achievement_label.text = "Error adding to\nCount Achievement"
	else:
		var achievement_update := result.get_achievement_update()
		if achievement_update.just_unlocked:
			count_achievement_label.text = "Count Achievement\njust unlocked!"
			count_achievement_unlocked = true
		else:
			update_count_achievement_progress()

	count_achievement_processing = false


func update_count_achievement_progress():
	var result: MetaPlatformSDK_Message = await MetaPlatformSDK.achievements_get_progress_by_name_async([COUNT_ACHIEVEMENT_NAME]).completed
	if result.is_error():
		count_achievement_label.text = "Error checking\nCount Achievement progress"
	else:
		var achievement_progress_array := result.get_achievement_progress_array()
		for achievement_progress: MetaPlatformSDK_AchievementProgress in achievement_progress_array:
			if achievement_progress.name != COUNT_ACHIEVEMENT_NAME:
				continue
			count_achievement_label.text = "Count is %s\nout of 3" % achievement_progress.count
			return


func add_field_bitfield_achievement(bitfield_position: int):
	if bitfield_achievement_processing or bitfield_achievement_unlocked:
		return

	bitfield_achievement_label.text = "Processing..."
	bitfield_achievement_processing = true

	var bitfield := ""
	for i in BITFIELD_ACHIEVEMENT_LENGTH:
		bitfield += "1" if i == bitfield_position else "0"

	var result: MetaPlatformSDK_Message = await MetaPlatformSDK.achievements_add_fields_async(BITFIELD_ACHIEVEMENT_NAME, bitfield).completed
	if result.is_error():
		bitfield_achievement_label.text = "Error adding field to\nBitfield Achievement"
	else:
		var achievement_update := result.get_achievement_update()
		if achievement_update.just_unlocked:
			bitfield_achievement_label.text = "Bitfield Achievement\njust unlocked!"
			bitfield_achievement_unlocked = true
		else:
			update_bitfield_achievement_progress()

	bitfield_achievement_processing = false


func update_bitfield_achievement_progress():
	var result: MetaPlatformSDK_Message = await MetaPlatformSDK.achievements_get_progress_by_name_async([BITFIELD_ACHIEVEMENT_NAME]).completed
	if result.is_error():
		bitfield_achievement_label.text = "Error checking\nBitfield Achievement progress"
	else:
		var achievement_progress_array := result.get_achievement_progress_array()
		for achievement_progress: MetaPlatformSDK_AchievementProgress in achievement_progress_array:
			if achievement_progress.name != BITFIELD_ACHIEVEMENT_NAME:
				continue
			var active_bits = achievement_progress.bitfield.split().count("1")
			bitfield_achievement_label.text = "%s of 5 bits active\nActivate 3 bits to unlock" % active_bits
			return


func _on_left_controller_button_pressed(name: String) -> void:
	if name == "trigger_click" and left_controller_ray_cast.is_colliding():
		var collider = left_controller_ray_cast.get_collider()
		update(collider.name)


func _on_right_controller_button_pressed(name: String) -> void:
	if name == "trigger_click" and right_controller_ray_cast.is_colliding():
		var collider = right_controller_ray_cast.get_collider()
		update(collider.name)


func _image_request_completed(_result: int, response_code: int, headers: PackedStringArray, body: PackedByteArray, image_request: HTTPRequest):
	if response_code != 200 or not headers.has("Content-Type: image/png"):
		push_error("Image request was not successful.")
		image_request.queue_free()
		return

	var image = Image.new()
	image.load_png_from_buffer(body)
	var image_texture = ImageTexture.create_from_image(image)
	user_image.texture = image_texture
	image_request.queue_free()
