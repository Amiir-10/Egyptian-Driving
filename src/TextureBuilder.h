#ifndef TEXTURE_BUILDER_H
#define TEXTURE_BUILDER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <GL/glut.h>

// Removed glew.h and glaux.h - not needed for MinGW
// Removed #pragma comment(lib, ...) - MSVC-only

//////////////////////////////////////////////////////////////////////
// AUX_RGBImageRec structure and custom BMP loader
//////////////////////////////////////////////////////////////////////

typedef struct _AUX_RGBImageRec_TB
{
    GLint sizeX, sizeY;
    unsigned char *data;
} AUX_RGBImageRec_TB;

// Custom BMP loader
static AUX_RGBImageRec_TB *auxDIBImageLoadA_TB(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
        return NULL;

    unsigned char header[54];
    if (fread(header, 1, 54, file) != 54)
    {
        fclose(file);
        return NULL;
    }
    if (header[0] != 'B' || header[1] != 'M')
    {
        fclose(file);
        return NULL;
    }

    int dataOffset = *(int *)&header[10];
    int width = *(int *)&header[18];
    int height = *(int *)&header[22];
    short bitsPerPixel = *(short *)&header[28];

    if (bitsPerPixel != 24)
    {
        fclose(file);
        return NULL;
    }

    int rowSize = ((width * 3 + 3) / 4) * 4;
    int imageSize = rowSize * height;

    unsigned char *rawData = (unsigned char *)malloc(imageSize);
    if (!rawData)
    {
        fclose(file);
        return NULL;
    }

    fseek(file, dataOffset, SEEK_SET);
    if (fread(rawData, 1, imageSize, file) != (size_t)imageSize)
    {
        free(rawData);
        fclose(file);
        return NULL;
    }
    fclose(file);

    AUX_RGBImageRec_TB *result = (AUX_RGBImageRec_TB *)malloc(sizeof(AUX_RGBImageRec_TB));
    if (!result)
    {
        free(rawData);
        return NULL;
    }

    result->sizeX = width;
    result->sizeY = height;
    result->data = (unsigned char *)malloc(width * height * 3);
    if (!result->data)
    {
        free(rawData);
        free(result);
        return NULL;
    }

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int srcIdx = y * rowSize + x * 3;
            int dstIdx = y * width * 3 + x * 3;
            result->data[dstIdx + 0] = rawData[srcIdx + 2];
            result->data[dstIdx + 1] = rawData[srcIdx + 1];
            result->data[dstIdx + 2] = rawData[srcIdx + 0];
        }
    }
    free(rawData);
    return result;
}

static void loadPPM(GLuint *textureID, char *strFileName, int width, int height, int wrap)
{
    BYTE *data;
    FILE *pFile = fopen(strFileName, "rb");
    if (pFile)
    {
        data = (BYTE *)malloc(width * height * 3);
        fread(data, 1, width * height * 3, pFile);
        fclose(pFile);
    }
    else
    {
        fprintf(stderr, "Error: Texture file not found: %s\n", strFileName);
        return;
    }

    glGenTextures(1, textureID);
    glBindTexture(GL_TEXTURE_2D, *textureID);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap ? GL_REPEAT : GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap ? GL_REPEAT : GL_CLAMP);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    free(data);
}

static void loadBMP(GLuint *textureID, char *strFileName, int wrap)
{
    AUX_RGBImageRec_TB *pBitmap = auxDIBImageLoadA_TB(strFileName);
    if (!pBitmap)
    {
        fprintf(stderr, "Error: Texture file not found or invalid: %s\n", strFileName);
        return;
    }

    glGenTextures(1, textureID);
    glBindTexture(GL_TEXTURE_2D, *textureID);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, pBitmap->sizeX, pBitmap->sizeY, GL_RGB, GL_UNSIGNED_BYTE, pBitmap->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap ? GL_REPEAT : GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap ? GL_REPEAT : GL_CLAMP);

    if (pBitmap)
    {
        if (pBitmap->data)
            free(pBitmap->data);
        free(pBitmap);
    }
}

#endif // TEXTURE_BUILDER_H