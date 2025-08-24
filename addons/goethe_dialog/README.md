# Goethe Dialog Plugin for Godot

A Godot plugin that integrates the Goethe Dialog System library to provide powerful dialog management capabilities for visual novels and interactive narratives.

## Features

- **YAML Dialog Support**: Load and save dialogs in both simple and advanced GOETHE formats
- **Character Management**: Support for character names, expressions, moods, portraits, and voice
- **Conditional Logic**: Advanced condition system with flags, variables, and quest states
- **Effect System**: Comprehensive effect system for game state changes
- **Compression**: Built-in compression support for efficient dialog storage
- **Statistics**: Real-time performance monitoring and analysis
- **Editor Tools**: Custom dialog editor with visual interface
- **GDScript Integration**: Native GDScript classes for easy integration

## Installation

1. Copy the `addons/goethe_dialog` folder to your Godot project's `addons` directory
2. Enable the plugin in Project Settings → Plugins
3. Build the extension (see Building section below)

## Building the Extension

### Prerequisites

- Godot 4.0 or later
- CMake 3.20 or later
- C++20 compatible compiler (Clang preferred, GCC fallback)
- yaml-cpp library
- zstd library (optional, for compression)

### Build Steps

1. Navigate to the plugin directory:
   ```bash
   cd addons/goethe_dialog
   ```

2. Run the build script:
   ```bash
   chmod +x build.sh
   ./build.sh
   ```

3. The extension will be built and placed in the appropriate `bin` directory.

## Usage

### Basic Usage

```gdscript
# Create a dialog manager
var dialog_manager = GoetheDialogManager.new()
add_child(dialog_manager)

# Load a dialog from file
dialog_manager.load_dialog_from_file("res://dialogs/chapter1.yaml")

# Start the dialog
dialog_manager.start_dialog()

# Connect to signals
dialog_manager.dialog_node_changed.connect(_on_dialog_node_changed)
dialog_manager.choice_made.connect(_on_choice_made)
```

### Dialog YAML Format

#### Simple Format
```yaml
id: chapter1_intro
nodes:
  - id: greeting
    speaker: alice
    text: Hello, welcome to our story!
  - id: response
    speaker: bob
    text: Thank you, I'm excited to begin!
```

#### Advanced Format
```yaml
kind: dialogue
id: chapter1_intro
startNode: intro

nodes:
  - id: intro
    speaker: marshal
    text: Welcome to the adventure!
    portrait: { id: marshal, mood: neutral }
    voice: { clipId: vo_intro }
    choices:
      - id: accept
        text: I accept the quest
        to: quest_start
        effects:
          - type: SET_FLAG
            target: quest_accepted
            value: true
      - id: refuse
        text: I need time to think
        to: farewell
```

### Character Management

```gdscript
# Create a character
var character = GoetheCharacter.new()
character.id = "alice"
character.name = "Alice"
character.add_portrait("happy", "neutral", "res://portraits/alice_happy.png")
character.add_voice_clip("greeting", "res://audio/alice_greeting.ogg")
```

### Condition System

```gdscript
# Set flags and variables
dialog_manager.set_flag("quest_completed", true)
dialog_manager.set_variable("player_level", 10)

# Conditions in YAML will automatically check these values
```

## Editor Tools

### Dialog Editor

The plugin includes a visual dialog editor that can be accessed from the Tools menu:

1. Go to Tools → Goethe Dialog Editor
2. Create new dialogs or load existing ones
3. Edit dialog nodes visually
4. Save your changes

### Custom Types

The plugin registers several custom types:

- **GoetheDialogManager**: Main dialog management node
- **GoetheDialogNode**: Individual dialog node resource
- **GoetheCharacter**: Character definition resource

## Examples

See the `examples` directory for complete examples:

- **Basic Dialog**: Simple dialog system implementation
- **Visual Novel**: Full visual novel example with characters and choices

## API Reference

### GoetheDialogManager

#### Properties
- `current_dialog: GoetheDialog` - The currently loaded dialog
- `current_node_index: int` - Index of the current node
- `flags: Dictionary` - Global flags for conditions
- `variables: Dictionary` - Global variables for conditions

#### Methods
- `load_dialog_from_file(file_path: String) -> bool`
- `load_dialog_from_yaml(yaml_content: String) -> bool`
- `start_dialog(dialog_id: String = "") -> bool`
- `next_node() -> bool`
- `previous_node() -> bool`
- `make_choice(choice_index: int) -> bool`
- `set_flag(flag_name: String, value: bool) -> void`
- `get_flag(flag_name: String) -> bool`
- `set_variable(var_name: String, value: Variant) -> void`
- `get_variable(var_name: String) -> Variant`

#### Signals
- `dialog_started(dialog_id: String)`
- `dialog_ended(dialog_id: String)`
- `dialog_node_changed(node: GoetheDialogNode)`
- `choice_made(choice_id: String, choice_text: String)`

### GoetheDialogNode

#### Properties
- `id: String` - Unique identifier for the node
- `speaker: String` - Charact
