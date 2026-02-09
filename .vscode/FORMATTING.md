# C/C++ Code Formatting Configuration

This directory contains cross-platform formatting configuration for the iModel Native codebase.

## Files

- **`.clang-format`** - ClangFormat configuration based on Google C++ Style Guide
- **`.editorconfig`** - Universal editor configuration (works with VS Code, Visual Studio, CLion, etc.)
- **`.vscode/settings.json`** - VS Code specific settings

## Formatting Rules

- **Indentation**: 4 spaces (no tabs)
- **Line Length**: No limit (ColumnLimit: 0)
- **Line Endings**: LF (Unix style) on all platforms
- **Pointer Alignment**: Left (`int* ptr`)
- **Brace Style**: Attached (Google style)

## Platform Support

These configurations work consistently across:
- **Windows** - Visual Studio Code, Visual Studio, CLion
- **Linux** - Visual Studio Code, CLion, Vim, Emacs
- **macOS** - Visual Studio Code, Xcode, CLion

## Auto-Formatting in VS Code

Formatting is automatically applied:
- **On Save** - Files are formatted when saved
- **On Paste** - Pasted code is formatted
- **On Type** - Code is formatted as you type

## Manual Formatting

### VS Code
- Format document: `Shift+Alt+F` (Windows/Linux) or `Shift+Option+F` (macOS)
- Format selection: Select code, then use the format command

### Command Line
```bash
# Format a single file
clang-format -i --style=file path/to/file.cpp

# Format all C/C++ files in a directory
find iModelCore/ECDb -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-format -i --style=file {} +
```

## Requirements

- **VS Code**: Install the `C/C++` extension (ms-vscode.cpptools)
- **clang-format**: Included with:
  - Visual Studio (Windows)
  - Xcode Command Line Tools (macOS)
  - clang package (Linux: `apt install clang-format` or `yum install clang-tools-extra`)

## Troubleshooting

If formatting doesn't work:
1. Ensure the C/C++ extension is installed in VS Code
2. Check that clang-format is installed (`clang-format --version`)
3. Verify the .clang-format file exists in the workspace root
4. Check VS Code settings for `C_Cpp.formatting` is set to `"clangFormat"`
