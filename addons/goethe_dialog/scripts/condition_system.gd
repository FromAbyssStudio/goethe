@tool
extends Node
class_name GoetheConditionSystem

# Autoload singleton for global condition management
static var instance: GoetheConditionSystem

# Global flags and variables
var flags: Dictionary = {}
var variables: Dictionary = {}
var quest_states: Dictionary = {}
var chapter_states: Dictionary = {}
var area_states: Dictionary = {}
var dialog_history: Array[String] = []
var choice_history: Array[String] = []

# Events and timers
var events: Dictionary = {}
var timers: Dictionary = {}

# Inventory system
var inventory: Dictionary = {}

# Door and access control
var door_states: Dictionary = {}
var access_permissions: Dictionary = {}

signal flag_changed(flag_name: String, new_value: bool)
signal variable_changed(var_name: String, new_value: Variant)
signal quest_state_changed(quest_id: String, new_state: String)
signal condition_evaluated(condition: Dictionary, result: bool)

func _ready():
	# Set up as singleton
	if instance == null:
		instance = self
		# Make sure this node persists
		process_mode = Node.PROCESS_MODE_ALWAYS
	else:
		queue_free()

func _enter_tree():
	# Ensure this is an autoload
	if get_parent() != get_tree().root:
		push_warning("GoetheConditionSystem should be an autoload singleton")

# Flag management
func set_flag(flag_name: String, value: bool) -> void:
	var old_value = flags.get(flag_name, false)
	flags[flag_name] = value
	if old_value != value:
		flag_changed.emit(flag_name, value)

func get_flag(flag_name: String) -> bool:
	return flags.get(flag_name, false)

func toggle_flag(flag_name: String) -> void:
	set_flag(flag_name, !get_flag(flag_name))

# Variable management
func set_variable(var_name: String, value: Variant) -> void:
	var old_value = variables.get(var_name, null)
	variables[var_name] = value
	if old_value != value:
		variable_changed.emit(var_name, value)

func get_variable(var_name: String) -> Variant:
	return variables.get(var_name, null)

func increment_variable(var_name: String, amount: int = 1) -> void:
	var current = get_variable(var_name)
	if current is int:
		set_variable(var_name, current + amount)
	else:
		set_variable(var_name, amount)

# Quest management
func set_quest_state(quest_id: String, state: String) -> void:
	var old_state = quest_states.get(quest_id, "")
	quest_states[quest_id] = state
	if old_state != state:
		quest_state_changed.emit(quest_id, state)

func get_quest_state(quest_id: String) -> String:
	return quest_states.get(quest_id, "")

func is_quest_active(quest_id: String) -> bool:
	return get_quest_state(quest_id) == "active"

func is_quest_completed(quest_id: String) -> bool:
	return get_quest_state(quest_id) == "completed"

func is_quest_failed(quest_id: String) -> bool:
	return get_quest_state(quest_id) == "failed"

# Chapter management
func set_chapter_active(chapter_id: String, active: bool) -> void:
	chapter_states[chapter_id] = active

func is_chapter_active(chapter_id: String) -> bool:
	return chapter_states.get(chapter_id, false)

# Area management
func set_area_entered(area_id: String, entered: bool) -> void:
	area_states[area_id] = entered

func has_entered_area(area_id: String) -> bool:
	return area_states.get(area_id, false)

# Dialog history
func add_dialog_to_history(dialog_id: String) -> void:
	if not dialog_history.has(dialog_id):
		dialog_history.append(dialog_id)

func has_visited_dialog(dialog_id: String) -> bool:
	return dialog_history.has(dialog_id)

func get_dialog_visit_count(dialog_id: String) -> int:
	return dialog_history.count(dialog_id)

# Choice history
func add_choice_to_history(choice_id: String) -> void:
	choice_history.append(choice_id)

func has_made_choice(choice_id: String) -> bool:
	return choice_history.has(choice_id)

func get_choice_count(choice_id: String) -> int:
	return choice_history.count(choice_id)

# Event system
func trigger_event(event_id: String) -> void:
	events[event_id] = Time.get_unix_time_from_system()

func has_event_occurred(event_id: String) -> bool:
	return events.has(event_id)

func get_event_time(event_id: String) -> float:
	return events.get(event_id, 0.0)

func time_since_event(event_id: String) -> float:
	if has_event_occurred(event_id):
		return Time.get_unix_time_from_system() - get_event_time(event_id)
	return 0.0

# Timer system
func start_timer(timer_id: String, duration: float) -> void:
	timers[timer_id] = {
		"start_time": Time.get_unix_time_from_system(),
		"duration": duration
	}

func is_timer_active(timer_id: String) -> bool:
	if not timers.has(timer_id):
		return false
	var timer = timers[timer_id]
	var elapsed = Time.get_unix_time_from_system() - timer.start_time
	return elapsed < timer.duration

func get_timer_remaining(timer_id: String) -> float:
	if not timers.has(timer_id):
		return 0.0
	var timer = timers[timer_id]
	var elapsed = Time.get_unix_time_from_system() - timer.start_time
	return max(0.0, timer.duration - elapsed)

# Inventory system
func add_to_inventory(item_id: String, quantity: int = 1) -> void:
	inventory[item_id] = inventory.get(item_id, 0) + quantity

func remove_from_inventory(item_id: String, quantity: int = 1) -> void:
	var current = inventory.get(item_id, 0)
	inventory[item_id] = max(0, current - quantity)

func has_inventory_item(item_id: String, quantity: int = 1) -> bool:
	return inventory.get(item_id, 0) >= quantity

func get_inventory_count(item_id: String) -> int:
	return inventory.get(item_id, 0)

# Door and access control
func set_door_locked(door_id: String, locked: bool) -> void:
	door_states[door_id] = locked

func is_door_locked(door_id: String) -> bool:
	return door_states.get(door_id, true)

func set_access_permission(permission_id: String, granted: bool) -> void:
	access_permissions[permission_id] = granted

func has_access_permission(permission_id: String) -> bool:
	return access_permissions.get(permission_id, false)

# Condition evaluation
func evaluate_condition(condition: Dictionary) -> bool:
	if not condition.has("type"):
		return false
	
	var condition_type = condition.type
	var result = false
	
	match condition_type:
		"FLAG":
			result = _evaluate_flag_condition(condition)
		"VAR":
			result = _evaluate_variable_condition(condition)
		"QUEST_STATE":
			result = _evaluate_quest_condition(condition)
		"CHAPTER_ACTIVE":
			result = _evaluate_chapter_condition(condition)
		"AREA_ENTERED":
			result = _evaluate_area_condition(condition)
		"DIALOGUE_VISITED":
			result = _evaluate_dialog_condition(condition)
		"CHOICE_MADE":
			result = _evaluate_choice_condition(condition)
		"EVENT":
			result = _evaluate_event_condition(condition)
		"TIME_SINCE":
			result = _evaluate_time_condition(condition)
		"INVENTORY_HAS":
			result = _evaluate_inventory_condition(condition)
		"DOOR_LOCKED":
			result = _evaluate_door_condition(condition)
		"ACCESS_ALLOWED":
			result = _evaluate_access_condition(condition)
		"ALL":
			result = _evaluate_all_condition(condition)
		"ANY":
			result = _evaluate_any_condition(condition)
		"NOT":
			result = _evaluate_not_condition(condition)
		_:
			push_error("Unknown condition type: " + condition_type)
			result = false
	
	condition_evaluated.emit(condition, result)
	return result

func _evaluate_flag_condition(condition: Dictionary) -> bool:
	var flag_name = condition.get("target", "")
	var expected_value = condition.get("value", true)
	return get_flag(flag_name) == expected_value

func _evaluate_variable_condition(condition: Dictionary) -> bool:
	var var_name = condition.get("target", "")
	var expected_value = condition.get("value", null)
	var actual_value = get_variable(var_name)
	
	if condition.has("operator"):
		var operator = condition.operator
		match operator:
			"==":
				return actual_value == expected_value
			"!=":
				return actual_value != expected_value
			">":
				return actual_value > expected_value
			"<":
				return actual_value < expected_value
			">=":
				return actual_value >= expected_value
			"<=":
				return actual_value <= expected_value
			_:
				return actual_value == expected_value
	else:
		return actual_value == expected_value

func _evaluate_quest_condition(condition: Dictionary) -> bool:
	var quest_id = condition.get("target", "")
	var expected_state = condition.get("value", "active")
	return get_quest_state(quest_id) == expected_state

func _evaluate_chapter_condition(condition: Dictionary) -> bool:
	var chapter_id = condition.get("target", "")
	return is_chapter_active(chapter_id)

func _evaluate_area_condition(condition: Dictionary) -> bool:
	var area_id = condition.get("target", "")
	return has_entered_area(area_id)

func _evaluate_dialog_condition(condition: Dictionary) -> bool:
	var dialog_id = condition.get("target", "")
	return has_visited_dialog(dialog_id)

func _evaluate_choice_condition(condition: Dictionary) -> bool:
	var choice_id = condition.get("target", "")
	return has_made_choice(choice_id)

func _evaluate_event_condition(condition: Dictionary) -> bool:
	var event_id = condition.get("target", "")
	return has_event_occurred(event_id)

func _evaluate_time_condition(condition: Dictionary) -> bool:
	var event_id = condition.get("target", "")
	var time_threshold = condition.get("value", 0.0)
	return time_since_event(event_id) >= time_threshold

func _evaluate_inventory_condition(condition: Dictionary) -> bool:
	var item_id = condition.get("target", "")
	var quantity = condition.get("value", 1)
	return has_inventory_item(item_id, quantity)

func _evaluate_door_condition(condition: Dictionary) -> bool:
	var door_id = condition.get("target", "")
	var expected_locked = condition.get("value", true)
	return is_door_locked(door_id) == expected_locked

func _evaluate_access_condition(condition: Dictionary) -> bool:
	var permission_id = condition.get("target", "")
	return has_access_permission(permission_id)

func _evaluate_all_condition(condition: Dictionary) -> bool:
	if not condition.has("children"):
		return true
	
	for child_condition in condition.children:
		if not evaluate_condition(child_condition):
			return false
	return true

func _evaluate_any_condition(condition: Dictionary) -> bool:
	if not condition.has("children"):
		return false
	
	for child_condition in condition.children:
		if evaluate_condition(child_condition):
			return true
	return false

func _evaluate_not_condition(condition: Dictionary) -> bool:
	if not condition.has("children") or condition.children.size() == 0:
		return true
	
	return !evaluate_condition(condition.children[0])

# Utility functions
func clear_all() -> void:
	flags.clear()
	variables.clear()
	quest_states.clear()
	chapter_states.clear()
	area_states.clear()
	dialog_history.clear()
	choice_history.clear()
	events.clear()
	timers.clear()
	inventory.clear()
	door_states.clear()
	access_permissions.clear()

func save_state() -> Dictionary:
	return {
		"flags": flags,
		"variables": variables,
		"quest_states": quest_states,
		"chapter_states": chapter_states,
		"area_states": area_states,
		"dialog_history": dialog_history,
		"choice_history": choice_history,
		"events": events,
		"timers": timers,
		"inventory": inventory,
		"door_states": door_states,
		"access_permissions": access_permissions
	}

func load_state(state: Dictionary) -> void:
	flags = state.get("flags", {})
	variables = state.get("variables", {})
	quest_states = state.get("quest_states", {})
	chapter_states = state.get("chapter_states", {})
	area_states = state.get("area_states", {})
	dialog_history = state.get("dialog_history", [])
	choice_history = state.get("choice_history", [])
	events = state.get("events", {})
	timers = state.get("timers", {})
	inventory = state.get("inventory", {})
	door_states = state.get("door_states", {})
	access_permissions = state.get("access_permissions", {})
