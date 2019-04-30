#include <log.hpp>
#include <util.hpp>

#include <fmt/ostream.h>
#include <fmt/color.h>

#include <iostream>

enum class LoggerMessageType
{
  Error,
  Warning,
  Info
};

class Logger
{
public:
  void update(LoggerMessageType type, const std::string& module, std::uint_fast32_t column, std::uint_fast32_t row)
  {
    this->type = type;
    this->module = module;
    this->column = column;
    this->row = row;
  }

  MessageCollector collect()
  {
    return MessageCollector(*this);
  }
  void print(const std::string& message)
  {
    fmt::print(fg(fmt::terminal_color::white), "{}:{}:{}: ", module, column, row);
    switch(type)
    {
    case LoggerMessageType::Error:
      fmt::print(fg(fmt::terminal_color::red) | (fmt::emphasis::bold), "error: ");
    break;

    case LoggerMessageType::Warning:
      fmt::print(fg(fmt::terminal_color::bright_black) | (fmt::emphasis::bold), "warning: ");
    break;

    case LoggerMessageType::Info:
      fmt::print(fg(fmt::terminal_color::blue) | (fmt::emphasis::bold), "info: ");
    break;
    }
    fmt::print(fg(fmt::terminal_color::white), "{}\n", message);
  }
private:
  LoggerMessageType type;

  std::string module;
  std::uint_fast32_t column;
  std::uint_fast32_t row;

  mutable std::vector<std::string> message;
};

MessageCollector::MessageCollector(Logger& log)
  : log(log)
{  }
MessageCollector::~MessageCollector()
try
{
  log.print(message);
}
catch(...)
{
  std::abort();
}

thread_local static Logger logger;

MessageCollector emit_error(const std::string& module, std::uint_fast32_t column, std::uint_fast32_t row)
{
  logger.update(LoggerMessageType::Error, module, column, row);
  
  return logger.collect();
}

MessageCollector emit_info(const std::string& module, std::uint_fast32_t column, std::uint_fast32_t row)
{
  logger.update(LoggerMessageType::Info, module, column, row);
  
  return logger.collect();
}

MessageCollector emit_warn(const std::string& module, std::uint_fast32_t column, std::uint_fast32_t row)
{
  logger.update(LoggerMessageType::Warning, module, column, row);
  
  return logger.collect();
}

const MessageCollector& operator<<(const MessageCollector& mc, const char* message)
{
  mc.message += message;
  return mc;
}

const MessageCollector& operator<<(const MessageCollector& mc, const std::string& message)
{
  mc.message += message;
  return mc;
}

