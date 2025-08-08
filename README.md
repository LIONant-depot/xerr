# xerr: Lightweight C++ Error Handling

**xerr** is a modern, constexpr-friendly C++ library for super efficient, type-safe error handling. 
Built for performance and simplicity, it offers customizable error states and compile-time safety with no dependencies.

## Key Features

* **Type-Safe Errors**: Enum-based states with compile-time checks.
* **Constexpr Support**: Zero-overhead error creation and querying.
* **No Dependencies**: Just include `source/xerr.h`.
* **Custom Messages**: Compile-time string literals for clear errors.
* **Header only**: Header only library.
* **MIT License**: Free to use and modify.

## Getting Started

Add `source/xerr.h` to your project and start using it!

## Code Example

```cpp
#include "source/xerr.h"
#include <cassert>

// Simple file handler class
class FileHandler {
public:
    // Custom error enum
    enum class Error : std::uint8_t 
    { OK            // These two are required by the system
    , FAILURE       // These two are required by the system
    , NOT_FOUND     // Custom...
    };

    constexpr xerr open(const std::string& filename) noexcept {
        if (filename.empty()) return xerr::create<Error::NOT_FOUND, "Empty filename">();
        m_isOpen = true;
        return {};
    }
private:
    bool m_isOpen = false;
};

int main() {
    FileHandler file;
    if( auto Err = file.open(""); Err )
    {
        std::cout << Err.m_pMessage << "\n";_
        assert(Err.getState<FileHandler::Error>() == FileHandler::Error::NOT_FOUND);
    }
    return 0;
}
```

## Contributing

Star, fork, and contribute to xerr on [github xerr](https://github.com/LIONant-depot/xerr)!