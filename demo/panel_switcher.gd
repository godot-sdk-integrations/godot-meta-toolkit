extends Control

@onready var switch_button: Button = %SwitchButton

var hybrid_app_mode := MetaHybridApp.HybridAppMode.NONE
var data := {}

func _ready() -> void:
	hybrid_app_mode = MetaHybridApp.get_mode()
	if hybrid_app_mode == MetaHybridApp.HybridAppMode.IMMERSIVE:
		switch_button.text = "Switch To Panel"
	elif hybrid_app_mode == MetaHybridApp.HybridAppMode.PANEL:
		switch_button.text = "Switch To Immersive"

func _on_switch_button_pressed() -> void:
	var data_string := JSON.stringify(data)

	if hybrid_app_mode == MetaHybridApp.HybridAppMode.IMMERSIVE:
		MetaHybridApp.switch_mode(MetaHybridApp.HybridAppMode.PANEL, data_string)
	elif hybrid_app_mode == MetaHybridApp.HybridAppMode.PANEL:
		MetaHybridApp.switch_mode(MetaHybridApp.HybridAppMode.IMMERSIVE, data_string)

