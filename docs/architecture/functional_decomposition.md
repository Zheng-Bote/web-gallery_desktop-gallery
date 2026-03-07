<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Functional Decomposition Diagram](#functional-decomposition-diagram)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Functional Decomposition Diagram

```mermaid
graph TD
    System[Desktop-Gallery System]

    System --> F1[File Management]
    System --> F2[Metadata Management]
    System --> F3[Image Processing]
    System --> F4[Cloud Integration]

    F1 --> F1_1[Recursive Indexing]
    F1 --> F1_2[Database Caching]
    F1 --> F1_3[Bulk Renaming]

    F2 --> F2_1[Read Exiv2 Data]
    F2 --> F2_2[Write Exiv2 Data]
    F2 --> F2_3[Standard Syncing]

    F3 --> F3_1[Thumbnail Generation]
    F3 --> F3_2[WebP Conversion]
    F3 --> F3_3[Watermarking]

    F4 --> F4_1[JWT Authentication]
    F4 --> F4_2[Multipart Upload]
    F4 --> F4_3[Progress Reporting]
```
