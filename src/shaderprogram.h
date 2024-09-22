#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <openglcontext.h>
#include <la.h>
#include <glm/glm.hpp>

#include "drawable.h"


class ShaderProgram
{
public:
    GLuint vertShader; // A handle for the vertex shader stored in this shader program
    GLuint fragShader; // A handle for the fragment shader stored in this shader program
    GLuint prog;       // A handle for the linked shader program stored in this class

    int attrPos; // A handle for the "in" vec4 representing vertex position in the vertex shader
    int attrNor; // A handle for the "in" vec4 representing vertex normal in the vertex shader
    int attrCol; // A handle for the "in" vec4 representing vertex color in the vertex shader
    int attrUV; // A handle for the "in" vec2 representing the UV coordinates in the vertex shader
    int attrJointIds;
    int attrJointFactors;

    int unifModel; // A handle for the "uniform" mat4 representing model matrix in the vertex shader
    int unifModelInvTr; // A handle for the "uniform" mat4 representing inverse transpose of the model matrix in the vertex shader
    int unifViewProj; // A handle for the "uniform" mat4 representing combined projection and view matrices in the vertex shader
    int unifView; // A handle for the "uniform" mat4 representing the view matrix in the vertex shader
    int unifProj; // A handle for the "uniform" mat4 representing the projection matrix in the vertex shader
    int unifCamPos; // A handle for the "uniform" vec4 representing color of geometry in the vertex shader

    int unifTexture;

    int unifSampler2D; // A handle to the "uniform" sampler2D that will be used to read the texture containing the scene render
    int unifTime; // A handle for the "uniform" float representing time in the shader

    int unifDimensions; // A handle to the "uniform" ivec2 that stores the width and height of the texture being rendered

    int unifBindMatrix;
    int unifCurrentJointMatrix;
public:
    ShaderProgram(OpenGLContext* context);
    // Sets up the requisite GL data and shaders from the given .glsl files
    void create(const char *vertfile, const char *fragfile);
    // Tells our OpenGL context to use this shader to draw things
    void useMe();
    // Pass the given model matrix to this shader on the GPU
    void setModelMatrix(const glm::mat4 &model);
    // Pass the given Projection * View matrix to this shader on the GPU
    void setViewProjMatrix(const glm::mat4 &v, const glm::mat4 &p);
    // Pass the given color to this shader on the GPU
    void setCamPos(glm::vec3 pos);

    void setBindMatrix(glm::mat4 matrix[100]);
    void setCurrentJointMatrix(glm::mat4 matrix[100]);
    // Draw the given object to our screen using this ShaderProgram's shaders
    void draw(Drawable &d);
    // Utility function used in create()
    char* textFileRead(const char*);
    // Utility function that prints any shader compilation errors to the console
    void printShaderInfoLog(int shader);
    // Utility function that prints any shader linking errors to the console
    void printLinkInfoLog(int prog);

    void setDimensions(glm::ivec2 dims);

    QString qTextFileRead(const char*);

    void setTexture(const int slot);



private:
    OpenGLContext* context;   // Since Qt's OpenGL support is done through classes like QOpenGLFunctions_3_2_Core,
        // we need to pass our OpenGL context to the Drawable in order to call GL functions
        // from within this class.
};


#endif // SHADERPROGRAM_H
