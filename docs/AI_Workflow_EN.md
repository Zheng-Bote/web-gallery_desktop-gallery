# AI Agent Instructions: SpecDD × GitHub Spec Kit Workflow

This document breaks down the guidelines from `docs/SDD-Howto_en.md` and `docs/SpecDD_SpecKit_Integration_EN.md` into concrete, executable process steps for AI agents. It serves as a practical guide for feature development in the `desktop-gallery` (`web-gallery_desktop-gallery`) project.

## 1. Role Distribution & Hierarchy

- **SpecDD (`.sdd`) = The Constitution:** Single Source of Truth for architecture (MVC layering), C++26/Qt6 guidelines, thread safety, database (`DatabaseManager`), and security. Changes rarely. Must **not** be violated by features.
- **Spec Kit (Markdown) = The Work Order:** Flow-Forward feature development (e.g., new geocoding adapter, new EXIF/IPTC/XMP mapping, UI enhancement). Iterative, detailed, and focused on a specific goal.

## 2. The Agentic Workflow (Flow-Forward)

AI agents process features sequentially according to the following pattern to prevent code drift:

### Step 1: Specify (Requirements)
*   **Trigger:** `/speckit.specify`
*   **AI Action:**
    *   Create a feature branch (e.g., `feature/ticket-123`).
    *   Determine the storage location for the Spec Kit:
        *   Cross-Layer (affects UI, Business Logic, Persistence): `specs/features/<name>/`.
        *   Isolated subsystem (e.g., `regeocode` only): `specs/<module>/<name>/` or `specs/features/<name>/`.
    *   **Result:** A Markdown feature spec with clear acceptance criteria (PRD).

### Step 2: Plan (Design & Architecture Check)
*   **Trigger:** `/speckit.plan`
*   **AI Action:**
    *   **Resolve Spec Chain:** Identify the *Target Path* (Scope) of the feature (e.g., `include/regeocode/`, `src/regeocode/`).
    *   Read the spec chain hierarchically from root (`desktop-gallery.sdd`) through all architecture specs linked via `References` (`docs/architecture/*.md`) down to the target module.
    *   **Architecture Check:** Validate the feature design against all inherited constraints (Are MVC layer boundaries respected? Does any operation block the Qt GUI thread? Are prepared statements used in `DatabaseManager`?).
    *   **Result:** An architecture plan (Technical Spec) that strictly designs the "how" within the inherited SpecDD boundaries.

### Step 3: Tasks (Define Work Packages)
*   **Trigger:** `/speckit.tasks`
*   **AI Action:**
    *   Break down the plan into atomic work packages (Tasks).
    *   **Mandatory:** Every task MUST explicitly name the affected files (`include/`, `src/`, `i18n/`, `CMakeLists.txt`).
    *   **Mandatory:** Observe ordering: Headers, interfaces & Signal/Slot contracts first, followed by implementation, UI/translations, and CMake build integration.
    *   **Result:** An ordered list of sequentially executable Agent Tasks.

### Step 4: Implement (Generate Code)
*   **Trigger:** `/speckit.implement`
*   **AI Action:**
    *   Execute the next open task.
    *   Write code and verify compilation via CMake (`cmake --build build`).
    *   **Code Standard Check (Mandatory Quality Gate for AI):**
        *   [ ] Are documentation, Doxygen tags, and comments in English?
        *   [ ] Does every source and header file have the SPDX header (`SPDX-License-Identifier: MIT`)?
        *   [ ] Are user-facing UI strings wrapped in `tr()` and maintained in `i18n/gallery_de.ts` / `gallery_en.ts`?
        *   [ ] Is the Qt GUI main thread kept completely free of blocking I/O?
        *   [ ] Are API keys and JWT secrets kept out of source code?
    *   **Result:** Code deltas for review.

### Step 5: Converge (Review & Iteration)
*   **Trigger:** `/speckit.converge`
*   **AI Action:**
    *   Compare the ACTUAL state (Code) with the TARGET state (Spec Kit Feature Spec + SpecDD `.sdd` rules).
    *   Verification: Is error handling missing? Are new headers with `Q_OBJECT` registered in `CMakeLists.txt`? Were EXIF/IPTC/XMP synchronization rules respected?
    *   If incomplete: Generate new tasks, append to the task list, and return to *Implement*.
    *   If complete: Feature is logically finished.

### Step 6: Release (Completion)
*   **AI Action:**
    *   Update `CHANGELOG.md` and, if applicable, architecture diagrams in `docs/architecture/`.
    *   Preparation for Push & Pull Request.

---

## 3. Artifact and Lifecycle Overview

This matrix combines the abstract development cycle with concrete SpecKit commands and artifacts:

| Phase | AI Command | Main Artifact | AI Quality Gate (Check) | Result |
| :--- | :--- | :--- | :--- | :--- |
| **1. Intake & Requirements** | `/speckit.specify` | Feature Spec (Markdown) | Problem, Scope, User & Non-Goals are clearly defined and testable. | AI knows *what* to implement. |
| **2. Design & Architecture** | `/speckit.plan` | Architecture Design | Alignment with `.sdd` (SpecDD) is conflict-free; MVC, Threading & Security confirmed. | AI knows *how* to build the system. |
| **3. Tasking** | `/speckit.tasks` | Agent Task Plan | Tasks are small, file-specific (`include/`, `src/`, `CMakeLists.txt`), bounded, and reviewable. | AI has controlled work packages. |
| **4. Implementation** | `/speckit.implement` | Code & Build | Code strictly fulfills SpecDD baselines (SPDX MIT, English, `tr()`, Non-Blocking UI). | Feature is realized. |
| **5. Testing & Review** | `/speckit.converge` | Delta / Review Report | Code matches Feature Specs and Architecture Rules completely; CMake build passes. | Deviations (Drift) eliminated. |
| **6. Release & Learning** | *(Manual / CI)* | Changelog, PR | `CHANGELOG.md` is up to date, cross-platform build (Linux C++26 / Windows C++23) is ensured. | Feature is ready to merge. |
