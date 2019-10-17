--------------------------------------
KEYBOARD AND MOUSE INPUT
--------------------------------------
Translation:
  'T' Key - change mode to translate mode and change plane on which to translate (xy/xz-plane)
  Mouse Movement - move model(s) according to movement of mouse

Rotation:
  'R' Key - change mode to rotate mode and change rotation axis to rotate around (x/y/z-axis)
  Mouse Left Click - rotate counter-clockwise by 10 degrees
  Mouse Right Click - rotate clockwise by 10 degrees

Scaling:
  'E' Key - change mode to scale mode
  Mouse Wheel Scroll Up - scale bigger by 1.2x
  Mouse Wheel Scroll Down - scale smaller by 0.8x

Loading Second Model:
  'L' Key - load second model adjacent to first model and reset all transformations on first model
  and make first model centered on the origin of world space

'Spacebar' Key - change mode to no mode
  No mouse input works.

Moving Lights:
  'UP Arrow' Key - move light1 up in the +y direction
  'DOWN Arrow' Key - move light1 down in the -y direction
  'RIGHT Arrow' Key - move light1 right in the +x direction
  'LEFT Arrow' Key - move light1 left in the +- direction
  'M' Key - move light1 to you in the +z direction
  'N' Key - move light1 away from you in the -z direction

  'W' Key - move ligth2 up in the +y direction
  'S' Key - move ligth2 down in the -y direction
  'D' Key - move ligth2 right in the +x direction
  'A' Key - move ligth2 left in the +- direction
  'X' Key - move ligth2 to you in the +z direction
  'Z' Key - move ligth2 away from you in the -z direction

Toggling Light Model:
  'O' Key - toggles the light models on/off

--------------------------------------
CHANGING MODELS
--------------------------------------
To change models, modify glWindow.cpp.
Go to its OpenGLWindow::initGL() method.
Line 328 and 378 - change object names.
Choose any of the following names to change with:
  sphere.obj
  suzanne.obj
  bunny.obj
