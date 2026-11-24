#include "td/core/Diagnostic.h"

#include <algorithm>
#include <iterator>
#include <utility>

#include "td/core/DiagnosticList.h"

namespace td::core {

Diagnostic::Diagnostic(Severity severity, std::string code, std::string fieldPath,
                       std::string message)
    : severity_(severity),
      code_(std::move(code)),
      fieldPath_(std::move(fieldPath)),
      message_(std::move(message)) {}

Diagnostic Diagnostic::error(std::string code, std::string fieldPath, std::string message) {
    return Diagnostic(Severity::Error, std::move(code), std::move(fieldPath), std::move(message));
}

Diagnostic Diagnostic::warning(std::string code, std::string fieldPath, std::string message) {
    return Diagnostic(Severity::Warning, std::move(code), std::move(fieldPath), std::move(message));
}

void DiagnosticList::add(Diagnostic d) { items_.push_back(std::move(d)); }

void DiagnosticList::merge(const DiagnosticList& other) {
    items_.insert(items_.end(), other.items_.begin(), other.items_.end());
}

bool DiagnosticList::hasErrors() const {
    return std::any_of(items_.begin(), items_.end(),
                       [](const Diagnostic& d) { return d.severity() == Severity::Error; });
}

std::vector<Diagnostic> DiagnosticList::errors() const {
    std::vector<Diagnostic> out;
    std::copy_if(items_.begin(), items_.end(), std::back_inserter(out),
                 [](const Diagnostic& d) { return d.severity() == Severity::Error; });
    return out;
}

std::vector<Diagnostic> DiagnosticList::warnings() const {
    std::vector<Diagnostic> out;
    std::copy_if(items_.begin(), items_.end(), std::back_inserter(out),
                 [](const Diagnostic& d) { return d.severity() == Severity::Warning; });
    return out;
}

}  // namespace td::core
