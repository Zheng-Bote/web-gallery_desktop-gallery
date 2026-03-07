<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Physical Decomposition Diagram](#physical-decomposition-diagram)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Physical Decomposition Diagram

```mermaid
graph TD
    App[Desktop-Gallery Executable]

    App --> Binaries[Compiled Binaries / DLLs]
    App --> Assets[Resource Files]
    App --> Config[Configuration Data]

    Binaries --> QtLibs[Qt6 Framework Libraries]
    Binaries --> Exiv2Lib[Exiv2 Metadata Library]

    Assets --> Icons["Icons & Logos (resources.qrc)"]
    Assets --> Translations["i18n / .qm Files"]

    Config --> DB[("gallery.db")]
    Config --> QSet["QSettings (.ini)"]
```
