/**
 * @file rz_metadata.hpp
 * @brief Central definition of all Metadata Tags, Defaults and Mappings
 * @version 0.9 (Full Default Lists)
 */
#pragma once

#include <QHash>
#include <QMap>
#include <QString>

/**
 * @namespace RzMetadata
 * @brief Contains definitions and mapping rules for image metadata.
 */
namespace RzMetadata {

/**
 * @struct DefaultMetaStruct
 * @brief Holds default metadata values for XMP, EXIF, and IPTC.
 */
struct DefaultMetaStruct {
  QHash<QString, QString> xmpDefault{};
  QHash<QString, QString> exifDefault{};
  QHash<QString, QString> iptcDefault{};
};

// --- MAPPINGS (Sync Rules) ---
// These tables control which fields are automatically synchronised.

// EXIF -> XMP
static const QHash<QString, QString> exif_to_xmp = {
    {"Exif.Image.Copyright", "Xmp.dc.rights"},
    {"Exif.Image.Artist", "Xmp.dc.creator"},
    {"Exif.Image.ImageDescription", "Xmp.dc.description"},
    {"Exif.Photo.UserComment", "Xmp.dc.description"},
    {"Exif.Image.Software", "Xmp.xmp.CreatorTool"},
    {"Exif.Photo.DateTimeOriginal", "Xmp.photoshop.DateCreated"}};

// EXIF -> IPTC
static const QHash<QString, QString> exif_to_iptc = {
    {"Exif.Image.Copyright", "Iptc.Application2.Copyright"},
    {"Exif.Image.Artist", "Iptc.Application2.Byline"},
    {"Exif.Image.ImageDescription", "Iptc.Application2.Caption"}};

// XMP -> EXIF
static const QHash<QString, QString> xmp_to_exif = {
    {"Xmp.dc.rights", "Exif.Image.Copyright"},
    {"Xmp.dc.creator", "Exif.Image.Artist"},
    {"Xmp.dc.description", "Exif.Image.ImageDescription"},
    {"Xmp.photoshop.DateCreated", "Exif.Photo.DateTimeOriginal"},
    {"Xmp.plus.CopyrightOwner", "Exif.Image.Copyright"}};

// XMP -> IPTC
static const QHash<QString, QString> xmp_to_iptc = {
    {"Xmp.dc.description", "Iptc.Application2.Caption"},
    {"Xmp.dc.rights", "Iptc.Application2.Copyright"},
    {"Xmp.dc.creator", "Iptc.Application2.Byline"},
    {"Xmp.dc.title", "Iptc.Application2.ObjectName"},
    {"Xmp.dc.subject", "Iptc.Application2.Keywords"},
    {"Xmp.plus.CopyrightOwner", "Iptc.Application2.Copyright"},
    // Address-Sync
    {"Xmp.photoshop.City", "Iptc.Application2.City"},
    {"Xmp.photoshop.State", "Iptc.Application2.ProvinceState"},
    {"Xmp.photoshop.Country", "Iptc.Application2.CountryName"},
    {"Xmp.iptc.CountryCode", "Iptc.Application2.CountryCode"},
    {"Xmp.iptc.Location", "Iptc.Application2.SubLocation"}};

// IPTC -> XMP
static const QHash<QString, QString> iptc_to_xmp = {
    {"Iptc.Application2.Caption", "Xmp.dc.description"},
    {"Iptc.Application2.Copyright", "Xmp.dc.rights"},
    {"Iptc.Application2.Byline", "Xmp.dc.creator"},
    {"Iptc.Application2.ObjectName", "Xmp.dc.title"},
    {"Iptc.Application2.Keywords", "Xmp.dc.subject"},
    // Address-Sync
    {"Iptc.Application2.City", "Xmp.photoshop.City"},
    {"Iptc.Application2.ProvinceState", "Xmp.photoshop.State"},
    {"Iptc.Application2.CountryName", "Xmp.photoshop.Country"},
    {"Iptc.Application2.CountryCode", "Xmp.iptc.CountryCode"},
    {"Iptc.Application2.SubLocation", "Xmp.iptc.Location"}};

// IPTC -> EXIF
static const QHash<QString, QString> iptc_to_exif = {
    {"Iptc.Application2.Caption", "Exif.Image.ImageDescription"},
    {"Iptc.Application2.Copyright", "Exif.Image.Copyright"},
    {"Iptc.Application2.ObjectName", "Exif.Image.DocumentName"},
    {"Iptc.Application2.Byline", "Exif.Image.Artist"}};

// --- DESCRIPTIONS (Display Texts) ---

static const QHash<QString, QString> xmpTagDescriptions = {
    {"Xmp.dc.title", "Short title or name of the image."},
    {"Xmp.dc.description", "Textual description of the content."},
    {"Xmp.dc.creator", "The artist or creator (Author)."},
    {"Xmp.dc.rights", "Copyright Notice (e.g. '© 2025 Name')."},
    {"Xmp.dc.subject", "Keywords/Tags (comma separated)."},
    {"Xmp.xmp.Rating", "User Rating (1-5 stars)."},
    {"Xmp.xmp.Label", "Color Label (e.g. Red, Blue)."},
    {"Xmp.plus.CopyrightOwner", "Name of the copyright holder."},
    {"Xmp.photoshop.City", "City / Municipality."},
    {"Xmp.photoshop.State", "State / Province / Region."},
    {"Xmp.photoshop.Country", "Country Name."},
    {"Xmp.iptc.CountryCode", "ISO Country Code (2 or 3 letters)."},
    {"Xmp.iptc.Location", "Sub-location / Street address."},
    {"Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCity",
     "City of the Creator."},
    {"Xmp.iptc.CreatorContactInfo/Iptc4xmpCore:CiAdrCtry",
     "Country of the Creator."}};

static const QHash<QString, QString> exifTagDescriptions = {
    {"Exif.Image.Artist", "Person who created the image."},
    {"Exif.Image.Copyright", "Copyright notice."},
    {"Exif.Image.ImageDescription", "Title or description."},
    {"Exif.Photo.UserComment", "General comments."},
    {"Exif.Photo.DateTimeOriginal", "Date and time of capture."},
    {"Exif.Image.Model", "Camera Model name."},
    {"Exif.Photo.FNumber", "F-Stop (Aperture)."},
    {"Exif.Photo.ISOSpeedRatings", "ISO Speed."}};

static const QHash<QString, QString> iptcTagDescriptions = {
    {"Iptc.Application2.ObjectName", "Title / Object Name."},
    {"Iptc.Application2.Copyright", "Copyright notice."},
    {"Iptc.Application2.Caption", "Description / Caption."},
    {"Iptc.Application2.Byline", "Creator / Artist."},
    {"Iptc.Application2.Keywords", "Keywords."},
    {"Iptc.Application2.City", "City."},
    {"Iptc.Application2.ProvinceState", "Province or State."},
    {"Iptc.Application2.CountryName", "Country Name."},
    {"Iptc.Application2.CountryCode", "Country Code (ISO)."},
    {"Iptc.Application2.SubLocation", "Location / Street."}};

// --- DEFAULT VALUES LIST (For DefaultMetaWidget) ---
// These lists define which rows are displayed in the default editor.

static const QMap<QString, QString> xmpDefaultTagsList = {
    {"Xmp.dc.creator", "Robert Zheng"},
    {"Xmp.dc.rights", "© 2025 Robert Zheng"},
    {"Xmp.plus.CopyrightOwner", "Robert Zheng"},
    {"Xmp.dc.title", ""},
    {"Xmp.dc.description", ""},
    {"Xmp.dc.subject", "Holiday, Family"},
    {"Xmp.xmp.Rating", "0"},
    {"Xmp.xmp.Label", ""},
    {"Xmp.photoshop.City", ""},
    {"Xmp.photoshop.State", ""},
    {"Xmp.photoshop.Country", ""},
    {"Xmp.iptc.CountryCode", ""},
    {"Xmp.iptc.Location", ""}};

static const QMap<QString, QString> exifDefaultTagsList = {
    {"Exif.Image.Artist", "Robert Zheng"},
    {"Exif.Image.Copyright", "© 2025 Robert Zheng"},
    {"Exif.Image.ImageDescription", ""},
    {"Exif.Photo.UserComment", ""}};

static const QMap<QString, QString> iptcDefaultTagsList = {
    {"Iptc.Application2.Byline", "Robert Zheng"},
    {"Iptc.Application2.Copyright", "© 2025 Robert Zheng"},
    {"Iptc.Application2.Caption", ""},
    {"Iptc.Application2.ObjectName", ""},
    {"Iptc.Application2.Keywords", "Tag1, Tag2"},
    {"Iptc.Application2.City", ""},
    {"Iptc.Application2.ProvinceState", ""},
    {"Iptc.Application2.CountryName", ""},
    {"Iptc.Application2.CountryCode", ""},
    {"Iptc.Application2.SubLocation", ""}};

} // namespace RzMetadata