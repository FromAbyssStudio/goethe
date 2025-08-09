## Goethe Engine (skeleton)

Minimal scaffold for the Goethe Engine described in the architectural overview. It builds a shared library exposing a C ABI and a tiny sample host.

Build:

```
cmake -S . -B build -DGOETHE_BUILD_SHARED=ON
cmake --build build -j
```

Run sample:

```
cmake --build build --target hello_vn
./build/samples/hello_vn/hello_vn
```

Options are in the top-level `CMakeLists.txt`; optional backends/deps default OFF for portability.

### Resources

- Visual novel overview and best practices: [How to Make Visual Novels](https://arimiadev.com/how-to-make-visual-novels/)


