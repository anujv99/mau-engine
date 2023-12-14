#include "pass.h"

namespace mau {

  Pass::Pass(const String& name): m_Name(name) {
  }

  Pass::~Pass() {
  }

  void Pass::RegisterSource(const String& name, const Source& source) {
  }

  void Pass::RegisterSink(const String& name, const Sink& sink) {
  }
  
}
