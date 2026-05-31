
## Face detection using Haar cascades
## Download cascade from:
## https://raw.githubusercontent.com/opencv/opencv/master/data/haarcascades/haarcascade_frontalface_default.xml
extends Control

var cam = null
var cam_active = false
var cascade_path = ""

func _ready() -> void:
	# Use absolute path to cascade file
	cascade_path = ProjectSettings.globalize_path("res://addons/gdopencv/data/haarcascade_frontalface_default.xml")

	cam = CVCamera.new()
	add_child(cam)
	cam.resolution = Vector2i(640, 480)
	cam_active = cam.open(0)

func _process(_delta) -> void:
	if not cam_active:
		return
	var frame = cam.read_frame()
	if frame == null or frame.is_empty():
		return

	var faces = frame.detect_haar_cascade(cascade_path, 1.3, 5, Vector2i(30, 30))
	for face in faces:
		frame.draw_rectangle(face, Color.GREEN, 2)

	$TextureRect.texture = frame.to_image_texture()

func _exit_tree() -> void:
	if cam:
		cam.close()
