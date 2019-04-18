#include <log.hpp>
#include <util.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

enum class LoggerMessageType
{
  Error,
  Warning,
  Info
};

class Logger
{
public:
  Logger()
  { spdlog::set_pattern("%v"); }

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
    // print message
    switch(type)
    {
    case LoggerMessageType::Error:
      spdlog::error("{}:{}:{}: error: {}", module, column, row, message);
    break;

    case LoggerMessageType::Warning:
      spdlog::warn("{}:{}:{}: warning: {}", module, column, row, message);
    break;

    case LoggerMessageType::Info:
      spdlog::info("{}:{}:{}: info: {}", module, column, row, message);
    break;
    }
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

const MessageCollector& operator<<(const MessageCollector& mc, const char* message)
{
  mc.message += message;
  return mc;
}

