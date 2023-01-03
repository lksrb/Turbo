#include "tbopch.h"
#include "Filepath.h"

namespace Turbo 
{
    Filepath::Filepath()
        : FString256()
    {
    }

    Filepath::Filepath(const char* str)
        : FString256(str)
    {
    }

    Filepath::Filepath(const std::string& str)
        : FString256(str.c_str())
    {
    }

    Filepath& Filepath::operator=(char* other)
    {
        FString256::operator=(other);

        return *this;
    }

    Filepath& Filepath::operator=(const char* other)
    {
        FString256::operator=(other);

        return *this;
    }

    bool Filepath::operator==(const Filepath& other) const
    {
        return strcmp(m_Buffer, other.m_Buffer) == 0;
    }

    bool Filepath::operator!=(const Filepath& other) const
    {
        return Filepath::operator==(other) == false;
    }

    Filepath Filepath::operator/(const char* other)
    {
        TBO_ENGINE_ASSERT((strlen(m_Buffer) + strlen(other)) < Cap());

        Filepath fp;
        strcat_s(fp.m_Buffer, m_Buffer);
        strcat_s(fp.m_Buffer, "\\");
        strcat_s(fp.m_Buffer, other);
        fp.m_Size = strlen(m_Buffer);

        return fp;
    }

    Filepath& Filepath::operator/=(const char* other)
    {
        TBO_ENGINE_ASSERT((strlen(m_Buffer) + strlen(other) + 1) < Cap());

        strcat_s(m_Buffer, "\\");
        strcat_s(m_Buffer, other);

        m_Size = strlen(m_Buffer);
        return *this;
    }

    Filepath& Filepath::operator/=(const FString64& other)
    {
        return Filepath::operator/=(other.c_str());
    }

    Filepath Filepath::operator/(const FString64& other)
    {
        return Filepath::operator/(other.c_str());
    }

    Filepath& Filepath::operator/=(const Filepath& other)
    {
        return Filepath::operator/=(other.c_str());
    }

    Filepath& Filepath::Append(const char* str)
    {
        TBO_ENGINE_ASSERT((strlen(m_Buffer) + strlen(str) + 1) < Cap());

        strcat_s(m_Buffer, str);

        m_Size = strlen(m_Buffer);

        return *this;
    }

    FString64 Filepath::Filename() const
    {
        if (Empty())
            return FString64{};

        size_t j = 0;
        FString64 fileName;
        char reversed[64]{ 0 };

        for (size_t i = m_Size - 1; i > 0; --i)
        {
            if (m_Buffer[i] == '\\')
            {
                break;
            }
            reversed[j] = m_Buffer[i];

            ++j;
        }

        char undo[64]{ 0 };
        size_t i = 0;
        for (i = 0; i < j; ++i)
        {
            undo[i] = reversed[j - 1 - i];
        }

        for (i = 0; i < j; ++i)
        {
            if (undo[i] == '.')
            {
                // Null-terminate it so strcpy can copy only part before it
                undo[i] = '\0';
                break;
            }
        }
        fileName = undo;
        return fileName;
    }

    Filepath Filepath::Directory() const
    {
        size_t j = 0;

        for (size_t i = m_Size - 1; i > 0; --i)
        {
            if (m_Buffer[i] == '\\')
                break;
            ++j;

        }

        char cut[256]{ 0 };

        for (size_t i = 0; i < m_Size - j - 1; ++i)
        {
            cut[i] = m_Buffer[i];
        }

        Filepath filePath = cut;

        return filePath;
    }

    FString32 Filepath::Extension()
    {
        if (Empty())
            return FString32{};

        size_t j = 0;
        char reversed[32]{ 0 };
        FString32 ext;
        for (size_t i = m_Size - 1; i > 0; --i)
        {
            reversed[j] = m_Buffer[i];
            if (reversed[j] == '.')
            {
                break;
            }
            else if (reversed[j] == '\\')
            {
                // File does not an extension
                return FString32{};
            }
            ++j;
        }

        char undo[32]{ 0 };
        for (size_t i = 0; i < j + 1; ++i)
        {
            undo[i] = reversed[j - i];
        }

        ext = undo;

        return ext;
    }

    // Static

    void Filepath::ConvertToBackslash(char* text)
    {
        size_t size = strlen(text);

        for (size_t i = 0; i < size; ++i)
        {
            if (text[i] == '/')
            {
                text[i] = '\\';
            }
        }
    }

}
