#pragma once

#include "BlockStorage.h"
#include <memory>
#include <cstddef>


using RecordId = uint64_t;

// 0 is the Header block so should always be an invalid RecordId externally.
static const RecordId kInvalidRecordId = 0;

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
            Block headerBlock = m_storage->create();
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

      std::vector<Block> blocks = findBlocks(recordId);
      for (Block& block: blocks) {
          data.insert(data.end(), block.data(), block.data() + block.size());
      }

      return data;
   }

   RecordId add(const uint8_t* data, size_t size)
   {
      std::vector<Block> blocks = getFreeBlocks(size);

      const uint8_t* dataAddress = data;
      size_t remainingSize = size;
      for (Block& block : blocks) {
         uint64_t maxCapacity = block.capacity() - sizeof(RecordFormat);
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
      std::vector<Block> blocks = findBlocks(recordId);
      markAsFree(blocks);
   }

   size_t size()
   {
       return getHeader()->size;
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


   static void initializeHeader(uint8_t* data)
   {

   }

#pragma pack(push, 8)
   struct RecordFormat
   {
      uint64_t nextBlockId; // 8 bytes
      uint64_t size;         // 8 bytes
      uint8_t data[1];       // 1 byte
                             // 7 bytes (padding)
   };
#pragma pack(pop)

   static bool hasNextBlockId(const Block& block)
   {
      RecordFormat* record = reinterpret_cast<RecordFormat*>(block.data());
      return record->nextBlockId != 0;
   }

   static uint64_t nextBlockId(const Block& block)
   {
      RecordFormat* record = reinterpret_cast<RecordFormat*>(block.data());
      return record->nextBlockId;
   }

   static uint64_t recordCapacity(const Block& block)
   {
      return block.capacity() - offsetof(RecordFormat, data);
   }

   static uint64_t recordDataSize(const Block& block)
   {
      RecordFormat* record = reinterpret_cast<RecordFormat*>(block.data());
      return record->size;
   }

   static uint8_t* recordData(const Block& block)
   {
      RecordFormat* record = reinterpret_cast<RecordFormat*>(block.data());
      return record->data;
   }

   std::unique_ptr<BlockStorage<BlockSize> > sorage;

   std::vector<Block> findBlocks(RecordId recordId)
   {
      std::vector<Block> blocks;

      Block block = m_storage->at(recordId);
      RecordFormat* recordHeader = reinterpret_cast<RecordFormat*>(block.data());
      blocks.push_back(block);
      while ((recordId = recordHeader->nextBlockId) != 0) {
         Block block = m_storage->at(recordId);
         recordHeader = reinterpret_cast<RecordFormat*>(block.data());
         blocks.push_back(block);
      }

      return blocks;
   }

   std::vector<uint8_t> toRecordFormat(const uint8_t* data, size_t size)
   {
      // TODO: numeric_cast
      uint32_t dataSizeInBytes = static_cast<uint32_t>(offsetof(RecordFormat, data) + size);

      std::vector<uint8_t> result(dataSizeInBytes);

      RecordFormat* record = reinterpret_cast<RecordFormat*>(result.data());
      record->nextBlockId = 0;
      record->size = size;
      memcpy(record->data, data, size);

      return result;
   }

   Header* getHeader()
   {
      Block headerBlock = m_storage->at(HEADER_BLOCK);
      return reinterpret_cast<Header*>(headerBlock.data());
   }

   Block getSpaceTrackingBlock()
   {
      Header* header = getHeader();
      if (header->freedBlocksBlockId == 0) {
         Block block = m_storage->create();
         header->freedBlockBlockId = block.id();
         return block;
      } else {
         return m_storage->at(header->freedBlocksBlockId);
      }
   }

   void markAsFree(Block& block)
   {
      Block spaceTrackingBlock = getSpaceTrackingBlock();
      while (hasNextBlockId(spaceTrackingBlock)) {
         spaceTrackingBlock = m_storage->at(nextBlockId(spaceTrackingBlock));
      }

      size_t neededSize = recordDataSize(spaceTrackingBlock) + sizeof(uint64_t);
      if (neededSize > recordCapacity(spaceTrackingBlock)) {
      } else {
      }



      Header* header = getHeader();
      header->numFreeBlocks -= 1;
   }

   std::vector<Block> getFreeBlocks(size_t size)
   {
       std::vector<Block> blocks;

       size_t remainder = size % BlockSize == 0 ? 0 : 1;
       size_t numBlocks = (size / BlockSize) + remainder;

       // TODO: Look in freed blocks first
       blocks.reserve(numBlocks);
       for (size_t i = 0; i < numBlocks; ++i) {
           blocks.push_back(m_storage->create());
       }

       return blocks;
   }
};
