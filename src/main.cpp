#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "RecordStorage.h"
#include <string>

TEST_CASE("Create/Delete BlockStorage", "[BlockStorage]") {
    std::unique_ptr<ISharedMemory> memory = std::make_unique<FakeSharedMemory>(0U);
    BlockStorage<1028> storage(std::move(memory));

    REQUIRE(storage.size() == 0U);
}

TEST_CASE("Create new Block", "[BlockStorage]") {
    std::unique_ptr<ISharedMemory> memory = std::make_unique<FakeSharedMemory>(0U);
    BlockStorage<1028> storage(std::move(memory));

    for (size_t index = 0; index < 100; ++index) {
        Block block = storage.create();

        REQUIRE(block.blockSize() == 1028);
        REQUIRE(block.size() == 0U);
        REQUIRE(block.capacity() == 1028 - 32);
        REQUIRE(block.id() == index);

        REQUIRE(storage.size() == index + 1);
    }
}

TEST_CASE("Index Storage", "[BlockStorage]") {
    std::unique_ptr<ISharedMemory> memory = std::make_unique<FakeSharedMemory>(0U);
    BlockStorage<1028> storage(std::move(memory));

    for (size_t index = 0; index < 100; ++index) {
        Block block = storage.create();
    }
    REQUIRE(storage.size() == 100U);

    for (size_t index = 0; index < 100; ++index) {
        Block block = storage.at(index);

        REQUIRE(block.blockSize() == 1028);
        REQUIRE(block.size() == 0U);
        REQUIRE(block.capacity() == 1028 - 32);
        REQUIRE(block.id() == index);
    }
}

TEST_CASE("Create/Delete RecordStorage", "[RecordStorage]") {
    RecordStorage<1028> storage(
        std::make_unique<BlockStorage<1028> >(
            std::make_unique<FakeSharedMemory>(0U)));

    REQUIRE(storage.size() == 0U);
}

TEST_CASE("Add Record to RecordStorage", "[RecordStorage]") {
    RecordStorage<1028> storage(
        std::make_unique<BlockStorage<1028> >(
            std::make_unique<FakeSharedMemory>(0U)));

    std::string testString = "TestString0";
    RecordId recordId = storage.add(
        reinterpret_cast<const uint8_t*>(testString.c_str()),
        sizeof(char) + (testString.size() + 1 /* null terminator */));

    REQUIRE(recordId != kInvalidRecordId);

    REQUIRE(storage.size() == 1U);
}

TEST_CASE("Get Record to RecordStorage", "[RecordStorage]") {
    RecordStorage<1028> storage(
        std::make_unique<BlockStorage<1028> >(
            std::make_unique<FakeSharedMemory>(0U)));

    std::string testString = "TestString0";
    RecordId recordId = storage.add(
        reinterpret_cast<const uint8_t*>(testString.c_str()),
        sizeof(char) + (testString.size() + 1 /* null terminator */));

    std::vector<uint8_t> data = storage.get(recordId);
    std::string result(reinterpret_cast<const char*>(data.data()), data.size() / sizeof(char) - 1 /* null terminator */);
    REQUIRE(result == testString);
}
