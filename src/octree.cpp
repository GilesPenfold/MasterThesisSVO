#include <octree.h>
#include <string>
#include <bitset>
#include <sstream>
#include <ios>
#include <fstream>
#include <iostream>
#include <cctype>
#include <cstdint>

// Based on: https://github.com/Forceflow/ooc_svo_builder/blob/master/src/svo_builder/OctreeBuilder.cpp

Octree::Octree(std::string _name, size_t _GridSize, float _UnitLength)
{
    m_FileName = _name;

    // setup our grid size and morton units
    m_GridSize = _GridSize;
    m_DataPosition = 2; // 2 for binary voxelization, 1 for non-binary
    m_NodePosition = 0;
    m_CurrentMorton = Morton(0);

    // Figure our max depth
    m_MaxDepth = log2((unsigned int)m_GridSize);

    // Resize our children nodes
    m_Children.resize(m_MaxDepth+1);
    for(int i = 0; i < m_MaxDepth + 1; i++)
    {
        m_Children[i].reserve(8);
    }

    // Encode our maximum morton grid cell
    uint_fast32_t mortmax = (uint_fast32_t)(m_GridSize-1);

    m_MaxMorton.encodeMortonMagic(MortonCoords(mortmax,mortmax,mortmax));

    _cube = Cube(_UnitLength);


}

void Octree::Begin(COMPRESSIONTYPE _compress)
{
    std::string octreeName = m_FileName + ".octree";

    m_File = fopen(octreeName.c_str(), "wb");

    m_Compression = _compress;

    std::string octName = m_FileName + ".octreecompress";

    if(_compress == SNAPPY )
    {
        m_outStream.open(octName);
    }
    else if(_compress == MINIZ)
    {
        m_outStream.open(octName);
    }
}


void Octree::Draw()
{
    for(int i = 0; i < m_Children.size(); i++)
    {
        for(int j = 0; j < m_Children[i].size(); j++)
        {
            if(m_Children[i][j].IsNull())
                continue;


        }
    }
}


void Octree::AddVoxel(Morton _m)
{
    if(_m.M() != m_CurrentMorton.M())
    {
        FastAddEmptyVoxel(_m.M() - m_CurrentMorton.M());
    }

    Node n = Node();
    n.m_Data = 1;
    m_Children.at(m_MaxDepth).push_back(n);
    Refine(m_MaxDepth);
    ++m_CurrentMorton;

}


void Octree::AddEmptyVoxel(size_t _b)
{
    m_Children[_b].push_back(Node());
    Refine(_b);
    m_CurrentMorton = Morton((uint64_t)m_CurrentMorton.M() + pow(8.0, m_MaxDepth - _b));
}

void Octree::FastAddEmptyVoxel(size_t _b)
{
    size_t target = _b;
    while(target > 0)
    {
        uint64_t child = FindBestFillChild(target);
        AddEmptyVoxel(child);
        size_t current = (size_t)pow(8.0,m_MaxDepth-child);
        target -= current;
    }

}

int Octree::FindBestFillChild(size_t _b)
{
    int test = m_MaxDepth - FindPowerOfEight(_b);
    if(test == m_MaxDepth)
        return m_MaxDepth;
    else
        return std::max(test, FindHighestNonEmptyChild());
}

int Octree::FindHighestNonEmptyChild()
{
    int test = m_MaxDepth;
    for (int i = m_MaxDepth; i >= 0; i--)
    {
        if(m_Children[i].size()==0)
            test--;
        else
            return test;
    }
    return test;
}

void Octree::Refine(size_t _b)
{
    for (int i = _b; i >= 0; i--)
    {
        if(m_Children[i].size() == 8)
        {
            assert(i - 1 >= 0);
            if(ChildEmpty(m_Children[i]))
                m_Children[i-1].push_back(Node());
            else
                m_Children[i-1].push_back(GroupNodes(m_Children[i]));
            m_Children.at(i).clear();

        }
        else
            break;
    }
}

void Octree::Finalize()
{
    if(m_CurrentMorton.M() < m_MaxMorton.M())
    {
        FastAddEmptyVoxel((m_MaxMorton.M() - m_CurrentMorton.M())+1);
    }

    WriteNode(m_Children[0][0]);

    WriteHeader();

    if(m_Compression != NONE)
        CompressFileIncremental(true);

    fclose(m_File);
}

Node Octree::GroupNodes(std::vector<Node> &_n)
{
    Node p = Node();
    bool first = true;

    for (int i = 0; i < 8; i++)
    {
        if(!_n[i].IsNull())
        {
            if(first)
            { // Baert uses this for OOC,
              // But we are just doing in core for now
                WriteNode(_n[i]);
                p.m_ChildrenBase = m_NodePosition;
                m_NodePosition++;
                p.m_ChildrenOffset[i] = 0;
                first = false;
            }
            else
            {
                WriteNode(_n[i]);
                p.m_ChildrenOffset[i] = m_NodePosition - p.m_ChildrenBase;
                m_NodePosition++;
            }
        }
        else
        {
            p.m_ChildrenOffset[i] = -1;
        }
    }

    return p;
}

bool Octree::ChildEmpty(std::vector<Node> &_n)
{
    for (int i = 0; i < 8; ++i)
    {
        if(!_n[i].IsNull())
            return false;
    }
    return true;
}

unsigned int Octree::FindPowerOfEight(size_t _b)
{
    if(_b == 0)
        return 0;

    unsigned int idx = 0;
    while(_b >>= 1)
        idx++;
    return idx/3;
}


void Octree::WriteNode(Node &_n)
{
    fwrite(&_n.m_Data, sizeof(size_t), 3, m_File);

    if(m_Compression != NONE)
        WriteNodeCompress(_n);

    if(m_Data.size() >= 1000)
        CompressFileIncremental(false);
}

void Octree::ReadNode(Node &_n)
{
    fread(&_n.m_Data, sizeof(size_t), 3, m_File);
}



void Octree::WriteHeader()
{
    std::ofstream _out;
    std::string compression;
    if(m_Compression == NONE)
        compression = "NONE";
    if(m_Compression == SNAPPY)
        compression = "SNAPPY";
    if(m_Compression == MINIZ)
        compression = "MINIZ";
    std::string name = m_FileName + ".octheader";
    _out.open(name.c_str());
    _out << compression << '\n';
    _out << m_GridSize << '\n';
    _out << m_NodePosition << '\n';
    _out << m_DataPosition << '\n';
    _out.close();
}

void Octree::ReadHeader()
{
    std::ifstream _in;
    std::string name = m_FileName + ".octheader";
    _in.open(name.c_str());
    std::string line;
    int count = 0;
    std::string compression;
    while(std::getline(_in, line))
    {
       if(count == 0)
           compression = line;
       if(count == 1)
           sscanf(line.c_str(), "%zu", &m_GridSize);
       if(count == 2)
           sscanf(line.c_str(), "%zu", &m_NodePosition);
       if(count == 3)
           sscanf(line.c_str(), "%zu", &m_DataPosition);
       count++;
    }

    if(compression == "NONE")
        m_Compression = NONE;
    if(compression == "SNAPPY")
        m_Compression = SNAPPY;
    if(compression == "MINIZ")
        m_Compression = MINIZ;
}

void Octree::ReadOctree()
{
    ReadHeader();

    std::string octreeName = m_FileName + ".octree";

    if(m_Compression != NONE)
    {
        std::string p = m_Compression == MINIZ ? "MINIZ" : "SNAPPY";
        std::cout << "File is compressed with " << p << " compression type...\nUncompressing...\n";
        ReadCompressedFile();
        std::cout << "Success!\n";
    }
    else
    {
        std::cout << "File is not compressed. Loading nodes...\n";

        m_File = fopen(octreeName.c_str(), "rb");

        m_Nodes.reserve(m_NodePosition);

        for(int j = 0; j < m_NodePosition; j++)
        {
            Node n = Node();
            ReadNode(n);
            m_Nodes.push_back(&n);
        }

        fclose(m_File);

    }


}

void Octree::WriteNodeCompress(Node &_n)
{

    std::string input;
    std::ostringstream oss;
    std::string delimB = "|";
    oss << _n.m_Data << delimB << _n.m_ChildrenBase << delimB;
    for(int i = 0; i < 8; i++)
    {
        if(_n.m_ChildrenOffset[i] >= 0)
            oss << (int)_n.m_ChildrenOffset[i] << delimB;
        else
            oss << ' ' << delimB;
    }

    input = oss.str();
    input += "$";
    std::string* d = new std::string(input);

    m_Data.push_back(d);

}

void Octree::ReadCompressedFile()
{
    std::string octreeName = m_FileName + ".octreecompress";
    m_inStream.open(octreeName, std::ios::binary );
    std::noskipws(m_inStream);
    std::string line,concat,delim;
    if(m_Compression == SNAPPY)
        delim = "$SNAPPY$";
    if(m_Compression == MINIZ)
        delim = "$MINIZ$";
    int pos = 0;
    std::vector<std::string> found;

    // Read our file into memory

    while(std::getline(m_inStream, line))
    {
        pos = line.find(delim);
        if(pos > 0)
        {

            concat+=line.substr(0, pos);
            //if(concat[concat.length()])
            if(found.size() != 0)
                concat = concat.substr(1,concat.length());
            found.push_back(concat);
            concat = "";
            line.erase(0, pos+delim.length());
            pos = 0;
        }
        concat+= line;

    }



    // Process everything now we've read it into memory

    m_Nodes.reserve(m_NodePosition);

    std::string *o = new std::string();
    *o = "";

    std::string delimA, delimB;
    delimB = "|";
    delimA = "$";
    std::string T, N, O;
    //char* c;
    //int tracker = 0;
    int posA, posB, counter;

    // Deal with those goddamn \r tokens that have made me despise windows beyond all comprehension

    int erased = 0;

    for(int i = 0; i < found.size(); i++)
    {
        for(int j = found[i].size()-1; j >= 0 ; j--)
        {   // Check our string for any potential windows carriage tokens that will destroy everything
            if(found[i][j]=='\r')
            {
                if(found[i].size() >= j+4)
                {   // If we find our delimeter by one of the \r tokens
                    if(found[i][j+1] == '[' && found[i][j+2] == 'R' && found[i][j+3] == 'R' && found[i][j+4] == ']')
                    {   // Then it is a valid \r for the compression and we remove the delimeter
                        found[i].erase(j+1, 4);
                    }
                    else
                    {   // Otherwise it's an annoying windows carriage return token and we erase it from existence
                        // Since it should never have existed in the first place, looking at YOU, MICROSOFT
                        found[i].erase(j, 1);
                        erased++;
                    }
                }
            }

            if(found[i][j]=='[')
            {   // If we find our delimeter for one of the \n tokens
                if(found[i].size() >= j+3)
                {   // And we have a matching token
                    if(found[i][j+1] == 'N' && found[i][j+2] == 'N' && found[i][j+3] == ']')
                    {   // Then it is a valid \n for the compression and we remove the delimeter
                        // Then shove the \n back in
                        found[i][j] = '\n';
                        found[i].erase(j+1,3);
                    }
                }
            }
        }

        erased=0;

    }


    std::cout << "File loaded. Filling " << m_NodePosition << " nodes...\n";

    for(int i = 0; i < found.size(); i++)
    {
        //std::cout << found[i].length() << '\n'; // WRONG SIZE! FUCK!
        snappy::Uncompress(found[i].c_str(), found[i].size(), o);

        while((posA = o->find(delimA))!=std::string::npos)
        {
            // Grab our first Node
            T = o->substr(0,posA);
            // Remove semicolon delimeter
            *o = o->substr(0,o->size()-1);
            counter = 0;
            Node n = Node();
            // Iterate through
            while((posB = T.find(delimB))!=std::string::npos)
            {
                N = T.substr(0,posB);
                if(counter == 0)
                { // If there is some error in the decompression that is out of my hands
                  // So, in most cases, N is 0 a this point, so we can set it to 0 and hope for the best
                    if(std::isdigit(N[0]) == 0)
                    {
                        N = "0";
                        std::cout << "Problem loading in Node Data... Attempting to correct.";
                    }
                    n.m_Data = std::stoi(N);
                }
                if(counter == 1)
                {   // In this case, if it is blank, then we are in trouble but it will most likely
                    // Be one voxel out of several million, let's set it to 0 too and hope that nothing blows up
                    if(std::isdigit(N[0]) == 0)
                    {
                        N = "0";
                        std::cout << "Problem loading in Node Children Base... Attempting to correct.";
                    }
                    n.m_ChildrenBase = std::stoi(N);
                }
                if(counter > 1)
                {
                    if(N == " ")
                        n.m_ChildrenOffset[counter-2] = (char)-1;
                    else
                    {
                        //c = std::strcpy(c, N.c_str());
                        n.m_ChildrenOffset[counter-2] = (char)N[0];
                    }
                }

                T.erase(0,posB+delimB.length());
                counter++;
            }
            o->erase(0,posA+delimA.length());
            m_Nodes.push_back(&n);
            //tracker++;
        }


    }


}

void Octree::CompressFileIncremental(bool _final)
{
    std::string d = "";
    std::string *o = new std::string();
    *o = "";

    int target = m_Data.size();

    for(int i = 0; i < target; i++)
    {
        d += *m_Data[i];
    }

    if(m_Compression == SNAPPY)
    {
        snappy::Compress(d.c_str(),(size_t)d.size(),o);



        //std::cout << o->substr(0, 30) << "\n";

        std::string a = *o;
        std::string toWrite;

        // We add a delimeter called '[RR]' to combat the stupid windows carriage return
        // As our compression uses '\r' too, so we mark those with our delimeter
        // Edit: Apparently it also loves to remove the \n but not the \r (this is stupid)
        // So we add another delimeter called '[NN]' to indicate there should be a \n there



        for(int j = 0; j < o->length(); j++)
        {
            toWrite.push_back( a[j] );
            if(a[j] == '\r')
            {
                toWrite.push_back('[');
                toWrite.push_back('R');
                toWrite.push_back('R');
                toWrite.push_back(']');
            }
            if(a[j] == '\n')
            {
                toWrite.push_back('[');
                toWrite.push_back('N');
                toWrite.push_back('N');
                toWrite.push_back(']');
            }
        }

        toWrite += "$SNAPPY$\n";


        m_outStream << toWrite;
    }
    else if(m_Compression == MINIZ)
    {

        uLong src_len = (uLong)strlen(d.c_str());
        uLong cmp_len = compressBound(src_len);
        unsigned char * pCmp = (unsigned char *)malloc((size_t)cmp_len);

        compress(pCmp, &cmp_len, (const unsigned char *)d.c_str(), src_len);


        std::string a = reinterpret_cast<char const*> (pCmp);
        std::string toWrite;

        for(int j = 0; j < a.length(); j++)
        {
            toWrite.push_back( a[j] );
            if(a[j] == '\r')
            {
                toWrite.push_back('[');
                toWrite.push_back('R');
                toWrite.push_back('R');
                toWrite.push_back(']');
            }
            if(a[j] == '\n')
            {
                toWrite.push_back('[');
                toWrite.push_back('N');
                toWrite.push_back('N');
                toWrite.push_back(']');
            }
        }

        toWrite += "$MINIZ$\n";

        m_outStream << toWrite;


    }

    m_Data.clear();

    if(_final)
        m_outStream.close();

}



