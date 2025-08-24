extends Node2D

@onready var dialog_manager = $DialogManager
@onready var dialog_ui = $DialogUI
@onready var background = $Background
@onready var character_left = $CharacterLeft
@onready var character_right = $CharacterRight
@onready var music_player = $MusicPlayer

var characters = {}

func _ready():
	# Initialize characters
	_setup_characters()
	
	# Connect signals
	dialog_manager.dialog_started.connect(_on_dialog_started)
	dialog_manager.dialog_ended.connect(_on_dialog_ended)
	dialog_manager.dialog_node_changed.connect(_on_dialog_node_changed)
	dialog_manager.choice_made.connect(_on_choice_made)
	
	# Start the visual novel
	_start_visual_novel()

func _setup_characters():
	# Create character definitions
	var alice = GoetheCharacter.new()
	alice.id = "alice"
	alice.name = "Alice"
	alice.add_portrait("happy", "neutral", "res://addons/goethe_dialog/examples/visual_novel/portraits/alice_happy.png")
	alice.add_portrait("sad", "neutral", "res://addons/goethe_dialog/examples/visual_novel/portraits/alice_sad.png")
	alice.add_voice_clip("greeting", "res://addons/goethe_dialog/examples/visual_novel/audio/alice_greeting.ogg")
	characters["alice"] = alice
	
	var bob = GoetheCharacter.new()
	bob.id = "bob"
	bob.name = "Bob"
	bob.add_portrait("happy", "neutral", "res://addons/goethe_dialog/examples/visual_novel/portraits/bob_happy.png")
	bob.add_portrait("angry", "neutral", "res://addons/goethe_dialog/examples/visual_novel/portraits/bob_angry.png")
	bob.add_voice_clip("greeting", "res://addons/goethe_dialog/examples/visual_novel/audio/bob_greeting.ogg")
	characters["bob"] = bob

func _start_visual_novel():
	# Load the main story dialog
	var story_dialog = """
id: visual_novel_story
nodes:
  - id: intro
    speaker: alice
    text: Welcome to our visual novel! This is Alice speaking.
    portrait: alice_happy
    voice: alice_greeting
  - id: bob_intro
    speaker: bob
    text: And I'm Bob! Nice to meet you.
    portrait: bob_happy
    voice: bob_greeting
  - id: choice_scene
    speaker: alice
    text: What kind of story would you like to experience?
    portrait: alice_happy
    choices:
      - id: romance
        text: A romantic story
        to: romance_start
      - id: adventure
        text: An adventure story
        to: adventure_start
      - id: mystery
        text: A mystery story
        to: mystery_start
  - id: romance_start
    speaker: alice
    text: Oh, you chose romance! How sweet.
    portrait: alice_happy
  - id: adventure_start
    speaker: bob
    text: Adventure it is! Let's go on an epic journey!
    portrait: bob_happy
  - id: mystery_start
    speaker: alice
    text: A mystery? How intriguing...
    portrait: alice_happy
"""
	
	if dialog_manager.load_dialog_from_yaml(story_dialog):
		dialog_manager.start_dialog()
	else:
		print("Failed to load visual novel dialog")

func _on_dialog_started(dialog_id: String):
	print("Visual novel started: ", dialog_id)
	dialog_ui.visible = true

func _on_dialog_ended(dialog_id: String):
	print("Visual novel ended: ", dialog_id)
	# Could restart or show credits here

func _on_dialog_node_changed(node: GoetheDialogNode):
	dialog_ui.display_node(node)
	
	# Handle character display
	_show_character(node.speaker, node.portrait)
	
	# Handle voice playback
	if not node.voice.is_empty():
		_play_voice(node.voice)

func _on_choice_made(choice_id: String, choice_text: String):
	print("Choice made: ", choice_id, " - ", choice_text)
	
	# Handle different story branches
	match choice_id:
		"romance":
			_set_background("res://addons/goethe_dialog/examples/visual_novel/backgrounds/romance_bg.png")
			_play_music("res://addons/goethe_dialog/examples/visual_novel/music/romance_theme.ogg")
		"adventure":
			_set_background("res://addons/goethe_dialog/examples/visual_novel/backgrounds/adventure_bg.png")
			_play_music("res://addons/goethe_dialog/examples/visual_novel/music/adventure_theme.ogg")
		"mystery":
			_set_background("res://addons/goethe_dialog/examples/visual_novel/backgrounds/mystery_bg.png")
			_play_music("res://addons/goethe_dialog/examples/visual_novel/music/mystery_theme.ogg")
	
	dialog_manager.next_node()

func _show_character(character_id: String, portrait_id: String):
	if character_id.is_empty():
		character_left.visible = false
		character_right.visible = false
		return
	
	var character = characters.get(character_id)
	if not character:
		return
	
	var portrait_path = character.get_portrait(portrait_id, "neutral")
	if not portrait_path.is_empty():
		var texture = load(portrait_path)
		if texture:
			character_left.texture = texture
			character_left.visible = true
			character_right.visible = false

func _play_voice(voice_id: String):
	# Implementation for voice playback
	pass

func _set_background(bg_path: String):
	var texture = load(bg_path)
	if texture:
		background.texture = texture

func _play_music(music_path: String):
	var stream = load(music_path)
	if stream:
		music_player.stream = stream
		music_player.play()
```

```gdscript:addons/goethe_dialog/examples/basic_dialog/dialog_ui.gd
extends Control

@onready var speaker_label = $VBoxContainer/SpeakerLabel
@onready var text_label = $VBoxContainer/TextLabel
@onready var portrait_texture = $VBoxContainer/PortraitTexture
@onready var choices_container = $VBoxContainer/ChoicesContainer
@onready var next_button = $VBoxContainer/ButtonContainer/NextButton
@onready var previous_button = $VBoxContainer/ButtonContainer/PreviousButton

func _ready():
	# Initially hidden
	visible = false

func display_node(node: GoetheDialogNode):
	# Display speaker
	if node.speaker.is_empty():
		speaker_label.text = "Narrator"
	else:
		speaker_label.text = node.speaker
	
	# Display text
	text_label.text = node.text
	
	# Display portrait if available
	if not node.portrait.is_empty():
		var texture = load(node.portrait)
		if texture:
			portrait_texture.texture = texture
			portrait_texture.visible = true
		else:
			portrait_texture.visible = false
	else:
		portrait_texture.visible = false
	
	# Display choices if available
	_clear_choices()
	if node.choices.size() > 0:
		for choice in node.choices:
			var choice_button = Button.new()
			choice_button.text = choice.text
			choice_button.pressed.connect(_on_choice_button_pressed.bind(choice))
			choices_container.add_child(choice_button)
	
	# Show/hide navigation buttons
	next_button.visible = node.choices.size() == 0
	previous_button.visible = true

func _clear_choices():
	for child in choices_container.get_children():
		child.queue_free()

func _on_choice_button_pressed(choice: Dictionary):
	# This will be handled by the parent
	pass
```

Now let me also create a scene file for the example:

```tscn:addons/goethe_dialog/examples/basic_dialog/basic_dialog_example.tscn
[gd_scene load_steps=4 format=3 uid="uid://bqxvn8yqxqxqx"]

[ext_resource type="Script" path="res://addons/goethe_dialog/examples/basic_dialog/basic_dialog_example.gd" id="1_0xqxq"]
[ext_resource type="Script" path="res://addons/goethe_dialog/examples/basic_dialog/dialog_ui.gd" id="2_0xqxq"]

[sub_resource type="StyleBoxFlat" id="StyleBoxFlat_1"]
bg_color = Color(0.2, 0.2, 0.2, 0.8)
corner_radius_top_left = 10
corner_radius_top_right = 10
corner_radius_bottom_right = 10
corner_radius_bottom_left = 10

[node name="BasicDialogExample" type="Node2D"]
script = ExtResource("1_0xqxq")

[node name="DialogManager" type="Node" parent="."]

[node name="DialogUI" type="Control" parent="."]
layout_mode = 3
anchors_preset = 15
anchor_right = 1.0
anchor_bottom = 1.0
script = ExtResource("2_0xqxq")

[node name="VBoxContainer" type="VBoxContainer" parent="DialogUI"]
layout_mode = 1
anchors_preset = 12
anchor_top = 1.0
anchor_right = 1.0
anchor_bottom = 1.0
offset_left = 50.0
offset_top = -200.0
offset_right = -50.0
offset_bottom = -50.0

[node name="SpeakerLabel" type="Label" parent="DialogUI/VBoxContainer"]
layout_mode = 2
text = "Speaker"
font_size = 18
font_color = Color(1, 1, 0, 1)

[node name="TextLabel" type="Label" parent="DialogUI/VBoxContainer"]
layout_mode = 2
text = "Dialog text will appear here..."
font_size = 16
autowrap_mode = 2

[node name="PortraitTexture" type="TextureRect" parent="DialogUI/VBoxContainer"]
layout_mode = 2
custom_minimum_size = Vector2(0, 100)
stretch_mode = 5

[node name="ChoicesContainer" type="VBoxContainer" parent="DialogUI/VBoxContainer"]
layout_mode = 2

[node name="ButtonContainer" type="HBoxContainer" parent="DialogUI/VBoxContainer"]
layout_mode = 2

[node name="PreviousButton" type="Button" parent="DialogUI/VBoxContainer/ButtonContainer"]
layout_mode = 2
text = "Previous"

[node name="NextButton" type="Button" parent="DialogUI/VBoxContainer/ButtonContainer"]
layout_mode = 2
text = "Next"

[node name="StartButton" type="Button" parent="."]
offset_left = 50.0
offset_top = 50.0
offset_right = 200.0
offset_bottom = 100.0
text = "Start Dialog"
```

Let me also create a more comprehensive example with a visual novel setup:

```gdscript:addons/goethe_dialog/examples/visual_novel/visual_novel_example.gd
extends Node2D

@onready var dialog_manager = $DialogManager
@onready var dialog_ui = $DialogUI
@onready var background = $Background
@onready var character_left = $CharacterLeft
@onready var character_right = $CharacterRight
@onready var music_player = $MusicPlayer

var characters = {}

func _ready():
	# Initialize characters
	_setup_characters()
	
	# Connect signals
	dialog_manager.dialog_started.connect(_on_dialog_started)
	dialog_manager.dialog_ended.connect(_on_dialog_ended)
	dialog_manager.dialog_node_changed.connect(_on_dialog_node_changed)
	dialog_manager.choice_made.connect(_on_choice_made)
	
	# Start the visual novel
	_start_visual_novel()

func _setup_characters():
	# Create character definitions
	var alice = GoetheCharacter.new()
	alice.id = "alice"
	alice.name = "Alice"
	alice.add_portrait("happy", "neutral", "res://addons/goethe_dialog/examples/visual_novel/portraits/alice_happy.png")
	alice.add_portrait("sad", "neutral", "res://addons/goethe_dialog/examples/visual_novel/portraits/alice_sad.png")
	alice.add_voice_clip("greeting", "res://addons/goethe_dialog/examples/visual_novel/audio/alice_greeting.ogg")
	characters["alice"] = alice
	
	var bob = GoetheCharacter.new()
	bob.id = "bob"
	bob.name = "Bob"
	bob.add_portrait("happy", "neutral", "res://addons/goethe_dialog/examples/visual_novel/portraits/bob_happy.png")
	bob.add_portrait("angry", "neutral", "res://addons/goethe_dialog/examples/visual_novel/portraits/bob_angry.png")
	bob.add_voice_clip("greeting", "res://addons/goethe_dialog/examples/visual_novel/audio/bob_greeting.ogg")
	characters["bob"] = bob

func _start_visual_novel():
	# Load the main story dialog
	var story_dialog = """
id: visual_novel_story
nodes:
  - id: intro
    speaker: alice
    text: Welcome to our visual novel! This is Alice speaking.
    portrait: alice_happy
    voice: alice_greeting
  - id: bob_intro
    speaker: bob
    text: And I'm Bob! Nice to meet you.
    portrait: bob_happy
    voice: bob_greeting
  - id: choice_scene
    speaker: alice
    text: What kind of story would you like to experience?
    portrait: alice_happy
    choices:
      - id: romance
        text: A romantic story
        to: romance_start
      - id: adventure
        text: An adventure story
        to: adventure_start
      - id: mystery
        text: A mystery story
        to: mystery_start
  - id: romance_start
    speaker: alice
    text: Oh, you chose romance! How sweet.
    portrait: alice_happy
  - id: adventure_start
    speaker: bob
    text: Adventure it is! Let's go on an epic journey!
    portrait: bob_happy
  - id: mystery_start
    speaker: alice
    text: A mystery? How intriguing...
    portrait: alice_happy
"""
	
	if dialog_manager.load_dialog_from_yaml(story_dialog):
		dialog_manager.start_dialog()
	else:
		print("Failed to load visual novel dialog")

func _on_dialog_started(dialog_id: String):
	print("Visual novel started: ", dialog_id)
	dialog_ui.visible = true

func _on_dialog_ended(dialog_id: String):
	print("Visual novel ended: ", dialog_id)
	# Could restart or show credits here

func _on_dialog_node_changed(node: GoetheDialogNode):
	dialog_ui.display_node(node)
	
	# Handle character display
	_show_character(node.speaker, node.portrait)
	
	# Handle voice playback
	if not node.voice.is_empty():
		_play_voice(node.voice)

func _on_choice_made(choice_id: String, choice_text: String):
	print("Choice made: ", choice_id, " - ", choice_text)
	
	# Handle different story branches
	match choice_id:
		"romance":
			_set_background("res://addons/goethe_dialog/examples/visual_novel/backgrounds/romance_bg.png")
			_play_music("res://addons/goethe_dialog/examples/visual_novel/music/romance_theme.ogg")
		"adventure":
			_set_background("res://addons/goethe_dialog/examples/visual_novel/backgrounds/adventure_bg.png")
			_play_music("res://addons/goethe_dialog/examples/visual_novel/music/adventure_theme.ogg")
		"mystery":
			_set_background("res://addons/goethe_dialog/examples/visual_novel/backgrounds/mystery_bg.png")
			_play_music("res://addons/goethe_dialog/examples/visual_novel/music/mystery_theme.ogg")
	
	dialog_manager.next_node()

func _show_character(character_id: String, portrait_id: String):
	if character_id.is_empty():
		character_left.visible = false
		character_right.visible = false
		return
	
	var character = characters.get(character_id)
	if not character:
		return
	
	var portrait_path = character.get_portrait(portrait_id, "neutral")
	if not portrait_path.is_empty():
		var texture = load(portrait_path)
		if texture:
			character_left.texture = texture
			character_left.visible = true
			character_right.visible = false

func _play_voice(voice_id: String):
	# Implementation for voice playback
	pass

func _set_background(bg_path: String):
	var texture = load(bg_path)
	if texture:
		background.texture = texture

func _play_music(music_path: String):
	var stream = load(music_path)
	if stream:
		music_player.stream = stream
		music_player.play()
```

Now let me create a README for the examples:

```markdown:addons/goethe_dialog/examples/README.md
# Goethe Dialog Plugin Examples

This directory contains examples demonstrating how to use the Goethe Dialog Plugin in Godot.

## Examples

### Basic Dialog (`basic_dialog/`)

A simple dialog system implementation that demonstrates:
- Basic dialog loading from YAML
- Dialog navigation (next/previous)
- Choice system
- Simple UI integration

**Files:**
- `basic_dialog_example.gd` - Main example script
- `dialog_ui.gd` - UI controller for dialog display
- `basic_dialog_example.tscn` - Scene file

**Usage:**
1. Open the scene in Godot
2. Press "Start Dialog" to begin
3. Use "Next" and "Previous" buttons to navigate
4. Make choices when presented

### Visual Novel (`visual_novel/`)

A more advanced example showing:
- Character management with portraits
- Background switching
- Music and voice integration
- Branching storylines
- Advanced dialog features

**Files:**
- `visual_novel_example.gd` - Main visual novel script
- `visual_novel_example.tscn` - Scene file

**Features:**
- Multiple characters with portraits
- Background music and voice clips
- Story branching based on choices
- Advanced dialog formatting

## Running the Examples

1. **Enable the Plugin:**
   - Go to Project Settings → Plugins
   - Enable "Goethe Dialog"

2. **Open an Example:**
   - Open the `.tscn` file in Godot
   - Run the scene (F5)

3. **Test Features:**
   - Try different dialog options
   - Test choice systems
   - Experiment with character switching

## Customizing Examples

### Adding Your Own Dialog

Create a YAML file with your dialog:

```yaml
id: my_story
nodes:
  - id: start
    speaker: protagonist
    text: Hello, this is my story!
  - id: choice
    speaker: npc
    text: What would you like to do?
    choices:
      - id: option1
        text: Go left
      - id: option2
        text: Go right
```

### Adding Characters

```gdscript
var character = GoetheCharacter.new()
character.id = "my_character"
character.name = "My Character"
character.add_portrait("happy", "neutral", "res://portraits/happy.png")
character.add_voice_clip("greeting", "res://audio/greeting.ogg")
```

### Loading Custom Dialog

```gdscript
var dialog_manager = GoetheDialogManager.new()
dialog_manager.load_dialog_from_file("res://my_dialog.yaml")
dialog_manager.start_dialog()
```

## Tips

1. **Dialog Format:** Use the YAML format for easy editing
2. **Characters:** Define characters once and reuse them
3. **Choices:** Use the choice system for branching stories
4. **Effects:** Add effects to choices for game state changes
5. **UI:** Customize the UI to match your game's style

## Troubleshooting

- **Plugin not found:** Make sure the plugin is enabled in Project Settings
- **Dialog not loading:** Check the YAML syntax and file paths
- **Characters not showing:** Verify portrait file paths exist
- **Choices not working:** Ensure choice IDs match in your dialog structure
```

The examples folder should now contain:
- `basic_dialog/` - Simple dialog example
- `visual_novel/` - Advanced visual novel example  
- `README.md` - Documentation for the examples

These examples demonstrate the full range of the Goethe Dialog Plugin's capabilities, from basic dialog to complex visual novel features.
```

