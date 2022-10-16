#include "data.h"

struct Data::DataIndex {
    const char *key;
    void *from;
    void *to;
};

static uint8_t amount;

void Data::setup() {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    // 首个位置存储数据的数量
    amount = eeprom_read_byte(nullptr);
}

bool Data::save(const char *key, void *value, size_t size) {
    for (uint8_t i = 0; i < amount; i++) {
        DataIndex index = {};
        eeprom_read_block(&index, (DataIndex *) (i * sizeof(DataIndex) + 1), sizeof(DataIndex));
        if (strcmp(index.key, key) == 0) {
            // 已存在，覆盖
            uint8_t oldValueSize = reinterpret_cast<int>(index.to) - reinterpret_cast<int>(index.from);
            if (size > oldValueSize) {
                // 长度不够，需要扩容
                // TODO: 原有存储空间不够
                return false;
            } else {
                // 长度够，直接覆盖
                eeprom_write_block(value, index.from, size);

                // 更新索引
                index.to = reinterpret_cast<void *>(reinterpret_cast<int>(index.from) + size);
                eeprom_write_block(&index, (DataIndex *) (i * sizeof(DataIndex) + 1), sizeof(DataIndex));

                return true;
            }
        }
    }

    // 不存在，新增
    // TODO: 判断空间是否足够
    uint8_t from = amount * sizeof(DataIndex) + 1;
    DataIndex index = {key, reinterpret_cast<void *>(from), reinterpret_cast<void *>(from + size)};
    eeprom_write_block(&index, (DataIndex *) (amount * sizeof(DataIndex) + 1), sizeof(DataIndex));
    eeprom_write_block(value, reinterpret_cast<void *>(from), size);
    return true;
}

bool Data::save(const char *key, const char *value) {
    return save(key, (void *) value, strlen(value));
}

bool Data::save(const char *key, int value) {
    return save(key, (void *) &value, sizeof(int));
}

bool Data::del(const char *key) {
    for (uint8_t i = 0; i < amount; i++) {
        DataIndex index = {};
        eeprom_read_block(&index, (DataIndex *) (i * sizeof(DataIndex) + 1), sizeof(DataIndex));

        // 已存在，删除
        if (strcmp(index.key, key) == 0) {
            // 最后一个，直接缩小索引数量
            if (i == amount - 1) {
                amount--;
                return true;
            }
            uint8_t amountToMove = amount - i - 1;
            uint8_t sizeToMove = amountToMove * sizeof(DataIndex);
            uint8_t from = (i + 1) * sizeof(DataIndex) + 1;
            char buff[BUFFER_SIZE];
            for (uint16_t j = 0; j < sizeToMove; j += BUFFER_SIZE) {
                uint16_t size = sizeToMove - j;
                if (size > BUFFER_SIZE) size = BUFFER_SIZE;

                eeprom_read_block(buff, reinterpret_cast<void *>(from + j), size);
                eeprom_write_block(buff, reinterpret_cast<void *>(from + j - sizeof(DataIndex)), size);
            }
            amount--;
            return true;
        }
    }

    // 不存在，不需要删除
    return false;
}

Data::DataIndex *Data::get(const char *key) {
    for (size_t i = 0; i < amount; i++) {
        auto index = new DataIndex;
        eeprom_read_block(index, (DataIndex *) (i * sizeof(DataIndex) + 1), sizeof(DataIndex));
        if (strcmp(index->key, key) == 0) {
            return index;
        }
        delete index;
    }
    return nullptr;
}

const char *Data::getString(const char *key) {
    auto index = get(key);
    if (index == nullptr) return nullptr;

    size_t size = reinterpret_cast<int>(index->to) - reinterpret_cast<int>(index->from);
    const char *string = (const char *) malloc(size);
    eeprom_read_block((void *) string, index->from, size);
    delete index;

    return string;
}

int Data::getInt(const char *key) {
    auto index = get(key);
    if (index == nullptr) return 0;

    // 存储大小不是一个 int 的整数倍，直接返回 0
    if (reinterpret_cast<int>(index->to) - reinterpret_cast<int>(index->from) != sizeof(int)) return 0;

    int value;
    eeprom_read_block(&value, index->from, sizeof(int));
    return value;
}
