#ifndef CODING_H
#define CODING_H

#include <string>

inline void encodeFix32(std::string *dst, uint32_t value) {
    char buf[sizeof(value)];
    
    uint8_t *buffer = reinterpret_cast<uint8_t*>(buf);
    buffer[0] = static_cast<uint8_t>(value);
    buffer[1] = static_cast<uint8_t>(value >> 8);
    buffer[2] = static_cast<uint8_t>(value >> 16);
    buffer[3] = static_cast<uint8_t>(value >> 24);

    dst->append(buf, sizeof(buf));
}

inline void encodeFix32(char *dst, uint32_t value) {
    uint8_t *buffer = reinterpret_cast<uint8_t*>(dst);
    buffer[0] = static_cast<uint8_t>(value);
    buffer[1] = static_cast<uint8_t>(value >> 8);
    buffer[2] = static_cast<uint8_t>(value >> 16);
    buffer[3] = static_cast<uint8_t>(value >> 24);
}

inline void encodeFix64(std::string *dst, uint64_t value) {
    char buf[sizeof(value)];

    uint8_t *buffer = reinterpret_cast<uint8_t*>(buf);
    buffer[0] = static_cast<uint8_t>(value);
    buffer[1] = static_cast<uint8_t>(value >> 8);
    buffer[2] = static_cast<uint8_t>(value >> 16);
    buffer[3] = static_cast<uint8_t>(value >> 24);
    buffer[4] = static_cast<uint8_t>(value >> 32);
    buffer[5] = static_cast<uint8_t>(value >> 40);
    buffer[6] = static_cast<uint8_t>(value >> 48);
    buffer[7] = static_cast<uint8_t>(value >> 56);
    
    dst->append(buf, sizeof(buf));
}

inline void encodeFix64(char *dst, uint64_t value) {
    char* buf = dst;

    uint8_t *buffer = reinterpret_cast<uint8_t*>(buf);
    buffer[0] = static_cast<uint8_t>(value);
    buffer[1] = static_cast<uint8_t>(value >> 8);
    buffer[2] = static_cast<uint8_t>(value >> 16);
    buffer[3] = static_cast<uint8_t>(value >> 24);
    buffer[4] = static_cast<uint8_t>(value >> 32);
    buffer[5] = static_cast<uint8_t>(value >> 40);
    buffer[6] = static_cast<uint8_t>(value >> 48);
    buffer[7] = static_cast<uint8_t>(value >> 56);
}

inline uint32_t decodeFix32(const char* ptr) {
    const uint8_t *buffer = reinterpret_cast<const uint8_t*>(ptr);
    return (static_cast<uint32_t>(buffer[0])) |
           (static_cast<uint32_t>(buffer[1]) << 8) | 
           (static_cast<uint32_t>(buffer[2]) << 16) |
           (static_cast<uint32_t>(buffer[3]) << 24);
}

inline uint64_t decodeFix64(const char* ptr) {
    const uint8_t *buffer = reinterpret_cast<const uint8_t*>(ptr);
    return (static_cast<uint64_t>(buffer[0])) |
           (static_cast<uint64_t>(buffer[1]) << 8)  | 
           (static_cast<uint64_t>(buffer[2]) << 16) |
           (static_cast<uint64_t>(buffer[3]) << 24) |
           (static_cast<uint64_t>(buffer[4]) << 32) |
           (static_cast<uint64_t>(buffer[5]) << 40) |
           (static_cast<uint64_t>(buffer[6]) << 48) |
           (static_cast<uint64_t>(buffer[7]) << 56) ;
}


inline void encodeVariant32(std::string *dst, uint32_t value) {
    char buf[5];
    
    uint8_t *ptr = reinterpret_cast<uint8_t*>(buf);
    static const int B = 128;
    if (value < (1 << 7)) {
        *(ptr++) = value;
    } else if (value < (1 << 14)) {
        *(ptr++) = value | B;
        *(ptr++) = value >> 7;
    } else if (value < (1 << 21)) {
        *(ptr++) = value | B;
        *(ptr++) = (value >> 7) | B;
        *(ptr++) = value >> 14;
    } else if (value < (1 << 28)) {
        *(ptr++) = value | B;
        *(ptr++) = (value >> 7) | B;
        *(ptr++) = (value >> 14) | B;
        *(ptr++) = value >> 21;
    } else {
        *(ptr++) = value | B;
        *(ptr++) = (value >> 7) | B;
        *(ptr++) = (value >> 14) | B;
        *(ptr++) = (value >> 21) | B;
        *(ptr++) = value >> 28;
    }
    
    dst->append(buf, reinterpret_cast<char*>(ptr) - buf);
}

inline void encodeVariant64(std::string *dst, uint64_t value) {
    char buf[10];

    uint8_t *ptr = reinterpret_cast<uint8_t*>(buf);
    static const int B = 128;
    while (value >= B) {
        *(ptr++) = value | B;
        value >>= 7;
    }
    *(ptr++) = static_cast<uint8_t>(value);    
    
    dst->append(buf, reinterpret_cast<char*>(ptr) - buf);
}

// inline int32_t DecodeVariant32(char* ptr) {
//     const uint8_t *buffer = reinterpret_cast<uint8_t*>(ptr);
//     return (static_cast<uint32_t>(buffer[0])) |
//            (static_cast<uint32_t>(buffer[1]) << 8) | 
//            (static_cast<uint32_t>(buffer[2]) << 16) |
//            (static_cast<uint32_t>(buffer[3]) << 24);
// }

// inline int64_t DecodeVariant64(char* ptr) {
//     const uint8_t *buffer = reinterpret_cast<uint8_t*>(ptr);
//     return (static_cast<uint32_t>(buffer[0])) |
//            (static_cast<uint32_t>(buffer[1]) << 8)  | 
//            (static_cast<uint32_t>(buffer[2]) << 16) |
//            (static_cast<uint32_t>(buffer[3]) << 24) |
//            (static_cast<uint32_t>(buffer[4]) << 32) |
//            (static_cast<uint32_t>(buffer[5]) << 40) |
//            (static_cast<uint32_t>(buffer[6]) << 48) |
//            (static_cast<uint32_t>(buffer[7]) << 56) ;
// }
#endif