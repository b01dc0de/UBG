#ifndef UTILS_H
#define UTILS_H

struct FileContentsT
{
    size_t Size;
    u8* Contents;

    void Load(const char* FileName, bool bNullTerm = false);
    void Release();
};

void GetDebugImage(ImageT& OutImage);
void LoadBMPFile(const char* Filename, ImageT& OutImage);

int GetRandomInt(int Min, int Max);

v4f GetRandomColorDim();

#endif // UTILS_H

