#include "datapack/format/binary_schema.hpp"
#include "datapack/format/binary.hpp"
#include "datapack/util/object.hpp"
#include <cstring>


namespace datapack {

static std::size_t get_tokens_end(const std::vector<BToken>& tokens, std::size_t begin) {
    std::size_t pos = begin;
    std::size_t depth = 0;
    while (true) {
        if (depth == 0 && pos != begin) {
            break;
        }
        if (pos >= tokens.size()) {
            throw LoadException("Invalid binary schema");
        }
        const auto& token = tokens[pos];
        pos++;

        // All these tokens are preceded by another value
        // so skip to the next token, such that the loop doesn't exit
        // if the depth is still zero
        if (std::get_if<btoken::Map>(&token)){
            continue;
        }
        else if (std::get_if<btoken::List>(&token)){
            continue;
        }
        else if (std::get_if<btoken::Optional>(&token)){
            continue;
        }
        // Explicit container tokens, that increment or decrement depth
        // Where depth is decreased, fall through to the end of the loop
        // body to check if depth is zero
        else if (std::get_if<btoken::ObjectBegin>(&token)){
            depth++;
            continue;
        }
        else if (std::get_if<btoken::ObjectEnd>(&token)){
            depth--;
        }
        else if (std::get_if<btoken::TupleBegin>(&token)){
            depth++;
            continue;
        }
        else if (std::get_if<btoken::TupleEnd>(&token)){
            depth--;
        }
        else if (std::get_if<btoken::VariantBegin>(&token)){
            depth++;
            continue;
        }
        else if (std::get_if<btoken::VariantEnd>(&token)){
            depth--;
        }
        else if (std::get_if<btoken::BinaryBegin>(&token)){
            depth++;
            continue;
        }
        else if (std::get_if<btoken::BinaryEnd>(&token)){
            depth--;
        }
        // Remaining tokens are values
        // Either these are in a container and depth remains unchanged
        // at a non-zero value, so continues, or they are the only value
        // and depth is still at zero, which stops the loop

        if (depth == 0) {
            break;
        }
    }
    return pos;
}

Object load_binary(const BinarySchema& schema, const std::vector<std::uint8_t>& data)
{
    Object object;
    ObjectWriter writer(object);
    BinaryReader reader(data);

    enum class StateType {
        None,
        Map,
        List,
        Optional,
        Variant,
        Binary
    };
    struct State {
        const StateType type;
        const std::size_t value_tokens_begin;
        const std::size_t value_tokens_end;
        int remaining;
        State(
            StateType type,
            std::size_t value_tokens_begin,
            std::size_t value_tokens_end,
            int remaining
        ):
            type(type),
            value_tokens_begin(value_tokens_begin),
            value_tokens_end(value_tokens_end),
            remaining(remaining)
        {}
    };
    std::stack<State> states;
    states.push(State(StateType::None, 0, 0, 0));

    std::size_t token_pos = 0;
    while (true) {
        if (token_pos == schema.tokens.size()) {
            break;
        }

        auto& state = states.top();
        if (state.type == StateType::Map) {
            std::string key;
            if (!reader.map_next(key)) {
                writer.map_end();
                token_pos = state.value_tokens_end;
                states.pop();
                continue;
            }
            writer.map_next(key);
            token_pos = state.value_tokens_begin;
        }
        else if (state.type == StateType::List) {
            if (!reader.list_next()) {
                writer.list_end();
                token_pos = state.value_tokens_end;
                states.pop();
                continue;
            }
            writer.list_next();
            token_pos = state.value_tokens_begin;
        }
        else if (state.type == StateType::Optional) {
            bool has_value = false;
            if (state.remaining) {
                has_value = reader.optional();
                writer.optional(has_value);
                state.remaining = 0;
            }
            if (!has_value) {
                token_pos = state.value_tokens_end;
                states.pop();
                continue;
            }
            token_pos = state.value_tokens_begin;
        }
        else if (state.type == StateType::Variant) {
            if (!state.remaining) {
                reader.variant_end();
                writer.variant_end();
                token_pos = state.value_tokens_end;
                states.pop();
                continue;
            }
            state.remaining = 0;
            token_pos = state.value_tokens_begin;
            // Fall-through to processing value below
        }
        else if (state.type == StateType::Binary) {
            if (state.remaining == 0) {
                token_pos = state.value_tokens_end;
                states.pop();
                if (!std::get_if<btoken::BinaryEnd>(&schema.tokens[token_pos])) {
                    throw LoadException("Invalid binary schema");
                }
                reader.binary_end();
                writer.list_end();
                token_pos++;
                continue;
            }
            writer.list_next();
            state.remaining--;
            token_pos = state.value_tokens_begin;
            // Fall-through to processing value below
        }

        const auto& token = schema.tokens[token_pos];
        token_pos++;

        if (std::get_if<btoken::ObjectBegin>(&token)) {
            states.push(State(StateType::None, 0, 0, 0));
            reader.object_begin();
            writer.object_begin();
            continue;
        }
        if (std::get_if<btoken::ObjectEnd>(&token)) {
            states.pop();
            reader.object_end();
            writer.object_end();
            continue;
        }
        if (auto value = std::get_if<btoken::ObjectNext>(&token)) {
            reader.object_next(value->key.c_str());
            writer.object_next(value->key.c_str());
            continue;
        }

        if (std::get_if<btoken::TupleBegin>(&token)) {
            states.push(State(StateType::None, 0, 0, 0));
            reader.tuple_begin();
            writer.tuple_begin();
            continue;
        }
        if (std::get_if<btoken::TupleEnd>(&token)) {
            states.pop();
            reader.tuple_end();
            writer.tuple_end();
            continue;
        }
        if (std::get_if<btoken::TupleNext>(&token)) {
            reader.tuple_next();
            writer.tuple_next();
            continue;
        }

        if (std::get_if<btoken::Map>(&token)) {
            reader.map_begin();
            writer.map_begin();
            states.push(State(
                StateType::Map,
                token_pos,
                get_tokens_end(schema.tokens, token_pos),
                0
            ));
            continue;
        }
        if (std::get_if<btoken::List>(&token)) {
            reader.list_begin();
            writer.list_begin();
            states.push(State(
                StateType::List,
                token_pos,
                get_tokens_end(schema.tokens, token_pos),
                0
            ));
            continue;
        }

        if (std::get_if<btoken::Optional>(&token)) {
            // Call writer.optional() elsewhere
            states.push(State(
                StateType::Optional,
                token_pos,
                get_tokens_end(schema.tokens, token_pos),
                1
            ));
            continue;
        }
        if (auto variant = std::get_if<btoken::VariantBegin>(&token)) {
            std::vector<const char*> labels_cstr;
            for (const auto& label: variant->labels) {
                labels_cstr.push_back(label.c_str());
            }

            reader.variant_begin(labels_cstr);

            bool found_match = false;
            std::size_t variant_start;
            // Move token_pos forward to the corresponding variant value
            while (true) {
                const auto& token = schema.tokens[token_pos];
                token_pos++;
                if (auto value = std::get_if<btoken::VariantNext>(&token)) {
                    if (reader.variant_match(value->type.c_str())) {
                        if (found_match) {
                            throw LoadException("Repeated variant labels");
                        }
                        found_match = true;
                        variant_start = token_pos;
                        writer.variant_begin(value->type.c_str(), labels_cstr);
                    }
                    token_pos = get_tokens_end(schema.tokens, token_pos);
                    continue;
                }
                else if (auto value = std::get_if<btoken::VariantEnd>(&token)) {
                    break;
                }
                throw LoadException("Invalid binary schema");
            }
            if (!found_match) {
                throw LoadException("No matching variant");
            }

            states.push(State(StateType::Variant, variant_start, token_pos, 1));
            continue;
        }
        if (auto binary = std::get_if<btoken::BinaryBegin>(&token)) {
            if (binary->stride == 0) {
                throw LoadException("Invalid binary schema");
            }
            std::size_t size = reader.binary_begin(binary->stride);
            std::size_t length = size / binary->stride;
            std::size_t value_end = get_tokens_end(schema.tokens, token_pos);

            writer.list_begin();
            states.push(State(
                StateType::Binary,
                token_pos,
                value_end,
                length
            ));
            continue;
        }

        if (std::get_if<std::int32_t>(&token)) {
            std::int32_t value;
            reader.value_i32(value);
            writer.value_i32(value);
        }
        else if (std::get_if<std::int64_t>(&token)) {
            std::int64_t value;
            reader.value_i64(value);
            writer.value_i64(value);
        }
        else if (std::get_if<std::uint32_t>(&token)) {
            std::uint32_t value;
            reader.value_u32(value);
            writer.value_u32(value);
        }
        else if (std::get_if<std::uint64_t>(&token)) {
            std::uint64_t value;
            reader.value_u64(value);
            writer.value_u64(value);
        }
        else if (std::get_if<float>(&token)) {
            float value;
            reader.value_f32(value);
            writer.value_f32(value);
        }
        else if (std::get_if<double>(&token)) {
            double value;
            reader.value_f64(value);
            writer.value_f64(value);
        }
        else if (std::get_if<bool>(&token)) {
            bool value;
            reader.value_bool(value);
            writer.value_bool(value);
        }
        else if (std::get_if<std::string>(&token)) {
            std::string value;
            reader.value_string(value);
            writer.value_string(value);
        }
        else if (auto value = std::get_if<btoken::Enumerate>(&token)) {
            std::vector<const char*> labels_cstr;
            for (const auto& label: value->labels) {
                labels_cstr.push_back(label.c_str());
            }
            int enum_value = reader.enumerate(labels_cstr);
            writer.enumerate(enum_value, labels_cstr);
        }
        else if (std::get_if<btoken::BinaryData>(&token)) {
            auto [data, size] = reader.binary_data();
            writer.binary_data(data, size);
        }
        else {
            throw LoadException("Shouldn't be here");
        }
    }

    return object;
}

} // namespace datapack
