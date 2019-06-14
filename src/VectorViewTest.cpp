#include <catch.hpp>

#include "BlockStorage.h"
#include "RecordStorage.h"

TEST_CASE("Create VectorView", "[VectorView]") {
    std::unique_ptr<ISharedMemory> memory = std::make_unique<FakeSharedMemory>(0U);
    BlockStorage<1028> storage(std::move(memory));
    REQUIRE(storage.size() == 0U);

    Block<1028> block = storage.create();
    VectorView<uint64_t, 1028> vector = VectorView<uint64_t, 1028>::createVectorView(block);
    REQUIRE(vector.id() == block.id());
    REQUIRE(vector.capacity() > 0U);
    REQUIRE(vector.size() == 0U);
}

TEST_CASE("Push and pop back VectorView", "[VectorView]") {
    std::unique_ptr<ISharedMemory> memory = std::make_unique<FakeSharedMemory>(0U);
    BlockStorage<1028> storage(std::move(memory));
    REQUIRE(storage.size() == 0U);

    Block<1028> block = storage.create();
    VectorView<uint64_t, 1028> vector = VectorView<uint64_t, 1028>::createVectorView(block);
    vector.push_back(1234);
    REQUIRE(vector.size() == 1U);

    uint64_t back = vector.pop_back();
    REQUIRE(back == 1234);
    REQUIRE(vector.size() == 0U);

    uint64_t numItemsForNewBlock = vector.capacity() + 1;
    for (size_t i = 0; i < numItemsForNewBlock; ++i) {
       vector.push_back(i);
    }
    REQUIRE(vector.size() == numItemsForNewBlock);
    REQUIRE(vector.numBlocks() == 2U);

    back = vector.pop_back();
    REQUIRE(back == (numItemsForNewBlock - 1));
    REQUIRE(vector.size() == numItemsForNewBlock - 1);
    REQUIRE(vector.numBlocks() == 1U);
}

TEST_CASE("Index VectorView", "[VectorView]") {
    std::unique_ptr<ISharedMemory> memory = std::make_unique<FakeSharedMemory>(0U);
    BlockStorage<1028> storage(std::move(memory));
    REQUIRE(storage.size() == 0U);

    Block<1028> block = storage.create();
    VectorView<uint64_t, 1028> vector = VectorView<uint64_t, 1028>::createVectorView(block);

    uint64_t numItemsForNewBlock = vector.capacity() + 1;
    for (size_t i = 0; i < numItemsForNewBlock; ++i) {
       vector.push_back(i);
    }
    REQUIRE(vector.size() == numItemsForNewBlock);
    REQUIRE(vector.numBlocks() == 2U);

    for (size_t i = 0; i < vector.size(); ++i) {
       REQUIRE(vector[i] == i);
    }
}
