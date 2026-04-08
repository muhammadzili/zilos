# ZilOS Test Suite

## Structure

```
tests/
├── unit/           # Unit tests for isolated components
├── integration/   # Integration tests
└── test_main.cpp   # Test entry point
```

## Running Tests

```bash
make test          # Run test suite
```

## Test Categories

### Unit Tests
- Memory allocation
- String utilities
- Data structures

### Integration Tests
- Boot sequence
- Driver initialization
- File system operations
- Network stack

## Notes

- Tests run in kernel context when possible
- QEMU-based tests for full system testing
- CI runs all tests on every push
