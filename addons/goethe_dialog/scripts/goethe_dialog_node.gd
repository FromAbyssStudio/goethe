@tool
extends Resource
class_name GoetheDialogNode

@export var id: String = ""
@export var speaker: String = ""
@export var text: String = ""
@export var portrait: String = ""
@export var voice: String = ""
@export var mood: String = ""
@export var expression: String = ""
@export var choices: Array[Dictionary] = []
@export var conditions: Array[Dictionary] = []
@export var effects: Array[Dictionary] = []

func _init():
	resource_name = "GoetheDialogNode"

func add_choice(choice_text: String, choice_id: String = "") -> void:
	"""Add a choice to this dialog node"""
	var choice = {
		"id": choice_id if not choice_id.is_empty() else "choice_" + str(choices.size()),
		"text": choice_text,
		"to": "",
		"effects": []
	}
	choices.append(choice)

func add_condition(condition_type: String, target: String, value: Variant) -> void:
	"""Add a condition to this dialog node"""
	var condition = {
		"type": condition_type,
		"target": target,
		"value": value
	}
	conditions.append(condition)

func add_effect(effect_type: String, target: String, value: Variant) -> void:
	"""Add an effect to this dialog node"""
	var effect = {
		"type": effect_type,
		"target": target,
		"value": value
	}
	effects.append(effect)

func evaluate_conditions() -> bool:
	"""Evaluate all conditions for this node"""
	for condition in conditions:
		if not _evaluate_condition(condition):
			return false
	return true

func _evaluate_condition(condition: Dictionary) -> bool:
	"""Evaluate a single condition"""
	# This will be implemented with the condition system
	# For now, return true as default
	return true
