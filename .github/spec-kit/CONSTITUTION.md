# Spec Kit Constitution for desktop-gallery

**IMPORTANT: This project uses a hybrid SpecDD & Spec Kit architecture.**

## 1. System Intent & Single Source of Truth (SpecDD)
The architectural intent (MVC layering), global constraints (C++26 on Linux / C++23 on Windows, Qt6 threading rules), security norms, and directory ownership are governed strictly by **SpecDD** (`.sdd` files).

Do NOT rely on this Constitution file for system rules. Instead, you MUST read the `.sdd` files (e.g., `desktop-gallery.sdd`) and `.specdd/bootstrap.md` to understand what you are allowed to do.

## 2. Role of Spec Kit (Feature Intent)
Spec Kit is used exclusively for **Feature-Level Intent** (isolated changes, new geocoding adapters, EXIF/IPTC/XMP mapping rules, UI dialogs, bug fixes).
When implementing a Spec Kit feature:
- Read the feature's Markdown template or the GitHub Issue.
- Verify that your proposed changes do not violate any `Must not` or `Forbids` rules from the SpecDD hierarchy.
- Ensure all Code Quality baselines (SPDX MIT headers, English docs, `tr()` i18n strings, non-blocking GUI thread) are met.

## 3. Integration Reference
For a full breakdown of how these frameworks integrate, read:
[docs/SpecDD_SpecKit_Integration_EN.md](../../docs/SpecDD_SpecKit_Integration_EN.md)
