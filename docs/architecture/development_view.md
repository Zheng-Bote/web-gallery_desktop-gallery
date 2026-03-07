<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Development View Diagram](#development-view-diagram)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Development View Diagram

```mermaid
graph TD
    SourceTree[Project Source Tree]

    SourceTree --> Include["include/ (*.hpp)"]
    SourceTree --> Src["src/ (*.cpp)"]
    SourceTree --> I18n["i18n/ (*.ts)"]
    SourceTree --> Res["resources/ (*.png, .qrc)"]
    SourceTree --> Docs["docs/architecture/"]

    Include --> API[Class Interfaces]
    Src --> Impl[Class Implementations]
    
    API -->|Implements| Impl

    Build[CMake Build System]
    Build -->|Reads| Include
    Build -->|Compiles| Src
    Build -->|Packs| Res
    Build -->|Links| Libs["Qt6, Exiv2"]
    
    Build -->|Outputs| Executable["Desktop-Gallery Binary"]
```
