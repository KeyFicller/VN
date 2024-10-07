#pragma once

#include <string>
#include <functional>
#include <iostream>
#include <mutex>

namespace VN
{
    /*
     * @brief unified logging
     */
    template <typename ...Args>
    void vn_log(const std::string& category, const std::string& fmt, Args&&... args)  // NOLINT(cppcoreguidelines-missing-std-forward)
    {
        static std::mutex s_mutex;

        std::lock_guard<std::mutex> lkm(s_mutex);

        char buffer[1024];
        sprintf_s(buffer, sizeof(buffer), fmt.c_str(), std::forward<Args>(args)...);
        std::cout << category << ": " << buffer << std::endl;
    }
}
