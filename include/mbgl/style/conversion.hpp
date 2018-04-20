#pragma once

#include <mbgl/util/optional.hpp>
#include <mbgl/util/feature.hpp>
#include <mbgl/util/geojson.hpp>

#include <string>

namespace mbgl {
namespace style {
namespace conversion {

/*
   The `conversion` namespace defines conversions from JSON structures conforming to the schema defined by
   the Mapbox Style Specification, to the various C++ types that form the C++ model of that domain:

       * `std::unique_ptr<Source>`
       * `std::unique_ptr<Layer>`
       * `Filter`
       * `PropertyValue<T>`

   A single template function serves as the public interface:

       template <class T>
       optional<T> convert(const Convertible& input, Error& error);

   Where `T` is one of the above types. If the conversion fails, the result is empty, and the
   error parameter includes diagnostic text suitable for presentation to a library user. Otherwise,
   a filled optional is returned.

   `Convertible` is a type that encapsulates a special form of polymorphism over various underlying types that
   can serve as input to the conversion algorithm. For instance, on macOS, we need to support
   conversion from both RapidJSON types, and a JSON structure represented with `NSArray`/`NSDictionary`/etc.
   On Qt, we need to support conversion from RapidJSON types and QVariant.

   We don't want to use traditional forms of polymorphism to accomplish this:

     * Compile time polymorphism using a template parameter for the actual value type leads to
       excessive code bloat and long compile times.
     * Runtime polymorphism using virtual methods requires extra heap allocation and ubiquitous
       use of std::unique_ptr, unsuitable for this performance-sensitive code.

   Therefore, we're using a custom implementation of runtime polymorphism where we manually create and
   dispatch through a table of function pointers (vtable), while keeping the storage for any of the possible
   underlying types inline on the stack, using `std::aligned_storage`.

   For a given underlying type T, an explicit specialization of `ConversionTraits<T>` must be provided. This
   specialization must provide the following static methods:

      * `isUndefined(v)` -- returns a boolean indication whether `v` is undefined or a JSON null

      * `isArray(v)` -- returns a boolean indicating whether `v` represents a JSON array
      * `arrayLength(v)` -- called only if `isArray(v)`; returns a size_t length
      * `arrayMember(v)` -- called only if `isArray(v)`; returns `V` or `V&`

      * `isObject(v)` -- returns a boolean indicating whether `v` represents a JSON object
      * `objectMember(v, name)` -- called only if `isObject(v)`; `name` is `const char *`; return value:
         * is true when evaluated in a boolean context iff the named member exists
         * is convertable to a `V` or `V&` when dereferenced
      * `eachMember(v, [] (const std::string&, const V&) -> optional<Error> {...})` -- called
         only if `isObject(v)`; calls the provided lambda once for each key and value of the object;
         short-circuits if any call returns an `Error`

      * `toBool(v)` -- returns `optional<bool>`, absence indicating `v` is not a JSON boolean
      * `toNumber(v)` -- returns `optional<float>`, absence indicating `v` is not a JSON number
      * `toDouble(v)` -- returns `optional<double>`, absence indicating `v` is not a JSON number
      * `toString(v)` -- returns `optional<std::string>`, absence indicating `v` is not a JSON string
      * `toValue(v)` -- returns `optional<Value>`, a variant type, for generic conversion,
        absence indicating `v` is not a boolean, number, or string. Numbers should be converted to
        unsigned integer, signed integer, or floating point, in descending preference.

   In addition, the type T must be move-constructable. And finally, `Convertible::Storage`, a typedef for
   `std::aligned_storage_t`, must be large enough to satisfy the memory requirements for any of the
   possible underlying types. (A static assert will fail if this is not the case.)

   `Convertible` itself is movable, but not copyable. A moved-from `Convertible` is in an invalid state;
   you must not do anything with it except let it go out of scope.
*/

struct Error { std::string message; };

template <typename T>
class ConversionTraits;

class Convertible {
public:
    template <typename T>
    Convertible(T&& value) : vtable(vtableForType<std::decay_t<T>>()) {
        static_assert(sizeof(Storage) >= sizeof(std::decay_t<T>), "Storage must be large enough to hold value type");
        new (static_cast<void*>(&storage)) std::decay_t<T>(std::forward<T>(value));
    }

    Convertible(Convertible&& v)
        : vtable(v.vtable)
    {
        if (vtable) {
            vtable->move(std::move(v.storage), this->storage);
        }
    }

    ~Convertible() {
        if (vtable) {
            vtable->destroy(storage);
        }
    }

    Convertible& operator=(Convertible&& v) {
        if (vtable) {
            vtable->destroy(storage);
        }
        vtable = v.vtable;
        if (vtable) {
            vtable->move(std::move(v.storage), this->storage);
        }
        v.vtable = nullptr;
        return *this;
    }

    Convertible()                              = delete;
    Convertible(const Convertible&)            = delete;
    Convertible& operator=(const Convertible&) = delete;

    friend inline bool isUndefined(const Convertible& v) {
        assert(v.vtable);
        return v.vtable->isUndefined(v.storage);
    }

    friend inline bool isArray(const Convertible& v) {
        assert(v.vtable);
        return v.vtable->isArray(v.storage);
    }

    friend inline std::size_t arrayLength(const Convertible& v) {
        assert(v.vtable);
        return v.vtable->arrayLength(v.storage);
    }

    friend inline Convertible arrayMember(const Convertible& v, std::size_t i) {
        assert(v.vtable);
        return v.vtable->arrayMember(v.storage, i);
    }

    friend inline bool isObject(const Convertible& v) {
        assert(v.vtable);
        return v.vtable->isObject(v.storage);
    }

    friend inline optional<Convertible> objectMember(const Convertible& v, const char * name) {
        assert(v.vtable);
        return v.vtable->objectMember(v.storage, name);
    }

    friend inline optional<Error> eachMember(const Convertible& v, const std::function<optional<Error> (const std::string&, const Convertible&)>& fn) {
        assert(v.vtable);
        return v.vtable->eachMember(v.storage, fn);
    }

    friend inline optional<bool> toBool(const Convertible& v) {
        assert(v.vtable);
        return v.vtable->toBool(v.storage);
    }

    friend inline optional<float> toNumber(const Convertible& v) {
        assert(v.vtable);
        return v.vtable->toNumber(v.storage);
    }

    friend inline optional<double> toDouble(const Convertible& v) {
        assert(v.vtable);
        return v.vtable->toDouble(v.storage);
    }

    friend inline optional<std::string> toString(const Convertible& v) {
        assert(v.vtable);
        return v.vtable->toString(v.storage);
    }

    friend inline optional<Value> toValue(const Convertible& v) {
        assert(v.vtable);
        return v.vtable->toValue(v.storage);
    }

    friend inline optional<GeoJSON> toGeoJSON(const Convertible& v, Error& error) {
        assert(v.vtable);
        return v.vtable->toGeoJSON(v.storage, error);
    }

private:
#if __ANDROID__
    // Android:     JSValue* or mbgl::android::Value
    using Storage = std::aligned_storage_t<32, 8>;
#elif __QT__
    // Qt:          JSValue* or QVariant
    using Storage = std::aligned_storage_t<32, 8>;
#else
    // Node:        JSValue* or v8::Local<v8::Value>
    // iOS/macOS:   JSValue* or id
    using Storage = std::aligned_storage_t<8, 8>;
#endif

    class VTable {
    public:
        virtual ~VTable() {};

        virtual void move(Storage&& src, Storage& dest) = 0;
        virtual void destroy(Storage&) = 0;

        virtual bool        isUndefined (const Storage&) = 0;
        virtual bool        isArray     (const Storage&) = 0;
        virtual std::size_t arrayLength (const Storage&) = 0;
        virtual Convertible arrayMember (const Storage&, std::size_t) = 0;

        virtual bool                  isObject     (const Storage&) = 0;
        virtual optional<Convertible> objectMember (const Storage&, const char *) = 0;
        virtual optional<Error>       eachMember   (const Storage&, const std::function<optional<Error> (const std::string&, const Convertible&)>&) = 0;

        virtual optional<bool>        toBool   (const Storage&) = 0;
        virtual optional<float>       toNumber (const Storage&) = 0;
        virtual optional<double>      toDouble (const Storage&) = 0;
        virtual optional<std::string> toString (const Storage&) = 0;
        virtual optional<Value>       toValue  (const Storage&) = 0;

        // https://github.com/mapbox/mapbox-gl-native/issues/5623
        virtual optional<GeoJSON> toGeoJSON(const Storage&, Error&) = 0;
    };

    template <class T>
    class VTableForType : public VTable {
    public:
        using Traits = ConversionTraits<T>;

        void move(Storage&& src, Storage& dest) override {
            auto srcValue = reinterpret_cast<T&&>(src);
            new (static_cast<void*>(&dest)) T(std::move(srcValue));
            srcValue.~T();
        }

        void destroy(Storage& s) override {
            reinterpret_cast<T&>(s).~T();
        }

        bool isUndefined(const Storage& s) override {
            return Traits::isUndefined(reinterpret_cast<const T&>(s));
        }

        bool isArray(const Storage& s) override {
            return Traits::isArray(reinterpret_cast<const T&>(s));
        }

        std::size_t arrayLength(const Storage& s) override {
            return Traits::arrayLength(reinterpret_cast<const T&>(s));
        }

        Convertible arrayMember(const Storage& s, std::size_t i) override {
            return Convertible(Traits::arrayMember(reinterpret_cast<const T&>(s), i));
        }

        bool isObject(const Storage& s) override {
            return Traits::isObject(reinterpret_cast<const T&>(s));
        }

        optional<Convertible> objectMember(const Storage& s, const char * key) override {
            optional<T> member = Traits::objectMember(reinterpret_cast<const T&>(s), key);
            if (member) {
                return optional<Convertible>(Convertible(std::move(*member)));
            } else {
                return optional<Convertible>();
            }
        }

        optional<Error> eachMember(const Storage& s, const std::function<optional<Error>(const std::string&, const Convertible&)>& fn) override {
            return ConversionTraits<T>::eachMember(reinterpret_cast<const T&>(s), [&](const std::string& k, T&& v) {
                return fn(k, Convertible(std::move(v)));
            });
        }

        optional<bool> toBool(const Storage& s) override {
            return Traits::toBool(reinterpret_cast<const T&>(s));
        }

        optional<float> toNumber(const Storage& s) override {
            return Traits::toNumber(reinterpret_cast<const T&>(s));
        }

        optional<double> toDouble(const Storage& s) override {
            return Traits::toDouble(reinterpret_cast<const T&>(s));
        }

        optional<std::string> toString(const Storage& s) override {
            return Traits::toString(reinterpret_cast<const T&>(s));
        }

        optional<Value> toValue(const Storage& s) override {
            return Traits::toValue(reinterpret_cast<const T&>(s));
        }

        optional<GeoJSON> toGeoJSON(const Storage& s, Error& err) override {
            return Traits::toGeoJSON(reinterpret_cast<const T&>(s), err);
        }
    };

    template <typename T>
    static VTable* vtableForType() {
        static VTableForType<T> vtable;
        return &vtable;
    }

    VTable* vtable;
    Storage storage;
};

template <class T, class Enable = void>
struct Converter;

template <class T, class...Args>
optional<T> convert(const Convertible& value, Error& error, Args&&...args) {
    return Converter<T>()(value, error, std::forward<Args>(args)...);
}

} // namespace conversion
} // namespace style
} // namespace mbgl
