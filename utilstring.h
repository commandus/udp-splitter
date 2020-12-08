#ifndef UTILSTRING_H_
#define UTILSTRING_H_

#include <string>

/**
 * @brief Return hex string
 * @param buffer buffer
 * @param size buffer size
 * @return hex string
 */
std::string hexString(const void *buffer, size_t size);

/**
 * @brief Return hex string
 * @param data binary data
 * @return string hex
 */
std::string hexString(const std::string &data);

/**
 * @brief Return binary data string
 * @param hex hex string
 * @return binary data string
 */
std::string hex2string(const std::string &hex);

#endif
