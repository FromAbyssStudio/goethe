extends Node2D

@onready var dialog_manager = $DialogManager
@onready var dialog_ui = $DialogUI
@onready var start_button = $StartButton

func _ready():
	# Connect dialog manager signals
	dialog_manager.dialog_started.connect(_on_dialog_started)
	dialog_manager.dialog_ended.connect(_on_dialog_ended)
	dialog_manager.dialog_node_changed.connect(_on_dialog_node_changed)
	dialog_manager.choice_made.connect(_on_choice_made)
	
	# Connect UI signals
	start_button.pressed.connect(_on_start_button_pressed)
	dialog_ui.next_button.pressed.connect(_on_next_button_pressed)
	dialog_ui.previous_button.pressed.connect(_on_previous_button_pressed)

func _on_start_button_pressed():
	# Load and start a sample dialog
	var sample_dialog = """
id: sample_dialog
nodes:
  - id: greeting
    speaker: Alice
    text: Hello! Welcome to our story.
  - id: response
    speaker: Bob
    text: Thank you! I'm excited to begin this adventure.
  - id: choice
    speaker: Alice
    text: What would you like to do?
    choices:
      - id: explore
        text: Explore the world
      - id: talk
        text: Talk to people
  - id: explore_result
    speaker: Narrator
    text: You decide to explore the world around you.
  - id: talk_result
    speaker: Narrator
    text: You decide to talk to the people you meet.
"""
	
	if dialog_manager.load_dialog_from_yaml(sample_dialog):
		dialog_manager.start_dialog()
	else:
		print("Failed to load dialog")

func _on_dialog_started(dialog_id: String):
	print("Dialog started: ", dialog_id)
	dialog_ui.visible = true
	start_button.visible = false

func _on_dialog_ended(dialog_id: String):
	print("Dialog ended: ", dialog_id)
	dialog_ui.visible = false
	start_button.visible = true

func _on_dialog_node_changed(node: GoetheDialogNode):
	dialog_ui.display_node(node)

func _on_choice_made(choice_id: String, choice_text: String):
	print("Choice made: ", choice_id, " - ", choice_text)
	# Handle choice logic here
	dialog_manager.next_node()

func _on_next_button_pressed():
	dialog_manager.next_node()

func _on_previous_button_pressed():
	dialog_manager.previous_node()
