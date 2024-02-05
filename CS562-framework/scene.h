////////////////////////////////////////////////////////////////////////
// The scene class contains all the parameters needed to define and
// draw a simple scene, including:
//   * Geometry
//   * Light parameters
//   * Material properties
//   * Viewport size parameters
//   * Viewing transformation values
//   * others ...
//
// Some of these parameters are set when the scene is built, and
// others are set by the framework in response to user mouse/keyboard
// interactions.  All of them can be used to draw the scene.

#include "shapes.h"
#include "object.h"
#include "texture.h"
#include "fbo.h"

enum ObjectIds {
    nullId	= 0,
    skyId	= 1,
    seaId	= 2,
    groundId	= 3,
    roomId	= 4,
    boxId	= 5,
    frameId	= 6,
    lPicId	= 7,
    rPicId	= 8,
    teapotId	= 9,
    spheresId	= 10,
    floorId     = 11,
    bunnyId     = 12,
    QuadId     = 13,
    SphereId     = 14,
};

class Shader;


class Scene
{
public:
    GLFWwindow* window;

    // @@ Declare interactive viewing variables here. (spin, tilt, ry, front back, ...)

    // Light parameters
    float lightSpin, lightTilt, lightDist;
    glm::vec3 lightPos;
    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;
    std::vector<float> lightRadius;
    // @@ Perhaps declare additional scene lighting values here. (lightVal, lightAmb)
    const unsigned int NR_LIGHTS = 32;


    bool drawReflective;
    bool nav;
    bool w_down, s_down, a_down, d_down;
    float spin, tilt, speed, ry, front, back;
    glm::vec3 eye, tr;
    float last_time;
    int smode; // Shadow on/off/debug mode
    int rmode; // Extra reflection indicator hooked up some keys and sent to shader
    int lmode; // BRDF mode
    int tmode; // Texture mode
    int imode; // Image Based Lighting mode
    bool flatshade;
    int mode; // Extra mode indicator hooked up to number keys and sent to shader
    bool localLights = false;
    // Viewport
    int width, height;

    // Transformations
    glm::mat4 WorldProj, WorldView, WorldInverse;

    // All objects in the scene are children of this single root object.
    Object* objectRoot;
    Object* objectRootLight;
    Object *central, *anim, *room, *floor, *teapot, *podium, *sky,*quad,
            *ground, *sea, *spheres, *leftFrame, *rightFrame, *bunny, *light,
        *bunny1, *bunny2, *bunny3, *bunny4;

    std::vector<Object*> animated;
    ProceduralGround* proceduralground;

    // Shader programs
    ShaderProgram* gBufferProgram;
    ShaderProgram* lightingProgram;
    ShaderProgram* lightBoxProgram;
    // @@ Declare additional shaders if necessary
    FBO* fbo;
    Texture* m_texture;

    // Options menu stuff
    bool show_demo_window;
    float lightX = 0;
    float lightY = 0;
    float lightZ = 0;
    void InitializeScene();
    void BuildTransforms();
    void DrawMenu();
    void DrawScene();

};
