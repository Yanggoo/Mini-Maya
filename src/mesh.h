#ifndef MESH_H
#define MESH_H


#include "drawable.h"
#include "smartpointerhelp.h"
#include "halfedge.h"
#include "texture.h"
#include "joint.h"


class Mesh : public Drawable
{
public:
    Mesh(OpenGLContext* context);
    std::vector<uPtr<Face>> faces;
    std::vector<uPtr<HalfEdge>> halfEdges;
    std::vector<uPtr<Vertex>> vertices;
    std::vector<uPtr<VertexPos>> verticesPos;
    std::vector<uPtr<Joint>> joints;
    void create() override;
    void Clear();
    GLenum drawMode() override;
    void ReadFile(std::string openFilePath);
    void ReadTexture(std::string openFilePath);
    uPtr<Texture> m_texture;
    void Subdivision();
    void ReIndex();
    void ReadSkeletonJson(std::string openFilePath);
    void PocessJsonRecursively(const QJsonObject& data, Joint* currentJoint);
    void CalculateJointInfluenceDistanceBased();
    void CalculateJointInfluenceHeatDiffusionBased();
    void Update();
    Vertex* FindClosetVertex(glm::vec3 pos);
    // void HeatDiffuseFrom(Joint* joint,Vertex* startVertex, float startFactor);
};

#endif // MESH_H
