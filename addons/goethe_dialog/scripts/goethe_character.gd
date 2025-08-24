@tool
extends Resource
class_name GoetheCharacter

@export var id: String = ""
@export var name: String = ""
@export var portraits: Dictionary = {}
@export var voice_clips: Dictionary = {}
@export var expressions: Array[String] = []
@export var moods: Array[String] = []

func _init():
	resource_name = "GoetheCharacter"

func add_portrait(expression: String, mood: String, portrait_path: String) -> void:
	"""Add a portrait for a specific expression and mood"""
	var key = expression + "_" + mood
	portraits[key] = portrait_path

func get_portrait(expression: String, mood: String) -> String:
	"""Get a portrait path for a specific expression and mood"""
	var key = expression + "_" + mood
	return portraits.get(key, "")

func add_voice_clip(clip_id: String, clip_path: String) -> void:
	"""Add a voice clip"""
	voice_clips[clip_id] = clip_path

func get_voice_clip(clip_id: String) -> String:
	"""Get a voice clip path"""
	return voice_clips.get(clip_id, "")

func add_expression(expression: String) -> void:
	"""Add an expression"""
	if not expressions.has(expression):
		expressions.append(expression)

func add_mood(mood: String) -> void:
	"""Add a mood"""
	if not moods.has(mood):
		moods.append(mood)
