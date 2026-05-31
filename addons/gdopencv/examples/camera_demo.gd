
## Camera capture with edge detection overlay
extends Control

var cam = null
var cam_active = false
var show_edges = false

func _ready() -> void:
	cam = CVCamera.new()
	add_child(cam)
	cam.resolution = Vector2i(640, 480)

	var opened = cam.open(0)
	if opened:
		cam_active = true
		print("Camera opened")
	else:
		print("Failed to open camera")

func _process(_delta) -> void:
	if not cam_active:
		return
	var frame = cam.read_frame()
	if frame == null or frame.is_empty():
		return

	if show_edges:
		var edges = frame.canny(50.0, 150.0)
		$Sprite2D.texture = edges.to_image_texture()
	else:
		$Sprite2D.texture = frame.to_image_texture()

func _input(event) -> void:
	if event.is_action_pressed("ui_accept"):
		show_edges = !show_edges

func _exit_tree() -> void:
	cam_active = false
	if cam:
		cam.close()
