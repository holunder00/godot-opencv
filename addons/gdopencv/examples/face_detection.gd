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
EOF

cat > addons/gdopencv/examples/camera_demo.gd << 'EOF'
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
        $TextureRect.texture = edges.to_image_texture()
    else:
        $TextureRect.texture = frame.to_image_texture()

func _input(event) -> void:
    if event.is_action_pressed("ui_accept"):
        show_edges = !show_edges

func _exit_tree() -> void:
    cam_active = false
    if cam:
        cam.close()
EOF

cat > addons/gdopencv/examples/edge_detection.gd << 'EOF'
## Various edge detection methods
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
EOF

cat > addons/gdopencv/examples/face_detection.gd << 'EOF'
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
