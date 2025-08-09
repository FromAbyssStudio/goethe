# Contributing to Goethe Engine

Thank you for considering contributing to Goethe Engine! Please follow these guidelines to help us maintain a clean and efficient workflow.

## Getting Started

1. Fork the repository and create your branch from `main`.
2. Build the project:
   ```
   cmake -S . -B build
   cmake --build build
   ```
3. Run tests:
   ```
   ctest --test-dir build
   ```

## Pull Requests

- Fill out the pull request template.
- Include tests for new features and fixes when possible.
- Ensure `cmake` and `ctest` run without errors before submitting.

## Code Style

- Follow modern C++20 practices.
- Prefer clarity over cleverness.

We appreciate your contributions!
