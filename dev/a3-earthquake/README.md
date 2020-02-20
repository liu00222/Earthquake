# Assignment 3: Earthquake

_by << Yupei Liu >>_

## C range requirements

In the initialization part of the "earth.cc" file, I make 60 slices and 60 stacks, which create 7200 triangles and 61 * 61 vertices and normals for the earth. 

To find the position of each vertex to be pushed back into the vertices arrays, I make use of the latter-defined LatLongToPlane() and LatLongToSphere() functions. In this way, as long as I digure out the latitude and the longitude of each vertex, I am able to find its Point3 version and add it to the vertices arrays of both plane and sphere. 

For the indices array, I draw triangles from the left down part. Using the (latitude, longitude) to describe this, I draw the vertex (-90, -180) first, and then move up until I draw the vertex (90, -180). Moving in this way, I finally draw the last vertex at (90, 180). 

Finally, I set the vertices, normals, and indices arryas to the mesh and then update the GPU memory. All the above things are made in the "init()" function in the "earth.cc". 

## B range requirements

I fill in the texture coordinate array in the "init()" function in the "earth.cc" file at line 74. The order I push back the coordinates into this array is identical to the order of vertices array. Similarly, I set the texture coordinates and update it to GPU memory at the end of the initialization function. 

When I plot the earthquakes on the map, I first use the "FindMostRecentQuake()" method of the quake's database class and then use a while-loop to update the earthquake information into the global vector called "earthquakes" that I define to store the recent quakes' information. Next, for each earthquake stored in the vector earthquakes, I derive its latitude, longitude, and magnitude. Then, I use the linear interpolation method of Mingfx's Point3 class using the earthquake's latitude and longitude to find its coordinate in our map. Note that using the linear interpolation can make sure that the earthquakes' little point is always on the earth, no matter it is in the plane form, sphere form, or during the conversion between these two. 

To meke the way that earthquakes' are represented in different, I make use of their magnitudes. The greater the magnitude, the more red the earthquake's point will be and the larger the earthquake's visual point will be. I calculate two related variables, "color" and "size", based on its magnitude. I think it is more realisitc and meaningful to determine the color and the size of the earthquake by its magitude, because if I implement in this way, the viewers of the earth would easily see how severe each earthquake is. 

## A range requirements

To change the earth to sphere, I firstly fill in the function "LatLongToSphere()" in the "earth.cc" file. Using this function, I can calculate the normals and vertices arrays of the sphere, and store them as global variables. Also, I can use this function in drawing the earthquakes, because when I call the linear interpolation in that part, I should know the earthquke's plane coordinate and sphere coordinate. This keeps the earthquake stick on the earth. To make the change, I add another function called "UpdateEarth()" in the "earth" class. Based on the input of this function, it can determine if the earth is in the plane mode, globe mode, or in the progress of transition between modes. Anyway, no matter which state the earth is in, I will update the vetices and normals to the GPU memory in this function. Each time the globe button is pressed, this function will be called properly and make the earth convert to the other representation mode. 

I design the algorithm to make the earth change smoothly between globe mode and plane mode in "UpdateEarth()" function in "earth.cc" file and "UpdateSimulation()" in "quakeapp.cc" file. I also increment the rotation of the earth in the "UpdateSimulation()" function and represent the rotation in "DrawUsingOpenGL()" part. Note that there is a time difference when the earth converts from sphere to plane.  More specifically, the earth would firstly rotate back to "rotation == 0" state and then start to transfer to the plane mode. In my previous test, some bugs may happen here. I fixed it by adding the case 3 from line 175 to line 179 in the "quakeapp.cc" file. 