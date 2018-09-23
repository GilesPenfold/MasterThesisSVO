#ifndef BBOXWRAPPER_H
#define BBOXWRAPPER_H
#include <vector>
#include <ngl/BBox.h>
#include <tri.h>

class BBoxWrapper
{
public:
    BBoxWrapper(){m_Tris = std::vector<Tri*>();}

    ///
    /// \brief clamp
    /// \param _T
    /// \param _A
    /// \param _B
    /// \return
    /// Clamps the value _T between values _A and _B
    float clamp(float _T, float _A, float _B)
    { // If outside of _A and _B, then return clamped values, otherwise return original value _T
        return _T > _B ? _B : (_T < _A ? _A : _T);
    }

    int m_NumTris(){return m_Tris.size();}

    ngl::BBox* m_BBoxGrid;
    std::vector<Tri*> m_Tris;

};

#endif // BBOXWRAPPER_H
