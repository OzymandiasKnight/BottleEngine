#include <stdio.h>
#include <math.h>
#include "space.h"
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

int line_values_count(const char* line) {
    int count=0;
    int i=0;
    
    while (line[i] != '\0') {
        if (line[i] == ' ') {
            count += 1;
        }
        i++;
    }
    return count;
}

Vector3D read_line_vector(const char* line) {
    Vector3D result;
    char value[64];
    int iv = 0;
    int i = 2;
    while(line[i] != ' ') {
        value[iv] = line[i];
        i += 1;
        iv += 1;
    }
    value[iv] = '\0';
    result.x = atof(value);
    iv = 0;
    i += 1;
    value[0] = '\0';
    while(line[i] != ' ') {
        value[iv] = line[i];
        i += 1;
        iv += 1;
    }
    value[iv] = '\0';
    result.y = -atof(value);
    iv = 0;
    i += 1;
    value[0] = '\0';
    
    while(line[i] != '\0') {
        value[iv] = line[i];
        i += 1;
        iv += 1;
    }
    
    value[iv] = '\0';
    result.z = atof(value);
    return result;
}

void read_line_face(const char* line, int result[4]) {
    char value[64];
    int iv = 0;
    int i = 2;
    while(line[i] != ' ') {
        value[iv] = line[i];
        i += 1;
        iv += 1;
    }
    value[iv] = '\0';
    result[0] = atof(value);
    iv = 0;
    if(line[i] == ' ') i++;

    while(line[i] != ' ') {
        value[iv] = line[i];
        i += 1;
        iv += 1;
    }
    value[iv] = '\0';
    result[1] = atof(value);
    iv = 0;
    if(line[i] == ' ') i++;

    while(line[i] != ' ' && line[i] != '\0') {
        value[iv] = line[i];
        i += 1;
        iv += 1;
    }
    value[iv] = '\0';
    result[2] = atof(value);
    iv = 0;
    if(line[i] == ' ') i++;

    while(line[i] != '\n') {
        value[iv] = line[i];
        i += 1;
        iv += 1;
    }
    
    value[iv] = '\0';
    result[3] = (iv > 0) ? atof(value) : 0.0;
}

void read_file(const char* file_path, Mesh* mesh_model) {
    FILE *fptr;
    fptr = fopen(file_path, "r");
    char line[64];


    int v_count = 0;
    int f_count = 0;
    int t_count = 0;
    while ((fgets(line, 63, fptr))) {
        if (line[0] == 'v') {   
            mesh_model->vertices[mesh_model->count_v] = read_line_vector(line);
            vector_display(mesh_model->vertices[mesh_model->count_v]);
            mesh_model->count_v += 1;
        }
    
        if (line[0] == 'f') {
            int val_count = line_values_count(line);
            int faces[4];
            read_line_face(line, faces);
            if (val_count == 4) {
                mesh_model->faces[f_count][0] = mesh_model->vertices[faces[0]-1];
                mesh_model->faces[f_count][1] = mesh_model->vertices[faces[1]-1];
                mesh_model->faces[f_count][2] = mesh_model->vertices[faces[2]-1];
                mesh_model->faces[f_count][3] = mesh_model->vertices[faces[3]-1];
                f_count += 1;
            }
            else {
                mesh_model->triangles[t_count][0] = mesh_model->vertices[faces[0]-1];
                mesh_model->triangles[t_count][1] = mesh_model->vertices[faces[1]-1];
                mesh_model->triangles[t_count][2] = mesh_model->vertices[faces[2]-1];
                t_count += 1;
            }
        }
    }

    mesh_model->count_f = f_count;
    mesh_model->count_t = t_count;

    fclose(fptr);
}


SDL_Window* create_window(const char* title, int x, int y, SDL_WindowFlags flags) {
    SDL_Window* window = SDL_CreateWindow(title, x, y,flags);
    
    if (!window) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return NULL;
    }

    return window;
}

void close_window(SDL_Window* window) {
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}

void draw_line(SDL_Renderer* render, Camera3D cam, Vector3D a, Vector3D b) {
    Vector3D projection_a = projection(a,cam);
    Vector3D projection_b = projection(b,cam);
    if (projection_a.z <= 0 || projection_b.z <= 0) {
        return;
    }


    SDL_RenderLine(render, projection_a.x, projection_a.y, projection_b.x, projection_b.y);
}

void draw_empty_triangle(SDL_Renderer* render, Camera3D cam, Vector3D a, Vector3D b, Vector3D c) {
    draw_line(render, cam, a, b);
    draw_line(render, cam, b, c);
    draw_line(render, cam, c, a);


}

void draw_triangle(SDL_Renderer* render, Camera3D cam,Vector3D a, Vector3D b, Vector3D c) {
    a = projection(a, cam);
    b = projection(b, cam);
    c = projection(c, cam);
    if (a.z <= 0 || b.z <= 0 || c.z <= 0) {
        return;
    }

    if (a.y > b.y) {
        Vector3D tmp = a;
        a = b;
        b = tmp;
    }
    if (b.y > c.y) {
        Vector3D tmp = b;
        b = c;
        c = tmp;
        
    }
    if (a.y > b.y) {
        Vector3D tmp = a;
        a = b;
        b = tmp;
    }

    for (int y=(int)a.y; y<=(int)b.y; y++) {
        if (y>0) {
            float left = interpolate_x(a, c, y);
            float right = interpolate_x(a, b, y);
            left = SDL_clamp(left, fmin(a.x, c.x), fmax(a.x, c.x));
            right = SDL_clamp(right, fmin(a.x, b.x), fmax(a.x, b.x));

            if (left > right) {
                float tmp = left;
                left = right;
                right = tmp;
            }

            SDL_RenderLine(render, left, y, right, y);
        }
    }

    for (int y=(int)b.y; y<=(int)c.y; y++) {
        float left = interpolate_x(a, c, y);
        float right = interpolate_x(b, c, y);
        left = SDL_clamp(left, fmin(a.x, c.x), fmax(a.x, c.x));
        right = SDL_clamp(right, fmin(b.x, c.x), fmax(b.x, c.x));


        if (left > right) {
            float tmp = left;
            left = right;
            right = tmp;
        }
        SDL_RenderLine(render, left, y, right, y);
    }



}

void draw_cube(SDL_Renderer* render, Camera3D cam, Cube cube) {
    for (int i = 0; i < 6; i++) {

        Triangle tris[2];
        face_to_tri(cube.faces[i], tris);
        draw_triangle(render, cam, tris[0].position[0], tris[0].position[1], tris[0].position[2]);

        draw_triangle(render, cam, tris[1].position[0], tris[1].position[1], tris[1].position[2]);
    }
}

void draw_mesh(SDL_Renderer* render, Camera3D cam, Mesh mesh_model) {

    for (int i = 0; i < mesh_model.count_f; i++) {
        Triangle tris[2];
        face_to_tri(mesh_model.faces[i], tris);
        draw_empty_triangle(render, cam, tris[0].position[0], tris[0].position[1], tris[0].position[2]);
        draw_empty_triangle(render, cam, tris[1].position[0], tris[1].position[1], tris[1].position[2]);
    }
    for (int i = 0; i < mesh_model.count_t; i++) {
        draw_empty_triangle(render, cam, mesh_model.triangles[i][0], mesh_model.triangles[i][1], mesh_model.triangles[i][2]);
    }
}

int main() {


    int closed = 0;

    Engine engine;
    Camera3D camera;
    camera.position = (Vector3D){0,0,0};
    camera.rotation = (Vector3D){0,0,0};
    camera.fov = 500;
    camera.size_x = 1920/2;
    camera.size_y = 1080/2;

    engine.camera = &camera;


    bool fullscreen = false;
    SDL_Window* window = create_window("Title", camera.size_x, camera.size_y, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    SDL_Event event;
    SDL_SetWindowRelativeMouseMode(window, true);
    SDL_CaptureMouse(true);

    float last_frame_time = 0.0f;

    Mesh import_mesh;


    printf("Importing...");
    read_file("Model.obj", &import_mesh);
    printf("Imported !");
    mesh_display(&import_mesh);

    while (closed == 0) {
        float delta_time = SDL_GetTicks() - last_frame_time;
        last_frame_time = SDL_GetTicks();

        Vector3D inputs = {0,0,0};

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                closed = 1;
            }
            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                camera.size_x = event.window.data1;
                camera.size_y = event.window.data2;

            }

            if (event.type == SDL_EVENT_KEY_DOWN) {
                SDL_Keycode key = event.key.key;

                if (key == SDLK_F11) {
                    fullscreen = !fullscreen;
                    SDL_SetWindowFullscreen(window, fullscreen);
                }
                /*
                Screenshot work in progress
                if (key == SDLK_F12) {
                    
                    SDL_Surface* surface = SDL_CreateSurface(camera.size_x, camera.size_y, SDL_PIXELFORMAT_ARGB8888);
                    SDL_RenderReadPixels(renderer, NULL);

                    IMG_SavePNG(surface, "./screen.png");
                }
                */
            }

           
           if (event.type == SDL_EVENT_MOUSE_MOTION) {
                float sensitivity = 0.05f;
                
                
                camera.rotation.x -= deg_to_rad(event.motion.yrel)*sensitivity;
                camera.rotation.y += (deg_to_rad(event.motion.xrel)*sensitivity);
                camera.rotation.z = 0;
                SDL_WarpMouseInWindow(window, camera.size_x/2, camera.size_y/2);
            }
        }

        int numkeys;
        const bool* keystate = SDL_GetKeyboardState(&numkeys);
        
        inputs.x=0;
        inputs.y=0;
        inputs.z=0;

        if (keystate[SDL_SCANCODE_W]) {
            inputs.y = 1;
        }
        if (keystate[SDL_SCANCODE_S]) {
            inputs.y -= 1;
        }
        if (keystate[SDL_SCANCODE_A]) {
            inputs.x -= 1;
        }
        if (keystate[SDL_SCANCODE_D]) {
            inputs.x += 1;
        }
        if (keystate[SDL_SCANCODE_SPACE]) {
            inputs.z += 1;
        }
        if (keystate[SDL_SCANCODE_LSHIFT]) {
            inputs.z -= 1;
        }

        //Inputs loop
        if (inputs.y != 0 || inputs.x != 0) {
            float speed = 0.01*delta_time;

            int input_number = abs(inputs.y)+abs(inputs.x);
            float cam_offset = (inputs.y == -1) ? deg_to_rad(180) : 0;
            cam_offset += deg_to_rad(90/input_number)*(inputs.x);
            cam_offset *= (inputs.y==0) ? 1 : inputs.y;
            Vector3D fw = forward(camera.rotation.y+cam_offset);
            
            fw = vector_mult(fw, speed);

            camera.position.x += fw.x;
            camera.position.z += fw.z;
        }
        camera.position.y -= inputs.z*delta_time*0.01;


        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        //draw_cube(renderer, camera, create_cube((Vector3D){1,1,1}, (Vector3D){0,0,0}));
        //draw_cube(renderer, camera, create_cube((Vector3D){1,1,1}, (Vector3D){0,1,0}));


        draw_mesh(renderer, camera, import_mesh);

        //draw_cube(renderer, camera, create_cube((Vector3D){4,1,1}, (Vector3D){2.5,1,0}));
        //draw_triangle(renderer, camera, (Vector3D){1,1,1}, (Vector3D){0,1,1}, (Vector3D){0,0,1});

        SDL_RenderPresent(renderer);
    }

    close_window(window);


    return 0;
}