<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Flowchart: Metadata Writing](#flowchart-metadata-writing)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Flowchart: Metadata Writing

```mermaid
flowchart TD
    A[User Edits Metadata in UI] --> B{Valid File?}
    B -- No --> C[Show Error Dialog]
    B -- Yes --> D[Open File with Exiv2]
    
    D --> E{Read Success?}
    E -- No --> C
    E -- Yes --> F[Parse UI Tables]
    
    F --> G[Extract EXIF/IPTC/XMP Arrays]
    G --> H["Sync Cross-Standard Metadata\n(e.g., Copyright -> dc:rights)"]
    H --> I[Write to Image Exiv2 Buffer]
    I --> J{Commit to Disk?}
    
    J -- Success --> K[Emit imageChanged Signal]
    J -- Exception --> L[Show Write Error]
    
    K --> M[Update Thumbnail Cache & UI]
```
