
## Face detection using Haar cascades
## Download cascade from:
## https://raw.githubusercontent.com/opencv/opencv/master/data/haarcascades/haarcascade_frontalface_default.xml
extends Node

@export var cam_index = 0

var cam = null
var cam_active = false
var cascade_path = null

func _ready() -> void:
	# Use absolute path to cascade file
	cascade_path = ProjectSettings.globalize_path("res://addons/gdopencv/examples/haarcascades/haarcascade_frontalface_default.xml")
	#cascade_path = ProjectSettings.globalize_path("res://addons/gdopencv/examples/haarcascades/haarcascade_frontalface_alt2.xml")

	cam = CVCamera.new()
	#Maybe Change to other Format
	cam.pixel_format = CVCamera.FORMAT_MJPG
	cam.resolution = Vector2i(640, 480)
	cam.set_fps(30.0)
	
	var opened = cam.open(cam_index)
	
	print(cam.get_capture_info())
	#print(cam.get_property_list())
	if opened:
		cam_active = true
		print("Camera opened")
	else:
		print("Failed to open camera")

func _process(_delta) -> void:
	if not cam_active:
		return
	var frame:CVImage = cam.read_frame()
	if frame == null or frame.is_empty():
		return
		
	var faces = frame.detect_haar_cascade(cascade_path, 1.3, 3, Vector2i(20, 20))
	#print(faces)
	for face in faces:
		frame.draw_rectangle(face, Color.GREEN, 2)
	
	var eye_cascade = ProjectSettings.globalize_path("res://addons/gdopencv/examples/haarcascades/haarcascade_eye_tree_eyeglasses.xml")
	var eyes = frame.detect_haar_cascade(eye_cascade)
	
	for eye in eyes:
		frame.draw_rectangle(eye, Color.RED, 1)
		
	$TextureRect.texture = frame.to_image_texture()

func _exit_tree() -> void:
	if cam:
		cam.close()
