
## Camera capture with edge detection overlay
extends Node2D

@export var cam_index = 0

var cam:CVCamera = null
var cam_active = false


enum Image_Filters {NONE, 
					CANNY_EDGE_DETECTION, 
					BLUR, 
					BLUR_GAUSSIAN, 
					BLUR_MEDIAN,
					THRESHOLD,
					DILATE,
					ERODE,
					MORPHOLOGY_EX,
					ADDAPTIVE_THRESHOLD}

@export var image_effect:Image_Filters


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

func _process(_delta) -> void:
	if not cam_active:
		return
	var frame = cam.read_frame()
	if frame == null or frame.is_empty():
		return
	
	match image_effect:
		Image_Filters.NONE:
			$Sprite2D.texture = frame.to_image_texture()
		Image_Filters.BLUR:
			var processed_frame = frame.blur(Vector2i(9.0, 9.0))
			$Sprite2D.texture = processed_frame.to_image_texture()
		Image_Filters.BLUR_GAUSSIAN:
			var processed_frame = frame.gaussian_blur(Vector2i(5.0, 5.0), 1.0)
			$Sprite2D.texture = processed_frame.to_image_texture()
		Image_Filters.CANNY_EDGE_DETECTION:
			var processed_frame = frame.canny(50.0, 150.0)
			$Sprite2D.texture = processed_frame.to_image_texture()
		Image_Filters.BLUR_MEDIAN:
			var processed_frame = frame.median_blur(13.0)
			$Sprite2D.texture = processed_frame.to_image_texture()
		Image_Filters.THRESHOLD:
			var processed_frame = frame.threshold(100.0, 200.0, 3)
			$Sprite2D.texture = processed_frame.to_image_texture()
		Image_Filters.DILATE:
			var processed_frame = frame.dilate(Vector2i(3, 12), 1)
			$Sprite2D.texture = processed_frame.to_image_texture()
		Image_Filters.ERODE:
			var processed_frame = frame.erode(Vector2i(9,9))
			$Sprite2D.texture = processed_frame.to_image_texture()
		Image_Filters.MORPHOLOGY_EX:
			var processed_frame = frame.morphology_ex(3, Vector2i(10, 10))
			$Sprite2D.texture = processed_frame.to_image_texture()
		Image_Filters.ADDAPTIVE_THRESHOLD:
			var processed_frame = frame.adaptive_threshold(255, 1, 0, 15, 10)
			$Sprite2D.texture = processed_frame.to_image_texture()

func _exit_tree() -> void:
	cam_active = false
	if cam:
		cam.close()
