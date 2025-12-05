extends Control
## JS Game Launcher - Scans user://games/ for AI-generated mini-games
## Each game gets its own JSSandbox for true per-game memory isolation
## When switching games, the sandbox is destroyed and all JS memory is freed

const GAMES_DIR = "user://games/"

var current_game_scene: Node = null
var current_sandbox: JSSandbox = null  # Per-game sandbox for isolation
var current_loader: AsyncSceneLoader = null  # Async loader for current game
var games: Array[Dictionary] = []
var pending_game: Dictionary = {}  # Game being loaded

@onready var launcher_ui: VBoxContainer = $LauncherUI
@onready var game_container: Node = $GameContainer
@onready var back_button: Button = $BackButton
@onready var game_list_container: VBoxContainer = $LauncherUI/GameList/GameListContainer
@onready var status_label: Label = $LauncherUI/StatusLabel
@onready var refresh_button: Button = $LauncherUI/Header/RefreshButton
@onready var loading_panel: PanelContainer = $LoadingPanel
@onready var loading_label: Label = $LoadingPanel/VBoxContainer/LoadingLabel
@onready var progress_bar: ProgressBar = $LoadingPanel/VBoxContainer/ProgressBar
@onready var stage_label: Label = $LoadingPanel/VBoxContainer/StageLabel


func _ready() -> void:
	refresh_button.pressed.connect(_on_refresh_pressed)
	back_button.pressed.connect(_on_back_pressed)

	# Ensure games directory exists
	DirAccess.make_dir_recursive_absolute(GAMES_DIR)

	scan_games()


func _input(event: InputEvent) -> void:
	# ESC to go back to launcher while in game or during loading
	# But only if the game tree is NOT paused (let game handle restart)
	if event.is_action_pressed("ui_cancel") and (current_game_scene or current_loader):
		if not get_tree().paused:
			_on_back_pressed()


func scan_games() -> void:
	games.clear()

	# Clear existing game buttons
	for child in game_list_container.get_children():
		child.queue_free()

	var dir: DirAccess = DirAccess.open(GAMES_DIR)
	if not dir:
		status_label.text = "No games folder found. Place games in: " + GAMES_DIR
		return

	dir.list_dir_begin()
	var folder_name: String = dir.get_next()

	while folder_name != "":
		if dir.current_is_dir() and not folder_name.begins_with("."):
			var game_path: String = GAMES_DIR + folder_name + "/"
			var manifest: Dictionary = load_manifest(game_path)
			if manifest:
				manifest["path"] = game_path
				manifest["folder"] = folder_name
				games.append(manifest)
		folder_name = dir.get_next()

	dir.list_dir_end()

	if games.is_empty():
		status_label.text = "No games found. Place game folders in: " + GAMES_DIR
	else:
		status_label.text = "Found %d game(s)" % games.size()
		create_game_buttons()


func load_manifest(game_path: String) -> Dictionary:
	var manifest_path: String = game_path + "game.json"

	if not FileAccess.file_exists(manifest_path):
		print("No game.json in: ", game_path)
		return {}

	var file: FileAccess = FileAccess.open(manifest_path, FileAccess.READ)
	if not file:
		print("Cannot open: ", manifest_path)
		return {}

	var json_text: String = file.get_as_text()
	file.close()

	var json: JSON = JSON.new()
	var error: Error = json.parse(json_text)
	if error != OK:
		print("JSON parse error in ", manifest_path, ": ", json.get_error_message())
		return {}

	var data: Variant = json.get_data()
	if not data is Dictionary:
		print("Invalid manifest format in: ", manifest_path)
		return {}

	# Validate required fields
	if not data.has("name") or not data.has("entry_scene"):
		print("Missing required fields in: ", manifest_path)
		return {}

	return data


func create_game_buttons() -> void:
	for game in games:
		var button: Button = Button.new()
		button.text = game.get("name", "Unknown Game")
		button.custom_minimum_size = Vector2(0, 50)
		button.pressed.connect(_on_game_selected.bind(game))

		# Add description if available
		if game.has("description"):
			button.tooltip_text = game["description"]

		game_list_container.add_child(button)


func _on_game_selected(game: Dictionary) -> void:
	print("Loading game: ", game["name"])

	var entry_scene_path: String = game["path"] + game["entry_scene"]

	if not FileAccess.file_exists(entry_scene_path):
		status_label.text = "Error: Entry scene not found: " + entry_scene_path
		return

	# Register input actions from manifest
	if game.has("input_actions"):
		register_input_actions(game["input_actions"])

	# Store pending game info
	pending_game = game

	# Create a new sandbox for this game (per-game isolation)
	current_sandbox = JSSandbox.new()
	print("Created isolated sandbox for game: ", game["name"])

	# Show loading UI
	launcher_ui.hide()
	loading_panel.show()
	loading_label.text = "Loading: " + game["name"]
	progress_bar.value = 0
	stage_label.text = "Starting..."

	# Start async loading
	current_loader = current_sandbox.load_scene_async(entry_scene_path)
	current_loader.progress_changed.connect(_on_loading_progress)
	current_loader.completed.connect(_on_loading_completed)
	current_loader.failed.connect(_on_loading_failed)


func _process(_delta: float) -> void:
	# Poll the async loader if active
	if current_loader and current_loader.is_loading():
		current_loader.poll()


func _on_loading_progress(progress: float, stage: String) -> void:
	progress_bar.value = progress * 100.0
	stage_label.text = "Stage: " + stage


func _on_loading_completed(scene: Node) -> void:
	current_game_scene = scene
	current_loader = null

	# Clear existing game content
	for child in game_container.get_children():
		child.queue_free()

	# Add game scene directly to game_container
	game_container.add_child(current_game_scene)

	# Switch to game view
	loading_panel.hide()
	back_button.show()

	status_label.text = "Playing: " + pending_game["name"]
	print("Game loaded with isolated sandbox: ", pending_game["name"])
	pending_game = {}


func _on_loading_failed(error: String) -> void:
	current_loader = null
	loading_panel.hide()
	launcher_ui.show()

	status_label.text = "Error: " + error
	print("Failed to load scene: ", error)
	cleanup_sandbox()
	pending_game = {}


func cleanup_sandbox() -> void:
	# Clean up loader if active
	if current_loader:
		current_loader = null

	if current_sandbox:
		# Reset the sandbox to release all JS resources
		current_sandbox.reset()
		current_sandbox = null
		print("Sandbox destroyed - all JS memory freed")


func register_input_actions(actions: Dictionary) -> void:
	for action_name: String in actions:
		var action_data: Dictionary = actions[action_name]

		# Remove existing action if present
		if InputMap.has_action(action_name):
			InputMap.erase_action(action_name)

		# Add new action
		InputMap.add_action(action_name)

		# Add key events
		if action_data.has("keys"):
			for key_str: String in action_data["keys"]:
				var event: InputEventKey = InputEventKey.new()
				event.keycode = get_keycode_from_name(key_str)
				if event.keycode != KEY_NONE:
					InputMap.action_add_event(action_name, event)


func get_keycode_from_name(key_name: String) -> Key:
	match key_name.to_upper():
		"A": return KEY_A
		"B": return KEY_B
		"C": return KEY_C
		"D": return KEY_D
		"E": return KEY_E
		"F": return KEY_F
		"G": return KEY_G
		"H": return KEY_H
		"I": return KEY_I
		"J": return KEY_J
		"K": return KEY_K
		"L": return KEY_L
		"M": return KEY_M
		"N": return KEY_N
		"O": return KEY_O
		"P": return KEY_P
		"Q": return KEY_Q
		"R": return KEY_R
		"S": return KEY_S
		"T": return KEY_T
		"U": return KEY_U
		"V": return KEY_V
		"W": return KEY_W
		"X": return KEY_X
		"Y": return KEY_Y
		"Z": return KEY_Z
		"0": return KEY_0
		"1": return KEY_1
		"2": return KEY_2
		"3": return KEY_3
		"4": return KEY_4
		"5": return KEY_5
		"6": return KEY_6
		"7": return KEY_7
		"8": return KEY_8
		"9": return KEY_9
		"SPACE": return KEY_SPACE
		"ENTER", "RETURN": return KEY_ENTER
		"ESCAPE", "ESC": return KEY_ESCAPE
		"TAB": return KEY_TAB
		"BACKSPACE": return KEY_BACKSPACE
		"LEFT": return KEY_LEFT
		"RIGHT": return KEY_RIGHT
		"UP": return KEY_UP
		"DOWN": return KEY_DOWN
		"SHIFT": return KEY_SHIFT
		"CTRL", "CONTROL": return KEY_CTRL
		"ALT": return KEY_ALT
		_: return KEY_NONE


func _on_back_pressed() -> void:
	# Cancel loading if in progress
	if current_loader:
		loading_panel.hide()
		cleanup_sandbox()
		pending_game = {}

	if current_game_scene:
		# Remove from parent first to trigger _exit_tree on JS scripts
		if current_game_scene.get_parent():
			current_game_scene.get_parent().remove_child(current_game_scene)
		current_game_scene.queue_free()
		current_game_scene = null

	# Safe to clear our reference - JSScript holds Ref<JSSandbox> so sandbox
	# stays alive until all scripts are actually freed by queue_free()
	cleanup_sandbox()

	# Switch back to launcher
	back_button.hide()
	launcher_ui.show()

	status_label.text = "Found %d game(s)" % games.size()


func _on_refresh_pressed() -> void:
	scan_games()
