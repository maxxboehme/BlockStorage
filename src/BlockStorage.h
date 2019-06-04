#pragma once

#include <memory>
#include <mutex>
#include <vector>
#include <cstring>

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

// class Block
// {
// public:
//    // TODO: static constexpr const uint64_t MIN_BLOCK_SIZE = sizeof(Header);
//    static void createBlock(uint64_t id, uint64_t blockSize, uint8_t* data)
//    {
//       Header* header = reinterpret_cast<Header*>(data);
//       header->id = id;
//       header->blockSize = blockSize;
//       header->size = 0;
//    }
// 
//    Block(uint8_t* data, size_t index)
//    : m_data(data)
//    {}
// 
//    uint64_t id() const
//    {
//       return reinterpret_cast<Header*>(m_data)->id;
//    }
// 
//    uint64_t blockSize() const
//    {
//       return reinterpret_cast<Header*>(m_data)->blockSize;
//    }
// 
//    uint64_t capacity() const
//    {
//       return blockSize() - sizeof(Header);
//    }
// 
//    uint64_t size() const
//    {
//       return reinterpret_cast<Header*>(m_data)->size;
//    }
// 
//    uint8_t* data()
//    {
//       return m_data + sizeof(Header);
//    }
// 
//    uint8_t* data() const
//    {
//       return m_data  + (+ sizeof(Header);
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
//       reinterpret_cast<Header*>(m_data)->size = size;
//    }
// 
// private:
// #pragma pack(push, 8)
//    struct Header
//    {
//       uint64_t id;        // 8 bytes
//       uint64_t blockSize; // 8 bytes
//       uint64_t size;      // 8 bytes
//    };
// #pragma pack(pop)
// 
//    uint8_t* m_data;
// };

template <size_t BlockSize>
class BlockStorage;

template <size_t BlockSize>
class Block
{
public:
   // TODO: static constexpr const uint64_t MIN_BLOCK_SIZE = sizeof(Header);
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

private:
#pragma pack(push, 8)
   struct Header
   {
      uint64_t id;        // 8 bytes
      uint64_t blockSize; // 8 bytes
      uint64_t size;      // 8 bytes
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
            header->blockSize = BlockSize;
         } else {
            Header* header = this->header();
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
      m_memory->realloc(m_memory->size() + BlockSize);
      static_cast<Header*>(m_memory->get())->size += 1;

      size_t zeroBasedIndex = size() - 1;
      uint8_t* blockAddress = getFirstBlockAddress() + (BlockSize * zeroBasedIndex);
      Block<BlockSize>::createBlock(zeroBasedIndex, BlockSize, blockAddress);
      return Block<BlockSize>(this, zeroBasedIndex);
   }

private:
   friend class Block<BlockSize>;
#pragma pack(push, 8)
   struct Header
   {
      uint64_t blockSize; // 8 bytes
      uint64_t size;      // 8 bytes
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
};


