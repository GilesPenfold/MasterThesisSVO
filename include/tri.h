#ifndef TRI_H
#define TRI_H
#include <ngl/Vec3.h>
#include <ngl/BBox.h>

struct Tri
{
public:
    Tri();
    ///
    /// \brief Tri
    /// \param _a
    /// \param _b
    /// \param _c
    /// Creates a Tri object to represent a triangular face on a mesh and to hold a bounding box for that triangle
    Tri(ngl::Vec3 _a, ngl::Vec3 _b, ngl::Vec3 _c)
    {
        m_a = _a;
        m_b = _b;
        m_c = _c;
    }

    ngl::BBox* CalculateBBox()
    {
        ngl::Real minx = std::min(m_a.m_x, std::min(m_b.m_x, m_c.m_x));
        ngl::Real miny = std::min(m_a.m_y, std::min(m_b.m_y, m_c.m_y));
        ngl::Real minz = std::min(m_a.m_z, std::min(m_b.m_z, m_c.m_z));

        ngl::Real maxx = std::max(m_a.m_x, std::max(m_b.m_x, m_c.m_x));
        ngl::Real maxy = std::max(m_a.m_y, std::max(m_b.m_y, m_c.m_y));
        ngl::Real maxz = std::max(m_a.m_z, std::max(m_b.m_z, m_c.m_z));

        return m_BBox = new ngl::BBox(minx,maxx,miny,maxy,minz,maxz);
    }

    ngl::Vec3 m_a; // Vert 1
    ngl::Vec3 m_b; // Vert 2
    ngl::Vec3 m_c; // Vert 3

    ngl::BBox* m_BBox; // Bounding box



};

#endif // TRI_H
