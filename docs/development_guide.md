# Development Guide

## Getting Started

1. Clone the repository
2. Install prerequisites (Visual Studio, CMake, CUDA Toolkit)
3. Create a feature branch
4. Implement your module
5. Test your module
6. Create a pull request

## Module Implementation Checklist

### Before Starting
- [ ] Read the architecture document
- [ ] Understand your module's responsibilities
- [ ] Review the interface header in `include/`
- [ ] Set up development environment

### During Development
- [ ] Implement stub functions in `src/`
- [ ] Add proper error handling
- [ ] Use common types from `common/types.h`
- [ ] Follow C++ best practices (RAII, const correctness)
- [ ] Add logging for debugging
- [ ] Test on local machine

### Testing
- [ ] Write unit tests in `tests/`
- [ ] Test on Windows VM
- [ ] Test integration with other modules
- [ ] Document test results

### Before PR
- [ ] Code compiles without warnings
- [ ] All tests pass
- [ ] Update CMakeLists.txt if needed
- [ ] Update documentation
- [ ] Commit with clear message

## Coding Standards

### C++ Style
- Use C++17 features
- Follow Google C++ Style Guide (or team standard)
- Use RAII for resource management
- Prefer smart pointers over raw pointers
- Use const correctness

### Naming Conventions
- Classes: `PascalCase`
- Functions: `camelCase`
- Variables: `snake_case`
- Constants: `UPPER_SNAKE_CASE`
- Namespaces: `snake_case`

### Error Handling
- Return `common::Status` for operations that can fail
- Use exceptions sparingly (only for exceptional cases)
- Log errors with context
- Clean up resources on error

## Testing Strategy

### Unit Tests
- Test each function independently
- Mock external dependencies
- Cover edge cases and error paths

### Integration Tests
- Test module interactions
- Test end-to-end workflows
- Verify data flow between modules

### VM Testing
- Test on clean Windows VM
- Verify installer works correctly
- Check system tray and GUI functionality

## Common Issues

### NVML Not Found
- Install NVIDIA CUDA Toolkit
- Add CUDA bin directory to PATH
- Link against nvml.lib

### WinAPI Errors
- Check return values
- Use GetLastError() for debugging
- Refer to MSDN documentation

### CMake Configuration Issues
- Clear build directory and reconfigure
- Check CMake version (3.20+)
- Verify Visual Studio installation

## Resources

- [CMake Documentation](https://cmake.org/documentation/)
- [NVML API Reference](https://docs.nvidia.com/deploy/nvml-api/)
- [Windows API Documentation](https://docs.microsoft.com/en-us/windows/win32/api/)
- [nlohmann/json Documentation](https://json.nlohmann.me/)
