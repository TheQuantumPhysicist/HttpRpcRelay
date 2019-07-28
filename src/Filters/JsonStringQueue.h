#ifndef JSONSTRINGQUEUE_H
#define JSONSTRINGQUEUE_H

#include <cassert>
#include <deque>
#include <stdexcept>
#include <string>
#include <vector>

class JsonStringQueue
{
    static const char     _openChar             = '{';
    static const char     _closeChar            = '}';
    static const uint32_t LARGEST_EXPECTED_JSON = (1 << 14);

    std::deque<char>         dataQueue;
    std::vector<std::string> parsedValuesQueue;
    uint32_t                 charCount    = 0;
    int32_t                  bracketLevel = 0;
    inline void                     parseAndAppendJson(const std::string& jsonStr);
    inline void                     parseAndAppendJson(std::string&& jsonStr);

public:
    inline void                     pushData(const std::string& data);
    inline std::vector<std::string> pullDataAndClear();
    inline void                     clear();
};

void JsonStringQueue::parseAndAppendJson(const std::string& jsonStr)
{
    parsedValuesQueue.push_back(jsonStr);
}

void JsonStringQueue::parseAndAppendJson(std::string&& jsonStr)
{
    parsedValuesQueue.push_back(std::move(jsonStr));
}

void JsonStringQueue::pushData(const std::string& data)
{
    dataQueue.insert(dataQueue.end(), data.cbegin(), data.cend());
    while (charCount < dataQueue.size()) {
        if (charCount > LARGEST_EXPECTED_JSON) {
            throw std::runtime_error("Huge unparsed json data");
        }
        if (dataQueue[charCount] == _openChar) {
            bracketLevel++;
            charCount++;
            continue;
        } else if (dataQueue[charCount] == _closeChar) {
            bracketLevel--;
            if (bracketLevel < 0) {
                throw std::runtime_error("Invalid bracket appeared in string: " +
                                         std::string(dataQueue.cbegin(), dataQueue.cend()));
            }
            charCount++;
            if (bracketLevel == 0) {
                parseAndAppendJson(std::string(dataQueue.begin(), dataQueue.begin() + charCount));
                dataQueue.erase(dataQueue.begin(), dataQueue.begin() + charCount);
                charCount -= std::distance(dataQueue.begin(), dataQueue.begin() + charCount);
                assert(charCount >= 0);
            }
            continue;
        } else {
            charCount++;
        }
    }
}

std::vector<std::string> JsonStringQueue::pullDataAndClear()
{
    std::vector<std::string> res = std::move(parsedValuesQueue);
    return res;
}

void JsonStringQueue::clear()
{
    dataQueue.clear();
    parsedValuesQueue.clear();
    charCount    = 0;
    bracketLevel = 0;
}

#endif // JSONSTRINGQUEUE_H
