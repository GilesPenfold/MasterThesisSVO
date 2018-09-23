#include <Voxelizer.h>
#include <string>

// This implementation is based on this paper: http://graphics.cs.kuleuven.be/publications/BLD14OCCSVO/
// Built in NGL using their sample code for reference: https://github.com/Forceflow/ooc_svo_builder
// We are modifying it to be an in-core method so we can test compression methods within it

// Order Of Matters
// 0 - Bounding box our mesh, locate max bounds, perform memory calculations
// 1 - Subgrid to power of 2 in relation to max bounds
// 2 - Locate our Morton start and end points
// 3 - Iterate over each of our triangles
// 3.5 - Construct bounding box for our triangle
// 4 - Iterate over our subgrid
// 5 - Create and perform tests
// 6 - Repeat 3.5 through 5 until complete
// 7 - Contruct the octree
// 8 - Save to file, compress if necessary

// Future Ideas
// Efficient storage of voxel and colour data (encode both into morton?)
// Colour data is 0-255, so 8 bits needed, which is 24 bits, so 32 bit uint can be used
// Which leaves us 8 bits for custom flags (normal data)
// Out Of Core implementation



void Voxelizer::Run(std::string _outname, COMPRESSIONTYPE _compress, int _gridSize)
{

    // DECLARE VARIABLES
    size_t gridSize = _gridSize;
    size_t numPartitions = 1;
    std::vector<BBoxWrapper*> grid;
    float unitlength = 1.0f;

    std::vector<Tri*> tris;

    // Grab our correct filename from the file path
    std::string temp = _outname;
    std::string found;
    std::string delim = "/";
    size_t pos;
    while((pos = temp.find(delim))!=std::string::npos)
    {
       found = temp.substr(0,pos);
       temp.erase(0,pos+delim.length());
    }

    m_FileName = temp.substr(0,temp.length()-4);


    std::string objName = _outname;

    // LOAD MESH - doesn't work because QTCreator seems to have an odd interaction with NGL
    m_Mesh.reset(new ngl::Obj(objName));
    m_Mesh->createVAO();
    m_Mesh->calcDimensions();

    std::cout <<"Non-Cube BBox Min: "<<m_Mesh->getBBox().minX() << " " << m_Mesh->getBBox().minY() << " " <<  m_Mesh->getBBox().minZ() << '\n';
    std::cout <<"Non-Cube BBox Max: "<<m_Mesh->getBBox().maxX() << " " << m_Mesh->getBBox().maxY() << " " <<  m_Mesh->getBBox().maxZ() << '\n';

    // Cubify our bounding box
    m_BoundBox = Cubify(m_Mesh.get());


    std::vector<ngl::Face> faces = m_Mesh->getFaceList();
    std::vector<ngl::Vec3> verts = m_Mesh->getVertexList();


    ngl::Vec3 bboxmin = ngl::Vec3(m_BoundBox->minX(),m_BoundBox->minY(),m_BoundBox->minZ());


    // Move everything back to the centre of the scene so we don't have errors voxelizing
    for(size_t i = 0; i < verts.size(); ++i)
    {
        verts[i] -= bboxmin;
    }


    for(size_t i = 0; i < faces.size(); ++i)
    {
        tris.push_back(new Tri(verts[faces[i].m_vert[0]],
                               verts[faces[i].m_vert[1]],
                               verts[faces[i].m_vert[2]]));
    }

    // Debug check
    std::cout << "Face list size: " << m_Mesh->getNumFaces() << '\n';
    std::cout << "Triangle list size: " << tris.size() << '\n';

    // GET GRID SIZE - DONE
    numPartitions = MemoryCalculations(gridSize);

    m_Mesh.get()->calcBoundingSphere();

    //unitlength = ((m_Mesh.get()->getSphereRadius()*2)/(float)gridSize);//*1.1f;
    unitlength = (float)(m_BoundBox->maxX() - m_BoundBox->minX()) / (float)gridSize;

    std::cout <<"Cube BBox Min: "<<m_BoundBox->minX() << " " << m_BoundBox->minY() << " " <<  m_BoundBox->minZ() << '\n';
    std::cout <<"Cube BBox Max: "<<m_BoundBox->maxX() << " " << m_BoundBox->maxY() << " " <<  m_BoundBox->maxZ() << '\n';



    // PARTITION - DONE
    SubGrid(tris, grid, numPartitions, gridSize, unitlength);

    // VOXELIZE - In Progress: Testing / Debugging
    Voxelize(numPartitions, gridSize, unitlength);

    std::cout << "Filled Voxels: " << m_FullVoxels << '\n';

    //m_Octree = Octree(m_FileName, gridSize, unitlength);
    m_Octree = Octree(m_FileName, gridSize, unitlength);
    m_Octree.Begin(_compress);

    // OCTREE - DONE
    RunOctree(numPartitions, gridSize);

    std::cout << "Octree Created Successfully\n" ;
    std::cout << "Saving to file...\n" ;
    // SAVE TO FILE & COMPRESSION
    SaveToFile();

    std::cout << "DONE!\n" ;
    m_Done = true;

    //m_Octree = Octree(m_FileName, gridSize, unitlength);





}

void Voxelizer::LoadOctree(std::string _fileName)
{

    if(_fileName.substr(_fileName.length()-8,_fileName.length()).compare("compress") == 0)
        m_FileName = _fileName.substr(0, _fileName.length()-15);
    else if(_fileName.substr(_fileName.length()-6,_fileName.length()).compare("octree") == 0)
        m_FileName = _fileName.substr(0, _fileName.length()-7);
    else
    {
        std::cout << "Error reading in file. Filetype not recognised.\n";
        return;
    }

    std::cout << "Reading in octree file...\n";

    m_Octree.m_FileName = m_FileName;

    m_Octree.ReadOctree();

    std::cout << "DONE! (If I got errors, send your complaint letter to Microsoft for their carriage returns)\n" ;



}
size_t Voxelizer::MemoryCalculations(size_t _gridSize)
{
    uint64_t size = (_gridSize * _gridSize * _gridSize * sizeof(char)) / 1024 / 1024;
    std::cout << size << '\n';

    size_t numPartitions = 1;
    while(size > m_MemoryLimit)
    {
        size /= 8;
        numPartitions *= 8;
    }

    std::cout << numPartitions << '\n';

    return numPartitions;

}

void Voxelizer::SubGrid(std::vector<Tri*> &_tris, std::vector<BBoxWrapper*> &_grid, size_t _numPartitions, size_t _gridSize, float _unitLength)
{

    _grid.resize(_numPartitions);
    for(size_t i = 0; i < _numPartitions; ++i)
    {
        _grid[i] = new BBoxWrapper();
    }

    uint_fast64_t morton_partition = (_gridSize*_gridSize*_gridSize) / _numPartitions;
    Morton m = Morton(morton_partition);

    MortonCoords mcMin, mcMax;

    // Iterate through our partitions, create bounding boxes in world space to fit to our grid
    // Make sure they are in Morton order by decoding our first and last morton partition
    // In that partition
    for(size_t i = 0; i < _numPartitions; ++i)
    {
        mcMin = m.decodeMortonMagic(morton_partition*i);
        mcMax = m.decodeMortonMagic(morton_partition*(i+1)-1);

        ngl::Real minx = (_unitLength * mcMin.m_x);
        ngl::Real miny = (_unitLength * mcMin.m_y);
        ngl::Real minz = (_unitLength * mcMin.m_z);

        ngl::Real maxx = (_unitLength * (mcMax.m_x+1));
        ngl::Real maxy = (_unitLength * (mcMax.m_y+1));
        ngl::Real maxz = (_unitLength * (mcMax.m_z+1));



        _grid[i]->m_BBoxGrid = new ngl::BBox(minx,maxx,miny,maxy,minz,maxz);

    }

    // THE NEXT BIT TO DO IS THE CREATION OF THE PARTITIONS WITH THE TRIS
    // BASICALLY SPLIT THE TRIS INTO THEIR PARTITIONS

    int triLength = _tris.size();
    ngl::BBox* A;
    ngl::BBox* B;

    for(size_t i = 0; i < triLength; ++i)
    {
        // Calculate Triangle Bounding Box
         A = _tris[i]->CalculateBBox();

        // Perform intersection with t-bbox and gridbbox
        for(size_t j = 0; j < _numPartitions; ++j)
        {
            B = _grid[j]->m_BBoxGrid;
            if(A->maxX() < B->minX() || A->maxY() < B->minY() || A->maxZ() < B->minZ() || A->minX() > B->maxX() || A->minY() > B->maxY() || A->minZ() > B->maxZ())
            { // Outside the bounding box of our wrapper, discard
                continue;
            }
            else
            { // Inside the bounding box of our wrapper, add to our wrapper array
                _grid[j]->m_Tris.push_back(_tris[i]);
            }

        }
    }

    for(int j = 0; j < _numPartitions; j++)
        std::cout << "Number of tris in " << j << ": " << _grid[j]->m_NumTris() << '\n';


    m_Grid = _grid;
}

// Based on the implementation by J.Baert at: https://github.com/Forceflow/ooc_svo_builder
// And adapted using the original paper: http://research.michael-schwarz.com/publ/2010/vox/
void Voxelizer::Voxelize(size_t _numPartitions, size_t _gridSize, float _unitLength)
{
    // Setup variables
    uint_fast64_t morton_partition = (_gridSize*_gridSize*_gridSize) / _numPartitions;
    Morton m = Morton(morton_partition);
    float M = 1.0f/_unitLength;
    ngl::Vec3 D = ngl::Vec3(_unitLength,_unitLength,_unitLength);
    MortonCoords mcMin, mcMax;

    ngl::Vec3 Normal, EdgeA, EdgeB, EdgeC, Critical;

    m_Voxels = new bool[(size_t)morton_partition];

    std::cout << "Starting Partition Iteration" << '\n';

    // When going over the grid, any non-empty parts are put into a morton array and this is then carried on
    // This is so that we don't iterate over lots of empty voxels
    for(size_t i = 0; i < _numPartitions; ++i)
    {
        mcMin = m.decodeMortonMagic(morton_partition*i);
        mcMax = m.decodeMortonMagic(morton_partition*(i+1)-1);

        std::cout << "Beginning triangle list iteration " << '\n';

        if(m_Grid[i]->m_NumTris() == 0)
            continue;
        else
            std::cout << "Triangles to process this partition: "<<m_Grid[i]->m_NumTris() << '\n';

        // Here we setup for Schwarz's Triangle-Box Overlap tests
        for(size_t j = 0; j < m_Grid[i]->m_NumTris(); ++j)
        {
            // Grab tri
            Tri* T = m_Grid[i]->m_Tris[j];


            // Grab the bounding box of the tri in world coords
            ngl::BBox* B = T->m_BBox;

            int LX = B->minX()*M;
            int LY = B->minY()*M;
            int LZ = B->minZ()*M;
            int HX = B->maxX()*M;
            int HY = B->maxY()*M;
            int HZ = B->maxZ()*M;


            // Clamp it to the grid coordinates
            unsigned int minx = m_Grid[i]->clamp((int)(LX), mcMin.m_x, mcMax.m_x );
            unsigned int miny = m_Grid[i]->clamp((int)(LY), mcMin.m_y, mcMax.m_y );
            unsigned int minz = m_Grid[i]->clamp((int)(LZ), mcMin.m_z, mcMax.m_z );
            unsigned int maxx = m_Grid[i]->clamp((int)(HX), mcMin.m_x, mcMax.m_x );
            unsigned int maxy = m_Grid[i]->clamp((int)(HY), mcMin.m_y, mcMax.m_y );
            unsigned int maxz = m_Grid[i]->clamp((int)(HZ), mcMin.m_z, mcMax.m_z );


            // Create triangle properties
            EdgeA = T->m_b - T->m_a;
            EdgeB = T->m_c - T->m_b;
            EdgeC = T->m_a - T->m_c;
            Normal = EdgeA.cross(EdgeB);
            Normal.normalize();


            // Create planar testing properties
            Critical = ngl::Vec3(0.0f,0.0f,0.0f);
            Normal.m_x > 0 ? Critical.m_x = _unitLength : Critical.m_x = 0.0f;
            Normal.m_y > 0 ? Critical.m_y = _unitLength : Critical.m_y = 0.0f;
            Normal.m_z > 0 ? Critical.m_z = _unitLength : Critical.m_z = 0.0f;

            float DotA = Normal.dot((Critical-T->m_a));
            float DotB = Normal.dot(((D-Critical)-T->m_a));


            // Next we create our projection tests
            // First we start with the XY plane

            // Attain new edge normals for our 2D triangular plane
            ngl::Vec2 XY_EdgeA = ngl::Vec2(-1.0f*EdgeA.m_y, EdgeA.m_x);
            ngl::Vec2 XY_EdgeB = ngl::Vec2(-1.0f*EdgeB.m_y, EdgeB.m_x);
            ngl::Vec2 XY_EdgeC = ngl::Vec2(-1.0f*EdgeC.m_y, EdgeC.m_x);

            // If our normal along the Z axis is less than 0, we invert our edge normals
            if(Normal.m_z < 0.0f)
            {
                XY_EdgeA *= -1.0f;
                XY_EdgeB *= -1.0f;
                XY_EdgeC *= -1.0f;
            }

            // Dot our normals and apply our unitlength
            float XY_DotA = (XY_EdgeA.dot(ngl::Vec2(T->m_a.m_x,T->m_a.m_y)) * -1.0f) + std::max(0.0f, _unitLength * XY_EdgeA.m_x) + std::max(0.0f, _unitLength * XY_EdgeA.m_y);
            float XY_DotB = (XY_EdgeB.dot(ngl::Vec2(T->m_b.m_x,T->m_b.m_y)) * -1.0f) + std::max(0.0f, _unitLength * XY_EdgeB.m_x) + std::max(0.0f, _unitLength * XY_EdgeB.m_y);
            float XY_DotC = (XY_EdgeC.dot(ngl::Vec2(T->m_c.m_x,T->m_c.m_y)) * -1.0f) + std::max(0.0f, _unitLength * XY_EdgeC.m_x) + std::max(0.0f, _unitLength * XY_EdgeC.m_y);

            // Repeat for the YZ plane

            // Attain new edge normals for our 2D triangular plane
            ngl::Vec2 YZ_EdgeA = ngl::Vec2(-1.0f*EdgeA.m_z, EdgeA.m_y);
            ngl::Vec2 YZ_EdgeB = ngl::Vec2(-1.0f*EdgeB.m_z, EdgeB.m_y);
            ngl::Vec2 YZ_EdgeC = ngl::Vec2(-1.0f*EdgeC.m_z, EdgeC.m_y);

            // If our normal along the Z axis is less than 0, we invert our edge normals
            if(Normal.m_x < 0.0f)
            {
                YZ_EdgeA *= -1.0f;
                YZ_EdgeB *= -1.0f;
                YZ_EdgeC *= -1.0f;
            }

            // Dot our normals and apply our unitlength
            float YZ_DotA = (YZ_EdgeA.dot(ngl::Vec2(T->m_a.m_y,T->m_a.m_z)) * -1.0f) + std::max(0.0f, _unitLength * YZ_EdgeA.m_x) + std::max(0.0f, _unitLength * YZ_EdgeA.m_y);
            float YZ_DotB = (YZ_EdgeB.dot(ngl::Vec2(T->m_b.m_y,T->m_b.m_z)) * -1.0f) + std::max(0.0f, _unitLength * YZ_EdgeB.m_x) + std::max(0.0f, _unitLength * YZ_EdgeB.m_y);
            float YZ_DotC = (YZ_EdgeC.dot(ngl::Vec2(T->m_c.m_y,T->m_c.m_z)) * -1.0f) + std::max(0.0f, _unitLength * YZ_EdgeC.m_x) + std::max(0.0f, _unitLength * YZ_EdgeC.m_y);

            // And again for the ZX plane, forming our final triangular plane

            // Attain new edge normals for our 2D triangular plane
            ngl::Vec2 ZX_EdgeA = ngl::Vec2(-1.0f*EdgeA.m_x, EdgeA.m_z);
            ngl::Vec2 ZX_EdgeB = ngl::Vec2(-1.0f*EdgeB.m_x, EdgeB.m_z);
            ngl::Vec2 ZX_EdgeC = ngl::Vec2(-1.0f*EdgeC.m_x, EdgeC.m_z);

            // If our normal along the Z axis is less than 0, we invert our edge normals
            if(Normal.m_y < 0.0f)
            {
                ZX_EdgeA *= -1.0f;
                ZX_EdgeB *= -1.0f;
                ZX_EdgeC *= -1.0f;
            }

            // Dot our normals and apply our unitlength
            float ZX_DotA = (ZX_EdgeA.dot(ngl::Vec2(T->m_a.m_z,T->m_a.m_x)) * -1.0f) + std::max(0.0f, _unitLength * ZX_EdgeA.m_x) + std::max(0.0f, _unitLength * ZX_EdgeA.m_y);
            float ZX_DotB = (ZX_EdgeB.dot(ngl::Vec2(T->m_b.m_z,T->m_b.m_x)) * -1.0f) + std::max(0.0f, _unitLength * ZX_EdgeB.m_x) + std::max(0.0f, _unitLength * ZX_EdgeB.m_y);
            float ZX_DotC = (ZX_EdgeC.dot(ngl::Vec2(T->m_c.m_z,T->m_c.m_x)) * -1.0f) + std::max(0.0f, _unitLength * ZX_EdgeC.m_x) + std::max(0.0f, _unitLength * ZX_EdgeC.m_y);

            // Now we perform our grid tests for overlap with the triangular planes
            // Found in Section 4.1 of Schwarz's paper

            Morton Index;
            MortonCoords Coords;
            ngl::Vec3 Plane;
            ngl::Vec2 Projection;
            float NDP;

            //std::cout << "Beginning testing for triangle: " << j << '\n';

            for(size_t x = minx; x <= maxx; ++x)
            {
                for(size_t y = miny; y <= maxy; ++y)
                {
                    for(size_t z = minz; z <= maxz; ++z)
                    {
                        Coords = MortonCoords(x,y,z);
                        Index.encodeMortonMagic(Coords);
                        uint64_t I = Index.M();

                        // If our voxel is already filled, jump to next voxel
                        if(m_Voxels[I-(morton_partition*i)] == FILLEDVOXEL)
                            continue;

                        // Otherwise, perform our tests
                        // We start with plane through box test

                        Plane = ngl::Vec3(x*_unitLength,y*_unitLength,z*_unitLength);
                        NDP = Normal.dot(Plane);
                        if((NDP + DotA) * (NDP + DotB) > 0.0f)
                            continue;

                        // Then we do our planar projection tests
                        // Start with XY

                        Projection = ngl::Vec2(Plane.m_x,Plane.m_y);
                        if((XY_EdgeA.dot(Projection) + XY_DotA) < 0.0f ||
                           (XY_EdgeB.dot(Projection) + XY_DotB) < 0.0f ||
                           (XY_EdgeC.dot(Projection) + XY_DotC) < 0.0f)
                            continue;

                        // Next is YZ

                        Projection = ngl::Vec2(Plane.m_y,Plane.m_z);
                        if((YZ_EdgeA.dot(Projection) + YZ_DotA) < 0.0f ||
                           (YZ_EdgeB.dot(Projection) + YZ_DotB) < 0.0f ||
                           (YZ_EdgeC.dot(Projection) + YZ_DotC) < 0.0f)
                            continue;


                        // Finally ZX

                        Projection = ngl::Vec2(Plane.m_z,Plane.m_x);
                        if((ZX_EdgeA.dot(Projection) + ZX_DotA) < 0.0f ||
                           (ZX_EdgeB.dot(Projection) + ZX_DotB) < 0.0f ||
                           (ZX_EdgeC.dot(Projection) + ZX_DotC) < 0.0f)
                            continue;

                        // If all tests pass, we have a voxel
                        m_Voxels[I-(morton_partition*i)] = FILLEDVOXEL;

                        m_FullVoxels++;

                        //std::cout << "Voxel filled" << '\n';

                        //continue;

                    }
                }
            }

            //std::cout << "Finished testing for triangle: " << j << '\n';
        }
    }
}

void Voxelizer::RunOctree(size_t _numPartitions, size_t _gridSize)
{

    std::cout << "Beginning Octree Construction...\n";

    Morton mn;
    uint_fast64_t morton_partition = (_gridSize*_gridSize*_gridSize) / _numPartitions;
    Morton m = Morton(morton_partition);
    for(size_t i = 0; i < _numPartitions; ++i)
    {
        for(size_t j = 0; j < morton_partition; ++j)
        {
            if(m_Voxels[j] == FILLEDVOXEL)
            {
                mn = Morton((morton_partition*i) + j);
                m_Octree.AddVoxel(mn);
            }

        }

    }
}

void Voxelizer::SaveToFile()
{
    m_Octree.Finalize();
}


ngl::BBox* Voxelizer::Cubify(ngl::Obj* _mesh)
{

    float offset = 1.00f;
    float minx = _mesh->getBBox().minX() * offset;
    float miny = _mesh->getBBox().minY() * offset;
    float minz = _mesh->getBBox().minZ() * offset;

    float maxx = _mesh->getBBox().maxX() * offset;
    float maxy = _mesh->getBBox().maxY() * offset;
    float maxz = _mesh->getBBox().maxZ() * offset;

    float H = _mesh->getBBox().height();
    float W = _mesh->getBBox().width();
    float D = _mesh->getBBox().depth();


    float maxUnit = W > H ? (W > D ? W : D) : (H > D ? H : D );

    float d = maxUnit - W;
    if(d != 0)
    {
        minx -= (d/2.0f);
        maxx += (d/2.0f);
    }

    d = maxUnit - H;
    if(d != 0)
    {
        miny -= (d/2.0f);
        maxy += (d/2.0f);
    }

    d = maxUnit - D;
    if(d != 0)
    {
        minz -= (d/2.0f);
        maxz += (d/2.0f);
    }

    return new ngl::BBox(minx,maxx,miny,maxy,minz,maxz);

}
