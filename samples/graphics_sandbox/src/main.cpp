#include <cassert>
#include <cstdlib>
#include <cstring>

#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include <raylib/rcamera.h>
 
// Create quad mesh (2 triangles)
Mesh CreateQuadMesh()
{
    Mesh mesh = {};

    // 4 vertices for quad
    mesh.vertexCount = 4;
    mesh.triangleCount = 2;

    // Allocate memory
    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
    mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount * 3 * sizeof(unsigned short));

    // Vertex positions (quad from -1 to 1)
    constexpr float vertices[] = {
        1.0f,  0.0f,  1.0f,
        1.0f, 0.0f,  -1.0f,
        -1.0f, 0.0f, -1.0f,
        -1.0f,  0.0f, 1.0f
    };

    // Indices for 2 triangles
    constexpr unsigned short indices[] = {
        0, 1, 2, // First triangle
        2, 3, 0 // Second triangle
    };

    memcpy(mesh.vertices, vertices, sizeof(vertices));
    memcpy(mesh.indices, indices, sizeof(indices));

    // Upload to GPU
    UploadMesh(&mesh, false);

    return mesh;
}

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    constexpr int grid_size = 10;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera free");

    SetTraceLogLevel(LOG_TRACE);

    // Create quad mesh
    Mesh quadMesh = CreateQuadMesh();

    // Load shader
    Shader shader = LoadShader("samples/graphics_sandbox/data/vertex.glsl", "samples/graphics_sandbox/data/fragment.glsl");
    assert(IsShaderValid(shader));

    // Get shader uniform locations
    const int gridSizeLoc = GetShaderLocation(shader, "u_gridSize");
    assert(gridSizeLoc >= 0);

    const int viewProjLoc = GetShaderLocation(shader, "u_viewProj");
    assert(viewProjLoc >= 0);

    const int worldTransformLoc = GetShaderLocation(shader, "u_worldTransform");
    assert(worldTransformLoc >= 0);

    const int cameraWorldPosLoc = GetShaderLocation(shader, "u_cameraWorldPos");
    assert(cameraWorldPosLoc >= 0);

    // const int cameraPosLoc = GetShaderLocation(shader, "u_cameraPos");
    // assert(gridSizeLoc >= 0);

    // Create material with shader
    Material material = LoadMaterialDefault();
    material.shader = shader;

    // Define the camera to look into our 3d world
    Camera3D camera = {{0}};
    camera.position = (Vector3){10.0f, 10.0f, 10.0f}; // Camera position
    camera.target = (Vector3){0.0f, 0.0f, 0.0f}; // Camera looking at point
    camera.up = (Vector3){0.0f, 1.0f, 0.0f}; // Camera up vector (rotation towards target)
    camera.fovy = 45.0f; // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE; // Camera projection type

    Matrix grid_world_transform = MatrixIdentity();

    DisableCursor(); // Limit cursor to relative movement inside the window

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_FREE);

        if (IsKeyPressed(KEY_Z))
            camera.target = (Vector3){0.0f, 0.0f, 0.0f};
        //----------------------------------------------------------------------------------

        const Matrix view_transform = GetCameraViewMatrix(&camera);
        const Matrix proj_transform = GetCameraProjectionMatrix(&camera, static_cast<float>(screenWidth)/screenHeight);
        const Matrix view_proj = view_transform * proj_transform;

        // float time = GetTime();
        SetShaderValue(shader, gridSizeLoc, &grid_size, SHADER_UNIFORM_INT);
        SetShaderValue(shader, cameraWorldPosLoc, &camera.position, SHADER_UNIFORM_VEC3);
        SetShaderValueMatrix(shader, viewProjLoc, view_proj);
        SetShaderValueMatrix(shader, worldTransformLoc, grid_world_transform);

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawMesh(quadMesh, material, MatrixIdentity());

            EndMode3D();

            DrawRectangle(10, 10, 320, 93, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(10, 10, 320, 93, BLUE);

            DrawText("Free camera default controls:", 20, 20, 10, BLACK);
            DrawText("- Mouse Wheel to Zoom in-out", 40, 40, 10, DARKGRAY);
            DrawText("- Mouse Wheel Pressed to Pan", 40, 60, 10, DARKGRAY);
            DrawText("- Z to zoom to (0, 0, 0)", 40, 80, 10, DARKGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    UnloadMesh(quadMesh);
    UnloadShader(shader);

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------}

    return 0;
}
