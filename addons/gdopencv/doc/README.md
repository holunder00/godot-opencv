bash
cat > addons/gdopencv/doc/README.md << 'EOF'
# GDOpenCV - OpenCV for Godot

OpenCV integration for Godot 4.4+ via GDExtension.

## Installation

1. Copy the `addons/gdopencv` folder into your project's `addons/` folder
2. Go to Project → Project Settings → Plugins
3. Enable "GDOpenCV"

## Classes

### CVImage
Image container with OpenCV processing methods.

```gdscript
var img = CVImage.new()

# Load from file
img.load("/path/to/image.png")

# Create from Godot Image
img.create_from_image(my_godot_image)

# Processing
var gray = img.to_grayscale()
var blurred = img.gaussian_blur(Vector2i(5, 5), 1.5)
var edges = img.canny(50.0, 150.0)
var resized = img.resize(Vector2i(320, 240))

# Convert back to Godot
var texture = img.to_image_texture()
var godot_image = img.to_image()

# Drawing
img.draw_rectangle(Rect2i(10, 10, 100, 100), Color.RED, 2)
img.draw_circle(Vector2i(50, 50), 25, Color.GREEN, -1)
img.draw_text("Hello", Vector2i(10, 30), 0, 1.0, Color.WHITE, 1)
