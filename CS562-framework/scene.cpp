
////////////////////////////////////////////////////////////////////////
// The scene class contains all the parameters needed to define and
// draw a simple scene, including:
//   * Geometry
//   * Light parameters
//   * Material properties
//   * viewport size parameters
//   * Viewing transformation values
//   * others ...
//
// Some of these parameters are set when the scene is built, and
// others are set by the framework in response to user mouse/keyboard
// interactions.  All of them can be used to draw the scene.

#include "math.h"
#include <iostream>
#include <stdlib.h>

#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>
using namespace gl;

#include <glu.h>                // For gluErrorString

#define GLM_FORCE_CTOR_INIT
#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/ext.hpp>          // For printing GLM objects with to_string

#include "framework.h"
#include "shapes.h"
#include "object.h"
#include "texture.h"
#include "transform.h"
const bool fullPolyCount = true; // Use false when emulating the graphics pipeline in software

const float PI = 3.14159f;
const float rad = PI/180.0f;    // Convert degrees to radians

glm::mat4 Identity;

const float grndSize = 100.0;    // Island radius;  Minimum about 20;  Maximum 1000 or so
const float grndOctaves = 4.0;  // Number of levels of detail to compute
const float grndFreq = 0.03;    // Number of hills per (approx) 50m
const float grndPersistence = 0.03; // Terrain roughness: Slight:0.01  rough:0.05
const float grndLow = -3.0;         // Lowest extent below sea level
const float grndHigh = 5.0;        // Highest extent above sea level

////////////////////////////////////////////////////////////////////////
// This macro makes it easy to sprinkle checks for OpenGL errors
// throughout your code.  Most OpenGL calls can record errors, and a
// careful programmer will check the error status *often*, perhaps as
// often as after every OpenGL call.  At the very least, once per
// refresh will tell you if something is going wrong.
#define CHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { fprintf(stderr, "OpenGL error (at line scene.cpp:%d): %s\n", __LINE__, gluErrorString(err)); exit(-1);} }

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// Create an RGB color from human friendly parameters: hue, saturation, value
glm::vec3 HSV2RGB(const float h, const float s, const float v)
{
    if (s == 0.0)
        return glm::vec3(v,v,v);

    int i = (int)(h*6.0) % 6;
    float f = (h*6.0f) - i;
    float p = v*(1.0f - s);
    float q = v*(1.0f - s*f);
    float t = v*(1.0f - s*(1.0f-f));
    if      (i == 0)     return glm::vec3(v,t,p);
    else if (i == 1)  return glm::vec3(q,v,p);
    else if (i == 2)  return glm::vec3(p,v,t);
    else if (i == 3)  return glm::vec3(p,q,v);
    else if (i == 4)  return glm::vec3(t,p,v);
    else   /*i == 5*/ return glm::vec3(v,p,q);
}

////////////////////////////////////////////////////////////////////////
// Constructs a hemisphere of spheres of varying hues
Object* SphereOfSpheres(Shape* SpherePolygons)
{
    Object* ob = new Object(NULL, nullId);
    
    for (float angle=0.0;  angle<360.0;  angle+= 18.0)
        for (float row=0.075;  row<PI/2.0;  row += PI/2.0/6.0) {   
            glm::vec3 hue = HSV2RGB(angle/360.0, 1.0f-2.0f*row/PI, 1.0f);

            Object* sp = new Object(SpherePolygons, spheresId,
                                    hue, glm::vec3(1.0, 1.0, 1.0), 120.0);
            float s = sin(row);
            float c = cos(row);
            ob->add(sp, Rotate(2,angle)*Translate(c,0,s)*Scale(0.075*c,0.075*c,0.075*c));
        }
    return ob;
}

////////////////////////////////////////////////////////////////////////
// Constructs a -1...+1  quad (canvas) framed by four (elongated) boxes
Object* FramedPicture(const glm::mat4& modelTr, const int objectId, 
                      Shape* BoxPolygons, Shape* QuadPolygons)
{
    // This draws the frame as four (elongated) boxes of size +-1.0
    float w = 0.05;             // Width of frame boards.
    
    Object* frame = new Object(NULL, nullId);
    Object* ob;
    
    glm::vec3 woodColor(87.0 / 255.0, 51.0 / 255.0, 35.0 / 255.0);
    ob = new Object(BoxPolygons, frameId,
                    woodColor, glm::vec3(0.2, 0.2, 0.2), 10.0);
    frame->add(ob, Translate(0.0, 0.0, 1.0+w)*Scale(1.0, w, w));
    frame->add(ob, Translate(0.0, 0.0, -1.0-w)*Scale(1.0, w, w));
    frame->add(ob, Translate(1.0+w, 0.0, 0.0)*Scale(w, w, 1.0+2*w));
    frame->add(ob, Translate(-1.0-w, 0.0, 0.0)*Scale(w, w, 1.0+2*w));



    ob = new Object(QuadPolygons, objectId,
                    woodColor, glm::vec3(0.0, 0.0, 0.0), 10.0);
    frame->add(ob, Rotate(0,90));

    return frame;
}

////////////////////////////////////////////////////////////////////////
// InitializeScene is called once during setup to create all the
// textures, shape VAOs, and shader programs as well as setting a
// number of other parameters.
void Scene::InitializeScene()
{
    //glEnable(GL_DEPTH_TEST);
    //glBlendFunc(GL_ONE, GL_ONE);
    //glDisable(GL_BLEND);

    CHECKERROR;

    // @@ Initialize interactive viewing variables here. (spin, tilt, ry, front back, ...)
    glfwGetFramebufferSize(window, &width, &height);
    // Set initial light parameters
    lightSpin = 150.0;
    lightTilt = -45.0;
    lightDist = 100.0;
    // @@ Perhaps initialize additional scene lighting values here. (lightVal, lightAmb)
    
    nav = false;
    w_down = s_down = a_down = d_down = false;
    spin = 0.0;
    tilt = 30.0;
    eye = glm::vec3(0.0, -20.0, 0.0);
    speed = 300.0/30.0;
    last_time = glfwGetTime();
    tr = glm::vec3(0.0, 0.0, 25.0);

    ry = 0.4;
    front = 0.5;
    back = 5000.0;

    CHECKERROR;
    objectRoot = new Object(NULL, nullId);
    objectRootLight = new Object(NULL, nullId);

    
    // Enable OpenGL depth-testing
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    // Create the lighting shader program from source code files.
    // @@ Initialize additional shaders if necessary
    //createFBO

    //m_texture = new Texture("textures/6670-normal.jpg");
    fbo = new FBO();
    fbo->CreateFBO(width, height);


    

    //gBuffer
    gBufferProgram = new ShaderProgram();
    gBufferProgram->AddShader("gBuffer.vert", GL_VERTEX_SHADER);
    gBufferProgram->AddShader("gBuffer.frag", GL_FRAGMENT_SHADER);
    //send
    glBindAttribLocation(gBufferProgram->programId, 0, "aPos");
    glBindAttribLocation(gBufferProgram->programId, 1, "aNormal");
    glBindAttribLocation(gBufferProgram->programId, 2, "aTexCoords");
    gBufferProgram->LinkProgram();

    //lighting
    lightingProgram = new ShaderProgram();
    lightingProgram->AddShader("lightingPhong.vert", GL_VERTEX_SHADER);
    lightingProgram->AddShader("lightingPhong.frag", GL_FRAGMENT_SHADER);

    //send
    glBindAttribLocation(lightingProgram->programId, 0, "vertex");
    glBindAttribLocation(lightingProgram->programId, 1, "vertexNormal");
    glBindAttribLocation(lightingProgram->programId, 2, "vertexTexture");
    glBindAttribLocation(lightingProgram->programId, 3, "vertexTangent");
    lightingProgram->LinkProgram();

    lightBoxProgram = new ShaderProgram();
    lightBoxProgram->AddShader("lightBox.vert", GL_VERTEX_SHADER);
    lightBoxProgram->AddShader("lightBox.frag", GL_FRAGMENT_SHADER);

    glBindAttribLocation(lightBoxProgram->programId, 0, "vertex");
    glBindAttribLocation(lightBoxProgram->programId, 1, "vertexNormal");
    glBindAttribLocation(lightBoxProgram->programId, 2, "vertexTexture");
    glBindAttribLocation(lightBoxProgram->programId, 3, "vertexTangent");
    lightBoxProgram->LinkProgram();
    
    // Create all the Polygon shapes
    proceduralground = new ProceduralGround(grndSize, 400,
                                     grndOctaves, grndFreq, grndPersistence,
                                     grndLow, grndHigh);
    
    Shape* TeapotPolygons =  new Teapot(fullPolyCount?12:2);
    Shape* BoxPolygons = new Box();
    Shape* SpherePolygons = new Sphere(32);
    Shape* RoomPolygons = new Ply("room.ply");
    Shape* FloorPolygons = new Plane(10.0, 10);
    Shape* QuadPolygons = new Quad();
    Shape* SeaPolygons = new Plane(2000.0, 50);
    Shape* GroundPolygons = proceduralground;
    Shape* BunnyPolygons = new Ply("bunny_short.ply");
    //Shape* BunnyPolygons = new Ply("bunny.ply"); //Texcoord가 반대임.
    Shape* lightSphere = new Sphere(16);
    // Various colors used in the subsequent models
    glm::vec3 woodColor(87.0/255.0, 51.0/255.0, 35.0/255.0);
    glm::vec3 brickColor(134.0/255.0, 60.0/255.0, 56.0/255.0);
    glm::vec3 floorColor(6*16/255.0, 5.5*16/255.0, 3*16/255.0);
    glm::vec3 brassColor(0.5, 0.5, 0.1);
    glm::vec3 grassColor(62.0/255.0, 102.0/255.0, 38.0/255.0);
    glm::vec3 waterColor(0.3, 0.3, 1.0);

    glm::vec3 black(0.0, 0.0, 0.0);
    glm::vec3 brightSpec(0.5, 0.5, 0.5);
    glm::vec3 polishedSpec(0.3, 0.3, 0.3);
    glm::vec3 polishedSpec1(0.7, 0.7, 0.7);
 
    // Creates all the models from which the scene is composed.  Each
    // is created with a polygon shape (possibly NULL), a
    // transformation, and the surface lighting parameters Kd, Ks, and
    // alpha.
    
    // @@ This is where you could read in all the textures and
    // associate them with the various objects being created in the
    // next dozen lines of code.

    // @@ To change an object's surface parameters (Kd, Ks, or alpha),
    // modify the following lines.
    
    central    = new Object(NULL, nullId);
    anim       = new Object(NULL, nullId);
    room       = new Object(RoomPolygons, roomId, brickColor, black, 1);
    quad       = new Object(QuadPolygons, QuadId, black, black, 1);
    floor      = new Object(FloorPolygons, floorId, floorColor, black, 1);
    teapot     = new Object(TeapotPolygons, teapotId, brassColor, brightSpec, 120);
    podium     = new Object(BoxPolygons, boxId, glm::vec3(woodColor), polishedSpec, 10); 
    sky        = new Object(SpherePolygons, skyId, black, black, 0);
    ground     = new Object(GroundPolygons, groundId, grassColor, black, 1);
    sea        = new Object(SeaPolygons, seaId, waterColor, brightSpec, 120);
    bunny      = new Object(BunnyPolygons, bunnyId, brickColor, brightSpec, 110);
    bunny1      = new Object(BunnyPolygons, bunnyId, woodColor, polishedSpec, 30);
    bunny2      = new Object(BunnyPolygons, bunnyId, brassColor, brightSpec, 70);
    bunny3      = new Object(BunnyPolygons, bunnyId, brickColor, polishedSpec1, 120);
    bunny4      = new Object(BunnyPolygons, bunnyId, grassColor, brightSpec, 12);
    light      = new Object(lightSphere, SphereId, brassColor, brightSpec, 120);


    srand(13);
    for (unsigned int i = 0; i < NR_LIGHTS; i++)
    {
        // calculate slightly random offsets
        float xPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
        float yPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 4.0);
        float zPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
        lightPositions.push_back(glm::vec3(xPos, yPos, zPos));
        // also calculate random color
        float rColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
        float gColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
        float bColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
        lightColors.push_back(glm::vec3(rColor, gColor, bColor));
        //light = new Object(lightSphere, SphereId, lightColors[i], brightSpec, 1);
        //objectRootLight->add(sky, Translate(lightPositions[i].x, lightPositions[i].y, lightPositions[i].z) * Scale(0.10, 0.10, 0.10));
    }
    lightPositions.push_back(glm::vec3(0.0, 0.0, 0.0));
    lightColors.push_back(glm::vec3(1.0, 1.0, 0.8));

    leftFrame  = FramedPicture(Identity, lPicId, BoxPolygons, QuadPolygons);
    rightFrame = FramedPicture(Identity, rPicId, BoxPolygons, QuadPolygons); 
    spheres    = SphereOfSpheres(SpherePolygons);
    
#ifdef REFL
    spheres->drawMe = true;
#else
    spheres->drawMe = false;
#endif


    // @@ To change the scene hierarchy, examine the hierarchy created
    // by the following object->add() calls and adjust as you wish.
    // The objects being manipulated and their polygon shapes are
    // created above here.

    // Scene is composed of sky, ground, sea, room and some central models
    if (fullPolyCount) {
        objectRoot->add(sky, Scale(2000.0, 2000.0, 2000.0));
        //objectRoot->add(sea); 
        //objectRoot->add(ground); 
    }
    objectRoot->add(central);
#ifndef REFL
    //objectRoot->add(room,  Translate(0.0, 0.0, 0.02));
    objectRoot->add(bunny, Translate(-2.0, 2.0, 0.00) * Rotate(0, 90) * Scale(10, 10, 10));
    objectRoot->add(bunny1, Translate(0.0, 0.0, 0.00) * Rotate(0, 90) * Scale(10, 10, 10));
    objectRoot->add(bunny2, Translate(2.0, -2.0, 0.00) * Rotate(0, 90) * Scale(10, 10, 10));
    objectRoot->add(bunny3, Translate(2.0, 2.0, 0.00) * Rotate(0, 90) * Scale(10, 10, 10));
    objectRoot->add(bunny4, Translate(-2.0, -2.0, 0.00) * Rotate(0, 90) * Scale(10, 10, 10));
#endif
    objectRoot->add(floor, Translate(0.0, 0.0, 0.02) * Rotate(0, 180));


    // Central model has a rudimentary animation (constant rotation on Z)
    animated.push_back(anim);

    // Central contains a teapot on a podium and an external sphere of spheres
    //central->add(podium, Translate(0.0, 0,0));
    central->add(anim, Translate(5.0, 0,0));
    //anim->add(teapot, Translate(0,0,1)*Scale(0.31,0.31,0.31));
    //anim->add(bunny, Translate(0,0,1)*Scale(0.31,0.31,0.31));

    if (fullPolyCount)
        anim->add(spheres, Translate(0.0, 0.0, 1)*Scale(16, 16, 16));
    //anim->add(bunny, Translate(0.0, 0.0, 0.00) * Rotate(0, 90) * Scale(10, 10, 10));
    
    // Room contains two framed pictures
    if (fullPolyCount) {
        //room->add(leftFrame, Translate(-1.5, 9.85, 1.)*Scale(0.8, 0.8, 0.8));
        //room->add(rightFrame, Translate( 1.5, 9.85, 1.)*Scale(0.8, 0.8, 0.8));

    }

    lightingProgram->UseShader();
    glUniform1i(glGetUniformLocation(lightingProgram->programId, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(lightingProgram->programId, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(lightingProgram->programId, "gDiffuse"), 2);
    glUniform1i(glGetUniformLocation(lightingProgram->programId, "gSpecular"), 3);

    CHECKERROR;

    // Options menu stuff
    show_demo_window = false;
}

void Scene::DrawMenu()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    //ImGui::ShowDemoWindow();
    ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
    //if (!ImGui::Begin("Dear ImGui Demo", 0, 0))
    //{
    //    // Early out if the window is collapsed, as an optimization.
    //    ImGui::End();
    //    return;
    //}
    //ImGui::GetWindowDrawList()->AddImage((ImTextureID)fbo->gPosition, ImVec2(ImGui::GetCursorScreenPos()),
    //    ImVec2(ImGui::GetCursorScreenPos().x + width, ImGui::GetCursorScreenPos().y + height), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::Begin("Dear ImGui Demo");
    ImGui::Image((ImTextureID)fbo->gPosition, ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::SameLine();
    ImGui::Image((ImTextureID)fbo->gDiffuse, ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::Image((ImTextureID)fbo->gNormal, ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::SameLine();
    ImGui::Image((ImTextureID)fbo->gSpecular, ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));

    ImGui::DragFloat("DragFloat Light X", &lightX, 0.005f, -FLT_MAX, +FLT_MAX, "%.3f", 0);
    ImGui::DragFloat("DragFloat Light Y", &lightY, 0.005f, -FLT_MAX, +FLT_MAX, "%.3f", 0);
    ImGui::DragFloat("DragFloat Light Z", &lightZ, 0.005f, -FLT_MAX, +FLT_MAX, "%.3f", 0);
    ImGui::Checkbox("Layout Local Lights", &localLights);
    ImGui::End();
    if (ImGui::BeginMainMenuBar()) {
        // This menu demonstrates how to provide the user a list of toggleable settings.
        if (ImGui::BeginMenu("Objects")) {
            if (ImGui::MenuItem("Draw spheres", "", spheres->drawMe))  {spheres->drawMe ^= true; }
            if (ImGui::MenuItem("Draw walls", "", room->drawMe))        { room->drawMe ^= true; }
            if (ImGui::MenuItem("Draw walls", "", bunny->drawMe))       { bunny->drawMe ^= true; }
            if (ImGui::MenuItem("Draw ground/sea", "", ground->drawMe)){ground->drawMe ^= true;
                							sea->drawMe = ground->drawMe;}
            ImGui::EndMenu(); }
                	
        // This menu demonstrates how to provide the user a choice
        // among a set of choices.  The current choice is stored in a
        // variable named "mode" in the application, and sent to the
        // shader to be used as you wish.
        if (ImGui::BeginMenu("Menu ")) {
            if (ImGui::MenuItem("<sample menu of choices>", "",	false, false)) {}
            if (ImGui::MenuItem("Do nothing 0", "",		mode==0)) { mode=0; }
            if (ImGui::MenuItem("Do nothing 1", "",		mode==1)) { mode=1; }
            if (ImGui::MenuItem("Do nothing 2", "",		mode==2)) { mode=2; }
            
            ImGui::EndMenu(); }
        
        ImGui::EndMainMenuBar(); }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    
}

void Scene::BuildTransforms()
{
    // Work out the eye position as the user move it with the WASD keys.
    float now = glfwGetTime();
    float dist = (now-last_time)*speed;
    last_time = now;
    if (w_down)
        eye += dist*glm::vec3(sin(spin*rad), cos(spin*rad), 0.0);
    if (s_down)
        eye -= dist*glm::vec3(sin(spin*rad), cos(spin*rad), 0.0);
    if (d_down)
        eye += dist*glm::vec3(cos(spin*rad), -sin(spin*rad), 0.0);
    if (a_down)
        eye -= dist*glm::vec3(cos(spin*rad), -sin(spin*rad), 0.0);

    eye[2] = proceduralground->HeightAt(eye[0], eye[1]) + 2.0;

    CHECKERROR;

    if (nav)
        WorldView = Rotate(0, tilt-90)*Rotate(2, spin) *Translate(-eye[0], -eye[1], -eye[2]);
    else
        WorldView = Translate(tr[0], tr[1], -tr[2]) *Rotate(0, tilt-90)*Rotate(2, spin);
    WorldProj = Perspective((ry*width)/height, ry, front, (mode==0) ? 1000 : back);


    // @@ Print the two matrices (in column-major order) for
    // comparison with the project document.
    //std::cout << "WorldView: " << glm::to_string(WorldView) << std::endl;
    //std::cout << "WorldProj: " << glm::to_string(WorldProj) << std::endl;
}

////////////////////////////////////////////////////////////////////////
// Procedure DrawScene is called whenever the scene needs to be
// drawn. (Which is often: 30 to 60 times per second are the common
// goals.)
void Scene::DrawScene()
{
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    // Set the viewport
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    CHECKERROR;
    // Calculate the light's position from lightSpin, lightTilt, lightDist
    lightPos = glm::vec3(lightDist*cos(lightSpin*rad)*sin(lightTilt*rad),
                         lightDist*sin(lightSpin*rad)*sin(lightTilt*rad), 
                         lightDist*cos(lightTilt*rad));

    // Update position of any continuously animating objects
    double atime = 360.0*glfwGetTime()/36;
    for (std::vector<Object*>::iterator m=animated.begin();  m<animated.end();  m++)
        (*m)->animTr = Rotate(2, atime);

    BuildTransforms();

    // The lighting algorithm needs the inverse of the WorldView matrix
    WorldInverse = glm::inverse(WorldView);


    ////////////////////////////////////////////////////////////////////////////////
    // Anatomy of a pass:
    //   Choose a shader  (create the shader in InitializeScene above)
    //   Choose and FBO/Render-Target (if needed; create the FBO in InitializeScene above)
    //   Set the viewport (to the pixel size of the screen or FBO)
    //   Clear the screen.
    //   Set the uniform variables required by the shader
    //   Draw the geometry
    //   Unset the FBO (if one was used)
    //   Unset the shader
    ////////////////////////////////////////////////////////////////////////////////

    CHECKERROR;
    int loc, programId;
    ////////////////////////////////////////////////////////////////////////////////
    //1. Geometry pass
    ////////////////////////////////////////////////////////////////////////////////
    // Choose the Geometry shader
    programId = gBufferProgram->programId;
    gBufferProgram->UseShader();

    //glEnable(GL_CULL_FACE);

    //glEnable(GL_BLEND);




    glViewport(0, 0, width, height);
    glClearColor(0.0, 0.0, 0.0, 1.0); // keep it black so it doesn't leak into g-buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    fbo->BindFBO();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 model = glm::mat4(1.0);

    loc = glGetUniformLocation(programId, "WorldProj");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldProj));
    loc = glGetUniformLocation(programId, "WorldView");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldView));


    CHECKERROR;

    objectRoot->Draw(gBufferProgram, Identity);//////
    //bunny->Draw(gBufferProgram, Translate(-2.0, 2.0, 0.00) * Rotate(0, 90) * Scale(10, 10, 10));//////
    //bunny->Draw(gBufferProgram, Translate(0.0, 0.0, 0.00) * Rotate(0, 90) * Scale(10, 10, 10));//////
    //bunny->Draw(gBufferProgram, Translate(2.0, -2.0, 0.00) * Rotate(0, 90) * Scale(10, 10, 10));//////

    CHECKERROR;

    CHECKERROR;
    fbo->UnbindFBO();
    gBufferProgram->UnuseShader();

    ////////////////////////////////////////////////////////////////////////////////
    //2. Lighting pass
    ////////////////////////////////////////////////////////////////////////////////
    
    // Choose the lighting shader
    lightingProgram->UseShader();
    programId = lightingProgram->programId;

    //   Choose and FBO/Render-Target (if needed; create the FBO in InitializeScene above)
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Set the viewport, and clear the screen
    //glViewport(0, 0, width, height);
    //glClearColor(0.0, 0.0, 0.0, 1.0);


    // @@ The scene specific parameters (uniform variables) used by
    // the shader are set here.  Object specific parameters are set in
    // the Draw procedure in object.cpp
    
    loc = glGetUniformLocation(programId, "WorldProj");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldProj));
    loc = glGetUniformLocation(programId, "WorldView");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldView));
    loc = glGetUniformLocation(programId, "WorldInverse");
    glUniformMatrix4fv(loc, 1, GL_FALSE, Pntr(WorldInverse));
    loc = glGetUniformLocation(programId, "lightPos");
    glUniform3fv(loc, 1, &(lightPos[0]));   
    loc = glGetUniformLocation(programId, "mode");
    glUniform1i(loc, mode);

    CHECKERROR;
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fbo->gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, fbo->gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, fbo->gDiffuse);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, fbo->gSpecular);
    //Sets depth - testing off, blending on for additive blending, and face culling on.
    if (lightRadius.size() == 0)
        lightRadius.resize(lightPositions.size(), 0);
    //fbo->BindTexture(0, programId, "gPosition");
    for (unsigned int i = 0; i < lightPositions.size(); i++)
    {

        glUniform3fv(glGetUniformLocation(programId, ("lights[" + std::to_string(i) + "].Position").c_str()), 1, &(lightPositions[i][0]));
        glUniform3fv(glGetUniformLocation(programId, ("lights[" + std::to_string(i) + "].Color").c_str()), 1, &(lightColors[i][0]));

        // update attenuation parameters and calculate radius
        const float constant = 1.0f;
        const float linear = 0.7f;
        const float quadratic = 1.8f;
        float lightMax = std::fmaxf(std::fmaxf(lightColors[i].r, lightColors[i].g), lightColors[i].b);
        float radius =
            0.1f * (-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - (256.0 / 5.0) * lightMax)))
            / (2 * quadratic);
        lightRadius[i] = radius;
        if (i == lightPositions.size() - 1)
        {
            lightRadius[i] *= 5.f;
            radius = lightRadius[i];
        }
        glUniform1f(glGetUniformLocation(programId, ("lights[" + std::to_string(i) + "].Linear").c_str()), linear);
        glUniform1f(glGetUniformLocation(programId, ("lights[" + std::to_string(i) + "].Quadratic").c_str()), quadratic);
        glUniform1f(glGetUniformLocation(programId, ("lights[" + std::to_string(i) + "].Radius").c_str()), radius);
    }
    glUniform3fv(glGetUniformLocation(programId, "viewPos"), 1, &(eye[0]));
    renderQuad();

    // Draw all objects (This recursively traverses the object hierarchy.)
    CHECKERROR;




    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo->fboID);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
    // blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
    // the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
    // depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ///// <summary>
    ///// /////////////////////////////////////
    ///// </summary>

    //objectRootLight->Draw(lightBoxProgram, Identity);
    CHECKERROR;
    lightingProgram->UnuseShader();
    CHECKERROR;


    lightBoxProgram->UseShader();
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);

    CHECKERROR;
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    programId = lightBoxProgram->programId;
    glUniformMatrix4fv(glGetUniformLocation(programId, "projection"), 1, GL_FALSE, &WorldProj[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(programId, "view"), 1, GL_FALSE, &WorldView[0][0]);
    glm::vec3 white(1.0, 1.0, 1.0);
    for (unsigned int i = 0; i < lightPositions.size()-1; i++)
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPositions[i]);
        //model = glm::scale(model, glm::vec3(0.125f ));
        model = glm::scale(model, glm::vec3( lightRadius[i]));
        glUniformMatrix4fv(glGetUniformLocation(programId, "model"), 1, GL_FALSE, &model[0][0]);
        glUniform3fv(glGetUniformLocation(programId, "lightColor"), 1, &lightColors[i][0]);
        glUniform3fv(glGetUniformLocation(programId, "lightPosition"), 1, &lightPositions[i][0]);
        glUniform1f(glGetUniformLocation(programId, "lightRadius"), lightRadius[i]);
        glUniform3fv(glGetUniformLocation(programId, "viewPos"), 1, &(eye[0]));

        //renderCube();
        if(localLights)
            light->Draw(lightBoxProgram, Identity);
    }
    //Global Light
    lightPositions[NR_LIGHTS].x = lightX;
    lightPositions[NR_LIGHTS].y = lightY;
    lightPositions[NR_LIGHTS].z = lightZ;
    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPositions[NR_LIGHTS]);
    model = glm::scale(model, glm::vec3(0.125f ));
    //model = glm::scale(model, glm::vec3(1.f));
    glUniformMatrix4fv(glGetUniformLocation(programId, "model"), 1, GL_FALSE, &model[0][0]);
    glUniform3fv(glGetUniformLocation(programId, "lightColor"), 1, &lightColors[NR_LIGHTS][0]);
    glUniform3fv(glGetUniformLocation(programId, "lightPosition"), 1, &lightPositions[NR_LIGHTS][0]);
    glUniform1f(glGetUniformLocation(programId, "lightRadius"), lightRadius[NR_LIGHTS]);
    glUniform3fv(glGetUniformLocation(programId, "viewPos"), 1, &(eye[0]));

    light->Draw(lightBoxProgram, Identity);
    //renderCube();

    CHECKERROR; 
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //
    //// Turn off the shader
    lightBoxProgram->UnuseShader();

    ////////////////////////////////////////////////////////////////////////////////
    // End of Lighting pass
    ////////////////////////////////////////////////////////////////////////////////
}

