@tool
extends Node
class_name GoetheDialogManager

signal dialog_started(dialog_id: String)
signal dialog_ended(dialog_id: String)
signal dialog_node_changed(node: GoetheDialogNode)
signal choice_made(choice_id: String, choice_text: String)
signal effect_triggered(effect_type: String, target: String, value: Variant)

# Define a simple dialog structure since we don't have the C++ extension yet
class GoetheDialog:
	var id: String = ""
	var nodes: Array[GoetheDialogNode] = []
	var start_node: String = ""

var current_dialog: GoetheDialog
var current_node_index: int = 0
var dialog_history: Array[GoetheDialogNode] = []
var condition_system: GoetheConditionSystem

# Try to get the condition system from autoloads
func _get_condition_system() -> GoetheConditionSystem:
	if condition_system == null:
		# Try to get from autoloads
		if get_node_or_null("/root/GoetheConditionSystem"):
			condition_system = get_node("/root/GoetheConditionSystem")
		else:
			# Create a local instance if autoload is not available
			condition_system = GoetheConditionSystem.new()
			add_child(condition_system)
	return condition_system

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
	current_dialog = GoetheDialog.new()
	current_dialog.id = "temp_dialog"
	current_dialog.nodes = []
	
	# Parse YAML content (enhanced implementation)
	var lines = yaml_content.split("\n")
	var in_nodes = false
	var current_node = null
	var in_choices = false
	var in_effects = false
	var in_conditions = false
	var current_choice = null
	var current_effect = null
	var current_condition = null
	
	for line in lines:
		line = line.strip_edges()
		var indent_level = _get_indent_level(line)
		var clean_line = line.strip_edges()
		
		if clean_line.is_empty():
			continue
		
		# Parse dialog metadata
		if clean_line.begins_with("id:"):
			current_dialog.id = clean_line.split(":", true, 1)[1].strip_edges()
		elif clean_line.begins_with("startNode:"):
			current_dialog.start_node = clean_line.split(":", true, 1)[1].strip_edges()
		elif clean_line.begins_with("kind:") and clean_line.contains("dialogue"):
			# Advanced GOETHE format
			pass
		elif clean_line.begins_with("nodes:"):
			in_nodes = true
			in_choices = false
			in_effects = false
			in_conditions = false
		elif in_nodes and indent_level == 2 and clean_line.begins_with("- id:"):
			# Save previous node
			if current_node != null:
				current_dialog.nodes.append(current_node)
			
			# Start new node
			current_node = GoetheDialogNode.new()
			current_node.id = clean_line.split(":", true, 1)[1].strip_edges()
			in_choices = false
			in_effects = false
			in_conditions = false
		elif current_node != null:
			# Parse node properties
			if indent_level == 4:
				if clean_line.begins_with("speaker:"):
					current_node.speaker = clean_line.split(":", true, 1)[1].strip_edges()
				elif clean_line.begins_with("text:"):
					current_node.text = clean_line.split(":", true, 1)[1].strip_edges()
				elif clean_line.begins_with("portrait:"):
					current_node.portrait = clean_line.split(":", true, 1)[1].strip_edges()
				elif clean_line.begins_with("voice:"):
					current_node.voice = clean_line.split(":", true, 1)[1].strip_edges()
				elif clean_line.begins_with("mood:"):
					current_node.mood = clean_line.split(":", true, 1)[1].strip_edges()
				elif clean_line.begins_with("expression:"):
					current_node.expression = clean_line.split(":", true, 1)[1].strip_edges()
				elif clean_line.begins_with("choices:"):
					in_choices = true
					in_effects = false
					in_conditions = false
				elif clean_line.begins_with("effects:"):
					in_effects = true
					in_choices = false
					in_conditions = false
				elif clean_line.begins_with("conditions:"):
					in_conditions = true
					in_choices = false
					in_effects = false
			
			# Parse choices
			elif in_choices and indent_level == 6:
				if clean_line.begins_with("- id:"):
					if current_choice != null:
						current_node.choices.append(current_choice)
					current_choice = {
						"id": clean_line.split(":", true, 1)[1].strip_edges(),
						"text": "",
						"to": "",
						"effects": []
					}
				elif current_choice != null:
					if clean_line.begins_with("text:"):
						current_choice.text = clean_line.split(":", true, 1)[1].strip_edges()
					elif clean_line.begins_with("to:"):
						current_choice.to = clean_line.split(":", true, 1)[1].strip_edges()
					elif clean_line.begins_with("effects:"):
						# Parse effects for this choice
						pass
			
			# Parse effects
			elif in_effects and indent_level == 6:
				if clean_line.begins_with("- type:"):
					if current_effect != null:
						current_node.effects.append(current_effect)
					current_effect = {
						"type": clean_line.split(":", true, 1)[1].strip_edges(),
						"target": "",
						"value": null
					}
				elif current_effect != null:
					if clean_line.begins_with("target:"):
						current_effect.target = clean_line.split(":", true, 1)[1].strip_edges()
					elif clean_line.begins_with("value:"):
						var value_str = clean_line.split(":", true, 1)[1].strip_edges()
						current_effect.value = _parse_value(value_str)
			
			# Parse conditions
			elif in_conditions and indent_level == 6:
				if clean_line.begins_with("- type:"):
					if current_condition != null:
						current_node.conditions.append(current_condition)
					current_condition = {
						"type": clean_line.split(":", true, 1)[1].strip_edges(),
						"target": "",
						"value": null
					}
				elif current_condition != null:
					if clean_line.begins_with("target:"):
						current_condition.target = clean_line.split(":", true, 1)[1].strip_edges()
					elif clean_line.begins_with("value:"):
						var value_str = clean_line.split(":", true, 1)[1].strip_edges()
						current_condition.value = _parse_value(value_str)
	
	# Add the last node, choice, effect, and condition
	if current_choice != null:
		current_node.choices.append(current_choice)
	if current_effect != null:
		current_node.effects.append(current_effect)
	if current_condition != null:
		current_node.conditions.append(current_condition)
	if current_node != null:
		current_dialog.nodes.append(current_node)
	
	return true

func _get_indent_level(line: String) -> int:
	var indent = 0
	for i in range(line.length()):
		if line[i] == ' ' or line[i] == '\t':
			indent += 1
		else:
			break
	return indent

func _parse_value(value_str: String) -> Variant:
	value_str = value_str.strip_edges()
	
	# Try to parse as boolean
	if value_str == "true":
		return true
	elif value_str == "false":
		return false
	
	# Try to parse as integer
	if value_str.is_valid_int():
		return value_str.to_int()
	
	# Try to parse as float
	if value_str.is_valid_float():
		return value_str.to_float()
	
	# Return as string (remove quotes if present)
	if value_str.begins_with('"') and value_str.ends_with('"'):
		return value_str.substr(1, value_str.length() - 2)
	
	return value_str

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
	
	# Apply effects from current node before moving
	_apply_node_effects(current_dialog.nodes[current_node_index])
	
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
			_apply_effect(effect)
			effect_triggered.emit(effect.type, effect.target, effect.value)

func can_show_node(node: GoetheDialogNode) -> bool:
	"""Check if a node can be shown based on its conditions"""
	if node.conditions.size() == 0:
		return true
	
	var condition_system = _get_condition_system()
	
	for condition in node.conditions:
		if not condition_system.evaluate_condition(condition):
			return false
	
	return true

func _apply_node_effects(node: GoetheDialogNode) -> void:
	"""Apply effects from a dialog node"""
	var condition_system = _get_condition_system()
	
	for effect in node.effects:
		_apply_effect(effect, condition_system)
		effect_triggered.emit(effect.type, effect.target, effect.value)

func _apply_effect(effect: Dictionary, condition_system: GoetheConditionSystem = null) -> void:
	"""Apply a single effect"""
	if condition_system == null:
		condition_system = _get_condition_system()
	
	match effect.type:
		"SET_FLAG":
			condition_system.set_flag(effect.target, effect.value)
		"TOGGLE_FLAG":
			condition_system.toggle_flag(effect.target)
		"SET_VAR":
			condition_system.set_variable(effect.target, effect.value)
		"INCREMENT_VAR":
			condition_system.increment_variable(effect.target, effect.value)
		"SET_QUEST_STATE":
			condition_system.set_quest_state(effect.target, effect.value)
		"TRIGGER_EVENT":
			condition_system.trigger_event(effect.target)
		"START_TIMER":
			condition_system.start_timer(effect.target, effect.value)
		"ADD_TO_INVENTORY":
			condition_system.add_to_inventory(effect.target, effect.value)
		"REMOVE_FROM_INVENTORY":
			condition_system.remove_from_inventory(effect.target, effect.value)
		"SET_DOOR_LOCKED":
			condition_system.set_door_locked(effect.target, effect.value)
		"SET_ACCESS_PERMISSION":
			condition_system.set_access_permission(effect.target, effect.value)
		"ADD_DIALOG_TO_HISTORY":
			condition_system.add_dialog_to_history(effect.target)
		_:
			push_warning("Unknown effect type: " + effect.type)
