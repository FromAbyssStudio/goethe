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
