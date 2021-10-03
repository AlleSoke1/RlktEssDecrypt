#include <windows.h>
#include <stdio.h>
#include <fstream>
#include <iostream>

#include <string>
#include <vector>


#define _REPLACE( src_str, old_part, new_part ) \
do \
{ \
    std::wstring wstrOld = old_part; \
    std::wstring wstrNew = new_part; \
    while( true ) \
    { \
        size_t iPos = src_str.find( wstrOld ); \
        if( iPos == std::wstring::npos ) \
        { \
            break; \
        } \
        src_str.replace( iPos, wstrOld.size(), wstrNew ); \
    } \
} while( false )


bool LoadStringTable(std::wstring& wstrFileName, std::vector< std::wstring >& vecStringTable)
{
    vecStringTable.clear();

    FILE* fp = NULL;
    _wfopen_s(&fp, wstrFileName.c_str(), L"rb");

    if (!fp)
    {
        return false;
    }

    // UTF-16LE의 BOM( Byte Order Mark )을 뛰어 넘는다.
    int iRetFSeek = fseek(fp, 2, SEEK_CUR);
    if (iRetFSeek != 0)
    {
        return false;
    }


    const int ciMaxStringLength = 2048;
    wchar_t wszStringRead[ciMaxStringLength];
    int iIndex = 0;
    while (fgetws(wszStringRead, ciMaxStringLength, fp))
    {
        std::wstring wstrStringRead(wszStringRead);
        std::wstring wstrFullString;

        size_t iEnd = wstrStringRead.find(L"\r\n");
        if (iEnd != std::wstring::npos)
        {
            wstrStringRead.erase(iEnd);
        }

        short iKey = 16;
        int nSize = wstrStringRead.length();
        for (int i = 0; i < nSize; i++)
        {
            wstrStringRead[i] = wstrStringRead[i] ^ iKey;
        }

        size_t iTabPos = wstrStringRead.find(L"\t");
        std::wstring wstrIndex = wstrStringRead.substr(0, iTabPos);
        int iIndexRead = _wtoi(wstrIndex.c_str());

        std::wstring wstrContent;
        if (iIndexRead == iIndex)
        {
            if (iTabPos != std::wstring::npos)
            {
           
                if ((iTabPos + 1) < wstrStringRead.size())
                {
                    wstrContent = wstrStringRead.substr(iTabPos + 1);
                }
            }
        }

        _REPLACE(wstrContent, L"\\n", L"\r\n");

        vecStringTable.push_back(wstrContent);
        iIndex++;
    }

    int iRetFClose = fclose(fp);

    return true;
}

BYTE* GetBytesFromFile(std::string filename, DWORD& dwPackedSize)
{
    std::fstream file(filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
    if (file.is_open())
    {
        int nSize = file.tellg();
        BYTE* pData = new BYTE[nSize];
        memset(pData, 0, nSize);
        file.seekg(0, std::ios::beg);

        file.read((char*)pData, nSize);
        file.close();

        dwPackedSize = nSize;

        return pData;
    }
    return NULL;
}

bool BytesToFile(std::string filename, BYTE* pData, long nSize)
{
    std::ofstream file(filename.c_str(), std::ios::out | std::ios::binary);
    if (file.is_open())
    {
        file.write((const char*)pData, nSize);
        file.close();
        return true;
    }
    return false;
}

int main(int argc, char** argv)
{
    if (argc < 2)
        return 1;

    std::string filename(argv[1]);
    std::wstring wfilename(filename.begin(),filename.end());
    std::vector<std::wstring> vecStrings;
    if (LoadStringTable(wfilename, vecStrings))
    {
        std::wstring outputString;
        int index = 0;
        for (const auto& it : vecStrings)
        {
            outputString += std::to_wstring(index++) + L" = " +  it + L"\r\n";
        }

        BytesToFile(filename + ".txt", (BYTE*)outputString.data(), outputString.size() * 2);
    }

    return 0;
}