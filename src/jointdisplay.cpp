#include "jointdisplay.h"

void  JointDisplay::create(){
    const static glm::vec3 xy_ring[16] = {
        glm::vec3(0.5f, 0.0f, 0.0f),
        glm::vec3(0.46194f, 0.19134f, 0.0f),
        glm::vec3(0.35355f, 0.35355f, 0.0f),
        glm::vec3(0.19134f, 0.46194f, 0.0f),
        glm::vec3(0.0f, 0.5f, 0.0f),
        glm::vec3(-0.19134f, 0.46194f, 0.0f),
        glm::vec3(-0.35355f, 0.35355f, 0.0f),
        glm::vec3(-0.46194f, 0.19134f, 0.0f),
        glm::vec3(-0.5f, 0.0f, 0.0f),
        glm::vec3(-0.46194f, -0.19134f, 0.0f),
        glm::vec3(-0.35355f, -0.35355f, 0.0f),
        glm::vec3(-0.19134f, -0.46194f, 0.0f),
        glm::vec3(0.0f, -0.5f, 0.0f),
        glm::vec3(0.19134f, -0.46194f, 0.0f),
        glm::vec3(0.35355f, -0.35355f, 0.0f),
        glm::vec3(0.46194f, -0.19134f, 0.0f)
    };
    const static glm::vec3 xz_ring[16] = {
        glm::vec3(0.5f, 0.0f, 0.0f),
        glm::vec3(0.46194f, 0.0f, 0.19134f),
        glm::vec3(0.35355f, 0.0f, 0.35355f),
        glm::vec3(0.19134f, 0.0f, 0.46194f),
        glm::vec3(0.0f, 0.0f, 0.5f),
        glm::vec3(-0.19134f, 0.0f, 0.46194f),
        glm::vec3(-0.35355f, 0.0f, 0.35355f),
        glm::vec3(-0.46194f, 0.0f, 0.19134f),
        glm::vec3(-0.5f, 0.0f, 0.0f),
        glm::vec3(-0.46194f, 0.0f, -0.19134f),
        glm::vec3(-0.35355f, 0.0f, -0.35355f),
        glm::vec3(-0.19134f, 0.0f, -0.46194f),
        glm::vec3(0.0f, 0.0f, -0.5f),
        glm::vec3(0.19134f, 0.0f, -0.46194f),
        glm::vec3(0.35355f, 0.0f, -0.35355f),
        glm::vec3(0.46194f, 0.0f, -0.19134f)
    };
    const static glm::vec3 yz_ring[16] = {
        glm::vec3(0.0f, 0.5f, 0.0f),
        glm::vec3(0.0f, 0.46194f, 0.19134f),
        glm::vec3(0.0f, 0.35355f, 0.35355f),
        glm::vec3(0.0f, 0.19134f, 0.46194f),
        glm::vec3(0.0f, 0.0f, 0.5f),
        glm::vec3(0.0f, -0.19134f, 0.46194f),
        glm::vec3(0.0f, -0.35355f, 0.35355f),
        glm::vec3(0.0f, -0.46194f, 0.19134f),
        glm::vec3(0.0f, -0.5f, 0.0f),
        glm::vec3(0.0f, -0.46194f, -0.19134f),
        glm::vec3(0.0f, -0.35355f, -0.35355f),
        glm::vec3(0.0f, -0.19134f, -0.46194f),
        glm::vec3(0.0f, 0.0f, -0.5f),
        glm::vec3(0.0f, 0.19134f, -0.46194f),
        glm::vec3(0.0f, 0.35355f, -0.35355f),
        glm::vec3(0.0f, 0.46194f, -0.19134f)
    };

    std::vector<glm::vec4> pos;
    std::vector<glm::vec4> nor;
    std::vector<glm::vec4> col;
    std::vector<GLuint> idx;

    glm::vec4 colorX ={1,0,0,1};
    glm::vec4 colorY ={0,1,0,1};
    glm::vec4 colorZ ={0,0,1,1};
    glm::vec4 colorLineParent = {1,0,0,1};
    glm::vec4 colorLineChild = {1,1,0,1};
    if(representJoint == selectedJoint){
        colorX ={0,1,1,1};
        colorY ={1,0,1,1};
        colorZ ={1,1,0,1};
    }

    if(representJoint){
        glm::mat4 transformMatrix = representJoint->getOverallTransformation();
        for(size_t i=0;i<16;i++){
            pos.emplace_back(transformMatrix*glm::vec4(xy_ring[i],1.0f));
            nor.emplace_back(0,0,1,1);
            col.emplace_back(colorZ);
        }
        for(size_t i=0;i<16;i++){
            pos.emplace_back(transformMatrix*glm::vec4(xz_ring[i],1.0f));
            nor.emplace_back(0,0,1,1);
            col.emplace_back(colorY);
        }
        for(size_t i=0;i<16;i++){
            pos.emplace_back(transformMatrix*glm::vec4(yz_ring[i],1.0f));
            nor.emplace_back(0,0,1,1);
            col.emplace_back(colorX);
        }
        for(size_t i=0;i<3;i++){
            for(size_t j=0;j<16;j++){
                idx.emplace_back(i*16+j);
                idx.emplace_back(i*16+(j+1)%16);
            }
        }

        if(representJoint->parent){
            Joint* parent = representJoint->parent;
            glm::vec4 parentPos = parent->getOverallTransformation()*glm::vec4(0.0f,0.0f,0.0f,1.0f);
            glm::vec4 childPos = transformMatrix*glm::vec4(0.0f,0.0f,0.0f,1.0f);
            pos.emplace_back(parentPos);
            nor.emplace_back(0,0,1,1);
            col.emplace_back(colorLineParent);
            idx.emplace_back(pos.size()-1);
            pos.emplace_back(childPos);
            nor.emplace_back(0,0,1,1);
            col.emplace_back(colorLineChild);
            idx.emplace_back(pos.size()-1);
        }

        count = idx.size();

        generateIdx();
        bindIdx();
        mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

        generateNor();
        bindNor();
        mp_context->glBufferData(GL_ARRAY_BUFFER, nor.size() * sizeof(glm::vec4), nor.data(), GL_STATIC_DRAW);

        generatePos();
        bindPos();
        mp_context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

        generateCol();
        bindCol();
        mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
    }




}
void JointDisplay::updateJoint(Joint* joint){
    selectedJoint = joint;
}

void JointDisplay::setRepresentJoint(Joint* joint){
    representJoint = joint;
}
GLenum JointDisplay::drawMode(){
return GL_LINES;
}
JointDisplay::JointDisplay(OpenGLContext* context): Drawable(context), representJoint(nullptr), selectedJoint(nullptr){

}
JointDisplay::~JointDisplay(){

}
