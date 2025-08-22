#ifndef BASE64_FALLBACK_H
#define BASE64_FALLBACK_H

#include <Arduino.h>

// Fallback base64 encoder class (use this if mbedtls doesn't work)
class Base64Encoder {
public:
    static String encode(const uint8_t* data, size_t length) {
        static const char base64_chars[] = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";
        
        String result;
        result.reserve(((length + 2) / 3) * 4);
        
        // Process complete triplets
        size_t complete_triplets = length / 3;
        for (size_t i = 0; i < complete_triplets; i++) {
            uint32_t octet_a = data[i * 3];
            uint32_t octet_b = data[i * 3 + 1];
            uint32_t octet_c = data[i * 3 + 2];
            
            uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;
            
            result += base64_chars[(triple >> 18) & 0x3F];
            result += base64_chars[(triple >> 12) & 0x3F];
            result += base64_chars[(triple >> 6) & 0x3F];
            result += base64_chars[triple & 0x3F];
        }
        
        // Handle remaining bytes with proper padding
        size_t remaining = length % 3;
        if (remaining == 1) {
            uint32_t octet_a = data[length - 1];
            uint32_t triple = octet_a << 16;
            
            result += base64_chars[(triple >> 18) & 0x3F];
            result += base64_chars[(triple >> 12) & 0x3F];
            result += '=';
            result += '=';
        } else if (remaining == 2) {
            uint32_t octet_a = data[length - 2];
            uint32_t octet_b = data[length - 1];
            uint32_t triple = (octet_a << 16) + (octet_b << 8);
            
            result += base64_chars[(triple >> 18) & 0x3F];
            result += base64_chars[(triple >> 12) & 0x3F];
            result += base64_chars[(triple >> 6) & 0x3F];
            result += '=';
        }
        
        return result;
    }
};

// Compatibility class for HTTPClient library
class base64 {
public:
    static String encode(const String& data) {
        return Base64Encoder::encode((const uint8_t*)data.c_str(), data.length());
    }
    
    static String encode(const char* data) {
        return Base64Encoder::encode((const uint8_t*)data, strlen(data));
    }
    
    static String encode(const uint8_t* data, size_t length) {
        return Base64Encoder::encode(data, length);
    }
};

#endif // BASE64_FALLBACK_H
