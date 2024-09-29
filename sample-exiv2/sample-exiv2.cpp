#include <exiv2/exiv2.hpp>

#include <cinttypes>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <format>    // C++20 (std::format)
#include <filesystem>

using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

static void printEXIF(const Exiv2::Image::UniquePtr& image)
{
   const Exiv2::ExifData& exifData = image->exifData();

   Exiv2::ByteOrder byteOrder = image->byteOrder();

   //
   // EXIF may be looked up by a tag name in Exiv2's key nomenclature (slow - walks all, until found)
   //
   const Exiv2::ExifData::const_iterator cameraMake = exifData.findKey(Exiv2::ExifKey("Exif.Image.Make"s));

   if(cameraMake != exifData.end())
      printf("Make: %s\n", cameraMake->toString().c_str());

   const Exiv2::ExifData::const_iterator cameraModel = exifData.findKey(Exiv2::ExifKey("Exif.Image.Model"s));

   if(cameraModel != exifData.end())
      printf("Model: %s\n", cameraModel->toString().c_str());

   //
   // Walk all tags in EXIF data and print them by tag data type, where
   // Exiv2 value interface allows.
   //
   printf("\nEXIF Tags: [IFD(IfdId)/TagId] Family.Group.Tag (type, size, count): value(s)\n\n");

   for(Exiv2::ExifData::const_iterator i = exifData.begin(); i != exifData.end(); ++i) {
      //
      // IFD IDs are represented with a scoped enum and need to be cast
      // to be interpreted as integers.
      // 
      // Group names are abbreviated and don't match EXIF folder names.
      // For example, ExifIFD/MakerNoteCanon/TimeZone is reported as
      // Exif.CanonTi.TimeZone.
      //
      printf("[%s(%x)/%x] %s.%s.%s (%s,%zd,%zd):", i->ifdName(), static_cast<uint32_t>(i->ifdId()), i->tag(), i->familyName(), i->groupName().c_str(), i->tagName().c_str(), i->typeName(), i->typeSize(), i->count());

      switch (i->typeId()) {
         case Exiv2::TypeId::unsignedByte:
            //
            // This type maps to Exiv2::DataValue, which doesn't provide a good
            // interface for byte access - we either need to copy the entire
            // byte sequence into our buffer or use one of the conversion methods,
            // like toUint32.
            //
            for(size_t c = 0; c < std::min<size_t>(20, i->count()); c++)
               printf(" %02hhx", static_cast<unsigned char>(i->toUint32(c)));
            break;
         case Exiv2::TypeId::signedByte:
            // see Exiv2::TypeId::unsignedByte
            for(size_t c = 0; c < std::min<size_t>(20, i->count()); c++)
               printf(" %" PRId64, i->toInt64(c));
            break;
         case Exiv2::TypeId::signedLong: {
            // all numeric types map to Exiv2::ValueType<T>, which exposes the value list as `value_`
            const Exiv2::ValueType<int32_t>& value = static_cast<const Exiv2::ValueType<int32_t>&>(i->value());

            for(Exiv2::LongValue::ValueList::const_iterator c = value.value_.begin(); c != value.value_.end(); ++c)
               printf(" %" PRId32, *c);
            }
            break;
         case Exiv2::TypeId::signedShort: {
            const Exiv2::ValueType<uint16_t>& value = static_cast<const Exiv2::ValueType<uint16_t>&>(i->value());

            for(Exiv2::UShortValue::ValueList::const_iterator c = value.value_.begin(); c != value.value_.end(); ++c)
               printf(" %" PRId16, *c);
            }
            break;
         case Exiv2::TypeId::unsignedLong:
         case Exiv2::TypeId::unsignedShort:
            // numberic types can also be obtained via toInt64, without casting
            for(size_t c = 0; c < std::min<size_t>(20, i->count()); c++)
               printf(" %" PRId64, static_cast<int64_t>(i->toInt64(c)));
            break;
         case Exiv2::TypeId::asciiString: {
            //
            // Exiv2::AsciiValue exposes the underlying string as a public data
            // member, which works out as a much faster way to access the string,
            // compared to the toString method, which uses a string stream.
            // 
            // Some values are zero-padded and zeros are included in the size,
            // so if an std::string is constructed from a pointer and the size,
            // it may contain extra null characters.
            //
            const Exiv2::AsciiValue& value = static_cast<const Exiv2::AsciiValue&>(i->value());

            printf(" %s", value.value_.c_str());
            }
            break;
         break;
         case Exiv2::TypeId::undefined: {
            const Exiv2::CommentValue *value;

            //
            // TypeId::comment is set inthernally when allocating a value for
            // this field, but the type used for this is discarded (e.g. in
            // Tiffreader) and `undefined` is set in the value using EXIF's
            // original type. Using dynamic_cast<CommentValue> is a slower,
            // but more reliable approach.
            //
            if(i->tag() == 0x9286 && (value = dynamic_cast<const Exiv2::CommentValue*>(&i->value())) != nullptr) {
               //
               // CommentValue::comment is very inefficient and scan the comment
               // string multiple times searching for the null character. It also
               // makes a copy of the string with substr. For ASCII comments (and
               // UTF-8 identified as ASCII), it is better to access the public
               // CommentValue::value_ and use EXIF spec to extract text and walk
               // the string once to locate non-null characters.
               // 
               // Note that this is an "undefined" type and even though it is
               // null-padded, it doesn't have to be null teminated.
               //
               printf("(charset: %s/%s) %s", Exiv2::CommentValue::CharsetInfo::code(value->charsetId()), Exiv2::CommentValue::CharsetInfo::name(value->charsetId()), value->comment().c_str());
            }
            else {
               //
               // Undefined field can have any underlying format, such as one byte
               // for a scene type or kilobytes for maker notes. This type is mapped
               // to DataValue.
               //
               for(size_t c = 0; c < std::min<size_t>(20, i->count()); c++)
                  printf(" %02hhx", static_cast<unsigned char>(i->toUint32(c)));
            }
            }
            break;
         case Exiv2::TypeId::signedRational: {
            // rational values also use ValueType<T>, with T=std::pair<int32_t>
            const Exiv2::ValueType<Exiv2::Rational>& value = static_cast<const Exiv2::ValueType<Exiv2::Rational>&>(i->value());

            // convenience type aliases are defined for all ValueType specializations, such as RationalValue in this case
            for(Exiv2::RationalValue::ValueList::const_iterator c = value.value_.begin(); c != value.value_.end(); ++c)
               printf(" %.5lf", static_cast<double>(c->first)/static_cast<double>(c->second));
            }
            break;
         case Exiv2::TypeId::unsignedRational: {
            // rational values also use ValueType<T>, with T=std::pair<uint32_t>
            const Exiv2::ValueType<Exiv2::URational>& value = static_cast<const Exiv2::ValueType<Exiv2::URational>&>(i->value());

            // toString() outputs rational parts as string, such as "266/32"
            for(Exiv2::URationalValue::ValueList::const_iterator c = value.value_.begin(); c != value.value_.end(); ++c)
               printf(" %.5lf", static_cast<double>(c->first)/static_cast<double>(c->second));
            }
            break;
         default:
            //
            // Exiv2 defines more types than may not be defined in EXIF, such
            // as Exiv2::DateValue. See Value::create method in Exiv2 source
            // for mapping between type Exiv IDs and value classes.
            //
            printf(" %s", i->toString().c_str());
            break;
      }

      printf("\n");
   }
}

static void printXMP(const Exiv2::Image::UniquePtr& image)
{
   //
   // XMP may be decoded when tax 0x2bc (XMLPacket) is iterated in
   // the function above, but the image parser already decoded the
   // data, so we print all XMP values from the image here instead.
   // 
   if(!image->xmpData().empty()) {
      //
      // XMP value key will include the XML namespace and will have
      // the `Xmp.` prefix, so for this XMP element:
      // 
      //   <xmp:Rating>5</xmp:Rating>
      //
      // , this key will be returned. There is no tag numbers in XMP.
      // 
      //   Xmp.xmp.Rating (XmpText,1): 5
      //
      printf("\nXMP Tags: Family.Group.Tag [TagLabel] (type, count): value(s)\n\n");

      for(Exiv2::XmpData::const_iterator i = image->xmpData().begin(); i != image->xmpData().end(); ++i) {
         printf("%s.%s.%s [%s] (%s,%zd): %s\n", i->familyName(), i->groupName().c_str(), i->tagName().c_str(), i->tagLabel().c_str(), i->typeName(), i->count(), i->toString().c_str());
      }
   }
}

int main(int argc, const char* argv[])
{
   if(argc != 2) {
      fprintf(stderr, "Usage: sample-exiv2 image-path\n");
      return EXIT_FAILURE;
   }

   try {
      Exiv2::XmpParser::initialize();
      Exiv2::enableBMFF();

      Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(std::filesystem::path(argv[1]));

      if(!image)
         throw std::runtime_error(std::format("Cannot open file {:s}"sv, argv[1]));

      image->readMetadata();

      printEXIF(image);
      printXMP(image);

      Exiv2::XmpParser::terminate();

      return EXIT_SUCCESS;
   }
   catch (const Exiv2::Error& error) {
      fprintf(stderr, "ERROR (exiv2): %s\n", error.what());
   }
   catch (const std::exception& error) {
      fprintf(stderr, "ERROR (std): %s\n", error.what());
   }
   catch (...) {
      fprintf(stderr, "ERROR (unknown)\n");
   }

   Exiv2::XmpParser::terminate();

   return EXIT_FAILURE;   
}
