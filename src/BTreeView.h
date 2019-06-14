#pragma once

#include "RecordStorage.h"

template <size_t BlockSize>
class BTreeView
{
   public:
      BTreeView(Block<BlockSize> block, std::shared_ptr<BlockStorage<BlockSize> > storage)
         : m_block(block)
           m_blockStorage(storage)
      {}

      void insert(uint8_t* key, size_t keySize, uint8_t* value, size_t valueSize);
      RecordId find(uint8_t* key, size_t keySize);
      void erase(uint8_t* key, size_t keySize);

   private:
      template <size_t BlockSize>
      class BTreeNode
      {
         public:
            BTreeNode(Block<BlockSize> block)
               : m_block(block);
         private:
#pragma pack(push, 8)
            struct Format
            {
               uint64_t mWay;                   // M-way node
               uint8_t  isLeaf;                 // is leaf node
               uint64_t size;                   // number key value pairs
               uint64_t valueOffset;            // Offset of the value Records in keyValueAndChildren
               uint64_t childrenOffset;         // Offset of the children Records in keyValueAndChildren
               RecordId keyValueAndChildren[1]; // This will hold the keys, values, and children RecordIds
            }
#pragma pack(pop)
            void insert(RecordId key, RecordId value);
            void splitChildren(uint64_t offset);

            Block<BlockSize> m_block;
      };

      Block<BlockSize> m_block;
      BlockStorage<BlockSize> m_blockStorage;
};
