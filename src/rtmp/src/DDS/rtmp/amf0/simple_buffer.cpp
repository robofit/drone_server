#include "simple_buffer.h"

#include <assert.h>
//#include <algorithm>
#include <iterator>

SimpleBuffer::SimpleBuffer()
    : _pos(0)
{
}

SimpleBuffer::SimpleBuffer(int32_t size, int8_t value)
    : _pos(0)
{
    _data = std::vector<char>(size, value);
}

SimpleBuffer::~SimpleBuffer()
{
}

void SimpleBuffer::write_1byte(int8_t val)
{
    _data.push_back(val);
}

void SimpleBuffer::write_2bytes(int16_t val)
{
    char *p = (char *)&val;

    for (int i = 1; i >= 0; --i) {
        _data.push_back(p[i]);
    }
}

void SimpleBuffer::write_3bytes(int32_t val)
{
    char *p = (char *)&val;

    for (int i = 2; i >= 0; --i) {
        _data.push_back(p[i]);
    }
}

void SimpleBuffer::write_4bytes(int32_t val)
{
    char *p = (char *)&val;

    for (int i = 3; i >= 0; --i) {
        _data.push_back(p[i]);
    }
}

void SimpleBuffer::write_8bytes(int64_t val)
{
    char *p = (char *)&val;

    for (int i = 7; i >= 0; --i) {
        _data.push_back(p[i]);
    }
}

void SimpleBuffer::write_string(std::string val)
{
    std::copy(val.begin(), val.end(), std::back_inserter(_data));
}

void SimpleBuffer::append(const char* bytes, int size)
{
    if (!bytes || size <= 0)
        return;

    _data.insert(_data.end(), bytes, bytes + size);
}

int8_t SimpleBuffer::read_1byte()
{
    assert(require(1));

    int8_t val = _data.at(0 + _pos);
    _pos++;

    return val;
}

int16_t SimpleBuffer::read_2bytes()
{
    assert(require(2));

    int16_t val = 0;
    char *p = (char *)&val;

    for (int i = 1; i >= 0; --i) {
        p[i] = _data.at(0 + _pos);
        _pos++;
    }

    return val;
}

int32_t SimpleBuffer::read_3bytes()
{
    assert(require(3));

    int32_t val = 0;
    char *p = (char *)&val;

    for (int i = 2; i >= 0; --i) {
        p[i] = _data.at(0 + _pos);
        _pos++;
    }

    return val;
}

int32_t SimpleBuffer::read_4bytes()
{
    assert(require(4));

    int32_t val = 0;
    char *p = (char *)&val;

    for (int i = 3; i >= 0; --i) {
        p[i] = _data.at(0 + _pos);
        _pos++;
    }

    return val;
}

int64_t SimpleBuffer::read_8bytes()
{
    assert(require(8));

    int64_t val = 0;
    char *p = (char *)&val;

    for (int i = 7; i >= 0; --i) {
        p[i] = _data.at(0 + _pos);
        _pos++;
    }

    return val;
}

std::string SimpleBuffer::read_string(int len)
{
    assert(require(len));

    std::string val(&_data[0] + _pos, len);
    _pos += len;

    return val;
}

void SimpleBuffer::skip(int size)
{
    _pos += size;
}

bool SimpleBuffer::require(int required_size)
{
    assert(required_size >= 0);

    return required_size <= _data.size() - _pos;
}

bool SimpleBuffer::empty()
{
    return _pos >= _data.size();
}

int SimpleBuffer::size()
{
    return _data.size();
}

int SimpleBuffer::pos()
{
    return _pos;
}

char *SimpleBuffer::data()
{
    return (size() == 0) ? nullptr : &_data[0];
}

void SimpleBuffer::clear()
{
    _pos = 0;
    _data.clear();
}

void SimpleBuffer::set_data(int pos, const char *data, int len)
{
    if (!data)
        return;

    if (pos + len > size()) {
        return;
    }

    for (int i = 0; i < len; i++) {
        _data[pos + i] = data[i];
    }
}

std::string SimpleBuffer::to_string()
{
    return std::string(_data.begin(), _data.end());
}
