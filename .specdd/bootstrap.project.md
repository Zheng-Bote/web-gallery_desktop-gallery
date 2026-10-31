<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [SpecDD project specific overrides](#specdd-project-specific-overrides)
  - [Spec Kit Integration](#spec-kit-integration)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# SpecDD project specific overrides

## Spec Kit Integration
- SpecDD remains the Single Source of Truth for system architecture (MVC layering, multithreading), global governance, and security.
- GitHub Spec Kit is used for feature-level specs (e.g. adding a new reverse geocoding adapter, new EXIF/IPTC/XMP mappings, UI enhancements, or upload/export features).
- Spec Kit features MUST NOT violate SpecDD `Must not` or `Forbids` constraints.
- For more details on the integration and mapping, refer to `docs/SpecDD_SpecKit_Integration_EN.md` (or the `_DE` version).
- For instructions on how AI Agents should interact with the SpecDD framework and resolve constraints, refer to `docs/AI_Workflow_DE.md`.
- For instructions on how AI Agents should interact with the SpecDD framework and resolve constraints, refer to `docs/AI_Workflow_EN.md` (or the DE equivalent).
