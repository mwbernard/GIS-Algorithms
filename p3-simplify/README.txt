p3 - simplify


to run from command line:

“./simplify -input brunsdem.asc -epsilon 10

all code written in c

KEYPRESS commands:

use 'w', 'a', 's', 'd' to move up, left, down, and right, respectively

use 'f' and 'b' to move the terrain forward and backward

use 'x,X' , 'y,Y' , and 'z,Z' to rotate around the three axis 

use 'r' and 'R' to adjust the resolution of the image

use 'm' and 'M' to adjust the magnitude of the elevation heights

use 'q' to quit 

***IMPORTANT
use ‘g’ to render the actual grid

use ‘h’ to render a simulated grid (interpolated using the tin) ***note this is very slow because it is a linear search. However, once it is computed once you can toggle between different viewings of the grid/tins

use ‘j’ to render the 3d tin

use ‘k’ to render the 2d tin