#pragma once
#include <string>

namespace td::core {

enum class Severity { Error, Warning };

// One message about one problem: what is wrong, where, and how serious it is.
class Diagnostic {
public:
    static Diagnostic error(std::string code, std::string fieldPath, std::string message);
    static Diagnostic warning(std::string code, std::string fieldPath, std::string message);

    Severity severity() const { return severity_; }
    const std::string& code() const { return code_; }            // machine-readable: "outOfRange"
    const std::string& fieldPath() const { return fieldPath_; }  // "waves[2].groups[0].count"
    const std::string& message() const { return message_; }

private:
    Diagnostic(Severity severity, std::string code, std::string fieldPath, std::string message);

    Severity severity_;
    std::string code_;
    std::string fieldPath_;
    std::string message_;
};

}  // namespace td::core
