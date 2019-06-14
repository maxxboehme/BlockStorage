#pragma once

#include <memory>
#include <mutex>
#include <vector>
#include <cstring>
#include <cassert>

class ISharedMemory
{
public:
   virtual ~ISharedMemory() {};

   virtual void lock() = 0;
   virtual void unlock() = 0;

   virtual void* get() = 0;
   virtual size_t size() = 0;
   virtual void realloc(size_t requestedSize) = 0;
};

class FakeSharedMemory : public ISharedMemory
{
public:
   FakeSharedMemory(size_t sizeInBytes)
      : m_mutex(),
        m_buffer(sizeInBytes)
   {}

   virtual void lock() override
   {
      m_mutex.lock();
   }
   virtual void unlock() override
   {
      m_mutex.unlock();
   }

   virtual void* get() override
   {
      return m_buffer.data();
   }

   virtual size_t size() override
   {
      return m_buffer.size();
   }

   virtual void realloc(size_t requestedSize) override
   {
      if (requestedSize <= size()) return;

      m_buffer.resize(requestedSize);
   }

private:
   mutable std::mutex m_mutex;
   std::vector<uint8_t> m_buffer;
};

template <size_t BlockSize>
class BlockStorage;

template <size_t BlockSize>
class Block
{
public:
   static const uint64_t MIN_BLOCK_SIZE;
   // TODO: static_assert(BlockSize > MIN_BLOCK_SIZE, "Block size is too small");

   static void createBlock(uint64_t id, uint64_t blockSize, uint8_t* data)
   {
      Header* header = reinterpret_cast<Header*>(data);
      header->id = id;
      header->blockSize = blockSize;
      header->size = 0;
   }

   Block(BlockStorage<BlockSize>* storage, uint64_t id)
   : m_storage(storage),
     m_index(id)
   {}

   uint64_t id() const
   {
      return getHeader()->id;
   }

   uint64_t blockSize() const
   {
      return getHeader()->blockSize;
   }

   uint64_t capacity() const
   {
      return blockSize() - sizeof(Header);
   }

   uint64_t size() const
   {
      return getHeader()->size;
   }

   uint8_t* data()
   {
      return reinterpret_cast<uint8_t*>(getHeader()) + sizeof(Header);
   }

   const uint8_t* data() const
   {
      return reinterpret_cast<const uint8_t*>(getHeader())  + sizeof(Header);
   }

   void set(uint8_t* data, uint64_t size)
   {
      if (size > blockSize()) {
         // TODO: Throw
      }

      // TODO: numeric_cast
      std::memcpy(this->data(), data, static_cast<size_t>(size));
      getHeader()->size = size;
   }

   const BlockStorage<BlockSize>& storage() const
   {
      return *m_storage;
   }

   BlockStorage<BlockSize>& storage()
   {
      return *m_storage;
   }

private:
#pragma pack(push, 8)
   struct Header
   {
      uint64_t id;        // 8 bytes
      uint64_t blockSize; // 8 bytes
      uint64_t size;      // 8 bytes
      uint8_t isFree;     // 1 bytes
                          // 7 bytes (padding)
   };
#pragma pack(pop)

   Header* getHeader()
   {
      return reinterpret_cast<Header*>(m_storage->getFirstBlockAddress() + (BlockSize * m_index));
   }

   const Header* getHeader() const
   {
      return reinterpret_cast<const Header*>(m_storage->getFirstBlockAddress() + (BlockSize * m_index));
   }

   BlockStorage<BlockSize>* m_storage;
   uint64_t m_index;
};

template <size_t BlockSize>
const uint64_t Block<BlockSize>::MIN_BLOCK_SIZE = sizeof(Block<BlockSize>::Header);

template <typename T, size_t BlockSize>
class VectorView
{
public:
   static VectorView<T, BlockSize> createVectorView(Block<BlockSize> block)
   {
      memset(block.data(), 0, block.capacity());
      // TODO: set magic number?
      return VectorView<T, BlockSize>(block);
   }

   VectorView(Block<BlockSize> block)
      : m_block(block)
   {
      // TODO: Validation on block to make sure that it is in Record Format
   }

   uint64_t id()
   {
      return m_block.id();
   }

   uint64_t capacity()
   {
      Block<BlockSize> block = m_block;
      uint64_t totalCapacity = recordCapacity(block);
      while (hasNextBlockId(block)) {
          block = m_block.storage().at(nextBlockId(block));
          totalCapacity += recordCapacity(block);
      }

      return totalCapacity / sizeof(T);
   }

   uint64_t numBlocks()
   {
      Block<BlockSize> block = m_block;
      uint64_t numBlocks = 1;
      while (hasNextBlockId(block)) {
          block = m_block.storage().at(nextBlockId(block));
          ++numBlocks;
      }

      return numBlocks;
   }

   uint64_t size()
   {
      Block<BlockSize> block = m_block;
      uint64_t totalSize = recordDataSize(block);
      while (hasNextBlockId(block)) {
          block = m_block.storage().at(nextBlockId(block));
          totalSize += recordDataSize(block);
      }

      assert(totalSize % sizeof(T) == 0);
      return totalSize / sizeof(T);
   }

   T& operator[](size_t idx)
   {
      if (idx >= size()) {
         // TODO: throw exception
      }

      Block<BlockSize> block = m_block;
      while (idx >= (recordDataSize(block) / sizeof(T))) {
         assert(recordDataSize(block) % sizeof(T) == 0);
         idx -= recordDataSize(block) / sizeof(T);
         // The size check at the beginning should also validate
         // this. asserting just to make sure.
         assert(hasNextBlockId(block));
         block = m_block.storage().at(nextBlockId(block));
      }

      assert(idx < recordDataSize(block) / sizeof(T));
      return reinterpret_cast<T*>(recordData(block))[idx];
   }

   void push_back(const T& data)
   {
      size_t sizeOfData = sizeof(T);
      Block<BlockSize> end = getLastBlock();

      size_t neededSize = static_cast<size_t>(recordDataSize(end) + sizeOfData);
      if (neededSize > recordCapacity(end)) {
         Block<BlockSize> newEnd = m_block.storage().create();
         initializeVectorFormat(newEnd);
         setPrevBlockId(newEnd, end.id());

         setNextBlockId(end, newEnd.id());

         recordDataAppend(newEnd, reinterpret_cast<const uint8_t*>(&data), sizeOfData);
      } else {
         recordDataAppend(end, reinterpret_cast<const uint8_t*>(&data), sizeOfData);
      }
   }

   T pop_back()
   {
      if (getFormat()->size == 0) {
         // TODO: throw exception
      }

      Block<BlockSize> end = getLastBlock();

      T item;
      recordDataPop(end, reinterpret_cast<uint8_t*>(&item), sizeof(T));

      // Check if that end is empty. If so we should free it.
      if (recordDataSize(end) == 0) {
         if (hasPrevBlockId(end)) {
            Block<BlockSize> prevBlock = m_block.storage().at(static_cast<size_t>(prevBlockId(end)));
            clearNextBlockId(prevBlock);

            m_block.storage().free(end);
         }
      }

      return item;
   }

private:
#pragma pack(push, 8)
   struct VectorFormat
   {
      uint64_t hasNextBlock; // 8 bytes
      uint64_t nextBlockId; // 8 bytes
      uint64_t hasPrevBlock; // 8 bytes
      uint64_t prevBlockId; // 8 bytes
      uint64_t size;        // 8 bytes
      uint8_t  data[1];       // 1 bytes
   };
#pragma pack(pop)

   Block<BlockSize> m_block;

   VectorFormat* getFormat()
   {
      return reinterpret_cast<VectorFormat*>(m_block.data());
   }

   Block<BlockSize> getLastBlock()
   {
      Block<BlockSize> block = m_block;
      while (hasNextBlockId(block)) {
          block = m_block.storage().at(nextBlockId(block));
      }

      return block;
   }

   // TODO: should probably pass in size to validate
   static void initializeVectorFormat(Block<BlockSize> block)
   {
      std::memset(block.data(), 0, offsetof(VectorFormat, data));
   }

   static VectorFormat* getVectorFormat(Block<BlockSize>& block)
   {
      return reinterpret_cast<VectorFormat*>(block.data());
   }
   static bool hasNextBlockId(const Block<BlockSize>& block)
   {
      const VectorFormat* record = reinterpret_cast<const VectorFormat*>(block.data());
      return record->hasNextBlock;
   }

   static uint64_t nextBlockId(const Block<BlockSize>& block)
   {
      const VectorFormat* record = reinterpret_cast<const VectorFormat*>(block.data());
      return record->nextBlockId;
   }

   static void setNextBlockId(Block<BlockSize>& block, uint64_t blockId)
   {
      VectorFormat* record = reinterpret_cast<VectorFormat*>(block.data());
      record->nextBlockId = blockId;
      record->hasNextBlock = true;
   }

   static void clearNextBlockId(Block<BlockSize>& block)
   {
      VectorFormat* record = reinterpret_cast<VectorFormat*>(block.data());
      record->nextBlockId = 0;
      record->hasNextBlock = false;
   }

   static bool hasPrevBlockId(const Block<BlockSize>& block)
   {
      const VectorFormat* record = reinterpret_cast<const VectorFormat*>(block.data());
      return record->hasPrevBlock;
   }

   static uint64_t prevBlockId(const Block<BlockSize>& block)
   {
      const VectorFormat* record = reinterpret_cast<const VectorFormat*>(block.data());
      return record->prevBlockId;
   }

   static void setPrevBlockId(Block<BlockSize>& block, uint64_t blockId)
   {
      VectorFormat* record = reinterpret_cast<VectorFormat*>(block.data());
      record->prevBlockId = blockId;
      record->hasPrevBlock = true;
   }

   static void clearPrevBlockId(Block<BlockSize>& block)
   {
      VectorFormat* record = reinterpret_cast<VectorFormat*>(block.data());
      record->prevBlockId = 0;
      record->hasPrevBlock = false;
   }

   static uint64_t recordCapacity(const Block<BlockSize>& block)
   {
      return block.capacity() - offsetof(VectorFormat, data);
   }

   static uint64_t recordDataSize(const Block<BlockSize>& block)
   {
      const VectorFormat* record = reinterpret_cast<const VectorFormat*>(block.data());
      return record->size;
   }

   static const uint8_t* recordData(const Block<BlockSize>& block)
   {
      const VectorFormat* record = reinterpret_cast<const VectorFormat*>(block.data());
      return record->data;
   }

   static uint8_t* recordData(Block<BlockSize>& block)
   {
      VectorFormat* record = reinterpret_cast<VectorFormat*>(block.data());
      return record->data;
   }

   static void recordDataAppend(Block<BlockSize>& block, const uint8_t* data, size_t size)
   {
      if (recordCapacity(block) < recordDataSize(block) + size) {
         // TODO: throw
      }

      uint8_t* address = recordData(block) + recordDataSize(block);
      std::memcpy(address, data, size);
      VectorFormat* vectorFormat = getVectorFormat(block);
      vectorFormat->size += size;
   }

   static void recordDataPop(Block<BlockSize>& block, uint8_t* data, size_t size)
   {
      if (recordDataSize(block) < size) {
         // TODO: throw
      }

      uint8_t* address = recordData(block) + recordDataSize(block) - size;
      std::memcpy(data, address, size);
      VectorFormat* vectorFormat = getVectorFormat(block);
      vectorFormat->size -= size;
   }
};

template <size_t BlockSize>
class RecordView
{
public:
   static RecordView<BlockSize> createRecordView(Block<BlockSize> block)
   {
      memset(block.data(), 0, block.capacity());
      // TODO: set magic number?
      return RecordView<BlockSize>(block);
   }

   RecordView(Block<BlockSize> block)
      : m_block(block)
   {
      // TODO: Validation on block to make sure that it is in Record Format
   }

   uint64_t id()
   {
      return m_block.id();
   }

   uint64_t capacity()
   {
      Block<BlockSize> block = m_block;
      uint64_t totalCapacity = recordCapacity(block);
      while (hasNextBlockId(block)) {
          block = m_block.storage().at(nextBlockId(block));
          totalCapacity += recordCapacity(block);
      }

      return totalCapacity;
   }

   uint64_t numBlocks()
   {
      Block<BlockSize> block = m_block;
      uint64_t numBlocks = 1;
      while (hasNextBlockId(block)) {
          block = m_block.storage().at(nextBlockId(block));
          ++numBlocks;
      }

      return numBlocks;
   }

   uint64_t size()
   {
      Block<BlockSize> block = m_block;
      uint64_t totalSize = recordDataSize(block);
      while (hasNextBlockId(block)) {
          block = m_block.storage().at(nextBlockId(block));
          totalSize += recordDataSize(block);
      }

      return totalSize;
   }

   std::vector<uint8_t> data()
   {
      std::vector<uint8_t> data;

      Block<BlockSize> block = m_block;
      data.insert(data.end(), recordData(block), recordData(block) + recordDataSize(block));
      while (hasNextBlockId(block)) {
          block = m_block.storage().at(nextBlockId(block));
          data.insert(data.end(), recordData(block), recordData(block) + recordDataSize(block));
      }

      // TODO: more straight forward way?
      // std::vector<Block<BlockSize> > blocks = getBlocks();
      // for (Block<BlockSize>& block: blocks) {
      //     data.insert(data.end(), recordData(block), recordData(block) + recordDataSize(block));
      // }

      return data;
   }

   uint8_t& operator[](size_t idx)
   {
      if (idx >= size()) {
         // TODO: throw exception
      }

      Block<BlockSize> block = m_block;
      while (idx >= recordDataSize(block)) {
         idx -= recordDataSize(block);
         assert(hasNextBlockId(block));
         block = m_block.storage().at(nextBlockId(block));
      }

      assert(idx < recordDataSize(block.size()));
      return recordData(block)[idx];
   }

   void assign(size_t n, uint8_t value)
   {
   }


   template <class InputIterator>
   void assign(InputIterator first, InputIterator last)
   {
      size_t dataSize = std::distance(first, last);

   }
private:
#pragma pack(push, 8)
   struct RecordFormat
   {
      uint64_t hasNextBlock; // 8 bytes
      uint64_t nextBlockId; // 8 bytes
      uint64_t hasPrevBlock; // 8 bytes
      uint64_t prevBlockId; // 8 bytes
      uint64_t size;        // 8 bytes
      uint8_t  data[1];       // 1 bytes
   };
#pragma pack(pop)

   Block<BlockSize> m_block;

   RecordFormat* getFormat()
   {
      return reinterpret_cast<RecordFormat*>(m_block.data());
   }

   Block<BlockSize> getLastBlock()
   {
      Block<BlockSize> block = m_block;
      while (hasNextBlockId(block)) {
          block = m_block.storage().at(nextBlockId(block));
      }

      return block;
   }

   // TODO: should probably pass in size to validate
   static void initializeRecordFormat(Block<BlockSize> block)
   {
      std::memset(block.data(), 0, offsetof(RecordFormat, data));
   }

   static RecordFormat* getRecordFormat(Block<BlockSize>& block)
   {
      return reinterpret_cast<RecordFormat*>(block.data());
   }
   static bool hasNextBlockId(const Block<BlockSize>& block)
   {
      const RecordFormat* record = reinterpret_cast<const RecordFormat*>(block.data());
      return record->hasNextBlock;
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
      record->hasNextBlock = true;
   }

   static void clearNextBlockId(Block<BlockSize>& block)
   {
      RecordFormat* record = reinterpret_cast<RecordFormat*>(block.data());
      record->nextBlockId = 0;
      record->hasNextBlock = false;
   }

   static bool hasPrevBlockId(const Block<BlockSize>& block)
   {
      const RecordFormat* record = reinterpret_cast<const RecordFormat*>(block.data());
      return record->hasPrevBlock;
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
      record->hasPrevBlock = true;
   }

   static void clearPrevBlockId(Block<BlockSize>& block)
   {
      RecordFormat* record = reinterpret_cast<RecordFormat*>(block.data());
      record->prevBlockId = 0;
      record->hasPrevBlock = false;
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

   static void recordDataAppend(Block<BlockSize>& block, const uint8_t* data, size_t size)
   {
      if (recordCapacity(block) < recordDataSize(block) + size) {
         // TODO: throw
      }

      uint8_t* address = recordData(block) + recordDataSize(block);
      std::memcpy(address, data, size);
      RecordFormat* vectorFormat = getRecordFormat(block);
      vectorFormat->size += size;
   }

   static void recordDataPop(Block<BlockSize>& block, uint8_t* data, size_t size)
   {
      if (recordDataSize(block) < size) {
         // TODO: throw
      }

      uint8_t* address = recordData(block) + recordDataSize(block) - size;
      std::memcpy(data, address, size);
      RecordFormat* vectorFormat = getRecordFormat(block);
      vectorFormat->size -= size;
   }


};

template <size_t BlockSize>
class BlockStorage
{
public:
   const size_t block_size;

   explicit BlockStorage(std::unique_ptr<ISharedMemory> memory)
      : block_size(BlockSize),
        m_memory(std::move(memory))
   {
      {
         std::lock_guard<BlockStorage<BlockSize> > lock(*this);

         // If shared memory has not been initialized by any other process, make sure it has enough space
         // to store the size of the table.
         if (m_memory->size() < sizeof(Header))
         {
            m_memory->realloc(sizeof(Header));
            std::memset(m_memory->get(), 0, sizeof(Header));

            Header* header = this->header();
            header->magicNumber = kMagicNumber;
            header->blockSize = BlockSize;
         } else {
            Header* header = this->header();
            if (header->magicNumber != kMagicNumber) {
               // TODO: throw exception
            }

            if (header->blockSize != BlockSize) {
               // TODO: throw exception
            }
         }
      }
   }

   void lock()
   {
      m_memory->lock();
   }

   void unlock()
   {
      m_memory->unlock();
   }

   size_t size()
   {
      // TODO: numeric_cast
      return static_cast<size_t>(static_cast<Header*>(m_memory->get())->size);
   }

   Block<BlockSize> at(size_t index)
   {
      if (index >= size()) {
         //TODO: throw exception
      }

      return Block<BlockSize>(this, index);
   }

   Block<BlockSize> create()
   {
      if (header()->numFreeBlocks > 0) {
         VectorView<uint64_t, BlockSize> spaceTracker = getSpaceTrackingVector();

         header()->size += 1;
         header()->numFreeBlocks -= 1;
         uint64_t freeBlock = spaceTracker.pop_back();
         return at(freeBlock);
      } else {
         m_memory->realloc(m_memory->size() + BlockSize);
         header()->size += 1;

         size_t zeroBasedIndex = size() - 1;
         uint8_t* blockAddress = getFirstBlockAddress() + (BlockSize * zeroBasedIndex);
         Block<BlockSize>::createBlock(zeroBasedIndex, BlockSize, blockAddress);
         return Block<BlockSize>(this, zeroBasedIndex);
      }
   }

   void free(uint64_t blockId)
   {
      this->free(at(blockId));
   }

   void free(Block<BlockSize> block)
   {
      // Something fishy if not
      assert(size() > 0);

      // std::cout << "markAsFree(block: id=" << block.id() << ")" << std::endl;
      VectorView<uint64_t, BlockSize> spaceTracker = getSpaceTrackingVector();
      spaceTracker.push_back(block.id());
      ++header()->numFreeBlocks;
      header()->size -= 1;

      // TODO: mark block itself as free
   }

   uint64_t numFreeBlocks()
   {
      return header()->numFreeBlocks;
   }

private:
   friend class Block<BlockSize>;

   static const uint64_t kMagicNumber = 12345654321;
#pragma pack(push, 8)
   struct Header
   {
      uint64_t magicNumber;   // 8 bytes
      uint64_t blockSize;     // 8 bytes
      uint64_t size;          // 8 bytes
      uint64_t numFreeBlocks; // 8 bytes
      uint64_t freedBlockId;  // 8 bytes
   };
#pragma pack(pop)

   std::unique_ptr<ISharedMemory> m_memory;

   uint8_t* getFirstBlockAddress()
   {
      return static_cast<uint8_t*>(m_memory->get()) + sizeof(Header);
   }

   const uint8_t* getFirstBlockAddress() const
   {
      return static_cast<uint8_t*>(m_memory->get()) + sizeof(Header);
   }

   Header* header()
   {
      return reinterpret_cast<Header*>(m_memory->get());
   }

   VectorView<uint64_t, BlockSize> getSpaceTrackingVector()
   {
      if (header()->freedBlockId == 0) {
         Block<BlockSize> block = create();
         VectorView<uint64_t, 1028> vector = VectorView<uint64_t, 1028>::createVectorView(block);

         // std::cout << "Creating Space Tracking block: id=" << block.id() << std::endl;
         header()->freedBlockId = block.id();
         // std::cout << "Returning Space Tracking block: id=" << header()->freedBlockId << std::endl;
         return block;
      } else {
         // std::cout << "Returning Space Tracking block: id=" << header()->freedBlockId << std::endl;
         // TODO: numeric_cast
         return at(static_cast<size_t>(header()->freedBlockId));
      }
   }
};


