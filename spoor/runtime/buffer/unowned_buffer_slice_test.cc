#include "spoor/runtime/buffer/unowned_buffer_slice.h"

#include "gtest/gtest.h"

namespace {

// TEST(UnownedBufferSlice, UnownedConstructor) {  // NOLINT
//   for (const SizeType capacity : {0, 1, 2, 10}) {
//     std::vector<int64> data(capacity);
//     {
//       BufferSlice slice{data.data(), capacity};
//       for (SizeType i{0}; i < capacity; ++i) {
//         slice.Push(i);
//       }
//     }
//     std::vector<int64> expected(capacity);
//     std::iota(expected.begin(), expected.end(), 0);
//     ASSERT_EQ(data, expected);
//   }
// }
//
// TEST(UnownedBufferSlice, UnownedConstructorNullptr) {  // NOLINT
//   for (const SizeType capacity : {0, 1, 2, 10}) {
//     BufferSlice slice{nullptr, capacity};
//     ASSERT_EQ(slice.Capacity(), 0);
//     ASSERT_TRUE(slice.Empty());
//     ASSERT_TRUE(slice.Full());
//     for (SizeType i{0}; i < 2 * capacity; ++i) {
//       slice.Push(i);
//       ASSERT_EQ(slice.Capacity(), 0);
//       ASSERT_TRUE(slice.Empty());
//       ASSERT_TRUE(slice.Full());
//     }
//   }
// }

}  // namespace
