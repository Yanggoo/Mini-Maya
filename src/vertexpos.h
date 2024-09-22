#ifndef VERTEXPOS_H
#define VERTEXPOS_H

#include<la.h>
#include<joint.h>
#include<QListWidgetItem>

struct JointANdFactor{
    Joint* joint;
    float factor;
};

class VertexPos :public QListWidgetItem
{
public:
    static int lastVertexPos;
    glm::vec3 pos;
    int id;             // unique integer to identify the Vertex
    QString name;
    JointANdFactor joints[2];
    VertexPos();
    VertexPos(glm::vec3 pos);
    bool addJointFactor(Joint *joint, float factor);
};

#endif // VERTEXPOS_H
