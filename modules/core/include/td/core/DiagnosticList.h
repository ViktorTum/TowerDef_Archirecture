#pragma once
#include <cstddef>
#include <vector>

#include "td/core/Diagnostic.h"

namespace td::core {

// Validators add every problem they find, so a developer sees all of them in
// one pass instead of fixing them one at a time.
class DiagnosticList {
public:
    void add(Diagnostic d);
    void merge(const DiagnosticList& other);

    bool hasErrors() const;
    std::vector<Diagnostic> errors() const;
    std::vector<Diagnostic> warnings() const;

    bool empty() const { return items_.empty(); }
    std::size_t size() const { return items_.size(); }

    std::vector<Diagnostic>::const_iterator begin() const { return items_.begin(); }
    std::vector<Diagnostic>::const_iterator end() const { return items_.end(); }

private:
    std::vector<Diagnostic> items_;
};

}  // namespace td::core
