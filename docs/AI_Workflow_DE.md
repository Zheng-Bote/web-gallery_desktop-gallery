<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [AI-Agent Instruktionen: SpecDD × GitHub Spec Kit Workflow](#ai-agent-instruktionen-specdd-%C3%97-github-spec-kit-workflow)
  - [1. Rollenverteilung & Hierarchie](#1-rollenverteilung--hierarchie)
  - [2. Der Agentic Workflow (Flow-Forward)](#2-der-agentic-workflow-flow-forward)
    - [Schritt 1: Specify (Requirements)](#schritt-1-specify-requirements)
    - [Schritt 2: Plan (Design & Architektur-Check)](#schritt-2-plan-design--architektur-check)
    - [Schritt 3: Tasks (Arbeitspakete definieren)](#schritt-3-tasks-arbeitspakete-definieren)
    - [Schritt 4: Implement (Code generieren)](#schritt-4-implement-code-generieren)
    - [Schritt 5: Converge (Überprüfung & Iteration)](#schritt-5-converge-%C3%9Cberpr%C3%BCfung--iteration)
    - [Schritt 6: Release (Abschluss)](#schritt-6-release-abschluss)
  - [3. Artefakt- und Lifecycle-Übersicht](#3-artefakt--und-lifecycle-%C3%9Cbersicht)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# AI-Agent Instruktionen: SpecDD × GitHub Spec Kit Workflow

Dieses Dokument bricht die Vorgaben aus `docs/SDD-Howto_en.md` und `docs/SpecDD_SpecKit_Integration_EN.md` in konkrete, ausführbare Prozessschritte für KI-Agenten herunter. Es dient als Handlungsleitfaden für die Feature-Entwicklung im `desktop-gallery` (`web-gallery_desktop-gallery`) Projekt.

## 1. Rollenverteilung & Hierarchie

- **SpecDD (`.sdd`) = Die Verfassung:** Single Source of Truth für Architektur (MVC-Layering), C++26/Qt6-Richtlinien, Thread-Safety, Datenbank (`DatabaseManager`) und Security. Ändert sich selten. Darf von Features **nicht** gebrochen werden.
- **Spec Kit (Markdown) = Der Arbeitsauftrag:** Flow-Forward Feature-Entwicklung (z. B. neuer Geocoding-Adapter, neues EXIF/IPTC/XMP-Mapping, UI-Erweiterung). Iterativ, detailliert und auf ein spezifisches Ziel fokussiert.

## 2. Der Agentic Workflow (Flow-Forward)

KI-Agenten arbeiten Features sequenziell nach folgendem Muster ab, um Code-Drift zu vermeiden:

### Schritt 1: Specify (Requirements)
*   **Auslöser:** `/speckit.specify`
*   **KI-Aktion:**
    *   Feature-Branch erstellen (z. B. `feature/ticket-123`).
    *   Speicherort für Spec Kit bestimmen:
        *   Schichtübergreifend (betrifft UI, Business-Logik, Persistenz): `specs/features/<name>/`.
        *   Isoliertes Subsystem (z. B. nur `regeocode`): `specs/<modul>/<name>/` oder `specs/features/<name>/`.
    *   **Ergebnis:** Eine Markdown-Feature-Spec mit klaren Akzeptanzkriterien (PRD).

### Schritt 2: Plan (Design & Architektur-Check)
*   **Auslöser:** `/speckit.plan`
*   **KI-Aktion:**
    *   **Spec Chain auflösen:** Identifiziere den *Target-Pfad* (Scope) des Features (z. B. `include/regeocode/`, `src/regeocode/`).
    *   Lies die Spec-Chain hierarchisch von Root (`desktop-gallery.sdd`) über alle via `References` verknüpften Architektur-Specs (`docs/architecture/*.md`) bis zum Zielmodul ein.
    *   **Architektur-Check:** Prüfe den Feature-Entwurf gegen alle geerbten Constraints (Werden MVC-Layer-Grenzen respektiert? Blockiert keine Operation den Qt-GUI-Thread? Werden Prepared Statements im `DatabaseManager` genutzt?).
    *   **Ergebnis:** Ein Architektur-Plan (Technical Spec), der das „Wie" strikt innerhalb der vererbten SpecDD-Grenzen entwirft.

### Schritt 3: Tasks (Arbeitspakete definieren)
*   **Auslöser:** `/speckit.tasks`
*   **KI-Aktion:**
    *   Plan in atomare Arbeitspakete (Tasks) zerlegen.
    *   **Pflicht:** Jeder Task MUSS die betroffenen Dateien (`include/`, `src/`, `i18n/`, `CMakeLists.txt`) explizit benennen.
    *   **Pflicht:** Reihenfolge beachten: Header, Interfaces & Signal/Slot-Verträge zuerst, danach Implementierung, UI/Übersetzungen und Build-Integration.
    *   **Ergebnis:** Eine geordnete Liste sequenziell abarbeitbarer Agent-Tasks.

### Schritt 4: Implement (Code generieren)
*   **Auslöser:** `/speckit.implement`
*   **KI-Aktion:**
    *   Abarbeiten des nächsten offenen Tasks.
    *   Code schreiben und Build via CMake (`cmake --build build`) verifizieren.
    *   **Code-Standard-Check (Zwingendes Quality Gate für die KI):**
        *   [ ] Sind Doku, Doxygen-Tags und Kommentare auf Englisch?
        *   [ ] Hat jede Quell- und Header-Datei den SPDX-Header (`SPDX-License-Identifier: MIT`)?
        *   [ ] Sind sichtbare UI-Strings in `tr()` gekapselt und in `i18n/gallery_de.ts` / `gallery_en.ts` gepflegt?
        *   [ ] Bleibt der Qt-GUI-Hauptthread frei von blockierendem I/O?
        *   [ ] Sind keine API-Keys oder JWT-Secrets im Quellcode hinterlegt?
    *   **Ergebnis:** Code-Deltas für den Review.

### Schritt 5: Converge (Überprüfung & Iteration)
*   **Auslöser:** `/speckit.converge`
*   **KI-Aktion:**
    *   Vergleich des IST-Zustands (Code) mit dem SOLL-Zustand (Spec Kit Feature Spec + SpecDD `.sdd` Vorgaben).
    *   Prüfung: Fehlt Error-Handling? Sind neue Header mit `Q_OBJECT` in `CMakeLists.txt` registriert? Wurden EXIF/IPTC/XMP-Synchronisationsregeln beachtet?
    *   Falls unvollständig: Neue Tasks generieren, an die Taskliste anhängen und zurück zu *Implement*.
    *   Falls vollständig: Feature ist logisch abgeschlossen.

### Schritt 6: Release (Abschluss)
*   **KI-Aktion:**
    *   `CHANGELOG.md` und ggf. Architekturdiagramme in `docs/architecture/` aktualisieren.
    *   Vorbereitung für Push & Pull Request.

---

## 3. Artefakt- und Lifecycle-Übersicht

Diese Matrix vereint den abstrakten Entwicklungszyklus mit den konkreten SpecKit-Kommandos und Artefakten:

| Phase | KI-Befehl | Haupt-Artefakt | KI-Quality Gate (Check) | Resultat |
| :--- | :--- | :--- | :--- | :--- |
| **1. Intake & Requirements** | `/speckit.specify` | Feature Spec (Markdown) | Problem, Scope, User & Non-Goals sind klar definiert und testbar. | KI weiß, *was* umgesetzt werden soll. |
| **2. Design & Architecture** | `/speckit.plan` | Architekturentwurf | Abgleich mit `.sdd` (SpecDD) ist konfliktfrei; MVC, Threading & Security bestätigt. | KI weiß, *wie* das System gebaut wird. |
| **3. Tasking** | `/speckit.tasks` | Agent Task Plan | Tasks sind klein, dateispezifisch (`include/`, `src/`, `CMakeLists.txt`), bounded und reviewbar. | KI hat kontrollierte Arbeitspakete. |
| **4. Implementation** | `/speckit.implement` | Code & Build | Code erfüllt SpecDD-Baselines (SPDX MIT, English, `tr()`, Non-Blocking UI) exakt. | Feature wird realisiert. |
| **5. Testing & Review** | `/speckit.converge` | Delta / Review Report | Code matcht Feature-Specs und Architektur-Regeln restlos; CMake-Build ist grün. | Abweichungen (Drift) eliminiert. |
| **6. Release & Learning** | *(Manuell / CI)* | Changelog, PR | `CHANGELOG.md` ist aktuell, plattformübergreifender Build (Linux C++26 / Windows C++23) ist sichergestellt. | Feature ist merge-bereit. |
