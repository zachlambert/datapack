# Concept

For a given type you want to serialize, write a serialize and deserialize function, either as a normal function or a class method.

Then it will be serializable to/from any implemented serialization protocol.

This is limited to "json-like" serialization protcols where:
- An object stores a set of key-value pairs (in a specific order)
- Values can be other objects, arrays or primitives
- A primitive can be a number, string, boolean, null or binary data.

For more generality, protocols may implement specialised methods for specific number types, or may encode all numbers the same way.

For text-based formats, text encoding (eg: base64) is required to encode binary data.

This focuses on being generic, so not all specialised features are supported. eg: References within YAML.

# Usage

## Implementing read and write methods

```c++
#include <datapack/yaml.h>

struct Foo: public datapack::Readable, datapack::Writeable {
    int x;
    int y;
    std::optional<int> z;

    void read(datapack::Reader& reader) override {
        reader.start_object();
        reader.key("x").value(x);
        reader.key("y").value(y);
        reader.key("z").value(z);
        reader.end_object();
    }
    void write(datapack::Writer& writer) const override {
        writer.start_object();
        writer.key("x").value(x);
        writer.key("y").value(y);
        reader.key("z").value(z);
        writer.end_object();
    }
};

int main() {
    Foo in;
    // set value...

    std::string encoded = datapack::encode<datapack::Yaml>(in);
    Foo out = datapack::decode<datapack::Yaml, Foo>(encoded);

    // OR, Foo will now work with any supported template objects

    std::unordered_map<std::string, std::optional<Foo>> in2;
    // set value...

    std::string encoded2 = datapack::encode<datapack::Yaml>(in2);
    auto out2 = datapack::decode_like<datapack::Yaml>(encoded, in2);

    return 0;
};


```

# Embedded targets

Reader and writer objects can be written or modified to be suitable for embedded applications, useful if exceptions and/or dynamic memory allocation cannot be used.

It still requires the target to support C++20.

# Installation

Install to system
```shell
git clone git@github.com:zachlambert/datapack.git
make
sudo make install
```

Include in cmake
```cmake
find_package(datapack REQUIRED)
target_link_library(my-target PUBLIC datapack)
```

Include via `FetchContent()`
```cmake
TODO
```
