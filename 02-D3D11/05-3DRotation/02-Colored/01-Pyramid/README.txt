Changes to incorporate Depth:

global:
1. Declare the depth-stencil view interface;

resize():
2a. Initialize Texture2D descriptor;
2b. Create a local 2D texture using the descriptor as storage for the depth buffer;

3a. Uninitialize the old depth-stencil view;
3b. Initialize depth-stencil view descriptor;
3c. Create a depth-stencil view using the depth buffer(2b) and descriptor(3b) and assign it to the global interface;

4. Uninitialize the local depth buffer texture(2b);

5. Use the depth-stencil view as the 3rd parameter to OMSetTargets;

display():
6. Clear the depth-stencil view;

uninitialize():
7. Uninitialize the depth-stencil view.