
#pragma once

#include <exception>
#include <string>

namespace XD {
    /// @brief 统一的异常处理类
    class Exce : public std::exception {
    public:
        /// @brief 统一的异常处理类
        /// @param line 异常触发的行号
        /// @param file 异常触发的文件名
        Exce(int line, const char* file, const char* info = "") noexcept;
        /// @brief 异常内容
        /// @return
        const char* what() const noexcept override;
        /// @brief 异常类型
        /// @return
        virtual const char* getType() const noexcept;
        /// @brief 异常详细信息
        /// @return
        std::string getInfo() const noexcept;

        /// @brief 异常所在行号
        /// @return
        int getLine() const noexcept { return _line; }
        /// @brief 异常所在文件名
        /// @return
        const std::string& getFile() const noexcept { return _file; }

    private:
        int _line;
        std::string _file;
        std::string _info;

    protected:
        mutable std::string wharBuffer;
    };
}