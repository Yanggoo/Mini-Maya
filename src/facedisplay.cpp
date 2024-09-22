#include "facedisplay.h"

FaceDisplay::FaceDisplay(OpenGLContext* context)
    : Drawable(context), representedFace(nullptr)
{}

FaceDisplay::~FaceDisplay()
{}

void FaceDisplay::create() {
    std::vector<glm::vec4> pos;
    std::vector<glm::vec4> nor;
    std::vector<glm::vec4> col;
    std::vector<GLuint> idx;
    int numSides = 0;

    if (representedFace) {
        HalfEdge *start = representedFace->edge;

        do {
            pos.push_back(glm::vec4(start->vertex->pos(), 1));
            col.push_back(glm::vec4(glm::vec3(1) - representedFace->color, 1));
            nor.push_back(glm::vec4(0, 0, 1, 1));

            start = start->next;
            numSides++;

        } while (start != representedFace->edge);

        for (int i = 0; i < numSides; i++) {
            idx.push_back(i);
            idx.push_back((i + 1) % numSides);
        }
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

GLenum FaceDisplay::drawMode() {
    return GL_LINES;
}

void FaceDisplay::updateFace(Face* face) {
    representedFace = face;
}
