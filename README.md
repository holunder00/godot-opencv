#####################################################################################################################################################
#####################################################################################################################################################
##This GDExtension was made by Holunder Heiss for "Interplayful Environments - University of Arts Linz" with the purpose do use Godot as a media art tool.

With this Extension you can access a Webcam on Windows/Linux/OSX and use basic OpenCV features. 
It is possible to use the Camera in GDScript and also use the Camera Image directly as a texture.

At the moment there is no possibilitie to detect the camera config automatically. You have to select the right camera index, and the Pixelformat yourself.

Some of the OPENCV tools are implementet at the moment does are this ones (see the examples):<br />
<ul>
    <li>Using Camera in Gamemode from GDScript</li>
    <li>Directly use Camera Image as Texture "CVCameraTexture" in Editor and Game</li>
    <li>Face Detection (the Haircascade files have to be downloaded and referenced)</li>
    <li>Basic OpenCV drawing functionality</li>
    <li>Image Filters: </li>
        <ul>
        <li>blur</li>
        <li>gaussian blur</li>
        <li>median blur</li>
        <li>canny edge detection</li>
        <li>threshold</li>
        <li>adaptive threshold</li>
        <li>dilate</li>
        <li>erode</li>
        <li>morphology_ex</li>
        </ul>
</ul>

AI information:
The Extension is created with a little help from Claude AI

To use it Download the Release built or built it yourself. For Linux there are some basic built Instructions in "how_to_compileLinux.txt", or you use the git built Actions.

See the Examples on how to use it.


## Dependencies

This repository uses the following Git submodules:

- `godot-cpp`, licensed under MIT
- OpenCV, licensed under Apache-2.0

After cloning, initialize dependencies with:

```bash
git submodule update --init --recursive

