#ifndef UTILS_H
#define UTILS_H

struct FileContentsT
{
    size_t Size = 0;
    u8* Contents = nullptr;
};

FileContentsT LoadFileContents(const char* FileName, bool bNullTerm = false);

struct RGBA32
{
    u8 R;
    u8 G;
    u8 B;
    u8 A;
};

struct ImageT
{
    u32 Width;
    u32 Height;
    size_t PxCount;
    size_t PxBufferSize;
    RGBA32* PxBuffer;
};

void GetDebugImage(ImageT& OutImage);
void LoadBMPFile(const char* Filename, ImageT& OutImage);

int GetRandomInt(int Min, int Max);

#endif // UTILS_H

