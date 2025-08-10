# Advanced Usage of xerr

This guide covers **xerr**’s advanced features: error chaining, RAII cleanup, debugging, and custom enums. For basics, see [Getting Started](getting-started.md).

## Error Chaining
Chain errors to track causes:
```cpp
enum class Error : std::uint8_t { OK, FAILURE, NOT_FOUND, INVALID };

xerr low_level() {
    return xerr::create<Error::NOT_FOUND, "File not found|Check path">();
}

xerr high_level() {
    if (auto err = low_level(); err) {
        return xerr::create<Error::INVALID, "Processing failed|Retry operation">(err);
    }
    return {};
}

int main() {
    if (auto err = high_level(); err) {
        xerr::ForEachInChain([](const xerr& e) {
            printf("Error: %s, Hint: %s\n", e.getMessage().data(), e.getHint().data());
        });
        // Outputs:
        // Error: File not found, Hint: Check path
        // Error: Processing failed, Hint: Retry operation
    }
}
```

**Note**: `ForEachInChain` iterates oldest to newest (root cause to latest). `ForEachInChainBackwards` iterates newest to oldest.

## RAII Cleanup
Use `xerr::cleanup` for automatic resource cleanup:
```cpp
xerr process_file(const char* path) {
    FILE* fp = fopen(path, "r");
    xerr err;
    xerr::cleanup s(err, [&fp] { if (fp) fclose(fp); });
    if ((err = open_file(path))) return err;
    return {};
}
```

## Debugging
Set a callback to log errors:
```cpp
void log_error(const char* func, std::uint8_t state, const char* msg) {
    printf("Error in %s: state=%u, msg=%s, hint=%s\n", func, state, msg, xerr{msg}.getHint().data());
}

int main() {
    xerr::m_pCallback = log_error;
    if (auto err = open_file(""); err) {
        // Logs: Error in main: state=2, msg=File not found|Check path, hint=Check path
    }
}
```

**Note**: Callbacks should take `const xerr&` to ensure safety.

## Custom Enums
Extend `default_states`:
```cpp
enum class MyError : std::uint8_t {
    OK = 0,      // Required
    FAILURE = 1, // Required
    TIMEOUT
};

xerr operation() {
    return xerr::create<MyError::TIMEOUT, "Operation timed out|Try again later">();
}
```

## Next Steps
- See API details in [API Reference](api-reference.md).
- Explore performance in [Performance](performance.md).
- Contribute at [Contributing](contributing.md).