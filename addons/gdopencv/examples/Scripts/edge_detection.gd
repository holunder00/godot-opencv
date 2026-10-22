
extends Control

func _ready() -> void:
	var img = CVImage.new()
	img.load("/path/to/your/image.png")  # Change this path

	# Canny edges
	var canny = img.canny(50.0, 150.0)

	# Sobel
	var sobel = img.to_grayscale().sobel(-1, 1, 0)

	# Laplacian
	var laplacian = img.to_grayscale().laplacian(-1)

	$TextureRect.texture = canny.to_image_texture()
