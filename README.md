# Projects 5 & 6: Lights, Camera & Action!

All project handouts can be found [here](https://cs1230.graphics/projects).

PROJECT 5: LIGHTS, CAMERA:

Design Choices:
- I decided to make 4 VAOs and 4 VBOs, one for each shape.
- The data for a unit shape is loaded into the VBO/VAOs every time we initializeGL or the settings are changed.
- This unit data is then drew and transformed for every shape using a for loop inside paintGL.
- I also have a for loop over the lights in paintGL to pass lighting data (direction and color) into a lighting struct in the fragment shader. 

Known Bugs:
- None

PROJECT 6: ACTION!:

Design Choices:
- My Filters are only able to work one at a time, if multiple are pressed, the one that is highest up on the UI will take effect
- To change default FBO, m_defaultFBO variable is on line 92 of realtime.cpp
- I used lab 11 as heavy inspiration for implementing the FBO part of the assignment

Known Bugs:
- None

Extra Credit:
- Extra Credit 1 checkbox will activate a grayscale per-pixel filter
- Extra Credit 2 checkbox will activate a gaussian blur kernel based filter
- I implemented my RayTracer, so when you check the raytrace button, it will show a scrollable image of the scene from that camera angle using the raytracer (I was having trouble sizing it down to fit all in the window since I'm not an expert with QT, but the entire image is there, it's just big), and to go back to realtime rendering, you can just uncheck the box
- Tesselation based on number of shapes in the scene: In the beginning of the render, the tesselation of the shapes is determined by determineTesselation() and sets the initial tesselation to an amount based on the number of shapes in the scene. This will change back to the numbers on the slider as soon as one of the sliders is changed.
- I implemented a mesh file parser (https://github.com/Bly7/OBJ-Loader). I edited scene parser so I can read mesh files and store the data, and I wrote functionality in realtime so I can passing it into a VBO/VAO pair. I have a bug where the mesh won't appear on the screen but I get no GL errors/crashes and I'm positive I'm super close to getting it to show up :(
I know it doesn't fully work, but I spent so long on this (there was so much more debugging required than i had anticipated) and would really really appreciate even just a point or two of extra credit for the effort if possible :')
Feel free to take a look at the steps I took to get the mesh file stuff working (in scene parser, realtime, OBJ_Loader, etc.) and if you notice what I did wrong/have missing, please let me know!