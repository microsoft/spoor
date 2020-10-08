#include <filesystem>

#include "gmock/gmock.h"
#include "spoor/runtime/trace/trace_writer.h"

namespace spoor::runtime::trace::testing {

class TraceWriterMock final : public TraceWriter {
 public:
  MOCK_METHOD(Result, Write,
              (const std::filesystem::path& file, const Header& header,
               Events& events, const Footer& footer),
              (const, override));
};

}  // namespace spoor::runtime::trace::testing
