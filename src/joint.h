#ifndef JOINT_H
#define JOINT_H

#pragma once
#include <QListWidget>
#include <la.h>
#include <smartpointerhelp.h>

class Joint: public QListWidgetItem{
public:
    static int lastJoint;
    QString name;
    int id;
    Joint* parent;
    std::vector<Joint*> children;
    glm::vec3 pos;
    glm::quat rotation;
    glm::mat4 bindMatrix;
    Joint();
    glm::mat4 getLocalTransformation();
    glm::mat4 getOverallTransformation();
    void InitializeBindMatrix();

    void rotateByMyself(float angle, glm::vec3 axis);
};
#endif // JOINT_H
