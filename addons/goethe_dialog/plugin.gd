@tool
extends EditorPlugin

const PLUGIN_NAME = "GoetheDialog"

func _enter_tree():
	# Register custom types
	add_custom_type("GoetheDialogManager", "Node", preload("res://addons/goethe_dialog/scripts/goethe_dialog_manager.gd"), preload("res://addons/goethe_dialog/icons/dialog_manager_icon.svg"))
	add_custom_type("GoetheDialogNode", "Resource", preload("res://addons/goethe_dialog/scripts/goethe_dialog_node.gd"), preload("res://addons/goethe_dialog/icons/dialog_node_icon.svg"))
	add_custom_type("GoetheCharacter", "Resource", preload("res://addons/goethe_dialog/scripts/goethe_character.gd"), preload("res://addons/goethe_dialog/icons/character_icon.svg"))
	
	# Add editor tools
	add_tool_menu_item("Goethe Dialog Editor", _open_dialog_editor)
	
	print("Goethe Dialog Plugin loaded successfully!")

func _exit_tree():
	# Remove custom types
	remove_custom_type("GoetheDialogManager")
	remove_custom_type("GoetheDialogNode")
	remove_custom_type("GoetheCharacter")
	
	# Remove editor tools
	remove_tool_menu_item("Goethe Dialog Editor")
	
	print("Goethe Dialog Plugin unloaded!")

func _open_dialog_editor():
	# Open the dialog editor window
	var editor = preload("res://addons/goethe_dialog/scenes/dialog_editor.tscn").instantiate()
	get_editor_interface().get_base_control().add_child(editor)
	editor.popup_centered()
