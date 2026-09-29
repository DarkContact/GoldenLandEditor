# GoldenLandEditor

- Don't try compile and run project!
- Don't use git!
- Don't run tests!
- Ignore `std::string aboutMessage` and `ImGui::Text("Root folder not specified!...` in file Application.cpp
- Use russian language in your thoughts and in those parts that interact with me as a user.

## Project Overview

GoldenLandEditor is tool for viewing and editing game resources for Russian CRPG "Goldenland 2".
Built with C++20. Use libraries SDL3 (For creating the main application window and render graphics) and ImGui (For GUI).

## Development Conventions

### Code Style

- Use `PascalCase` for classes and structs.
- Use `camelCase` for member variables and functions.
- Use `camelCase` for local variables.
- Use `m_` prefix for private members.
- Comments on russian language.
- Header guards are implemented with `#pragma once`.

### Project Structure

- **`src/`** Contains all the source code for the editor.
  - **`windows/`** UI viewers for different file types (e.g., `LevelViewer`, `CsxViewer`).
  - **`parsers/`** Parsers for various Goldenland file formats.
  - **`utils/`** Utility classes and functions (file I/O, string manipulation, etc.).
  - **`graphics/`** Graphics-related classes (textures, animations, etc.).
- **`external/`** Holds third-party libraries
- **`CMakeLists.txt`** The main CMake build script.
