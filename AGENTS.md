# GoldenLandEditor

- Don't try compile and run project!
- Don't use git!
- Don't run tests!
- Ignore `std::string aboutMessage` and `ImGui::Text("Root folder not specified!...` in file Application.cpp
- Use russian language in your thoughts and in those parts that interact with me as a user.

## Project Overview

GoldenLandEditor is open-source tool for viewing and editing game resources for Russian CRPG "Goldenland 2".
Built with C++20 with following libraries:

- **SDL3:** For creating the main application window and handling events.
- **ImGui:** To create a user-friendly and feature-rich graphical interface.
- **stb_image:** For loading a wide variety of image formats.

## Development Conventions

### Code Style

- The project uses a consistent, modern C++ style.
  Please adhere to the existing formatting and naming conventions when contributing.
- Use `PascalCase` for classes and structs.
- Use `camelCase` for member variables and functions.
- Use `camelCase` for local variables.
- Use `m_` prefix for private members.
- Comments on russian language.
- Header guards are implemented with `#pragma once`.

### Project Structure

- **`src/`** Contains all the source code for the editor.
  - **`windows/`**  UI viewers for different file types (e.g., `LevelViewer`, `CsxViewer`).
  - **`parsers/`**  Parsers for various Goldenland file formats.
  - **`utils/`**  Utility classes and functions (file I/O, string manipulation, etc.).
  - **`graphics/`**  Graphics-related classes (textures, animations, etc.).
- **`external/`**  Holds third-party libraries like SDL3, ImGui, and stb_image.
- **`test/`** Contains tests (Used google-test).
- **`docs/`** Documentation about in-game resources and script language description.
- **`CMakeLists.txt`** The main CMake build script.
