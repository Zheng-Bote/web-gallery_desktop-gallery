<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Reverse Geocoding](#reverse-geocoding)
  - [API](#api)
  - [welche Metadaten in welchen Namespace (Adressdaten schreiben)](#welche-metadaten-in-welchen-namespace-adressdaten-schreiben)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Reverse Geocoding

## API

**Nominatim**

Hinweise zur Benutzung
User-Agent: Ich habe DesktopGalleryApp/1.0 gesetzt. Nominatim sieht es gerne, wenn dort eine echte Kontaktmöglichkeit steht (z.B. eine URL oder E-Mail, falls die App öffentlich verteilt wird), aber für den privaten Gebrauch reicht der App-Name meist aus.

Felder-Logik: OSM ist sehr granular. Ein Ort kann ein city, town, village oder hamlet sein. Mein Code prüft diese Reihenfolge automatisch, sodass immer der sinnvollste Ortsname im IPTC/XMP City Feld landet.

Country: OSM liefert sowohl country (Name) als auch country_code (ISO). Ich schreibe beides in die passenden Felder (CountryName vs CountryCode).

## welche Metadaten in welchen Namespace (Adressdaten schreiben)

Das ist eine sehr wichtige Frage, da verschiedene Programme unterschiedliche Felder bevorzugen.

Im Code (Methode onGeoNamesReply in MainWindow.cpp) habe ich eine "Breitband-Strategie" implementiert. Das bedeutet, die Daten werden gleichzeitig in IPTC (Legacy Standard) und XMP (Moderner Standard/Adobe Photoshop) geschrieben, um maximale Kompatibilität sicherzustellen.

Hier ist die genaue Zuordnung, die aktuell im Code steht:

1. Straße & Hausnummer
   IPTC: Iptc.Application2.SubLocation

XMP (IPTC Core): Xmp.iptc.Location

XMP (Contact Info): Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrExtadr

2. Stadt (City)
   IPTC: Iptc.Application2.City

XMP (Photoshop): Xmp.photoshop.City

XMP (Contact Info): Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCity

3. Bundesland / Provinz (State)
   IPTC: Iptc.Application2.ProvinceState

XMP (Photoshop): Xmp.photoshop.State

XMP (Contact Info): Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrRegion

4. Ländercode (Country Code)
   IPTC: Iptc.Application2.CountryCode

XMP (IPTC Core): Xmp.iptc.CountryCode

XMP (Contact Info): Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCtry

5. Postleitzahl (ZIP)
   XMP (Contact Info): Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrPcode (Hinweis: Das alte IPTC IIM Format hat kein dediziertes, standardisiertes PLZ-Feld, das von allen Programmen gleich interpretiert wird, daher landet die PLZ primär im XMP).

Warum so viele Felder?

Ältere Programme lesen oft nur IPTC.

Adobe Produkte (Lightroom, Photoshop) bevorzugen die Xmp.photoshop._ und Xmp.iptc._ Namespaces.

Durch das Schreiben in alle Felder stellst du sicher, dass die Adresse später sowohl im Windows Explorer, als auch in Lightroom oder Online-Galerien auftaucht.
