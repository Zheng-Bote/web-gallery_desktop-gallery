<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Spec Kit Constitution for desktop-gallery](#spec-kit-constitution-for-desktop-gallery)
  - [1. System Intent & Single Source of Truth (SpecDD)](#1-system-intent--single-source-of-truth-specdd)
  - [2. Role of Spec Kit (Feature Intent)](#2-role-of-spec-kit-feature-intent)
  - [3. Integration Reference](#3-integration-reference)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

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
