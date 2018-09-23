#ifndef CUBE_H
#define CUBE_H

#include <ngl/VAOPrimitives.h>
#include <ngl/AbstractVAO.h>
#include "ngl/Util.h"
#include <iostream>
#include "ngl/VAOFactory.h"
#include "ngl/SimpleVAO.h"



class Cube
{
public:

    Cube(){}
    ///
    /// \brief Cube
    /// \param _unitLength
    /// Creates a Cube VAO to render with
    Cube(float _unitLength)
    {
        vao = ngl::VAOFactory::createVAO("simpleVAO", GL_LINES);
        vao->bind();

        std::vector<ngl::vertData> v;

        v.resize(8);

        float a = 1.0f * _unitLength;

        v[0].x = v[1].x = v[2].x = v[3].x = -a;
        v[4].x = v[5].x = v[6].x = v[7].x = a;
        v[0].y = v[1].y = v[4].y = v[5].y = -a;
        v[2].y = v[3].y = v[6].y = v[7].y = a;
        v[1].z = v[2].z = v[5].z = v[6].z = -a;
        v[0].z = v[3].z = v[4].z = v[7].z = a;

        vao->setData((ngl::SimpleVAO::VertexData(v.size()*sizeof(ngl::vertData),v[0].u)));

        vao->setVertexAttributePointer(0,3,GL_FLOAT,sizeof(ngl::vertData),5);
        vao->setVertexAttributePointer(1,2,GL_FLOAT,sizeof(ngl::vertData),0);
        vao->setVertexAttributePointer(2,3,GL_FLOAT,sizeof(ngl::vertData),2);
        vao->setNumIndices(v.size());
        vao->unbind();

    }

    ///
    /// \brief Draw
    /// Draws the cube
    void inline Draw()
    {
        vao->bind();
        vao->setMode(GL_LINES);
        vao->draw();
        vao->unbind();
    }

private:
     ngl::AbstractVAO *vao;

};

#endif // CUBE_H
