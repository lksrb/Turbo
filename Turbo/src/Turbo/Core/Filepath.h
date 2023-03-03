#pragma once

#include "Turbo/Core/String.h"

namespace Turbo 
{
    struct Filepath : public String
    {
    public:
        Filepath();

        Filepath(const Filepath&) = default;
        Filepath(Filepath&&) = default;
        Filepath(const std::string& str);
        Filepath(const char* str);

        // Assign operators
        Filepath& operator=(const char* other);
        Filepath& operator=(char* other);
        Filepath& operator=(const Filepath&) = default;

        // Compare operators
        bool operator==(const Filepath& other) const;
        bool operator!=(const Filepath& other) const;

        Filepath operator/(const char* other);
        Filepath operator/(const String64& other);
        Filepath& operator/=(const char* other);
        Filepath& operator/=(const Filepath& other);

        Filepath& operator/=(const String64& other);

        Filepath& Append(const char* str);

        // Gets filename WITHOUT EXTENSION from the path
        String64 Filename() const;

        // Gets root directory
        Filepath Directory() const;

        // Gets file extension
        String32 Extension() const;
    public:
        static void ConvertToBackslash(char* text);
    };
}

/*
template<typename OStream>
OStream& operator<<(OStream& os, const Turbo::Filepath& filepath)
{
    return os << '\"' << filepath.CStr() << '\"';
}
*/
