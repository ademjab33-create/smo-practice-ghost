#pragma once

namespace pe {

enum class PeepaLanguage {
    English,
    Japanese,
    German,
    French
};

const char* getLocalizedString(const char* msg);

} // namespace pe
