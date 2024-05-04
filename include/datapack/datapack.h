#pragma once

#include <string>
#include <variant>
#include <vector>
#include <optional>
#include <iostream>

namespace datapack {

#if 0
using ObjectToken = std::variant<
    double,
    std::nullopt_t
>;

struct ObjectNode {
    int parent;
    int child;
    int prev;
    int next;
    ObjectNode():
        parent(-1),
        child(-1),
        prev(-1),
        next(-1)
    {}
};

class Object {
public:
    class Handle {
    public:
        Handle(Object* object, int index):
            object(object),
            index(index)
        {}
        bool up() {
            if (index == -1) return false;
            const auto& node = object->nodes[index];
            if (node.parent == -1) return false;
            index = node.parent;
            return true;
        }
        bool down() {
            if (index == -1) return false;
            const auto& node = object->nodes[index];
            if (node.child == -1) return false;
            index = node.child;
            return true;
        }
        bool prev() {
            if (index == -1) return false;
            const auto& node = object->nodes[index];
            if (node.prev == -1) return false;
            index = node.prev;
            return true;
        }
        bool next() {
            if (index == -1) return false;
            const auto& node = object->nodes[index];
            if (node.next == -1) return false;
            index = node.next;
            return true;
        }
        void erase() {
            // TODO
        }
        void append() {

        }
    private:
        Object* object;
        int index;
    };

    Object():
        root_index(-1)
    {}

    Handle root() {
        return Handle(this, root_index);
    }

private:
    std::vector<ObjectNode> nodes;
    int root_index;
};

class Reader {
public:
    virtual double read_f64() = 0;
    virtual bool check_optional() = 0;
    std::optional<double> read_f64_opt() {
        if (check_optional()) {
            return read_f64();
        }
        return std::nullopt;
    }
};

class BinaryReader: public Reader {
public:
    BinaryReader(const std::vector<std::uint8_t>& data):
        error_(false),
        data(data),
        pos(0)
    {}

    double read_f64() override {
        if (pos + sizeof(double) > data.size()) {
            error_ = true;
            return 0;
        }
        double result = *((const double*)&data[pos]);
        pos += sizeof(double);
        return result;
    }

    bool check_optional() override {
        if (pos + 1 > data.size()) {
            error_ = true;
            return false;
        }
        pos++;
        if (data[pos-1] == 0x00) {
            return false;
        } else if (data[pos-1] == 0x01) {
            return true;
        } else {
            error_ = true;
            return false;
        }
    }

    bool error() const { return error_; }

private:
    bool error_;
    const std::vector<std::uint8_t>& data;
    std::size_t pos;
};

class Readable {
public:
    void read(Reader& reader);
};

class Writer {
public:
    virtual void write_f64(double value) = 0;
    virtual void mark_optional(bool has_value) = 0;
    void write_f64_opt(const std::optional<double>& value) {
        if (value.has_value()) {
            mark_optional(true);
            write_f64(value.value());
        } else {
            mark_optional(false);
        }
    }
};

class BinaryWriter: public Writer {
public:
    void write_f64(double value) override {
        data_.resize(data_.size() + sizeof(double));
        *((double*)&data_[data_.size()-sizeof(double)]) = value;
    }
    void mark_optional(bool has_value) override {
        data_.push_back(has_value ? 0x01 : 0x00);
    }

    const std::vector<std::uint8_t>& data() const { return data_; }
private:
    std::vector<std::uint8_t> data_;
};

class Writeable {
public:
    void write(Writer& writer) const;
};

class RandomReader: public Reader {
public:
    double read_f64() override {
        return (double)rand() / RAND_MAX;
    }

    bool check_optional() override {
        return rand() % 2 == 1;
    }
};

class PrintWriter: public Writer {
public:
    PrintWriter(std::ostream& os): os(os) {}

    void write_f64(double value) override {
        os << value << "\n";
    }
    void mark_optional(bool has_value) override {
        if (has_value) {
            os << "value: ";
        } else {
            os << "value: none\n";
        }
    }

private:
    std::ostream& os;
};
#endif

}
