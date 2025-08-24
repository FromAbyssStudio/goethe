@tool
extends Window

@onready var dialog_tree = $VBoxContainer/HSplitContainer/DialogTree
@onready var node_editor = $VBoxContainer/HSplitContainer/NodeEditor
@onready var file_menu = $VBoxContainer/MenuBar/FileMenu

var current_dialog: GoetheDialog
var current_file_path: String = ""

func _ready():
	# Set up the window
	title = "Goethe Dialog Editor"
	size = Vector2(1200, 800)
	
	# Connect signals
	file_menu.get_popup().id_pressed.connect(_on_file_menu_selected)
	
	# Initialize dialog tree
	_setup_dialog_tree()
	
	# Initialize node editor
	_setup_node_editor()

func _setup_dialog_tree():
	# Set up the dialog tree for displaying dialog structure
	dialog_tree.set_columns(3)
	dialog_tree.set_column_title(0, "ID")
	dialog_tree.set_column_title(1, "Speaker")
	dialog_tree.set_column_title(2, "Text")
	
	dialog_tree.item_selected.connect(_on_dialog_node_selected)

func _setup_node_editor():
	# Set up the node editor for editing individual nodes
	pass

func _on_file_menu_selected(id: int):
	match id:
		0: # New
			_new_dialog()
		1: # Open
			_open_dialog()
		2: # Save
			_save_dialog()
		3: # Save As
			_save_dialog_as()

func _new_dialog():
	current_dialog = GoetheDialog.new()
	current_dialog.id = "new_dialog"
	current_file_path = ""
	_refresh_dialog_tree()

func _open_dialog():
	var file_dialog = FileDialog.new()
	file_dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
	file_dialog.add_filter("*.yaml", "YAML Files")
	file_dialog.add_filter("*.yml", "YAML Files")
	
	file_dialog.file_selected.connect(_on_file_selected)
	add_child(file_dialog)
	file_dialog.popup_centered()

func _on_file_selected(path: String):
	current_file_path = path
	
	var dialog_manager = GoetheDialogManager.new()
	if dialog_manager.load_dialog_from_file(path):
		current_dialog = dialog_manager.current_dialog
		_refresh_dialog_tree()
	else:
		OS.alert("Failed to load dialog file", "Error")

func _save_dialog():
	if current_file_path.is_empty():
		_save_dialog_as()
	else:
		_save_dialog_to_path(current_file_path)

func _save_dialog_as():
	var file_dialog = FileDialog.new()
	file_dialog.file_mode = FileDialog.FILE_MODE_SAVE_FILE
	file_dialog.add_filter("*.yaml", "YAML Files")
	file_dialog.add_filter("*.yml", "YAML Files")
	
	file_dialog.file_selected.connect(_on_save_file_selected)
	add_child(file_dialog)
	file_dialog.popup_centered()

func _on_save_file_selected(path: String):
	current_file_path = path
	_save_dialog_to_path(path)

func _save_dialog_to_path(path: String):
	if not current_dialog:
		return
	
	# Convert dialog to YAML and save
	var yaml_content = _dialog_to_yaml(current_dialog)
	
	var file = FileAccess.open(path, FileAccess.WRITE)
	if file:
		file.store_string(yaml_content)
		file.close()
		OS.alert("Dialog saved successfully", "Success")
	else:
		OS.alert("Failed to save dialog file", "Error")

func _refresh_dialog_tree():
	dialog_tree.clear()
	
	if not current_dialog:
		return
	
	var root = dialog_tree.create_item()
	root.set_text(0, current_dialog.id)
	root.set_text(1, "Dialog")
	root.set_text(2, str(current_dialog.nodes.size()) + " nodes")
	
	for i in range(current_dialog.nodes.size()):
		var node = current_dialog.nodes[i]
		var item = dialog_tree.create_item(root)
		item.set_text(0, node.id)
		item.set_text(1, node.speaker)
		item.set_text(2, node.text.left(50) + "..." if node.text.length() > 50 else node.text)

func _on_dialog_node_selected():
	var selected_item = dialog_tree.get_selected()
	if not selected_item or not current_dialog:
		return
	
	# Find the selected node and show it in the editor
	var node_index = selected_item.get_index()
	if node_index < current_dialog.nodes.size():
		_show_node_in_editor(current_dialog.nodes[node_index])

func _show_node_in_editor(node: GoetheDialogNode):
	# Show the node in the node editor
	pass

func _dialog_to_yaml(dialog: GoetheDialog) -> String:
	# Convert dialog to YAML string
	var yaml = "id: " + dialog.id + "\n"
	yaml += "nodes:\n"
	
	for node in dialog.nodes:
		yaml += "  - id: " + node.id + "\n"
		if not node.speaker.is_empty():
			yaml += "    speaker: " + node.speaker + "\n"
		if not node.text.is_empty():
			yaml += "    text: " + node.text + "\n"
		if not node.portrait.is_empty():
			yaml += "    portrait: " + node.portrait + "\n"
		if not node.voice.is_empty():
			yaml += "    voice: " + node.voice + "\n"
	
	return yaml
```

```

