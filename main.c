#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

struct HEADER
{
    unsigned char riff[4];           // RIFF
    unsigned int fileSize;           // File size
    unsigned char wave[4];           // WAVE
    unsigned char formatString[4];   // Format string
    unsigned int sizeofFormatString; // Format length
    unsigned int formatType;         // Format type: 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
    unsigned int numberOfChannels;   // Number of channels
    unsigned int sampleRate;         // Sampling rate (blocks per second)
    unsigned int byteRate;           // Sample Rate * Number of Channels * BitsPerSample/8
    unsigned int blockAlign;         // Number of Channels * BitsPerSample/8
    unsigned int bitsPerSample;      // Bits per sample: 8- 8bits, 16- 16 bits etc
    unsigned char dataHeader [4];    // DATA string or FLLR string
    unsigned int sizeOfDataHeader;   // NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
} __attribute__((__packed__)) _header;

// Initialize header
void initStruct(struct HEADER *h)
{
    memset(&h->riff[0], 0, sizeof(h->riff));
    h->fileSize = 0;
    memset(&h->wave[0], 0, sizeof(h->wave));
    memset(&h->formatString[0], 0, sizeof(h->formatString));
    h->sizeofFormatString = 0;
    h->formatType = 0;
    h->numberOfChannels = 0;
    h->sampleRate = 0;
    h->byteRate = 0;
    h->blockAlign = 0;
    h->bitsPerSample = 0;
    memset(&h->dataHeader[0], 0, sizeof(h->dataHeader));
    h->sizeOfDataHeader = 0;
    printf("[+] -> Header initialized to known values.\n");
}

// Print the header
void printStruct(struct HEADER *h)
{
    printf("-----HEADER-----\n");
    printf("RIFF: %s\n", h->riff);
    printf("File Size: %d\n", h->fileSize);
    printf("WAVE: %s\n", h->wave);
    printf("Format String: %s\n", h->formatString);
    printf("Size of Format String: %d\n", h->sizeofFormatString);
    printf("Format Type: %d\n", h->formatType);
    printf("Number of Channels: %d\n", h->numberOfChannels);
    printf("Sample Rate: %d\n", h->sampleRate);
    printf("Byte Rate: %d\n", h->byteRate);
    printf("Block Align: %d\n", h->blockAlign);
    printf("Bits Per Sample: %d\n", h->bitsPerSample);
    printf("Data Header: %s\n", h->dataHeader);
    printf("Size of Data Header: %d\n", h->sizeOfDataHeader);
    printf("----------------\n");
}

// Write to file in Little Endian
void writeLE(unsigned int data, int byteSize, FILE *f)
{
    unsigned int buf;
    while(byteSize > 0)
    {   buf = data & 0xff;
        fwrite(&buf, 1,1, f);
        byteSize--;
        data >>= 8;
    }
}

int main(int argc, char *argv[])
{
    FILE *filePtr=NULL, *savePtr=NULL;
    char *readFile=NULL, *saveFile=NULL;
    unsigned char longBuf[4]={0,0,0,0}, shortBuf[2]={0,0};

    // Check arg count
    if(argc < 3)
    {
        printf("[!] -> No file specified! Suggested use: ./381 <wave input file> <wave output file>\n");
        return -1;
    }
    else // Try to open and read the file!
    {
        printf("[+] -> Attempting to open wave file.\n");
        // Get file name
        readFile = argv[1];
        // Get save file name
        saveFile = argv[2];
        // Open file to read
        filePtr = fopen(readFile, "rb");
        if(filePtr == NULL)
        {
            printf("[!] -> Error opening file.\n");
            return -1;
        }
        printf("[+] -> Parsing wave file!\n");

        // Initialize header struct
        initStruct(&_header);
        // Print the struct to verify initialize sequence
        printStruct(&_header);

        // Start reading the wave file
        // Read RIFF
        if(fread(_header.riff, sizeof(_header.riff), 1, filePtr) <= 0)
        {
            printf("[!] -> An error occurred reading the header entry RIFF.\n");
            return -1;
        }

        // Read file size
        if(fread(longBuf, sizeof(longBuf), 1, filePtr) <= 0)
        {
            printf("[!] -> An error occurred reading the header entry fileSize.\n");
            return -1;
        }
        else // Convert to big endian
            _header.fileSize = (longBuf[0] | longBuf[1]<<8 | longBuf[2] << 16 | longBuf[3] << 24);

        // Read WAVE string
        if(fread(_header.wave, sizeof(_header.wave), 1, filePtr) <= 0)
        {
            printf("[!] -> An error occurred reading the header entry wave.\n");
            return -1;
        }

        // Read Format string
        if(fread(_header.formatString, sizeof(_header.formatString), 1, filePtr) <= 0)
        {
            printf("[!] -> An error occurred reading the header entry Format String.\n");
            return -1;
        }

        // Read sizeofFormatString
        if(fread(longBuf, sizeof(longBuf), 1, filePtr) <= 0)
        {
            printf("[!] -> An error occurred reading the header entry sizeofFormatString.\n");
            return -1;
        }
        else // Convert to big endian
            _header.sizeofFormatString = (longBuf[0] | longBuf[1]<<8 | longBuf[2] << 16 | longBuf[3] << 24);

        // Read formatType
        if(fread(shortBuf, sizeof(shortBuf), 1, filePtr) <= 0)
        {
            printf("[!] -> An error occurred reading the header entry formatType.\n");
            return -1;
        }
        else // Convert to big endian
            _header.formatType = (shortBuf[0] | shortBuf[1]<<8);

        // Read numberOfChannels
        if(fread(shortBuf, sizeof(shortBuf), 1, filePtr) <= 0)
        {
            printf("[!] -> An error occurred reading the header entry numberOfChannels.\n");
            return -1;
        }
        else // Convert to big endian
            _header.numberOfChannels = (shortBuf[0] | shortBuf[1]<<8);

        // Read sampleRate
        if(fread(longBuf, sizeof(longBuf), 1, filePtr) <= 0)
        {
            printf("[!] -> An error occurred reading the header entry sampleRate.\n");
            return -1;
        }
        else // Convert to big endian
            _header.sampleRate = (longBuf[0] | longBuf[1]<<8 | longBuf[2] << 16 | longBuf[3] << 24);

        // Read byteRate
        if(fread(longBuf, sizeof(longBuf), 1, filePtr) <= 0)
        {
            printf("[!] -> An error occurred reading the header entry byteRate.\n");
            return -1;
        }
        else // Convert to big endian
            _header.byteRate = (longBuf[0] | longBuf[1]<<8 | longBuf[2] << 16 | longBuf[3] << 24);

        // Read blockAlign
        if(fread(shortBuf, sizeof(shortBuf), 1, filePtr) <= 0)
        {
            printf("[!] -> An error occurred reading the header entry blockAlign.\n");
            return -1;
        }
        else // Convert to big endian
            _header.blockAlign = (shortBuf[0] | shortBuf[1]<<8);

        // Read bitsPerSample
        if(fread(shortBuf, sizeof(shortBuf), 1, filePtr) <= 0)
        {
            printf("[!] -> An error occurred reading the header entry bitsPerSample.\n");
            return -1;
        }
        else // Convert to big endian
            _header.bitsPerSample = (shortBuf[0] | shortBuf[1]<<8);

        // Read dataHeader
        if(fread(_header.dataHeader, sizeof(_header.dataHeader), 1, filePtr) <= 0)
        {
            printf("[!] -> An error occurred reading the header entry dataHeader.\n");
            return -1;
        }

        // Read sizeOfDataHeader
        if(fread(longBuf, sizeof(longBuf), 1, filePtr) <= 0)
        {
            printf("[!] -> An error occurred reading the header entry byteRate.\n");
            return -1;
        }
        else // Convert to big endian
            _header.sizeOfDataHeader = (longBuf[0] | longBuf[1]<<8 | longBuf[2] << 16 | longBuf[3] << 24);

        // Check header
        printStruct(&_header);

        // All header values have been read in.. now we need to read in the samples one by one..
        // Setup variables to keep track of sample information
        savePtr = fopen(saveFile, "wb");
        long numberOfSamples = (8 * _header.sizeOfDataHeader) / (_header.numberOfChannels * _header.bitsPerSample);
        long sampleSize = (_header.numberOfChannels * _header.bitsPerSample) / 8;
        long bytesPerChannel = (sampleSize/_header.numberOfChannels);
        long upperLimit = 0, lowerLimit = 0; // Amplitude based on bits/sample
        // Calculate amplitude ranges
        switch(_header.bitsPerSample)
        {
        case 8:
            lowerLimit = -128;
            upperLimit = 127;
            break;
        case 16:
            lowerLimit = -32768;
            upperLimit = 32767;
            break;
        case 32:
            lowerLimit = -2147483648;
            upperLimit = 2147483647;
            break;
        default:
            lowerLimit = 0;
            upperLimit = 0;
        }

        // Read samples
        char dataBuf[sampleSize];
        int prevSample = 0;
        static int firstTime=0;
        memset(&dataBuf[0], 0, sizeof(dataBuf));
        printf("[+] -> Reading samples!\n");
        for(long i=1; i<numberOfSamples; i++)
        {
            if(fread(dataBuf, sizeof(dataBuf), 1, filePtr) <= 0)
            {
                printf("[!] -> Error reading data samples.\n");
                return -1;
            }
            else // process samples
            {
                int tmpData = 0;
                // Write the header first..
                fwrite("RIFF", 1, 4, savePtr);
                writeLE(_header.fileSize, 4, savePtr);
                fwrite("WAVE", 1, 4, savePtr);
                fwrite("fmt ", 1 ,4, savePtr);
                writeLE(_header.sizeofFormatString, 4, savePtr);
                writeLE(_header.formatType, 2, savePtr);
                writeLE(_header.numberOfChannels, 2, savePtr);
                writeLE(_header.sampleRate, 4, savePtr);
                writeLE(_header.byteRate, 4, savePtr);
                writeLE(_header.blockAlign, 2, savePtr);
                writeLE(_header.bitsPerSample, 2, savePtr);
                fwrite("data", 1, 4, savePtr);
                writeLE(_header.sizeOfDataHeader, 4, savePtr);
                for(unsigned int ch=0; ch<_header.numberOfChannels; ch++)
                {
                    // Use bytesPerChannel to determine how big the samepleSize is in bytes. This will allow us to keep track of the samples.
                    switch(bytesPerChannel)
                    {
                    case 4:
                        tmpData = (dataBuf[0] | dataBuf[1]<<8 | dataBuf[2] << 16 | dataBuf[3] << 24);
                        break;
                    case 2:
                        tmpData = (dataBuf[0] | dataBuf[1]<<8);
                        break;
                    case 1:
                        tmpData = dataBuf[0];
                        break;
                    default:
                        printf("[!] -> Error calculating bytesPerChannel!\n");
                        return -1;
                    }

                    // Calculate new signal and write it.
                    if(firstTime==0)
                        prevSample = tmpData;
                    int avgSample = (tmpData + prevSample)/2;
                    int amplitude=8000, frequency=2400, sampleRate=_header.sampleRate;
//                    printf("[+] -> Average Sample ((cur+prev)/2): %d\n", tmpData);
//                    printf("[+] -> Adding sine wave A*sin((2*pi*f)/sampleRate): %d*sin((2*pi*%d)/%d)\n", amplitude, frequency, sampleRate);
                    int addSinWave = amplitude * sin((2*M_PI*frequency)/sampleRate);
                    avgSample = addSinWave + avgSample;
                    if(avgSample < lowerLimit)
                        avgSample = lowerLimit;
                    else if(avgSample > upperLimit)
                        avgSample = upperLimit;
//                    printf("[+] -> New sample to be written: %d\n", avgSample);
                    writeLE(avgSample, sampleSize, savePtr);
                }
                prevSample = tmpData;
            }
        }
    }
    printf("[+] -> Done! Closing Files.\n");
    fclose(savePtr);
    fclose(filePtr);
    return 0;
}
