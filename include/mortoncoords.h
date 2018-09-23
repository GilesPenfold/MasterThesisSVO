#ifndef MORTONCOORDS_H
#define MORTONCOORDS_H

struct MortonCoords
{

public:

    ///
    /// \brief MortonCoords
    /// Stores three integer values to reprsent grid spaces
    MortonCoords(){m_x=0;m_y=0;m_z=0;}
    MortonCoords(unsigned int _x, unsigned int _y, unsigned int _z){m_x=_x;m_y=_y;m_z=_z;}

    unsigned int m_x;
    unsigned int m_y;
    unsigned int m_z;


};

#endif // MORTONCOORDS_H
