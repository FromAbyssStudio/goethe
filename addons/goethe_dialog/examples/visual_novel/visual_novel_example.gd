@tool
extends Node2D
class_name VisualNovelExample

# Visual Novel Example for Goethe Dialog System
# This demonstrates a complete visual novel with:
# - Character management
# - Background changes
# - Music and sound effects
# - Save/load system
# - Multiple dialog paths
# - Condition-based branching

signal dialog_started
signal dialog_ended
signal choice_made(choice_id: String, choice_text: String)

@onready var dialog_manager: GoetheDialogManager = $DialogManager
@onready var dialog_ui: Control = $DialogUI
@onready var background_sprite: Sprite2D = $Background
@onready var character_sprites: Node2D = $Characters
@onready var music_player: AudioStreamPlayer = $MusicPlayer
@onready var sfx_player: AudioStreamPlayer = $SFXPlayer
@onready var save_system: Node = $SaveSystem

# Character sprites
@onready var alice_sprite: Sprite2D = $Characters/Alice
@onready var bob_sprite: Sprite2D = $Characters/Bob
@onready var narrator_sprite: Sprite2D = $Characters/Narrator

# UI elements
@onready var speaker_label: Label = $DialogUI/Background/VBoxContainer/SpeakerLabel
@onready var text_label: Label = $DialogUI/Background/VBoxContainer/TextLabel
@onready var portrait_texture: TextureRect = $DialogUI/Background/VBoxContainer/PortraitTexture
@onready var choices_container: VBoxContainer = $DialogUI/Background/VBoxContainer/ChoicesContainer
@onready var next_button: Button = $DialogUI/Background/VBoxContainer/ButtonContainer/NextButton
@onready var previous_button: Button = $DialogUI/Background/VBoxContainer/ButtonContainer/PreviousButton
@onready var auto_button: Button = $DialogUI/Background/VBoxContainer/ButtonContainer/AutoButton
@onready var skip_button: Button = $DialogUI/Background/VBoxContainer/ButtonContainer/SkipButton

# Menu UI
@onready var menu_button: Button = $MenuButton
@onready var menu_panel: Panel = $MenuPanel
@onready var save_button: Button = $MenuPanel/VBoxContainer/SaveButton
@onready var load_button: Button = $MenuPanel/VBoxContainer/LoadButton
@onready var settings_button: Button = $MenuPanel/VBoxContainer/SettingsButton
@onready var quit_button: Button = $MenuPanel/VBoxContainer/QuitButton

# Settings UI
@onready var settings_panel: Panel = $SettingsPanel
@onready var text_speed_slider: HSlider = $SettingsPanel/VBoxContainer/TextSpeedSlider
@onready var music_volume_slider: HSlider = $SettingsPanel/VBoxContainer/MusicVolumeSlider
@onready var sfx_volume_slider: HSlider = $SettingsPanel/VBoxContainer/SFXVolumeSlider
@onready var auto_speed_slider: HSlider = $SettingsPanel/VBoxContainer/AutoSpeedSlider

# Game state
var current_chapter: String = "chapter1"
var is_dialog_active: bool = false
var is_auto_mode: bool = false
var is_skip_mode: bool = false
var text_speed: float = 1.0
var auto_speed: float = 2.0
var music_volume: float = 0.7
var sfx_volume: float = 0.8

# Character data
var characters: Dictionary = {
	"alice": {
		"name": "Alice",
		"portraits": {
			"happy": preload("res://addons/goethe_dialog/examples/visual_novel/assets/alice_happy.png"),
			"sad": preload("res://addons/goethe_dialog/examples/visual_novel/assets/alice_sad.png"),
			"angry": preload("res://addons/goethe_dialog/examples/visual_novel/assets/alice_angry.png"),
			"surprised": preload("res://addons/goethe_dialog/examples/visual_novel/assets/alice_surprised.png")
		},
		"voice": preload("res://addons/goethe_dialog/examples/visual_novel/assets/alice_voice.ogg")
	},
	"bob": {
		"name": "Bob",
		"portraits": {
			"happy": preload("res://addons/goethe_dialog/examples/visual_novel/assets/bob_happy.png"),
			"sad": preload("res://addons/goethe_dialog/examples/visual_novel/assets/bob_sad.png"),
			"angry": preload("res://addons/goethe_dialog/examples/visual_novel/assets/bob_angry.png"),
			"surprised": preload("res://addons/goethe_dialog/examples/visual_novel/assets/bob_surprised.png")
		},
		"voice": preload("res://addons/goethe_dialog/examples/visual_novel/assets/bob_voice.ogg")
	},
	"narrator": {
		"name": "Narrator",
		"portraits": {},
		"voice": null
	}
}

# Background data
var backgrounds: Dictionary = {
	"school_hallway": preload("res://addons/goethe_dialog/examples/visual_novel/assets/school_hallway.png"),
	"classroom": preload("res://addons/goethe_dialog/examples/visual_novel/assets/classroom.png"),
	"park": preload("res://addons/goethe_dialog/examples/visual_novel/assets/park.png"),
	"cafe": preload("res://addons/goethe_dialog/examples/visual_novel/assets/cafe.png"),
	"library": preload("res://addons/goethe_dialog/examples/visual_novel/assets/library.png")
}

# Music data
var music_tracks: Dictionary = {
	"main_theme": preload("res://addons/goethe_dialog/examples/visual_novel/assets/main_theme.ogg"),
	"romantic": preload("res://addons/goethe_dialog/examples/visual_novel/assets/romantic.ogg"),
	"tense": preload("res://addons/goethe_dialog/examples/visual_novel/assets/tense.ogg"),
	"happy": preload("res://addons/goethe_dialog/examples/visual_novel/assets/happy.ogg"),
	"sad": preload("res://addons/goethe_dialog/examples/visual_novel/assets/sad.ogg")
}

# Sound effects
var sound_effects: Dictionary = {
	"page_turn": preload("res://addons/goethe_dialog/examples/visual_novel/assets/page_turn.ogg"),
	"bell": preload("res://addons/goethe_dialog/examples/visual_novel/assets/bell.ogg"),
	"footsteps": preload("res://addons/goethe_dialog/examples/visual_novel/assets/footsteps.ogg"),
	"door_open": preload("res://addons/goethe_dialog/examples/visual_novel/assets/door_open.ogg"),
	"door_close": preload("res://addons/goethe_dialog/examples/visual_novel/assets/door_close.ogg")
}

func _ready():
	# Connect signals
	dialog_manager.dialog_started.connect(_on_dialog_started)
	dialog_manager.dialog_ended.connect(_on_dialog_ended)
	dialog_manager.dialog_node_changed.connect(_on_dialog_node_changed)
	dialog_manager.choice_made.connect(_on_choice_made)
	dialog_manager.effect_triggered.connect(_on_effect_triggered)
	
	# Connect UI signals
	next_button.pressed.connect(_on_next_button_pressed)
	previous_button.pressed.connect(_on_previous_button_pressed)
	auto_button.pressed.connect(_on_auto_button_pressed)
	skip_button.pressed.connect(_on_skip_button_pressed)
	
	menu_button.pressed.connect(_on_menu_button_pressed)
	save_button.pressed.connect(_on_save_button_pressed)
	load_button.pressed.connect(_on_load_button_pressed)
	settings_button.pressed.connect(_on_settings_button_pressed)
	quit_button.pressed.connect(_on_quit_button_pressed)
	
	# Connect settings signals
	text_speed_slider.value_changed.connect(_on_text_speed_changed)
	music_volume_slider.value_changed.connect(_on_music_volume_changed)
	sfx_volume_slider.value_changed.connect(_on_sfx_volume_changed)
	auto_speed_slider.value_changed.connect(_on_auto_speed_changed)
	
	# Initialize UI
	_initialize_ui()
	
	# Load initial dialog
	_load_chapter(current_chapter)

func _initialize_ui():
	# Set initial UI state
	dialog_ui.visible = false
	menu_panel.visible = false
	settings_panel.visible = false
	
	# Set initial settings
	text_speed_slider.value = text_speed
	music_volume_slider.value = music_volume
	sfx_volume_slider.value = sfx_volume
	auto_speed_slider.value = auto_speed
	
	# Set initial volumes
	music_player.volume_db = linear_to_db(music_volume)
	sfx_player.volume_db = linear_to_db(sfx_volume)

func _load_chapter(chapter_id: String):
	"""Load a chapter dialog file"""
	var dialog_file = "res://addons/goethe_dialog/examples/visual_novel/dialogs/" + chapter_id + ".yaml"
	
	if not FileAccess.file_exists(dialog_file):
		push_error("Dialog file not found: " + dialog_file)
		return
	
	if dialog_manager.load_dialog_from_file(dialog_file):
		print("Loaded chapter: " + chapter_id)
		_play_music("main_theme")
		_set_background("school_hallway")
	else:
		push_error("Failed to load chapter: " + chapter_id)

func _on_dialog_started(dialog_id: String):
	"""Called when a dialog starts"""
	is_dialog_active = true
	dialog_ui.visible = true
	dialog_started.emit()
	print("Dialog started: " + dialog_id)

func _on_dialog_ended(dialog_id: String):
	"""Called when a dialog ends"""
	is_dialog_active = false
	dialog_ui.visible = false
	dialog_ended.emit()
	print("Dialog ended: " + dialog_id)
	
	# Check for next chapter
	_check_chapter_progression()

func _on_dialog_node_changed(node: GoetheDialogNode):
	"""Called when the dialog node changes"""
	_update_ui_for_node(node)
	_play_character_voice(node.speaker)
	_play_sound_effect("page_turn")

func _on_choice_made(choice_id: String, choice_text: String):
	"""Called when a choice is made"""
	choice_made.emit(choice_id, choice_text)
	print("Choice made: " + choice_id + " - " + choice_text)
	
	# Hide choices after selection
	_hide_choices()

func _on_effect_triggered(effect_type: String, target: String, value: Variant):
	"""Called when an effect is triggered"""
	print("Effect triggered: " + effect_type + " -> " + target + " = " + str(value))
	
	# Handle specific effects
	match effect_type:
		"CHANGE_BACKGROUND":
			_set_background(target)
		"PLAY_MUSIC":
			_play_music(target)
		"PLAY_SFX":
			_play_sound_effect(target)
		"SHOW_CHARACTER":
			_show_character(target, value)
		"HIDE_CHARACTER":
			_hide_character(target)
		"CHANGE_CHAPTER":
			_change_chapter(target)

func _update_ui_for_node(node: GoetheDialogNode):
	"""Update the UI to display the current dialog node"""
	# Update speaker name
	speaker_label.text = node.speaker if node.speaker else ""
	
	# Update dialog text
	text_label.text = node.text if node.text else ""
	
	# Update portrait
	_update_portrait(node.speaker, node.expression)
	
	# Update choices
	_update_choices(node.choices)
	
	# Update character sprites
	_update_character_sprites(node.speaker, node.mood)

func _update_portrait(speaker: String, expression: String):
	"""Update the character portrait"""
	if speaker.is_empty() or not characters.has(speaker.to_lower()):
		portrait_texture.texture = null
		return
	
	var character = characters[speaker.to_lower()]
	if expression.is_empty():
		expression = "happy"
	
	if character.portraits.has(expression):
		portrait_texture.texture = character.portraits[expression]
	else:
		portrait_texture.texture = null

func _update_choices(choices: Array):
	"""Update the choice buttons"""
	# Clear existing choices
	for child in choices_container.get_children():
		child.queue_free()
	
	# Add new choices
	for i in range(choices.size()):
		var choice = choices[i]
		var button = Button.new()
		button.text = choice.text
		button.custom_minimum_size = Vector2(0, 40)
		button.pressed.connect(_on_choice_button_pressed.bind(i))
		choices_container.add_child(button)

func _update_character_sprites(speaker: String, mood: String):
	"""Update character sprites on screen"""
	# Hide all characters first
	for child in character_sprites.get_children():
		child.visible = false
	
	# Show the speaking character
	if not speaker.is_empty() and character_sprites.has_node(speaker.capitalize()):
		var sprite = character_sprites.get_node(speaker.capitalize())
		sprite.visible = true
		
		# Update mood/expression if available
		if not mood.is_empty() and characters.has(speaker.to_lower()):
			var character = characters[speaker.to_lower()]
			if character.portraits.has(mood):
				sprite.texture = character.portraits[mood]

func _hide_choices():
	"""Hide all choice buttons"""
	for child in choices_container.get_children():
		child.queue_free()

func _set_background(background_id: String):
	"""Change the background image"""
	if backgrounds.has(background_id):
		background_sprite.texture = backgrounds[background_id]
		print("Background changed to: " + background_id)

func _play_music(track_id: String):
	"""Play background music"""
	if music_tracks.has(track_id):
		music_player.stream = music_tracks[track_id]
		music_player.play()
		print("Playing music: " + track_id)

func _play_sound_effect(sfx_id: String):
	"""Play a sound effect"""
	if sound_effects.has(sfx_id):
		sfx_player.stream = sound_effects[sfx_id]
		sfx_player.play()
		print("Playing SFX: " + sfx_id)

func _play_character_voice(speaker: String):
	"""Play character voice"""
	if speaker.is_empty() or not characters.has(speaker.to_lower()):
		return
	
	var character = characters[speaker.to_lower()]
	if character.voice:
		sfx_player.stream = character.voice
		sfx_player.play()

func _show_character(character_id: String, position: Vector2 = Vector2.ZERO):
	"""Show a character sprite"""
	if character_sprites.has_node(character_id.capitalize()):
		var sprite = character_sprites.get_node(character_id.capitalize())
		sprite.visible = true
		if position != Vector2.ZERO:
			sprite.position = position

func _hide_character(character_id: String):
	"""Hide a character sprite"""
	if character_sprites.has_node(character_id.capitalize()):
		var sprite = character_sprites.get_node(character_id.capitalize())
		sprite.visible = false

func _change_chapter(chapter_id: String):
	"""Change to a different chapter"""
	current_chapter = chapter_id
	_load_chapter(chapter_id)

func _check_chapter_progression():
	"""Check if we should progress to the next chapter"""
	var condition_system = dialog_manager._get_condition_system()
	
	# Example chapter progression logic
	if current_chapter == "chapter1" and condition_system.get_flag("chapter1_completed"):
		_change_chapter("chapter2")
	elif current_chapter == "chapter2" and condition_system.get_flag("chapter2_completed"):
		_change_chapter("chapter3")

# UI Event Handlers
func _on_next_button_pressed():
	if is_dialog_active:
		dialog_manager.next_node()

func _on_previous_button_pressed():
	if is_dialog_active:
		dialog_manager.previous_node()

func _on_auto_button_pressed():
	is_auto_mode = !is_auto_mode
	auto_button.text = "Auto (ON)" if is_auto_mode else "Auto"
	
	if is_auto_mode:
		_start_auto_mode()
	else:
		_stop_auto_mode()

func _on_skip_button_pressed():
	is_skip_mode = !is_skip_mode
	skip_button.text = "Skip (ON)" if is_skip_mode else "Skip"
	
	if is_skip_mode:
		_start_skip_mode()
	else:
		_stop_skip_mode()

func _on_choice_button_pressed(choice_index: int):
	dialog_manager.make_choice(choice_index)

func _on_menu_button_pressed():
	menu_panel.visible = !menu_panel.visible
	settings_panel.visible = false

func _on_save_button_pressed():
	_save_game()

func _on_load_button_pressed():
	_load_game()

func _on_settings_button_pressed():
	settings_panel.visible = !settings_panel.visible
	menu_panel.visible = false

func _on_quit_button_pressed():
	get_tree().quit()

# Settings Event Handlers
func _on_text_speed_changed(value: float):
	text_speed = value

func _on_music_volume_changed(value: float):
	music_volume = value
	music_player.volume_db = linear_to_db(value)

func _on_sfx_volume_changed(value: float):
	sfx_volume = value
	sfx_player.volume_db = linear_to_db(value)

func _on_auto_speed_changed(value: float):
	auto_speed = value

# Auto and Skip Mode
func _start_auto_mode():
	# Auto mode will automatically advance dialog
	pass

func _stop_auto_mode():
	# Stop auto mode
	pass

func _start_skip_mode():
	# Skip mode will skip through dialog quickly
	pass

func _stop_skip_mode():
	# Stop skip mode
	pass

# Save/Load System
func _save_game():
	"""Save the current game state"""
	var save_data = {
		"current_chapter": current_chapter,
		"dialog_manager_state": dialog_manager.save_state(),
		"condition_system_state": dialog_manager._get_condition_system().save_state(),
		"settings": {
			"text_speed": text_speed,
			"music_volume": music_volume,
			"sfx_volume": sfx_volume,
			"auto_speed": auto_speed
		}
	}
	
	var save_file = FileAccess.open("user://savegame.save", FileAccess.WRITE)
	if save_file:
		save_file.store_string(JSON.stringify(save_data))
		save_file.close()
		print("Game saved successfully")

func _load_game():
	"""Load a saved game state"""
	var save_file = FileAccess.open("user://savegame.save", FileAccess.READ)
	if save_file:
		var content = save_file.get_as_text()
		save_file.close()
		
		var save_data = JSON.parse_string(content)
		if save_data:
			current_chapter = save_data.get("current_chapter", "chapter1")
			dialog_manager.load_state(save_data.get("dialog_manager_state", {}))
			dialog_manager._get_condition_system().load_state(save_data.get("condition_system_state", {}))
			
			var settings = save_data.get("settings", {})
			text_speed = settings.get("text_speed", 1.0)
			music_volume = settings.get("music_volume", 0.7)
			sfx_volume = settings.get("sfx_volume", 0.8)
			auto_speed = settings.get("auto_speed", 2.0)
			
			# Update UI
			text_speed_slider.value = text_speed
			music_volume_slider.value = music_volume
			sfx_volume_slider.value = sfx_volume
			auto_speed_slider.value = auto_speed
			
			# Reload chapter
			_load_chapter(current_chapter)
			print("Game loaded successfully")

# Input handling
func _input(event):
	if event.is_action_pressed("ui_accept") and is_dialog_active:
		_on_next_button_pressed()
	elif event.is_action_pressed("ui_cancel"):
		if menu_panel.visible:
			menu_panel.visible = false
		elif settings_panel.visible:
			settings_panel.visible = false
		elif is_dialog_active:
			_on_previous_button_pressed()
	elif event.is_action_pressed("ui_menu"):
		_on_menu_button_pressed()
```
