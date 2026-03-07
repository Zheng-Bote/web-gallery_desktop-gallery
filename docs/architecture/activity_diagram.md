<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Activity Diagram: Background Image Indexing](#activity-diagram-background-image-indexing)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Activity Diagram: Background Image Indexing

```mermaid
stateDiagram-v2
    [*] --> Init: User clicks Folder
    Init --> ScanFiles: ImageIndexer run() started in thread
    ScanFiles --> ProcessFile: For each file in tree
    
    state ProcessFile {
        [*] --> CheckDB: File already in DB?
        CheckDB --> ExtractMeta: No
        CheckDB --> NextFile: Yes
        
        ExtractMeta --> ReadGPS: Photo getGpsData()
        ReadGPS --> ReadCopyright: Photo getCopyright()
        ReadCopyright --> InsertDB: DBManager insertImage
        InsertDB --> CheckFilter: Matches current filter?
        
        CheckFilter --> NotifyUI: Yes -> emit modelUpdated()
        CheckFilter --> NextFile: No
    }
    
    NextFile --> ProcessFile: More files?
    NextFile --> Finish: No more files
    Finish --> [*]: Thread ends execution
```
