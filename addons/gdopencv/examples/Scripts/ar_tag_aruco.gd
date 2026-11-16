
## Camera capture with edge detection overlay
extends Node2D

@export var cam_index = 0

var cam:CVCamera = null
var cam_active = false

var aru:Array


func _ready() -> void:
	cam = CVCamera.new()
	#Maybe Change to other Format
	cam.pixel_format = CVCamera.FORMAT_MJPG
	cam.resolution = Vector2i(800, 600)
	cam.set_fps(30.0)
	
	var opened = cam.open(cam_index)
	
	print(cam.get_capture_info())
	#print(cam.get_property_list())
	if opened:
		cam_active = true
		print("Camera opened")
	else:
		print("Failed to open camera")

func _draw() -> void:
	if aru.size() > 0:
		for a in aru:
			var center = a["center"]
			#print(a)
			draw_circle(center, 10, Color(1.0, 1.0, 1.0, 1.0))
			draw_line(a["corners"][0], a["corners"][1], Color(1.0, 0.0, 0.0, 1.0), 3, true)
			draw_line(a["corners"][1], a["corners"][2], Color(1.0, 0.0, 0.0, 1.0), 3, true)
			draw_line(a["corners"][2], a["corners"][3], Color(1.0, 0.0, 0.0, 1.0), 3, true)
			draw_line(a["corners"][3], a["corners"][0], Color(1.0, 0.0, 0.0, 1.0), 3, true)
			pass

func _process(_delta) -> void:
	if not cam_active:
		return
	var frame = cam.read_frame()
	if frame == null or frame.is_empty():
		return
	
	aru = frame.detect_aruco_markers(CVImage.DICT_4X4_50)
	print(aru)
	$TextureRect.texture = frame.to_image_texture()
	queue_redraw()

func _exit_tree() -> void:
	cam_active = false
	if cam:
		cam.close()
