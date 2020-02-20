/** CSci-4611 Assignment 3:  Earthquake
 */

#include "earth.h"
#include "config.h"

#include <vector>

// for M_PI constant
#define _USE_MATH_DEFINES
#include <math.h>


// global vertices and normals arrays for plane and sphere

std::vector<Point3> plane_vertices;
std::vector<Vector3> plane_normals;

std::vector<Point3> sphere_vertices;
std::vector<Vector3> sphere_normals;


Earth::Earth() {
}

Earth::~Earth() {
}

void Earth::Init(const std::vector<std::string> &search_path) {
    // init shader program
    shader_.Init();
    
    // init texture: you can change to a lower-res texture here if needed
    earth_tex_.InitFromFile(Platform::FindFile("earth-2k.png", search_path));

    // init geometry
    
    // 60 * 60 pieces of rectangles
    // 120 * 60 pieces of triangles
    // 61 * 61 vertices, normals, and texture coordinates
    
    const int nslices = 60;
    const int nstacks = 60;

    // TODO: This is where you need to set the vertices and indiceds for earth_mesh_.
    
    // local indices and texture coordinates arrays
    
    std::vector<unsigned int> indices;
    std::vector<Point2> tex_coords;

    
    
    // fill in the global and plane verices arrays and normal arrays
    
    for (int i = 0; i <= nslices; i++)
        for (int j = 0; j <= nstacks; j++) {
            
            // make use of the functions that I will define latter to find the
            
            float lat = ((float) j)/nstacks * 180 - 90;
            float lon = ((float) i)/nslices * 360 - 180;
            
            Point3 plane_coord = LatLongToPlane(lat, lon);
            plane_vertices.push_back(Point3(plane_coord.x(), plane_coord.y(), 0));
            plane_normals.push_back(Vector3(0, 0, 1));

            Point3 global_coord = LatLongToSphere(lat, lon);
            sphere_vertices.push_back(global_coord);
            sphere_normals.push_back(Vector3(global_coord.x(), global_coord.y(), global_coord.z()));
            
            // fill in the texture coordinates array
            
            tex_coords.push_back(Point2(((float) i)/nslices, 1 - ((float) j)/nstacks));
        }
    
    
    // fill in the indices array 3 by 3 to draw the triangles
    
    // note that The mesh connectivity and tex_coords do not need to change when moving from plane to global
    
    for(int i = 0; i < nstacks; i++)
        for(int j = 0; j < nslices; j++) {
            indices.push_back((unsigned int) (i + j * (nslices + 1)));
            indices.push_back((unsigned int) (i + (j + 1) * (nslices + 1)));
            indices.push_back((unsigned int) (i + (j + 1) * (nslices + 1) + 1));

            indices.push_back((unsigned int) (i + j * (nslices + 1)));
            indices.push_back((unsigned int) (i + (j + 1) * (nslices + 1) + 1));
            indices.push_back((unsigned int) (i + j * (nslices + 1) + 1));
        }
    
    
    // update the mesh data to GPU
    
    earth_mesh_.SetVertices(plane_vertices);
    earth_mesh_.SetNormals(plane_normals);
    earth_mesh_.SetIndices(indices);
    earth_mesh_.SetTexCoords(0, tex_coords);
    earth_mesh_.UpdateGPUMemory();
}



void Earth::Draw(const Matrix4 &model_matrix, const Matrix4 &view_matrix, const Matrix4 &proj_matrix) {
    // Define a really bright white light.  Lighting is a property of the "shader"
    DefaultShader::LightProperties light;
    light.position = Point3(10,10,10);
    light.ambient_intensity = Color(1,1,1);
    light.diffuse_intensity = Color(1,1,1);
    light.specular_intensity = Color(1,1,1);
    shader_.SetLight(0, light);

    // Adust the material properties, material is a property of the thing
    // (e.g., a mesh) that we draw with the shader.  The reflectance properties
    // affect the lighting.  The surface texture is the key for getting the
    // image of the earth to show up.
    DefaultShader::MaterialProperties mat;
    mat.ambient_reflectance = Color(0.5, 0.5, 0.5);
    mat.diffuse_reflectance = Color(0.75, 0.75, 0.75);
    mat.specular_reflectance = Color(0.75, 0.75, 0.75);
    mat.surface_texture = earth_tex_;

    // Draw the earth mesh using these settings
    if (earth_mesh_.num_triangles() > 0) {
        shader_.Draw(model_matrix, view_matrix, proj_matrix, &earth_mesh_, mat);
    }
}


Point3 Earth::LatLongToSphere(double latitude, double longitude) const {
    // TODO: We recommend filling in this function to put all your
    // lat,long --> sphere calculations in one place.
    
    
    // convert the latitude and longitude from degrees to radians
    
    // then we convert latitude and longitude into the 3-D Cartesian coordinates using the provided formulas
    
    float lat = GfxMath::ToRadians((float) latitude);
    float lon = GfxMath::ToRadians((float) longitude);
    
    return Point3(cosf(lat) * sinf(lon), sinf(lat), cosf(lat) * cosf(lon));
}

Point3 Earth::LatLongToPlane(double latitude, double longitude) const {
    // TODO: We recommend filling in this function to put all your
    // lat,long --> plane calculations in one place.
    
    return Point3((float) longitude / 180 * (float) M_PI, (float) latitude / 180 * (float) M_PI, 0);
}



void Earth::DrawDebugInfo(const Matrix4 &model_matrix, const Matrix4 &view_matrix, const Matrix4 &proj_matrix) {
    // This draws a cylinder for each line segment on each edge of each triangle in your mesh.
    // So it will be very slow if you have a large mesh, but it's quite useful when you are
    // debugging your mesh code, especially if you start with a small mesh.
    for (int t=0; t<earth_mesh_.num_triangles(); t++) {
        std::vector<unsigned int> indices = earth_mesh_.triangle_vertices(t);
        std::vector<Point3> loop;
        loop.push_back(earth_mesh_.vertex(indices[0]));
        loop.push_back(earth_mesh_.vertex(indices[1]));
        loop.push_back(earth_mesh_.vertex(indices[2]));
        quick_shapes_.DrawLines(model_matrix, view_matrix, proj_matrix,
            Color(1,1,0), loop, QuickShapes::LinesType::LINE_LOOP, 0.005);
    }
}



void Earth::UpdateEarth(const float scalar_parameter) {
    
    // when the transformation is done, directly update the earth to decrease the amount of calculation
    
    if (scalar_parameter == 0.0) {
        earth_mesh_.SetVertices(plane_vertices);
        earth_mesh_.SetNormals(plane_normals);
    }
    
    else if (scalar_parameter == 1.0) {
        earth_mesh_.SetVertices(sphere_vertices);
        earth_mesh_.SetNormals(sphere_normals);
    }
    
    
    // when the transformation is in the progress
    
    else {
        std::vector<Point3> vertices;
        std::vector<Vector3> normals;

        for (int i = 0; i < plane_vertices.size(); i++) {
            vertices.push_back(plane_vertices[i].Lerp(sphere_vertices[i], scalar_parameter));
            normals.push_back(plane_normals[i].Lerp(sphere_normals[i], scalar_parameter));
        }
        earth_mesh_.SetVertices(vertices);
        earth_mesh_.SetNormals(normals);
    }
    
    earth_mesh_.UpdateGPUMemory();
}
