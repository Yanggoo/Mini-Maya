#include "vertexpos.h"
#include <limits>
float MAX_FLOAT = std::numeric_limits<float>::max();
int VertexPos::lastVertexPos =0;

VertexPos::VertexPos(): QListWidgetItem(){
    name = "Vertex ";
    id = lastVertexPos++;
    name.append(QString::number(id));
    this->pos = glm::vec3(0.0f);
    joints[0].factor = 0;
    joints[1].factor = 0;
    joints[0].joint = nullptr;
    joints[1].joint = nullptr;
    this->setText(name);
}
VertexPos::VertexPos(glm::vec3 pos): QListWidgetItem(){
    name = "Vertex ";
    id = lastVertexPos++;
    name.append(QString::number(id));
    this->pos = pos;
    joints[0].factor = 0;
    joints[1].factor = 0;
    joints[0].joint = nullptr;
    joints[1].joint = nullptr;
    this->setText(name);
}

bool VertexPos::addJointFactor(Joint *joint, float factor){
    if(factor>joints[0].factor){
        joints[1].factor = joints[0].factor;
        joints[1].joint = joints[0].joint;
        joints[0].factor = factor;
        joints[0].joint = joint;
        return true;
    }else if(factor>joints[1].factor && joint!= joints[0].joint){
        joints[1].factor = factor;
        joints[1].joint = joint;
        return true;
    }
    return false;
}
