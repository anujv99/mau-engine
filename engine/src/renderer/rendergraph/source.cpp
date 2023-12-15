#include "source.h"

#include "sink.h"

namespace mau {

  Source::Source(const Sink &sink)
      : m_Name(sink.m_Name), m_Resources(sink.m_Resources) { }

} // namespace mau
