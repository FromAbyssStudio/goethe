@tool
extends Node
class_name GoetheDialogManager

signal dialog_started(dialog_id: String)
signal dialog_ended(dialog_id: String)
signal dialog_node_changed(node: GoetheDialogNode)
signal choice_made(choice_id: String, choice_text: String)

# Define a simple dialog structure since we don't have the C++ extension yet
class GoetheDialog:
	var id: String = ""
	var nodes: Array[GoetheDialogNode] = []

var current_dialog: GoetheDialog
var current_node_index: int = 0
var dialog_history: Array[GoetheDialogNode] = []
var flags: Dictionary = {}
var variables: Dictionary = {}

func _ready():
	# Initialize the dialog manager
	pass

func load_dialog_from_file(file_path: String) -> bool:
	"""Load a dialog from a YAML file"""
	if not FileAccess.file_exists(file_path):
		push_error("Dialog file not found: " + file_path)
		return false
	
	var file = FileAccess.open(file_path, FileAccess.READ)
	if file == null:
		push_error("Failed to open dialog file: " + file_path)
		return false
	
	var content = file.get_as_text()
	file.close()
	
	return load_dialog_from_yaml(content)

func load_dialog_from_yaml(yaml_content: String) -> bool:
	"""Load a dialog from YAML string content"""
	# This will be implemented in the C++ extension
	# For now, we'll create a basic structure
	current_dialog = GoetheDialog.new()
	current_dialog.id = "temp_dialog"
	current_dialog.nodes = []
	
	# Parse YAML content (basic implementation)
	var lines = yaml_content.split("\n")
	var in_nodes = false
	var current_node = null
	
	for line in lines:
		line = line.strip_edges()
		if line.begins_with("id:"):
			current_dialog.id = line.split(":", true, 1)[1].strip_edges()
		elif line.begins_with("nodes:"):
			in_nodes = true
		elif in_nodes and line.begins_with("- id:"):
			if current_node != null:
				current_dialog.nodes.append(current_node)
			current_node = GoetheDialogNode.new()
			current_node.id = line.split(":", true, 1)[1].strip_edges()
		elif in_nodes and line.begins_with("  speaker:") and current_node != null:
			current_node.speaker = line.split(":", true, 1)[1].strip_edges()
		elif in_nodes and line.begins_with("  text:") and current_node != null:
			current_node.text = line.split(":", true, 1)[1].strip_edges()
	
	if current_node != null:
		current_dialog.nodes.append(current_node)
	
	return true

func start_dialog(dialog_id: String = "") -> bool:
	"""Start a dialog"""
	if current_dialog == null:
		push_error("No dialog loaded")
		return false
	
	if dialog_id.is_empty():
		dialog_id = current_dialog.id
	
	current_node_index = 0
	dialog_history.clear()
	
	dialog_started.emit(dialog_id)
	
	if current_dialog.nodes.size() > 0:
		_show_current_node()
	
	return true

func next_node() -> bool:
	"""Move to the next dialog node"""
	if current_dialog == null or current_node_index >= current_dialog.nodes.size():
		return false
	
	current_node_index += 1
	
	if current_node_index < current_dialog.nodes.size():
		_show_current_node()
		return true
	else:
		dialog_ended.emit(current_dialog.id)
		return false

func previous_node() -> bool:
	"""Move to the previous dialog node"""
	if current_dialog == null or current_node_index <= 0:
		return false
	
	current_node_index -= 1
	_show_current_node()
	return true

func make_choice(choice_index: int) -> bool:
	"""Make a choice in the current dialog node"""
	if current_dialog == null or current_node_index >= current_dialog.nodes.size():
		return false
	
	var current_node = current_dialog.nodes[current_node_index]
	if current_node.choices.size() <= choice_index:
		return false
	
	var choice = current_node.choices[choice_index]
	choice_made.emit(choice.id, choice.text)
	
	# Handle choice effects
	_handle_choice_effects(choice)
	
	return true

func get_current_node() -> GoetheDialogNode:
	"""Get the current dialog node"""
	if current_dialog == null or current_node_index >= current_dialog.nodes.size():
		return null
	
	return current_dialog.nodes[current_node_index]

func set_flag(flag_name: String, value: bool) -> void:
	"""Set a flag value"""
	flags[flag_name] = value

func get_flag(flag_name: String) -> bool:
	"""Get a flag value"""
	return flags.get(flag_name, false)

func set_variable(var_name: String, value: Variant) -> void:
	"""Set a variable value"""
	variables[var_name] = value

func get_variable(var_name: String) -> Variant:
	"""Get a variable value"""
	return variables.get(var_name, null)

func _show_current_node() -> void:
	"""Show the current dialog node"""
	var node = get_current_node()
	if node != null:
		dialog_node_changed.emit(node)

func _handle_choice_effects(choice: Dictionary) -> void:
	"""Handle the effects of a choice"""
	if choice.has("effects"):
		for effect in choice.effects:
			match effect.type:
				"SET_FLAG":
					set_flag(effect.target, effect.value)
				"SET_VAR":
					set_variable(effect.target, effect.value)
				# Add more effect types as needed
