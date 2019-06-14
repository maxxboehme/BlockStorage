#include <catch.hpp>

#include "BlockStorage.h"

TEST_CASE("Create RecordView", "[RecordView]") {
    std::unique_ptr<ISharedMemory> memory = std::make_unique<FakeSharedMemory>(0U);
    BlockStorage<1028> storage(std::move(memory));
    REQUIRE(storage.size() == 0U);

    Block<1028> block = storage.create();
    RecordView<1028> record = RecordView<1028>::createRecordView(block);
    REQUIRE(record.id() == block.id());
    REQUIRE(record.capacity() > 0U);
    REQUIRE(record.size() == 0U);
    REQUIRE(record.data().empty() == true);
}

