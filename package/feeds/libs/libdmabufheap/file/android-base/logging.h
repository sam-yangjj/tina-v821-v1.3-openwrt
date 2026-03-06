
#pragma once

#include <functional>
#include <memory>
#include <ostream>
#include <sstream>

#ifdef LOG_TAG
#define _LOG_TAG_INTERNAL LOG_TAG
#else
#define _LOG_TAG_INTERNAL nullptr
#endif

enum LogSeverity {
    VERBOSE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL_WITHOUT_ABORT,
    FATAL,
};

// Changing this definition will cause you a lot of pain.  A majority of
// vendor code defines LIKELY and UNLIKELY this way, and includes
// this header through an indirect path.
#define LIKELY( exp )       (__builtin_expect( (exp) != 0, true  ))
#define UNLIKELY( exp )     (__builtin_expect( (exp) != 0, false ))

// A macro to disallow the copy constructor and operator= functions
// This must be placed in the private: declarations for a class.
//
// For disallowing only assign or copy, delete the relevant operator or
// constructor, for example:
// void operator=(const TypeName&) = delete;
// Note, that most uses of DISALLOW_ASSIGN and DISALLOW_COPY are broken
// semantically, one should either use disallow both or neither. Try to
// avoid these in new code.
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;      \
  void operator=(const TypeName&) = delete


// Data for the log message, not stored in LogMessage to avoid increasing the
// stack size.
class LogMessageData;

class LogMessage {
public:
    LogMessage(LogSeverity severity, const char* tag, int error);
    ~LogMessage();

    // Returns the stream associated with the message, the LogMessage performs
    // output when it goes out of scope.
    std::ostream& stream();

    // The routine that performs the actual logging.
    static void LogLine(LogSeverity severity, const char* tag, const char* msg);

private:
    std::ostringstream buffer_;
    DISALLOW_COPY_AND_ASSIGN(LogMessage);
};

#define PLOG(severity) \
    LogMessage(severity, _LOG_TAG_INTERNAL, errno).stream()

#define LOG(severity) \
    PLOG(severity)

// Check whether condition x holds and LOG(FATAL) if not. The value of the
// expression x is only evaluated once. Extra logging can be appended using <<
// after. For example:
//
//     CHECK(false == true) results in a log message of
//       "Check failed: false == true".

#define CHECK(x)                               \
  LIKELY((x)) ||                               \
      LogMessage(FATAL, _LOG_TAG_INTERNAL, -1) \
              .stream()                        \
          << "Check failed: " #x << " "

