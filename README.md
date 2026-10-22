#####################################################################################################################################################
#####################################################################################################################################################
##This GDExtension was made by Holunder Heiss for "Interplayful Environments - University of Arts Linz" with the purpose f using Godot as a media art tool.

With this extension you can access a webcam on Windows/Linux/OSX and use basic OpenCV features. 
It is possible to use the camera in GDScript and also use the camera image directly as a texture.

At the moment there is no possibility to detect the camera config automatically. You have to select the right camera index and the Pixelformat yourself.

Some of the OPENCV tools that are implementet at the moment are these ones (see the examples):<br />
<ul>
    <li>Using Camera in Game Mode from GDScript</li>
    <li>Directly use the camera image as texture "CVCameraTexture" in the editor and game</li>
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

To use it download the release built or build it yourself. For Linux there are some basic built instructions in "how_to_compileLinux.txt", or just use the git built actions.

See the examples on how to use it.


## Dependencies

This repository uses the following Git submodules:

- `godot-cpp`, licensed under MIT
- OpenCV, licensed under Apache-2.0

After cloning, initialize dependencies with:

```bash
git submodule update --init --recursive

