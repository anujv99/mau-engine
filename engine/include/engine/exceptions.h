#pragma once

#include <string_view>
#include <exception>

namespace mau {

  class WindowException: public std::exception {
  public:
    WindowException(std::string_view message): m_Message(message){};
    std::string_view what() { return m_Message; }

  private:
    std::string_view m_Message;
  };

  class GraphicsException: public std::exception {
  public:
    GraphicsException(std::string_view message): m_Message(message){};
    std::string_view what() { return m_Message; }

  private:
    std::string_view m_Message;
  };

} // namespace mau
