extends Node2D

var img = CVImage.new()


func _ready() -> void:
	# The path has to be absolut for opencv. We get it by using the globalize_path function
	var path = ProjectSettings.globalize_path("res://addons/gdopencv/examples/assets/icon.png")
	# Load from file
	img.load(path)
	
	# Create from Godot Image
	#img.create_from_image(my_godot_image)
	
	var godot_image:Image = img.to_image()
	
	$TextureRect.texture = img.to_image_texture()

	# Processing
	var resized = img.resize(Vector2i(200, 200))
	var gray = resized.to_grayscale()
	var blurred = resized.gaussian_blur(Vector2i(7, 7), 4.5)
	var edges = resized.canny(50.0, 150.0)
	
	$ConvertedImage1.texture = resized.to_image_texture()
	$ConvertedImage2.texture = gray.to_image_texture()
	$ConvertedImage3.texture = blurred.to_image_texture()
	$ConvertedImage4.texture = edges.to_image_texture()
		
	# Drawing
	var img_draw = CVImage.new()
	img_draw.create_from_image(godot_image)
	#img_draw.create_from_image(img)
	img_draw.draw_rectangle(Rect2i(75, 75, 450, 450), Color.RED, 12)
	img_draw.draw_circle(Vector2i(300, 300), 100, Color.GREEN, 12)
	img_draw.draw_text("Hello GodotOpenCV", Vector2i(75, 50), 0, 1.4, Color.WHITE, 6)

	$Drawing.texture = img_draw.to_image_texture()
	
