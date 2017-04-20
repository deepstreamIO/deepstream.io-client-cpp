#ifndef DEEPSTREAM_LIB_TYPE_SERIALIZER_HPP
#define DEEPSTREAM_LIB_TYPE_SERIALIZER_HPP

#include <deepstream/core/buffer.hpp> // Buffer
#include <deepstream/core/client.hpp> // PayloadType
#include <deepstream/core/error_handler.hpp> // ErrorHandler
#include <deepstream/lib/json.hpp> // nlohmann::json

namespace deepstream {

using json = nlohmann::json;

struct TypeSerializer {

public:
    TypeSerializer() = delete;

    TypeSerializer(const TypeSerializer &) = delete;

    TypeSerializer &operator=(const TypeSerializer &);

    TypeSerializer(ErrorHandler &error_handler)
        : error_handler_(error_handler)
    {
    }

    Buffer to_buffer(const json &data)
    {
        std::string str(data.dump());
        return Buffer(str);
    }

    PayloadType get_type(const json &data) const
    {
        if (data.is_object() || data.is_array()) {
            return PayloadType::OBJECT;
        } else if (data.is_string()) {
            return PayloadType::STRING;
        } else if (data.is_number()) {
            return PayloadType::NUMBER;
        } else if (data.is_boolean()) {
            return data ? PayloadType::TRUE : PayloadType::FALSE;
        } else if (data.is_null()) {
            return PayloadType::NULL_;
        } else {
            throw new std::runtime_error("Unable to serialize type: " + data.dump());
        }
    }

    Buffer to_prefixed_buffer(const json &data)
    {
        const PayloadType prefix = get_type(data);
        // only certain types should carry a payload
        switch (prefix) {
            case PayloadType::OBJECT:
            case PayloadType::NUMBER:
                {
                    std::string str = data.dump();
                    // prepend the prefix
                    str.insert(0, 1, static_cast<char>(prefix));
                    return Buffer(str);
                } break;
            case PayloadType::STRING:
                {
                    std::string str = data.get<std::string>();
                    str.insert(0, 1, static_cast<char>(prefix));
                    return Buffer(str);
                } break;
            case PayloadType::NULL_:
            case PayloadType::UNDEFINED:
            case PayloadType::TRUE:
            case PayloadType::FALSE:
                {
                    return Buffer{ static_cast<char>(prefix) };
                } break;
        }
    }

    json to_json(const Buffer &buff, const std::size_t offset = 0)
    {
        assert(offset <= buff.size());
        std::string str(buff.data() + offset, buff.size() - offset);
        try {
            return json::parse(str);
        } catch (std::invalid_argument e) {
            error_handler_.on_error("failed to parse object: " + str);
            return json(nullptr);
        }
    }

    json prefixed_to_json(const Buffer &buff)
    {
        if (buff.size() < 1) {
            error_handler_.on_error("Received unprefixed empty buffer");
            return nullptr;
        }
        const PayloadType prefix = static_cast<PayloadType>(buff[0]);
        switch (prefix) {
            case PayloadType::STRING:
                {
                    return std::string(buff.data() + 1, buff.size() - 1);
                } break;

            case PayloadType::NULL_:
            case PayloadType::UNDEFINED:
                {
                    return nullptr;
                } break;

            case PayloadType::TRUE:
                {
                    return true;
                } break;

            case PayloadType::FALSE:
                {
                    return false;
                } break;

            case PayloadType::OBJECT:
            case PayloadType::NUMBER:
                {
                    return to_json(buff, 1);
                } break;
        }
    }

private:
    ErrorHandler &error_handler_;
};
}

#endif // DEEPSTREAM_LIB_TYPE_SERIALIZER_HPP
