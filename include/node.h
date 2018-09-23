#ifndef NODE_H
#define NODE_H
#include <stdint.h>
#include <memory>

// Based on https://github.com/Forceflow/ooc_svo_builder/blob/master/src/svo_builder/Node.h

const char LEAF[8] = {-1,-1,-1,-1,-1,-1,-1,-1};

class Node
{
  public:
    ///
    /// \brief Node
    /// Creates a node with binary data, the base offset of the children, and the offset of each child
    Node()
    {
        m_Data = 0;
        m_ChildrenBase = 0;

        memset(m_ChildrenOffset,(char)(-1),8);
    }
    ///
    /// \brief HasChild
    /// \param _i
    /// \return
    /// Checks if a child exists at the position _i
    bool HasChild(unsigned int _i)
    {
        return !(m_ChildrenOffset[_i] == -1);
    }
    ///
    /// \brief GetChildPos
    /// \param _i
    /// \return
    /// Locates the Node of the child _i if it is found within the children of this node
    size_t GetChildPos(unsigned int _i)
    {
        if(m_ChildrenOffset[_i] == -1)
            return 0;
        else
            return m_ChildrenBase + m_ChildrenOffset[_i];
    }
    ///
    /// \brief IsLeaf
    /// \return
    /// Checks if the current node is a leaf node
    bool IsLeaf()
    {
        if(memcmp(m_ChildrenOffset, LEAF, 8*sizeof(char))==0)
            return true;
        else
            return false;
    }
    ///
    /// \brief HasData
    /// \return
    /// Checks if there is a voxel in this node
    bool HasData()
    {
        return !(m_Data==0);
    }
    ///
    /// \brief IsNull
    /// \return
    /// Checks if the node is a null node (being a leaf node and having no data)
    bool IsNull()
    {
        return IsLeaf() && !HasData();
    }


    size_t m_Data;
    size_t m_ChildrenBase;
    char m_ChildrenOffset[8];




};

#endif // NODE_H
