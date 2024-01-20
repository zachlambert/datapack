# Idea

Provide a way to provide a serialization/deserialization method for struct-like objects, that can be re-used for a range of formats.

Each format must be structured as an object of key-value pairs, where each value is an object, array or primitive.
Primitives are:
- Number types: f32, f64, i32, i64
- String
- Boolean
- Null
- Binary data

This includes:
- JSON
- Binary JSON (BSON)
- YAML
- MsgPack

Important notes:
- Not all features of each format will be useable.
- If a format doesn't support a given primitive natively, it can do it's own handling. For example, JSON has a single number format and is text-based so cannot represent binary data as-is, and instead needs to use base64 encoding or similar.

# Implementing a format

A format may implement one or both of:
- A writer: Converts a writeable object to the protocol's format
- A reader: Parses the protcol's format into a readable object

The protocol's format may be bytes or some other data structure.
For example:
- A JSON writer can write to a in-memory JSON object
- Another JSON writer can write to bytes directly
- The JSON object itself can be writeable, so can be converted to JSON as bytes

Objects can be readable/writeable either by inheritance or template pattern matching, which can be more formally represented by C++20 concepts.

This allows for making pre-existing objects readable/writeable as well as allowing read/write methods where this is more convenient.

An example read and write method would be:
```c++
struct Foo {
    double a;
    double b;
    std::optional<Bar> bar;
};

// Already provided in the library: Support for common types, including
// templated types like std::optional<T>

template <datapack::readable T>
void datapack::read(datapack::Reader& reader, std::optional<T>& value) {
    if (reader.peek_null()) {
        value = std::nullopt;
        return;
    }
    value = reader.read<T>();
}

template <datapack::writeable T>
void datapack::write(datapack::Writer& writer, const std::optional<T>& value) {
    if (value.has_value()) {
        reader.write(value.value());
        return;
    }
    reader.write_null();
}

template <>
void datapack::read(datapack::Reader& reader, Foo& value) {
    reader.start_object();
    reader.read_f64("a", value.a);
    reader.read_f64("b", value.b);
    reader.read("bar", value.bar);
    reader.end_object();
}

template <>
void datapack::write(datapack::Writer& writer, const Foo& value) {
    writer.start_object();
    writer.write_f64("a", value.a);
    writer.write_f64("b", value.b);
    writer.write("bar", value.bar);
    writer.end_object();
}
```

May also allow macros for convenience, eg:
```c++
template <>
void datapack::write(datapack::Writer& writer, const Foo& value) {
    writer.start_object();
    writer.write_f64("a", value.a);
    writer.write_f64("b", value.b);
    writer.write("bar", value.bar);
    writer.end_object();
}
```
