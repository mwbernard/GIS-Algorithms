a4 - flow


to run from command line:

./flow -input brunsdem.asc -fd flowDirectionFile.asc -fa flowAccumulationFile.asc

for the flow direction and flow accumulation file these are outputs so the user comes up
with the name



KEYPRESS commands:

use 'w', 'a', 's', 'd' to move up, left, down, and right, respectively

use 'f' and 'b' to move the terrain forward and backward

use 'x,X' , 'y,Y' , and 'z,Z' to rotate around the three axis 

use 'r' and 'R' to adjust the resolution of the image

use 'm' and 'M' to adjust the magnitude of the elevation heights

use 'q' to quit 

*********RENDERING INSTRUCTIONS

use ‘g’ to render the regular grid 2d
use ‘h’ to render the flow accumulation grid 2d
use ‘j’ to render the flow direction grid 2d

Also, important to note that in all three renderings, the higher values associate with blue. Thus, the places with the greatest flow accumulation occur in the dark blue areas. However, in the case of the regular grid, the dark blue values are actually high elevations. The focus of this program is the flow accumulation, so the rendering colors focus on that case (use ‘h’ to render)