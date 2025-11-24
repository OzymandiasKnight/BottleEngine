#include <stdio.h>
#include <math.h>
#include <SDL3/SDL.h>


float deg_to_rad(float degrees) {
    return degrees*3.14519/180.0;
}

typedef struct {
    float x;
    float y;
    float z;
} Vector3D;

typedef struct {
    Vector3D position[3];
} Triangle;

typedef struct {
    Vector3D position;
    Vector3D rotation;
    double fov;
    int size_x;
    int size_y;
} Camera3D;

typedef struct {
    float value[3][3];
} Matrix3D;

typedef struct {
    SDL_Renderer* renderer;
    Camera3D* camera;
} Engine;

typedef struct {
    Vector3D vertex[8];
    Vector3D edges[12][2];
    Vector3D faces[6][4];
} Cube;

typedef struct {
    Vector3D vertices[1000];
    int count_v;
    Vector3D faces[1000][4];
    int count_f;
    Vector3D triangles[1000][3];
    int count_t;
} Mesh;

void vector_display(Vector3D a) {
    printf("x : %f y : %f z : %f\n", a.x, a.y, a.z);
}

void tri_display(Triangle tri) {
    vector_display(tri.position[0]);
    vector_display(tri.position[1]);
    vector_display(tri.position[2]);
}

void mesh_display(Mesh* mesh) {
    printf("\n--- Vertices ---\n");
    for (int i=0; i<mesh->count_v; i++) {
        printf("%d - ", i);
        vector_display(mesh->vertices[i]);

    }

    printf("--- Triangles ---\n");
    for (int i=0; i<mesh->count_t; i++) {
        printf("%d : \n", i);
        vector_display(mesh->triangles[i][0]);
        vector_display(mesh->triangles[i][1]);
        vector_display(mesh->triangles[i][2]);
    }
}

void face_to_tri(Vector3D face[4], Triangle triangles[2]) {
    triangles[0].position[0] = face[0];
    triangles[0].position[1] = face[1];
    triangles[0].position[2] = face[2];
    triangles[1].position[0] = face[0];
    triangles[1].position[1] = face[3];
    triangles[1].position[2] = face[2];
}



Cube create_cube(Vector3D scale, Vector3D position) {
    Cube cube;
    cube.vertex[0] = (Vector3D){1, 1, 1};
    cube.vertex[1] = (Vector3D){-1, 1, 1};
    cube.vertex[2] = (Vector3D){-1, -1, 1};
    cube.vertex[3] = (Vector3D){1, -1, 1};
    cube.vertex[4] = (Vector3D){1, 1, -1};
    cube.vertex[5] = (Vector3D){-1, 1, -1};
    cube.vertex[6] = (Vector3D){-1, -1, -1};
    cube.vertex[7] = (Vector3D){1, -1, -1};

    int edges[12][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0}, // front face
    {4, 5}, {5, 6}, {6, 7}, {7, 4}, // back face
    {0, 4}, {1, 5}, {2, 6}, {3, 7}  // connecting edges
    };

    for (int i=0; i<8; i++) {
        cube.vertex[i].x *= scale.x;
        cube.vertex[i].x += position.x*2;
        cube.vertex[i].y *= scale.y;
        cube.vertex[i].y += position.y*2;
        cube.vertex[i].z *= scale.z;
        cube.vertex[i].z += position.z*2;
    }

    for (int i=0; i<12; i++) {
        cube.edges[i][0] = cube.vertex[edges[i][0]];
        cube.edges[i][1] = cube.vertex[edges[i][1]];
    }

    Vector3D faces[6][4] = {
        { cube.vertex[0], cube.vertex[1], cube.vertex[2], cube.vertex[3] }, // front
        { cube.vertex[4], cube.vertex[5], cube.vertex[6], cube.vertex[7] }, // back
        { cube.vertex[0], cube.vertex[4], cube.vertex[5], cube.vertex[1] }, // top
        { cube.vertex[2], cube.vertex[6], cube.vertex[7], cube.vertex[3] }, // bottom
        { cube.vertex[0], cube.vertex[3], cube.vertex[7], cube.vertex[4] }, // right
        { cube.vertex[1], cube.vertex[2], cube.vertex[6], cube.vertex[5] }  // left
    };

    for (int f = 0; f < 6; f++) {
        for (int v = 0; v < 4; v++) {
            cube.faces[f][v] = faces[f][v];
        }
    }

    return cube;

}

Vector3D vector_sub(Vector3D a, Vector3D b) {
    Vector3D vector;
    vector.x = a.x-b.x;
    vector.y = a.y-b.y;
    vector.z = a.z-b.z;
    return vector;
}



void matrice_display(Matrix3D matrice) {
    for (int y=0; y<3; y++) {
        for (int x=0; x<3; x++) {
            printf("%d ", matrice.value[y][x]);
        }
        printf("\n");
    }
    printf("\n");
}

Vector3D vector_matrice_mult(Matrix3D a, Vector3D b) {
    Vector3D vector;
    vector = b;
    vector.x = a.value[0][0]*b.x + a.value[0][1]*b.y + a.value[0][2]*b.z;
    vector.y = a.value[1][0]*b.x + a.value[1][1]*b.y + a.value[1][2]*b.z;
    vector.z = a.value[2][0]*b.x + a.value[2][1]*b.y + a.value[2][2]*b.z;
    return vector;
}

Matrix3D matrice_mult(Matrix3D a,Matrix3D b) {
    Matrix3D matrice;
    for (int y=0; y<3; y++) {
        for (int x=0; x<3; x++) {
            matrice.value[y][x] = a.value[y][0]*b.value[0][x];
            matrice.value[y][x] += a.value[y][1]*b.value[1][x];
            matrice.value[y][x] += a.value[y][2]*b.value[2][x];
        }
    }
    return matrice;
};

Vector3D vector_mult(Vector3D vector, float factor){
    Vector3D vec = {vector.x*factor,vector.y*factor,vector.z*factor};
    return vec;
}

Vector3D projection(Vector3D point, Camera3D camera) {
    Vector3D relative = vector_sub(point, camera.position);
    
    float pitch = camera.rotation.x;
    float yaw = camera.rotation.y;
    float roll = camera.rotation.z;

    //Wikipedia matrix transform

    //For x
    Matrix3D rot_pitch = {{
        {1, 0, 0},
        {0, cos(pitch), sin(pitch)},
        {0, -sin(pitch), cos(pitch)}
    }};

    //For y

    Matrix3D rot_yaw = {{
        {cos(yaw), 0, -sin(yaw)},
        {0, 1, 0},
        {sin(yaw), 0, cos(yaw)}
    }};

    //For z
    Matrix3D rot_roll = {{
        {cos(roll), sin(roll), 0},
        {-sin(roll), cos(roll), 0},
        {0, 0, 1}
    }};

    Matrix3D rotation = matrice_mult(rot_roll, matrice_mult(rot_pitch,rot_yaw));

    Vector3D camera_transform = vector_matrice_mult(rotation,relative);

    double z = camera_transform.z;
    if (fabs(z) < 0.0001) z = 0.0001;

    Vector3D result;
    result.x = (camera.size_x / 2.0) + (camera.fov * camera_transform.x) / z;
    result.y = (camera.size_y / 2.0) + (camera.fov * camera_transform.y) / z;
    result.z = z;
    return result;
}


Vector3D forward(float radian) {
    Vector3D vec = {0,0,0};
    vec.x = sin(radian);
    vec.z = cos(radian);
    return vec;
}

float interpolate_x(Vector3D p1, Vector3D p2, float y) {
    if (p1.y == p2.y) {
        return p1.x;
    }
    return p1.x + (y-p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
}
