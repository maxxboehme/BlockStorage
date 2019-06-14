#pragma once

#include <algorithm>
#include "BlockStorage.h"
#include <memory>
#include <cstddef>

#include <iostream>


using RecordId = uint64_t;

// 0 is the Header block so should always be an invalid RecordId externally.
static const RecordId kInvalidRecordId = 0;

// template <size_t BlockSize>
// class RecordView
// {
// public:
//    RecordView(Block<BlockSize> block)
//       : m_block(Block)
//    {
//       // TODO: Validation on block to make sure that it is in Record Format
//    }
// 
//    uint64_t id() const
//    {
//       return m_block->id();
//    }
// 
//    uint64_t capacity() const
//    {
//       // TODO: need to add up all block capacities
//       return m_block.capacity() - sizeof(Header);
//    }
// 
//    uint64_t size() const
//    {
//       // TODO: need to add up all block sizes
//       return getHeader()->size;
//    }
// 
//    uint8_t* data()
//    {
//       return reinterpret_cast<uint8_t*>(getHeader()) + sizeof(Header);
//    }
// 
//    const uint8_t* data() const
//    {
//       return reinterpret_cast<const uint8_t*>(getHeader())  + sizeof(Header);
//    }
// 
//    void set(uint8_t* data, uint64_t size)
//    {
//       if (size > blockSize()) {
//          // TODO: Throw
//       }
// 
//       // TODO: numeric_cast
//       std::memcpy(this->data(), data, static_cast<size_t>(size));
//       getHeader()->size = size;
//    }
// 
//    bool hasNextBlockId(const Block<BlockSize>& block)
//    {
//       const RecordFormat* record = reinterpret_cast<const RecordFormat*>(block.data());
//       return record->nextBlockId != kInvalidRecordId;
//    }
// 
//    Block<BlockSize> nextBlockId()
//    {
//       const RecordFormat* record = reinterpret_cast<const RecordFormat*>(block.data());
//       return record->nextBlockId;
//    }
// 
//    void setNextBlockId(uint64_t blockId)
//    {
//       RecordFormat* record = reinterpret_cast<RecordFormat*>(block.data());
//       record->nextBlockId = blockId;
//    }
// 
// private:
// #pragma pack(push, 8)
//    struct Header
//    {
//       uint64_t nextBlockId; // 8 bytes
//       uint64_t prevBlockId; // 8 bytes
//       uint8_t isFree;       // 1 bytes
//                             // 7 bytes (padding)
//       uint64_t size;        // 8 bytes
//    };
// #pragma pack(pop)
// 
//    Block<BlockSize> m_block;
// 
//    Header* getHeader()
//    {
//       Block<BlockSize> headerBlock = m_storage->at(HEADER_BLOCK);
//       return reinterpret_cast<Header*>(headerBlock.data());
//    }
// 
//    static bool hasNextBlockId(const Block<BlockSize>& block)
//    {
//       const RecordFormat* record = reinterpret_cast<const RecordFormat*>(block.data());
//       return record->nextBlockId != kInvalidRecordId;
//    }
// 
//    static uint64_t nextBlockId(const Block<BlockSize>& block)
//    {
//       const RecordFormat* record = reinterpret_cast<const RecordFormat*>(block.data());
//       return record->nextBlockId;
//    }
// 
//    static void setNextBlockId(Block<BlockSize>& block, uint64_t blockId)
//    {
//       RecordFormat* record = reinterpret_cast<RecordFormat*>(block.data());
//       record->nextBlockId = blockId;
//    }
// 
//    static bool hasPrevBlockId(const Block<BlockSize>& block)
//    {
//       const RecordFormat* record = reinterpret_cast<const RecordFormat*>(block.data());
//       return record->prevBlockId != kInvalidRecordId;
//    }
// 
//    static uint64_t prevBlockId(const Block<BlockSize>& block)
//    {
//       const RecordFormat* record = reinterpret_cast<const RecordFormat*>(block.data());
//       return record->prevBlockId;
//    }
// 
//    static void setPrevBlockId(Block<BlockSize>& block, uint64_t blockId)
//    {
//       RecordFormat* record = reinterpret_cast<RecordFormat*>(block.data());
//       record->prevBlockId = blockId;
//    }
// 
// 
//    static bool isRecordFree(const Block<BlockSize>& block)
//    {
//       const RecordFormat* record = reinterpret_cast<const RecordFormat*>(block.data());
//       return !!record->isFree;
//    }
// 
//    static void setRecordFree(Block<BlockSize>& block, bool isFree)
//    {
//       RecordFormat* record = reinterpret_cast<RecordFormat*>(block.data());
//       record->isFree = isFree;
//    }
// 
//    static uint64_t recordCapacity(const Block<BlockSize>& block)
//    {
//       return block.capacity() - offsetof(RecordFormat, data);
//    }
// 
//    static uint64_t recordDataSize(const Block<BlockSize>& block)
//    {
//       const RecordFormat* record = reinterpret_cast<const RecordFormat*>(block.data());
//       return record->size;
//    }
// 
//    static const uint8_t* recordData(const Block<BlockSize>& block)
//    {
//       const RecordFormat* record = reinterpret_cast<const RecordFormat*>(block.data());
//       return record->data;
//    }
// 
//    static uint8_t* recordData(Block<BlockSize>& block)
//    {
//       RecordFormat* record = reinterpret_cast<RecordFormat*>(block.data());
//       return record->data;
//    }
// 
// 
//    static void recordDataAppend(Block<BlockSize>& block, uint8_t* data, size_t size)
//    {
//       if (recordCapacity(block) < recordDataSize(block) + size) {
//          // TODO: throw
//       }
// 
//       uint8_t* address = recordData(block) + recordDataSize(block);
//       std::memcpy(address, data, size);
//       RecordFormat* recordFormat = getRecordFormat(block);
//       recordFormat->size += size;
//    }
// 
//    static void recordDataPop(Block<BlockSize>& block, uint8_t* data, size_t size)
//    {
//       if (recordDataSize(block) < size) {
//          // TODO: throw
//       }
// 
//       uint8_t* address = recordData(block) + recordDataSize(block) - size;
//       std::memcpy(data, address, size);
//       RecordFormat* recordFormat = getRecordFormat(block);
//       recordFormat->size -= size;
//    }
// 
// 
// };

template <size_t BlockSize>
class RecordStorage
{
public:
   RecordStorage(std::unique_ptr<BlockStorage<BlockSize> > storage)
      : m_storage(std::move(storage))
   {
      {
         std::lock_guard<BlockStorage<BlockSize> > lock(*m_storage);

         if (m_storage->size() == 0) {
            Block<BlockSize> headerBlock = m_storage->create();
            assert(headerBlock.id() == HEADER_BLOCK);

            // Setting header
            Header* header = getHeader();
            header->size = 0;
            header->numFreeBlocks = 0;
            header->freedBlocksBlockId = 0;
         }
      }
   }

   void lock()
   {
      m_storage->lock();
   }

   void unlock()
   {
      m_storage->unlock();
   }

   std::vector<uint8_t> get(RecordId recordId)
   {
      std::vector<uint8_t> data;

      std::vector<Block<BlockSize> > blocks = findBlocks(recordId);
      for (Block<BlockSize>& block: blocks) {
          data.insert(data.end(), recordData(block), recordData(block) + recordDataSize(block));
      }

      return data;
   }

   RecordId add(const uint8_t* data, size_t size)
   {
      // std::cout << "add(data, size=" << size << ")" << std::endl;
      std::vector<Block<BlockSize> > blocks = getFreeBlocks(size);

      const uint8_t* dataAddress = data;
      uint64_t remainingSize = static_cast<uint64_t>(size);
      for (Block<BlockSize>& block : blocks) {
         uint64_t maxCapacity = block.capacity() - offsetof(RecordFormat, data);
         if (std::max(maxCapacity, remainingSize) == remainingSize) {
            std::vector<uint8_t> recordData = toRecordFormat(dataAddress, maxCapacity);
            block.set(recordData.data(), recordData.size());
            dataAddress += maxCapacity;
         } else {
            std::vector<uint8_t> recordData = toRecordFormat(dataAddress, remainingSize);
            block.set(recordData.data(), recordData.size());
            dataAddress += remainingSize;
         }
      }

      Header* header = getHeader();
      header->size += 1;

      // No matter what size (even when 0) there should be at least one
      // block allocated.
      assert(blocks.size() > 0U);
      return blocks[0].id();
   }

   void erase(RecordId recordId)
   {
      // std::cout << "erase(recordId=" << recordId << ")" << std::endl;

      std::vector<Block<BlockSize> > blocks = findBlocks(recordId);
      for (Block<BlockSize> & block : blocks) {
         markAsFree(block);
      }

      Header* header = getHeader();
      header->size -= 1;
   }

   size_t size()
   {
       // TODO: numeric_cast
       return static_cast<size_t>(getHeader()->size);
   }

   // TODO: just for testing
   std::vector<uint64_t> getFreeBlockIds()
   {
      // std::cout << "getFreeBlockIds()" << std::endl;

      std::vector<uint64_t> freeBlockIds;

      Block<BlockSize> spaceTrackingBlock = getSpaceTrackingBlock();
      uint64_t* blockIds = reinterpret_cast<uint64_t*>(recordData(spaceTrackingBlock));
      // TODO: numeric_cast
      size_t size = static_cast<size_t>(recordDataSize(spaceTrackingBlock) / sizeof(uint64_t));
      freeBlockIds.insert(freeBlockIds.end(), blockIds, blockIds + size);
      while (hasNextBlockId(spaceTrackingBlock)) {
         // TODO: numeric_cast
         spaceTrackingBlock = m_storage->at(static_cast<size_t>(nextBlockId(spaceTrackingBlock)));

         blockIds = reinterpret_cast<uint64_t*>(recordData(spaceTrackingBlock));
         // TODO: numeric_cast
         size = static_cast<size_t>(recordDataSize(spaceTrackingBlock) / sizeof(uint64_t));
         freeBlockIds.insert(freeBlockIds.end(), blockIds, blockIds + size);
      }

      return freeBlockIds;
   }

private:
   static const uint64_t HEADER_BLOCK = 0;
   std::unique_ptr<BlockStorage<BlockSize> > m_storage;

#pragma pack(push, 8)
   struct Header
   {
      uint64_t size;               // 8 bytes
      uint64_t numFreeBlocks;      // 8 bytes
      uint64_t freedBlocksBlockId; // 8 bytes
   };
#pragma pack(pop)

#pragma pack(push, 8)
   struct RecordFormat
   {
      uint64_t nextBlockId; // 8 bytes
      uint64_t prevBlockId; // 8 bytes
      uint8_t isFree;       // 1 bytes
                            // 7 bytes (padding)
      uint64_t size;        // 8 bytes
      uint8_t data[1];      // 1 byte
                            // 7 bytes (padding)
   };
#pragma pack(pop)

   // TODO: should probably pass in size to validate
   static void initializeHeader(uint8_t* data)
   {
      std::memset(data, 0, sizeof(Header));
   }

   // TODO: should probably pass in size to validate
   static void initializeRecordFormat(uint8_t* data)
   {
      std::memset(data, 0, offsetof(RecordFormat, data));
   }

   static void initializeRecordFormat(Block<BlockSize>& block)
   {
       initializeRecordFormat(block.data());
   }


   static RecordFormat* getRecordFormat(Block<BlockSize>& block)
   {
      return reinterpret_cast<RecordFormat*>(block.data());
   }

   static bool hasNextBlockId(const Block<BlockSize>& block)
   {
      const RecordFormat* record = reinterpret_cast<const RecordFormat*>(block.data());
      return record->nextBlockId != kInvalidRecordId;
   }

   static uint64_t nextBlockId(const Block<BlockSize>& block)
   {
      const RecordFormat* record = reinterpret_cast<const RecordFormat*>(block.data());
      return record->nextBlockId;
   }

   static void setNextBlockId(Block<BlockSize>& block, uint64_t blockId)
   {
      RecordFormat* record = reinterpret_cast<RecordFormat*>(block.data());
      record->nextBlockId = blockId;
   }

   static bool hasPrevBlockId(const Block<BlockSize>& block)
   {
      const RecordFormat* record = reinterpret_cast<const RecordFormat*>(block.data());
      return record->prevBlockId != kInvalidRecordId;
   }

   static uint64_t prevBlockId(const Block<BlockSize>& block)
   {
      const RecordFormat* record = reinterpret_cast<const RecordFormat*>(block.data());
      return record->prevBlockId;
   }

   static void setPrevBlockId(Block<BlockSize>& block, uint64_t blockId)
   {
      RecordFormat* record = reinterpret_cast<RecordFormat*>(block.data());
      record->prevBlockId = blockId;
   }


   static bool isRecordFree(const Block<BlockSize>& block)
   {
      const RecordFormat* record = reinterpret_cast<const RecordFormat*>(block.data());
      return !!record->isFree;
   }

   static void setRecordFree(Block<BlockSize>& block, bool isFree)
   {
      RecordFormat* record = reinterpret_cast<RecordFormat*>(block.data());
      record->isFree = isFree;
   }

   static uint64_t recordCapacity(const Block<BlockSize>& block)
   {
      return block.capacity() - offsetof(RecordFormat, data);
   }

   static uint64_t recordDataSize(const Block<BlockSize>& block)
   {
      const RecordFormat* record = reinterpret_cast<const RecordFormat*>(block.data());
      return record->size;
   }

   static const uint8_t* recordData(const Block<BlockSize>& block)
   {
      const RecordFormat* record = reinterpret_cast<const RecordFormat*>(block.data());
      return record->data;
   }

   static uint8_t* recordData(Block<BlockSize>& block)
   {
      RecordFormat* record = reinterpret_cast<RecordFormat*>(block.data());
      return record->data;
   }


   static void recordDataAppend(Block<BlockSize>& block, uint8_t* data, size_t size)
   {
      if (recordCapacity(block) < recordDataSize(block) + size) {
         // TODO: throw
      }

      uint8_t* address = recordData(block) + recordDataSize(block);
      std::memcpy(address, data, size);
      RecordFormat* recordFormat = getRecordFormat(block);
      recordFormat->size += size;
   }

   static void recordDataPop(Block<BlockSize>& block, uint8_t* data, size_t size)
   {
      if (recordDataSize(block) < size) {
         // TODO: throw
      }

      uint8_t* address = recordData(block) + recordDataSize(block) - size;
      std::memcpy(data, address, size);
      RecordFormat* recordFormat = getRecordFormat(block);
      recordFormat->size -= size;
   }


   std::unique_ptr<BlockStorage<BlockSize> > sorage;

   std::vector<Block<BlockSize> > findBlocks(RecordId recordId)
   {
      std::vector<Block<BlockSize> > blocks;

      // TODO: numeric_cast
      Block<BlockSize> block = m_storage->at(static_cast<size_t>(recordId));
      RecordFormat* recordHeader = reinterpret_cast<RecordFormat*>(block.data());
      blocks.push_back(block);
      while ((recordId = recordHeader->nextBlockId) != kInvalidRecordId) {
         // TODO: numeric_cast
         Block<BlockSize> block = m_storage->at(static_cast<size_t>(recordId));
         recordHeader = reinterpret_cast<RecordFormat*>(block.data());
         blocks.push_back(block);
      }

      return blocks;
   }

   std::vector<uint8_t> toRecordFormat(const uint8_t* data, uint64_t size)
   {
      // TODO: numeric_cast
      uint32_t dataSizeInBytes = static_cast<uint32_t>(offsetof(RecordFormat, data) + size);

      std::vector<uint8_t> result(dataSizeInBytes);

      RecordFormat* record = reinterpret_cast<RecordFormat*>(result.data());
      record->nextBlockId = kInvalidRecordId;
      record->size = size;
      // TODO: numeric_cast
      memcpy(record->data, data, static_cast<size_t>(size));

      return result;
   }

   Header* getHeader()
   {
      Block<BlockSize> headerBlock = m_storage->at(HEADER_BLOCK);
      return reinterpret_cast<Header*>(headerBlock.data());
   }

   Block<BlockSize> getSpaceTrackingBlock()
   {
      // std::cout << "getSpaceTrackingBlock()" << std::endl;

      if (getHeader()->freedBlocksBlockId == 0) {
         Block<BlockSize> block = m_storage->create();
         initializeRecordFormat(block);

         // std::cout << "Creating Space Tracking block: id=" << block.id() << std::endl;
         getHeader()->freedBlocksBlockId = block.id();
         // std::cout << "Returning Space Tracking block: id=" << getHeader()->freedBlocksBlockId << std::endl;
         return block;
      } else {
         // std::cout << "Returning Space Tracking block: id=" << getHeader()->freedBlocksBlockId << std::endl;
         // TODO: numeric_cast
         return m_storage->at(static_cast<size_t>(getHeader()->freedBlocksBlockId));
      }
   }

   void markAsFree(Block<BlockSize>& block)
   {
      // std::cout << "markAsFree(block: id=" << block.id() << ")" << std::endl;
      Block<BlockSize> spaceTrackingBlock = getSpaceTrackingBlock();
      while (hasNextBlockId(spaceTrackingBlock)) {
         // TODO: numeric_cast
         spaceTrackingBlock = m_storage->at(static_cast<size_t>(nextBlockId(spaceTrackingBlock)));
      }

      // TODO: numeric_cast
      size_t neededSize = static_cast<size_t>(recordDataSize(spaceTrackingBlock) + sizeof(uint64_t));
      if (neededSize > recordCapacity(spaceTrackingBlock)) {
         Block<BlockSize> newSpaceTrackingBlock = m_storage->create();
         initializeRecordFormat(newSpaceTrackingBlock);
         setPrevBlockId(newSpaceTrackingBlock, spaceTrackingBlock.id());

         setNextBlockId(spaceTrackingBlock, newSpaceTrackingBlock.id());

         uint64_t blockId = block.id();
         recordDataAppend(newSpaceTrackingBlock, reinterpret_cast<uint8_t*>(&blockId), sizeof(uint64_t));
      } else {
         // std::cout << "Appending Free block: id=" << block.id() << std::endl;
         // std::cout << "   Block size before: size=" << recordDataSize(spaceTrackingBlock) << std::endl;
         uint64_t blockId = block.id();
         recordDataAppend(spaceTrackingBlock, reinterpret_cast<uint8_t*>(&blockId), sizeof(uint64_t));
         // std::cout << "   Block size after: size=" << recordDataSize(spaceTrackingBlock) << std::endl;
      }

      setRecordFree(block, true);

      getHeader()->numFreeBlocks += 1;
   }

   Block<BlockSize> popFreeBlock()
   {
      // std::cout << "popFreeBlock" << std::endl;
      if (getHeader()->numFreeBlocks == 0) {
         // TODO: throw exception
      }

      Block<BlockSize> spaceTrackingBlock = getSpaceTrackingBlock();
      while (hasNextBlockId(spaceTrackingBlock)) {
         // TODO: numeric_cast
         spaceTrackingBlock = m_storage->at(static_cast<size_t>(nextBlockId(spaceTrackingBlock)));
      }

      uint64_t freeBlockId = kInvalidRecordId;
      // std::cout << "   Block size before: size=" << recordDataSize(spaceTrackingBlock) << std::endl;
      recordDataPop(spaceTrackingBlock, reinterpret_cast<uint8_t*>(&freeBlockId), sizeof(uint64_t));
      // std::cout << "   Block size after: size=" << recordDataSize(spaceTrackingBlock) << std::endl;

      // Check if that spaceTrackingBlock is empty. If so we should free it.
      if (recordDataSize(spaceTrackingBlock) == 0) {
         if (hasPrevBlockId(spaceTrackingBlock)) {
            Block<BlockSize> prevSpaceTrackingBlock = m_storage->at(static_cast<size_t>(prevBlockId(spaceTrackingBlock)));
            setNextBlockId(prevSpaceTrackingBlock, 0);

            markAsFree(spaceTrackingBlock);
         }
      }

      getHeader()->numFreeBlocks -= 1;
      return m_storage->at(static_cast<size_t>(freeBlockId));
   }

   std::vector<Block<BlockSize> > getFreeBlocks(size_t size)
   {
       // std::cout << "getFreeBlocks(size=" << size << ")" << std::endl;
       std::vector<Block<BlockSize> > blocks;

       size_t remainder = size % BlockSize == 0 ? 0 : 1;
       size_t numBlocks = (size / BlockSize) + remainder;

       blocks.reserve(numBlocks);

       while (numBlocks > 0 && getHeader()->numFreeBlocks > 0) {
          blocks.push_back(popFreeBlock());
          --numBlocks;
       }

       for (size_t i = 0; i < numBlocks; ++i) {
           blocks.push_back(m_storage->create());
       }

       return blocks;
   }
};
