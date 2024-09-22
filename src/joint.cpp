#include"joint.h"
#include <glm/gtx/quaternion.hpp>
int Joint::lastJoint=0;
Joint::Joint():name("Joint "),
    parent(nullptr),
    children{},
    pos(0,0,0),
    rotation(glm::quat()),
    bindMatrix(glm::mat4())
{
    id = lastJoint++;
    name.append(QString::number(id));
    this->setText(name);
}
glm::mat4 Joint::getLocalTransformation(){
    glm::mat4 transformMtrix = glm::translate(glm::mat4(1.0),pos);
    glm::mat4 rotationMtrix = glm::toMat4(rotation);
    return transformMtrix*rotationMtrix;
}
glm::mat4 Joint::getOverallTransformation(){
    if(parent==nullptr)
        return getLocalTransformation();
    glm::mat4 overallMatrix = parent->getOverallTransformation()*getLocalTransformation();
    return overallMatrix;
}

void Joint::rotateByMyself(float angle, glm::vec3 axis){
    float radians = glm::radians(angle);
    glm::vec4 axisReal = glm::toMat4(rotation)*glm::vec4(axis,0);
    glm::quat rotationQuat = glm::angleAxis(radians, glm::vec3(axisReal.x,axisReal.y,axisReal.z));
    rotation = rotationQuat*rotation;
}

void Joint::InitializeBindMatrix(){
    auto tmp = getOverallTransformation();
    bindMatrix = glm::inverse(tmp);
}
