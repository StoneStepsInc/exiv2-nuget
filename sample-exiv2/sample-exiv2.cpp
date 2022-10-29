#include <exiv2/exiv2.hpp>

#include <cinttypes>
#include <cstdint>
#include <cstdlib>
#include <string>

using namespace std::literals::string_literals;

void printEXIF(Exiv2::Image::AutoPtr image)
{
   const Exiv2::ExifData& exifData = image->exifData();

   Exiv2::ByteOrder byteOrder = image->byteOrder();

   //
   // EXIF may be looked up by a tag name in Exiv2's key nomenclature.
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
   printf("\nEXIF Tags: IFD/tag (type, size, count):\n\n");

   for(Exiv2::ExifData::const_iterator i = exifData.begin(); i != exifData.end(); ++i) {
      printf("[%x/%x] %s/%s (%s,%ld,%ld):", i->ifdId(), i->tag(), i->ifdName(), i->tagName().c_str(), i->typeName(), i->typeSize(), i->count());

      switch (i->typeId()) {
         case Exiv2::TypeId::unsignedByte:
            //
            // This type maps to Exiv2::DataValue, which doesn't provide a good
            // interface for byte access - we either need to copy the entire
            // byte sequence into our buffer or use one of the conversion methods,
            // like toLong.
            //
            for(long c = 0; c < std::min<long>(20, i->count()); c++)
               printf(" %02hhx", static_cast<unsigned char>(i->toLong(c)));
            break;
         case Exiv2::TypeId::signedByte:
            // see Exiv2::TypeId::unsignedByte
            for(long c = 0; c < std::min<long>(20, i->count()); c++)
               printf(" %ld", i->toLong(c));
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
            // numberic types can also be obtained via toLong, without casting
            for(long c = 0; c < std::min<long>(20, i->count()); c++)
               printf(" %lu", static_cast<unsigned long>(i->toLong(c)));
            break;
         case Exiv2::TypeId::asciiString: {
            //
            // Exiv2::AsciiValue doesn't provide any additional string interface
            // and only ensures strings are handled as strings behind the scenes.
            // This means there's no fast access to string data and data may be
            // obtained either via toString, which uses a string stream buffer
            // or via `copy`, which copies the entire sequence of bytes.
            //
            const Exiv2::AsciiValue& value = static_cast<const Exiv2::AsciiValue&>(i->value());

            printf(" %s", i->toString().c_str());
            }
            break;
         case Exiv2::TypeId::undefined: {
            //
            // Undefined field can have any underlying format, such as one byte
            // for a scene type or kilobytes for maker notes. This type is mapped
            // to DataValue.
            //
            for(long c = 0; c < std::min<long>(20, i->count()); c++)
               printf(" %02hhx", static_cast<unsigned char>(i->toLong(c)));
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

int main(int argc, const char* argv[])
{
   if(argc != 2) {
      printf("Usage: sample-exiv2 image-path\n");
      return EXIT_FAILURE;
   }

   try {
      Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(argv[1]);

      if(!image)
         throw std::runtime_error("Cannot open file "s + argv[1]);

      image->readMetadata();

      printEXIF(std::move(image));

      return EXIT_SUCCESS;
   }
   catch (const Exiv2::Error& error) {
      printf("ERROR (exiv2): %s\n", error.what());
   }
   catch (const Exiv2::AnyError& error) {
      printf("ERROR (exiv2/any): %s\n", error.what());
   }
   catch (const std::exception& error) {
      printf("ERROR (std): %s\n", error.what());
   }
   catch (...) {
      printf("ERROR (unknown)\n");
   }

   return EXIT_FAILURE;   
}
