#include <catch.hpp>

#include "BlockStorage.h"

TEST_CASE("Create/Delete BlockStorage", "[BlockStorage]") {
    std::unique_ptr<ISharedMemory> memory = std::make_unique<FakeSharedMemory>(0U);
    BlockStorage<1028> storage(std::move(memory));

    REQUIRE(storage.size() == 0U);
    REQUIRE(storage.numFreeBlocks() == 0U);
}

TEST_CASE("Create new Block", "[BlockStorage]") {
    std::unique_ptr<ISharedMemory> memory = std::make_unique<FakeSharedMemory>(0U);
    BlockStorage<1028> storage(std::move(memory));

    for (size_t index = 0; index < 100; ++index) {
        Block<1028> block = storage.create();

        REQUIRE(block.blockSize() == 1028);
        REQUIRE(block.size() == 0U);
        REQUIRE(block.capacity() == 1028 - Block<1028>::MIN_BLOCK_SIZE);
        REQUIRE(block.id() == index);

        REQUIRE(storage.size() == index + 1);
    }

    REQUIRE(storage.numFreeBlocks() == 0U);
}

TEST_CASE("Index into Storage", "[BlockStorage]") {
    std::unique_ptr<ISharedMemory> memory = std::make_unique<FakeSharedMemory>(0U);
    BlockStorage<1028> storage(std::move(memory));

    for (size_t index = 0; index < 100; ++index) {
        Block<1028> block = storage.create();
    }
    REQUIRE(storage.size() == 100U);

    for (size_t index = 0; index < 100; ++index) {
        Block<1028> block = storage.at(index);

        REQUIRE(block.blockSize() == 1028);
        REQUIRE(block.size() == 0U);
        REQUIRE(block.capacity() == 1028 - Block<1028>::MIN_BLOCK_SIZE);
        REQUIRE(block.id() == index);
    }
}

TEST_CASE("Free a Block", "[BlockStorage]") {
    std::unique_ptr<ISharedMemory> memory = std::make_unique<FakeSharedMemory>(0U);
    BlockStorage<1028> storage(std::move(memory));

    std::vector<uint64_t> blockIds;
    for (size_t index = 0; index < 100; ++index) {
        Block<1028> block = storage.create();

        REQUIRE(block.blockSize() == 1028);
        REQUIRE(block.size() == 0U);
        REQUIRE(block.capacity() == 1028 - Block<1028>::MIN_BLOCK_SIZE);
        REQUIRE(block.id() == index);
        blockIds.push_back(block.id());

        REQUIRE(storage.size() == index + 1);
    }
    REQUIRE(storage.size() == 100);

    REQUIRE(storage.numFreeBlocks() == 0U);
    for (size_t index = 0; index < blockIds.size(); ++index) {
       storage.free(blockIds[index]);

       size_t oneBasedIndex = index + 1;
       REQUIRE(storage.numFreeBlocks() == oneBasedIndex);
       REQUIRE(storage.size() == (blockIds.size() - oneBasedIndex + 1 /* space tracking block */));
    }

    for (size_t index = 0; index < blockIds.size(); ++index) {
        Block<1028> block = storage.create();

        REQUIRE(block.blockSize() == 1028);
        REQUIRE(block.size() == 0U);
        REQUIRE(block.capacity() == 1028 - Block<1028>::MIN_BLOCK_SIZE);
        blockIds[index] = block.id();

        size_t oneBasedIndex = index + 1;
        REQUIRE(storage.size() == oneBasedIndex + 1 /* space tracking block */);
        REQUIRE(storage.numFreeBlocks() == (blockIds.size() - oneBasedIndex));
    }
}
