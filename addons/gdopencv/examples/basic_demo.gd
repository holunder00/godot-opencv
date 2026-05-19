bash
cat > addons/gdopencv/examples/basic_demo.gd << 'EOF'
## Basic OpenCV image processing demo
extends Control

func _ready() -> void:
    print("OpenCV Version: ", CVUtils.get_opencv_version())

    # Create image from Godot Image
    var img = CVImage.new()
    var godot_img = Image.create(640, 480, false, Image.FORMAT_RGB8)
    godot_img.fill(Color.CORNFLOWER_BLUE)
    img.create_from_image(godot_img)

    # Draw shapes
    img.draw_rectangle(Rect2i(50, 50, 200, 150), Color.RED, 3)
    img.draw_circle(Vector2i(320, 240), 80, Color.GREEN, -1)
    img.draw_text("Hello OpenCV!", Vector2i(100, 400), 0, 1.5, Color.WHITE, 2)

    # Display
    $TextureRect.texture = img.to_image_texture()
