#include <catch.hpp>

#include "BlockStorage.h"
#include "RecordStorage.h"
#include <iterator>
#include <random>
#include <string>
#include <vector>

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

TEST_CASE("Get Record from RecordStorage", "[RecordStorage]") {
    RecordStorage<1028> storage(
        std::make_unique<BlockStorage<1028> >(
            std::make_unique<FakeSharedMemory>(0U)));

    std::string testString = "TestString0";
    RecordId recordId = storage.add(
        reinterpret_cast<const uint8_t*>(testString.c_str()),
        sizeof(char) * (testString.size() + 1 /* null terminator */));

    std::vector<uint8_t> data = storage.get(recordId);
    std::string result(reinterpret_cast<const char*>(data.data()), (data.size() / sizeof(char)) - 1 /* null terminator */);
    REQUIRE(result == testString);
}

TEST_CASE("Erase Record from RecordStorage", "[RecordStorage]") {
    RecordStorage<1028> storage(
        std::make_unique<BlockStorage<1028> >(
            std::make_unique<FakeSharedMemory>(0U)));

    std::vector<RecordId> recordIds;
    std::string testString = "TestString0";
    RecordId recordId0 = storage.add(
        reinterpret_cast<const uint8_t*>(testString.c_str()),
        sizeof(char) * (testString.size() + 1 /* null terminator */));
    REQUIRE(storage.size() == 1U);

    std::string testString1 = "TestString1";
    RecordId recordId1 = storage.add(
        reinterpret_cast<const uint8_t*>(testString1.c_str()),
        sizeof(char) * (testString1.size() + 1 /* null terminator */));
    REQUIRE(storage.size() == 2U);

    storage.erase(recordId0);
    REQUIRE(storage.size() == 1U);

    std::vector<uint64_t> freeBlocks = storage.getFreeBlockIds();
    REQUIRE(freeBlocks.size() == 1U);
    REQUIRE(freeBlocks[0] == recordId0);

    storage.erase(recordId1);
    REQUIRE(storage.size() == 0U);

    freeBlocks = storage.getFreeBlockIds();
    REQUIRE(freeBlocks.size() == 2U);
    REQUIRE(freeBlocks[0] == recordId0);
    REQUIRE(freeBlocks[1] == recordId1);

    std::string testString2 = "TestString2";
    RecordId recordId2 = storage.add(
        reinterpret_cast<const uint8_t*>(testString2.c_str()),
        sizeof(char) * (testString2.size() + 1 /* null terminator */));
    REQUIRE(storage.size() == 1U);

    std::string testString3 = "TestString3";
    RecordId recordId3 = storage.add(
        reinterpret_cast<const uint8_t*>(testString3.c_str()),
        sizeof(char) * (testString3.size() + 1 /* null terminator */));
    REQUIRE(storage.size() == 2U);
}

template <size_t N>
static void addData(RecordStorage<N>& storage, size_t size, std::vector<RecordId>& recordIds)
{
   for (size_t i = 0; i < size; ++i) {
      std::string testString = "TestString" + std::to_string(i);
      RecordId recordId = storage.add(
          reinterpret_cast<const uint8_t*>(testString.c_str()),
          sizeof(char) * (testString.size() + 1 /* null terminator */));
      recordIds.push_back(recordId);
   }
}

template <size_t N, typename InputIt>
static void eraseRecords(RecordStorage<N>& storage, InputIt first, InputIt last)
{
   while (first != last) {
      storage.erase(*first);
      ++first;
   }
}

// TEST_CASE("Stress Erase Record from RecordStorage", "[RecordStorage]") {
//     RecordStorage<1028> storage(
//         std::make_unique<BlockStorage<1028> >(
//             std::make_unique<FakeSharedMemory>(0U)));
// 
//     std::random_device rd;
//     std::default_random_engine::result_type seed = rd();
//     std::cout << "seed=" << seed << std::endl;
//     std::default_random_engine generator(seed);
// 
//     std::vector<RecordId> recordIds;
//     addData(storage, 10, recordIds);
//     REQUIRE(recordIds.size() == 10U);
//     REQUIRE(storage.size() == 10U);
// 
//     std::shuffle(recordIds.begin(), recordIds.end(), generator);
// 
//     std::vector<RecordId>::iterator beginErase = recordIds.begin();
//     std::advance(beginErase, 5);
//     eraseRecords(
//         storage,
//         beginErase,
//         recordIds.end());
//     recordIds.erase(beginErase, recordIds.end());
//     REQUIRE(recordIds.size() == 5U);
//     REQUIRE(storage.size() == 5U);
// 
//     std::uniform_int_distribution<> addDist(1, 1000);
//     for (size_t i = 0; i < 100; ++i) {
//        const size_t numberToAdd = addDist(generator);
//        addData(storage, numberToAdd, recordIds);
//        REQUIRE(storage.size() == recordIds.size());
// 
//        std::uniform_int_distribution<> removeDist(1, recordIds.size());
//        const size_t numberToRemove = removeDist(generator);
// 
//        std::shuffle(recordIds.begin(), recordIds.end(), generator);
//        std::vector<RecordId>::iterator beginErase = recordIds.begin();
//        std::advance(beginErase, recordIds.size() - numberToRemove);
//        eraseRecords(
//            storage,
//            beginErase,
//            recordIds.end());
//        recordIds.erase(beginErase, recordIds.end());
//        REQUIRE(storage.size() == recordIds.size());
//     }
// 
//     eraseRecords(
//         storage,
//         recordIds.begin(),
//         recordIds.end());
//     REQUIRE(storage.size() == 0U);
// 
// }
