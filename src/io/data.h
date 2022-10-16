#ifndef WATCHDOOG_DATA_H
#define WATCHDOOG_DATA_H

#include <Arduino.h>

#define BUFFER_SIZE 32

#ifdef __cplusplus
extern "C" {
#endif

class Data {
public:
    static const unsigned int TOTAL_BYTES = 1024;

    static void setup();

    static bool save(const char *key, const char *value);

    static bool save(const char *key, int value);

    static bool del(const char *key);

    static const char *getString(const char *key);

    static int getInt(const char *key);

private:
    struct DataIndex;

    static bool save(const char *key, void *value, size_t size);

    static Data::DataIndex *get(const char *key);
};

#ifdef __cplusplus
}
#endif

#endif //WATCHDOOG_DATA_H
