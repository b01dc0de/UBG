#include "UBG.h" // UBG.h includes Utils.h

#include <random>

void FileContentsT::Load(const char* FileName, bool bNullTerm)
{
    Release();

    FILE* FileHandle = nullptr;
    fopen_s(&FileHandle, FileName, "rb");
    if (FileHandle)
    {
        fseek(FileHandle, 0, SEEK_END);
        long FileSize = ftell(FileHandle);
        fseek(FileHandle, 0, SEEK_SET);

        u8* Buffer = nullptr;

        if (bNullTerm)
        {
            Buffer = new u8[FileSize + 1];
            fread_s(Buffer, FileSize, FileSize, 1, FileHandle);
            Buffer[FileSize] = '\0';
        }
        else
        {
            Buffer = new u8[FileSize];
            fread_s(Buffer, FileSize, FileSize, 1, FileHandle);
        }

        Size = FileSize;
        Contents = Buffer;

        fclose(FileHandle);
    }
}

void FileContentsT::Release()
{
    if (Contents)
    {
        delete[] Contents;
    }
    Size = 0;
    Contents = nullptr;
}

void GetDebugImage(ImageT& OutImage)
{
    unsigned int DebugImageLength = 16;
    OutImage.Width = DebugImageLength;
    OutImage.Height = DebugImageLength;
    OutImage.PxCount = OutImage.Width * OutImage.Height;
    OutImage.PxBufferSize = sizeof(RGBA32) * OutImage.PxCount;
    OutImage.PxBuffer = new RGBA32[OutImage.PxCount];

    const RGBA32 Pink{ 255u, 73u, 173u, 255u };
    const RGBA32 Black{ 0u, 0u, 0u, 255u };
    for (int PxIdx = 0; PxIdx < OutImage.PxCount; PxIdx++)
    {
        int PxRow = PxIdx / OutImage.Width;
        int PxCol = PxIdx % OutImage.Width;

        bool bEvenCell = (PxRow + PxCol) % 2 == 0;
        OutImage.PxBuffer[PxIdx] = bEvenCell ? Black : Pink;
    }
    int LastRowOffset = (OutImage.Height - 1) * OutImage.Width;
    int LastColOffset = OutImage.Width - 1;
    OutImage.PxBuffer[0] = { 255u, 0u, 0u, 255u }; // Red
    OutImage.PxBuffer[LastColOffset] = { 0u, 255u, 0u, 255u }; // Green
    OutImage.PxBuffer[LastRowOffset] = { 0u, 0u, 255u, 255u }; // Blue
    OutImage.PxBuffer[LastRowOffset + LastColOffset] = { 255u, 255u, 255u, 255u }; // White
}

#pragma pack(push, 1)
struct BMPFileHeader
{
    u16 Type; // Always Ascii BM
    u32 SizeInBytes; // Size (bytes) of file
    u16 Res1; // 0
    u16 Res2; // 0
    u32 OffsetBytes; // Offset (bytes) to actual pixel data
};

struct BMPInfoHeader
{
    u32 StructSize; // Size (bytes) of InfoHeader
    s32 Width;
    s32 Height; // NOTE(chris): If positive, pixel data is bottom to top
    u16 Planes; // Must be 1
    u16 BitsPerPixel; // Bits-per-pixel (0, 1, 4, 8, 16, 24, 32)
    u32 Compression; // *Should* be 0
    u32 Unused_ImgSize; // Only used if Compression is weird (not 0)
    s32 HRes; // Horizontal resolution
    s32 VRes; // Vertical resolution
    u32 ColorsUsed; // 0 for our purposes
    u32 ColorsImportant; // 0 for our purposes
};
struct BMPHeader
{
    BMPFileHeader FileHeader;
    BMPInfoHeader InfoHeader;
};
#pragma pack(pop)

void LoadBMPFile(const char* Filename, ImageT& OutImage)
{
    FileContentsT LoadedFile = {};
    LoadedFile.Load(Filename);
    if (LoadedFile.Size && LoadedFile.Contents)
    {
        u8* FileReadPtr = LoadedFile.Contents;
        BMPHeader BmpHdr = {};

        memcpy(&BmpHdr.FileHeader, FileReadPtr, sizeof(BMPFileHeader));
        FileReadPtr += sizeof(BMPFileHeader);
        memcpy(&BmpHdr.InfoHeader, FileReadPtr, sizeof(BMPInfoHeader));
        FileReadPtr += sizeof(BMPInfoHeader);

        if (BmpHdr.InfoHeader.BitsPerPixel == 32)
        {
            size_t BitmapFileSize = BmpHdr.FileHeader.SizeInBytes;
            size_t RemainingFileSize = BitmapFileSize - sizeof(BMPHeader);

            OutImage.Width = BmpHdr.InfoHeader.Width;
            OutImage.Height = BmpHdr.InfoHeader.Height;
            OutImage.PxCount = OutImage.Width * OutImage.Height;
            OutImage.PxBufferSize = sizeof(RGBA32) * OutImage.PxCount;
            OutImage.PxBuffer = new RGBA32[OutImage.PxCount];

            size_t ColorTableSize = RemainingFileSize - OutImage.PxBufferSize;
            constexpr bool bReadColorTable = false;
            if (bReadColorTable)
            {
                u8* ColorTable = new u8[ColorTableSize];
                memcpy(ColorTable, FileReadPtr, ColorTableSize);
                delete[] ColorTable;
            }
            FileReadPtr += ColorTableSize;
            if (BmpHdr.InfoHeader.Height < 0)
            {
                memcpy(OutImage.PxBuffer, FileReadPtr, OutImage.PxBufferSize);
            }
            else
            {
                size_t Stride = OutImage.Width * sizeof(RGBA32);
                RGBA32* PxBufferWrite = OutImage.PxBuffer;
                for (int RowIdx = OutImage.Height - 1; RowIdx >= 0; RowIdx--)
                {
                    u8* RowReadPtr = FileReadPtr + (Stride * RowIdx);
                    memcpy(PxBufferWrite, RowReadPtr, Stride);
                    PxBufferWrite += OutImage.Width;
                }
            }
        }

        for (size_t PxIdx = 0; PxIdx < OutImage.PxCount; PxIdx++)
        {
            RGBA32& PxColor = OutImage.PxBuffer[PxIdx];
            unsigned char Tmp = PxColor.R;
            PxColor.R = PxColor.B;
            PxColor.B = Tmp;
        }

        delete[] LoadedFile.Contents;
    }
}

int GetRandomInt(int Min, int Max)
{
    ASSERT(Min <= Max);
    static std::random_device RandomDevice{};
    static std::mt19937 MersenneTwisterEngine{ RandomDevice() };

    std::uniform_int_distribution<int> Distrib(Min, Max);

    return Distrib(MersenneTwisterEngine);
}

