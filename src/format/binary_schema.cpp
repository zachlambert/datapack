#include "datapack/format/binary_schema.hpp"
#include "datapack/util/object.hpp"
#include <cstring>


namespace datapack {

static std::size_t get_tokens_end(const std::vector<BToken>& tokens, std::size_t begin) {
    std::size_t pos = begin;
    std::size_t depth = 0;
    while (true){
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
        else if (std::get_if<btoken::Binary>(&token)){
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
        bool done; // Used for optional, variant, binary
        State(StateType type, std::size_t value_tokens_begin, std::size_t value_tokens_end):
            type(type),
            value_tokens_begin(value_tokens_begin),
            value_tokens_end(value_tokens_end),
            done(false)
        {}
    };
    std::stack<State> states;
    states.push(State(StateType::None, 0, 0));

    std::size_t token_pos = 0;
    std::size_t data_pos;
    while (true) {
        if (token_pos == schema.tokens.size()) {
            break;
        }

        auto& state = states.top();
        if (state.type == StateType::Map) {
            if (data_pos >= data.size()) {
                throw LoadException("Data too short");
            }
            std::uint8_t flag = data[data_pos];
            data_pos++;
            if (flag >= 0x02) {
                throw LoadException("Unexpected byte for bool");
            }
            bool has_next = flag;

            if (has_next) {
                const std::size_t max_len = data.size() - data_pos;
                std::size_t key_len = strnlen((char*)&data[data_pos], max_len);
                if (key_len == max_len) {
                    throw LoadException("Unterminated string");
                }
                writer.map_next((char*)&data[data_pos]);
                data_pos += (key_len + 1);
                token_pos = state.value_tokens_begin;
                // Fall-through to processing value below
            } else {
                token_pos = state.value_tokens_end;
                states.pop();
                writer.map_end();
                continue;
            }
        }
        else if (state.type == StateType::List) {
            if (data_pos >= data.size()) {
                throw LoadException("Data too short");
            }
            std::uint8_t flag = data[data_pos];
            data_pos++;
            if (flag >= 0x02) {
                throw LoadException("Unexpected byte for bool");
            }
            bool has_next = flag;

            if (has_next) {
                writer.list_next();
                token_pos = state.value_tokens_begin;
                // Fall-through to processing value below
            } else {
                token_pos = state.value_tokens_end;
                states.pop();
                writer.list_end();
                continue;
            }
        }
        else if (state.type == StateType::Optional) {
            bool has_value = false;

            if (!state.done) {
                state.done = true;

                if (data_pos >= data.size()) {
                    throw LoadException("Data too short");
                }
                std::uint8_t flag = data[data_pos];
                data_pos++;
                if (flag >= 0x02) {
                    throw LoadException("Unexpected byte for bool");
                }
                has_value = flag;
                writer.optional(has_value);
            }

            if (has_value) {
                token_pos = state.value_tokens_begin;
                // Fall-through to processing value below
            } else {
                token_pos = state.value_tokens_end;
                states.pop();
                continue;
            }
        }
        else if (state.type == StateType::Variant) {
            if (state.done) {
                token_pos = state.value_tokens_end;
                states.pop();
                continue;
            }
            state.done = true;
            // Fall-through to processing value below
        }
        else if (state.type == StateType::Binary) {
            if (state.done) {
                token_pos = state.value_tokens_end;
                states.pop();
                continue;
            }
            state.done = true;
            // Fall-through to processing value below
        }

        const auto& token = schema.tokens[token_pos];
        if (std::get_if<btoken::ObjectBegin>(&token)) {
            states.push(State(StateType::None, 0, 0));
            writer.object_begin();
            token_pos++;
            continue;
        }
        if (std::get_if<btoken::ObjectEnd>(&token)) {
            states.pop();
            writer.object_end();
            token_pos++;
            continue;
        }
        if (std::get_if<btoken::ObjectNext>(&token)) {
            const std::size_t max_len = data.size() - data_pos;
            std::size_t key_len = strnlen((char*)&data[data_pos], max_len);
            if (key_len == max_len) {
                throw LoadException("Unterminated string");
            }
            writer.object_next((char*)&data[data_pos]);
            data_pos += (key_len + 1);
            token_pos++;
            continue;
        }

        if (std::get_if<btoken::TupleBegin>(&token)) {
            states.push(State(StateType::None, 0, 0));
            writer.tuple_begin();
            token_pos++;
            continue;
        }
        if (std::get_if<btoken::TupleEnd>(&token)) {
            states.pop();
            writer.tuple_end();
            token_pos++;
            continue;
        }
        if (std::get_if<btoken::TupleNext>(&token)) {
            writer.tuple_next();
            token_pos++;
            continue;
        }

        if (std::get_if<btoken::Map>(&token)) {
            writer.map_begin();
            token_pos++;
            states.push(State(
                StateType::Map,
                token_pos,
                get_tokens_end(schema.tokens, token_pos)
            ));
            continue;
        }
        if (std::get_if<btoken::List>(&token)) {
            writer.list_begin();
            token_pos++;
            states.push(State(
                StateType::List,
                token_pos,
                get_tokens_end(schema.tokens, token_pos)
            ));
            continue;
        }

        if (std::get_if<btoken::Optional>(&token)) {
            // Call writer.optional() elsewhere
            token_pos++;
            states.push(State(
                StateType::Optional,
                token_pos,
                get_tokens_end(schema.tokens, token_pos)
            ));
            continue;
        }
        if (auto variant = std::get_if<btoken::VariantBegin>(&token)) {
            const std::size_t max_len = data.size() - data_pos;
            const char* variant_label = (char*)&data[data_pos];
            std::size_t label_len = strnlen(variant_label, max_len);
            if (label_len == max_len) {
                throw LoadException("Unterminated string");
            }

            std::vector<const char*> labels_cstr;
            for (const auto& label: variant->labels) {
                labels_cstr.push_back(label.c_str());
            }

            writer.variant_begin(variant_label, labels_cstr);
            data_pos += (label_len + 1);

            std::size_t variant_end = get_tokens_end(schema.tokens, token_pos);
            // Note: value_tokens_begin = token_pos unused below
            states.push(State(StateType::Variant, token_pos, variant_end));
            token_pos++;

            // Move token_pos forward to the corresponding variant value
            while (true) {
                const auto& token = schema.tokens[token_pos];
                token_pos++;
                if (auto value = std::get_if<btoken::VariantNext>(&token)) {
                    if (value->type == variant_label) {
                        break;
                    }
                    token_pos = get_tokens_end(schema.tokens, token_pos);
                }
                else if (auto value = std::get_if<btoken::VariantEnd>(&token)) {
                    throw LoadException("No matching variant");
                }
                throw LoadException("Invalid binary schema");
            }
            // token_pos now at the correct value
            continue;
        }
    }

    return object;
}

} // namespace datapack
