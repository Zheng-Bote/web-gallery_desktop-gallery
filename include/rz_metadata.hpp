/**
 * @file rz_metadata.hpp
 * @author ZHENG Robert (robert.hase-zheng.net)
 * @brief Central definition of all Metadata Tags, Defaults and Mappings
 * @version 0.7 (Fixed Sync Holes)
 */
#pragma once

#include <QHash>
#include <QString>
#include <QMap>

namespace RzMetadata {

    // --- STRUCTS FÜR DEFAULT META WIDGET ---
    struct DefaultMetaStruct {
        QHash<QString, QString> xmpDefault{};
        QHash<QString, QString> exifDefault{};
        QHash<QString, QString> iptcDefault{};
    };

    // --- MAPPING TABELLEN ---
    
    // EXIF -> XMP
    static const QHash<QString, QString> exif_to_xmp = {
        {"Exif.Image.Copyright", "Xmp.dc.rights"}, 
        {"Exif.Image.Artist", "Xmp.dc.creator"},
        {"Exif.Image.ImageDescription", "Xmp.dc.description"},
        {"Exif.Photo.UserComment", "Xmp.dc.description"}, 
        {"Exif.Image.Software", "Xmp.xmp.CreatorTool"},
        {"Exif.Photo.DateTimeOriginal", "Xmp.photoshop.DateCreated"} 
    };

    // EXIF -> IPTC
    static const QHash<QString, QString> exif_to_iptc = {
        {"Exif.Image.Copyright", "Iptc.Application2.Copyright"},
        {"Exif.Image.Artist", "Iptc.Application2.Byline"},
        {"Exif.Image.ImageDescription", "Iptc.Application2.Caption"}
    };

    // XMP -> EXIF
    static const QHash<QString, QString> xmp_to_exif = {
        {"Xmp.dc.rights", "Exif.Image.Copyright"},
        {"Xmp.dc.creator", "Exif.Image.Artist"},
        {"Xmp.dc.description", "Exif.Image.ImageDescription"},
        {"Xmp.photoshop.DateCreated", "Exif.Photo.DateTimeOriginal"},
        // NEU: Damit 'Copyright Owner' auch Exif Copyright aktualisiert
        {"Xmp.plus.CopyrightOwner", "Exif.Image.Copyright"} 
    };

    // XMP -> IPTC
    static const QHash<QString, QString> xmp_to_iptc = {
        {"Xmp.dc.description", "Iptc.Application2.Caption"},
        {"Xmp.dc.rights", "Iptc.Application2.Copyright"},
        {"Xmp.dc.creator", "Iptc.Application2.Byline"},
        {"Xmp.dc.title", "Iptc.Application2.ObjectName"},
        {"Xmp.dc.subject", "Iptc.Application2.Keywords"}, 
        {"Xmp.plus.CopyrightOwner", "Iptc.Application2.Copyright"} 
    };

    // IPTC -> XMP
    static const QHash<QString, QString> iptc_to_xmp = {
        {"Iptc.Application2.Caption", "Xmp.dc.description"},
        {"Iptc.Application2.Copyright", "Xmp.dc.rights"},
        {"Iptc.Application2.Byline", "Xmp.dc.creator"},
        {"Iptc.Application2.ObjectName", "Xmp.dc.title"},
        {"Iptc.Application2.Keywords", "Xmp.dc.subject"}
    };

    // IPTC -> EXIF
    static const QHash<QString, QString> iptc_to_exif = {
        {"Iptc.Application2.Caption", "Exif.Image.ImageDescription"},
        {"Iptc.Application2.Copyright", "Exif.Image.Copyright"},
        {"Iptc.Application2.ObjectName", "Exif.Image.DocumentName"},
        // NEU: Hat gefehlt!
        {"Iptc.Application2.Byline", "Exif.Image.Artist"} 
    };

    // --- TAG BESCHREIBUNGEN ---

    static const QHash<QString, QString> xmpTagDescriptions = {
        {"Xmp.dc.title", "Short title or name of the image."},
        {"Xmp.dc.description", "Textual description of the content."},
        {"Xmp.dc.creator", "The artist or creator (Author)."},
        {"Xmp.dc.rights", "Copyright Notice (e.g. '© 2025 Name')."},
        {"Xmp.dc.subject", "Keywords/Tags (comma separated)."},
        {"Xmp.xmp.Rating", "User Rating (1-5 stars)."},
        {"Xmp.xmp.Label", "Color Label (e.g. Red, Blue)."},
        {"Xmp.plus.CopyrightOwner", "Name of the copyright holder (Person/Company)."},
        {"Xmp.photoshop.Credit", "Credit line required when publishing."},
        {"Xmp.photoshop.Source", "Original source of the image."},
        {"Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCity", "City of the Creator."},
        {"Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCtry", "Country of the Creator."},
        // Adressdaten (Photoshop Schema ist am robustesten)
        {"Xmp.photoshop.City", "City / Municipality."},
        {"Xmp.photoshop.State", "State / Province / Region."},
        {"Xmp.photoshop.Country", "Country Name."},
        {"Xmp.iptc.CountryCode", "ISO Country Code (2 or 3 letters)."},
        {"Xmp.iptc.Location", "Sub-location / Street address."},
        // (CreatorContactInfo lassen wir drin, ist aber komplexer)
        {"Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCity", "City of the Creator."},
        {"Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCtry", "Country of the Creator."}
    };

    static const QHash<QString, QString> exifTagDescriptions = {
        {"Exif.Image.Artist", "Person who created the image."},
        {"Exif.Image.Copyright", "Copyright notice."},
        {"Exif.Image.ImageDescription", "Title or description."},
        {"Exif.Photo.UserComment", "General comments."},
        {"Exif.Photo.DateTimeOriginal", "Date and time of capture."},
        {"Exif.Image.Model", "Camera Model name."},
        {"Exif.Photo.FNumber", "F-Stop (Aperture)."},
        {"Exif.Photo.ISOSpeedRatings", "ISO Speed."}
    };

    static const QHash<QString, QString> iptcTagDescriptions = {
        {"Iptc.Application2.ObjectName", "Shorthand reference for the object (64B)."},
        {"Iptc.Application2.Copyright", "Contains any necessary copyright notice (128B)."},
        {"Iptc.Application2.Caption", "A textual description of the object data (2000B)."},
        {"Iptc.Application2.Byline", "Name of the creator/artist."},
        {"Iptc.Application2.Keywords", "Keywords or tags."},
        // Adressdaten (IPTC Legacy)
        {"Iptc.Application2.City", "City."},
        {"Iptc.Application2.ProvinceState", "Province or State."},
        {"Iptc.Application2.CountryName", "Country Name."},
        {"Iptc.Application2.CountryCode", "Country Code (ISO)."},
        {"Iptc.Application2.SubLocation", "Location details within city (Street)."}
    };

    // --- DEFAULT VALUES LISTE (Für DefaultMetaWidget) ---

    static const QMap<QString, QString> xmpDefaultTagsList = {
        {"Xmp.dc.creator", "Robert Zheng"},
        {"Xmp.dc.rights", "© 2025 Robert Zheng. All rights reserved."},
        {"Xmp.plus.CopyrightOwner", "Robert Zheng"},
        {"Xmp.dc.subject", "Holiday, Family, Nature"},
        {"Xmp.xmp.Rating", "0"}, 
        {"Xmp.xmp.Label", "None"}
    };

    static const QMap<QString, QString> exifDefaultTagsList = {
        {"Exif.Image.Artist", "Robert Zheng"},
        {"Exif.Image.Copyright", "© 2025 Robert Zheng"},
        {"Exif.Image.ImageDescription", ""}
    };

    static const QMap<QString, QString> iptcDefaultTagsList = {
        {"Iptc.Application2.Byline", "Robert Zheng"},
        {"Iptc.Application2.Copyright", "© 2025 Robert Zheng"},
        {"Iptc.Application2.Caption", ""},
        {"Iptc.Application2.Keywords", "Tag1, Tag2"}
    };

} // namespace RzMetadata