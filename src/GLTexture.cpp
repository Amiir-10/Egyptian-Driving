//////////////////////////////////////////////////////////////////////
//
// OpenGL Texture Class
// by: Matthew Fairfax
//
// GLTexture.cpp: implementation of the GLTexture class.
// This class loads a texture file and prepares it
// to be used in OpenGL. It can open a bitmap or a
// targa file. The min filter is set to mipmap b/c
// they look better and the performance cost on
// modern video cards in negligible. I leave all of
// the texture management to the application. I have
// included the ability to load the texture from a
// Visual Studio resource. The bitmap's id must be
// be surrounded by quotation marks (i.e. "Texture.bmp").
// The targa files must be in a resource type of "TGA"
// (including the quotes). The targa's id must be
// surrounded by quotation marks (i.e. "Texture.tga").
//
// Usage:
// GLTexture tex;
// GLTexture tex1;
// GLTexture tex3;
//
// tex.Load("texture.bmp"); // Loads a bitmap
// tex.Use();				// Binds the bitmap for use
//
// tex1.LoadFromResource("texture.tga"); // Loads a targa
// tex1.Use();				 // Binds the targa for use
//
// // You can also build a texture with a single color and use it
// tex3.BuildColorTexture(255, 0, 0);	// Builds a solid red texture
// tex3.Use();				 // Binds the targa for use
//
//////////////////////////////////////////////////////////////////////

#include "GLTexture.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// Cross-platform helper: duplicate a string
static char *str_dup(const char *str)
{
	size_t len = strlen(str) + 1;
	char *copy = (char *)malloc(len);
	if (copy)
		memcpy(copy, str, len);
	return copy;
}

// Cross-platform helper: convert string to lowercase in-place
static char *str_to_lower(char *str)
{
	for (char *p = str; *p; ++p)
		*p = tolower((unsigned char)*p);
	return str;
}

//////////////////////////////////////////////////////////////////////
// AUX_RGBImageRec structure and custom BMP loader
// (Replaces the MSVC-only glaux library)
//////////////////////////////////////////////////////////////////////

typedef struct _AUX_RGBImageRec
{
	GLint sizeX, sizeY;
	unsigned char *data;
} AUX_RGBImageRec;

// Custom BMP loader to replace auxDIBImageLoad
static AUX_RGBImageRec *auxDIBImageLoad(const char *filename)
{
	FILE *file = fopen(filename, "rb");
	if (!file)
		return NULL;

	// Read BMP header
	unsigned char header[54];
	if (fread(header, 1, 54, file) != 54)
	{
		fclose(file);
		return NULL;
	}

	// Check BMP signature
	if (header[0] != 'B' || header[1] != 'M')
	{
		fclose(file);
		return NULL;
	}

	// Extract image info from header
	int dataOffset = *(int *)&header[10];
	int width = *(int *)&header[18];
	int height = *(int *)&header[22];
	short bitsPerPixel = *(short *)&header[28];

	// Only support 24-bit BMPs
	if (bitsPerPixel != 24)
	{
		fclose(file);
		return NULL;
	}

	// Calculate row size (rows are padded to 4-byte boundaries)
	int rowSize = ((width * 3 + 3) / 4) * 4;
	int imageSize = rowSize * height;

	// Allocate memory for image data
	unsigned char *rawData = (unsigned char *)malloc(imageSize);
	if (!rawData)
	{
		fclose(file);
		return NULL;
	}

	// Seek to pixel data and read
	fseek(file, dataOffset, SEEK_SET);
	if (fread(rawData, 1, imageSize, file) != (size_t)imageSize)
	{
		free(rawData);
		fclose(file);
		return NULL;
	}
	fclose(file);

	// Create the result structure
	AUX_RGBImageRec *result = (AUX_RGBImageRec *)malloc(sizeof(AUX_RGBImageRec));
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

	// Convert BGR to RGB and remove padding
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int srcIdx = y * rowSize + x * 3;
			int dstIdx = y * width * 3 + x * 3;
			result->data[dstIdx + 0] = rawData[srcIdx + 2]; // R
			result->data[dstIdx + 1] = rawData[srcIdx + 1]; // G
			result->data[dstIdx + 2] = rawData[srcIdx + 0]; // B
		}
	}

	free(rawData);
	return result;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GLTexture::GLTexture()
{
}

GLTexture::~GLTexture()
{
}

void GLTexture::Load(char *name)
{
	// make the texture name all lower case
	texturename = str_to_lower(str_dup(name));

	// strip "'s
	if (strstr(texturename, "\""))
		texturename = strtok(texturename, "\"");

	// check the file extension to see what type of texture
	if (strstr(texturename, ".bmp"))
		LoadBMP(texturename);
	if (strstr(texturename, ".tga"))
		LoadTGA(texturename);
}

void GLTexture::LoadFromResource(char *name)
{
	// make the texture name all lower case
	texturename = str_to_lower(str_dup(name));

	// check the file extension to see what type of texture
	if (strstr(texturename, ".bmp"))
		LoadBMPResource(name);
	if (strstr(texturename, ".tga"))
		LoadTGAResource(name);
}

void GLTexture::Use()
{
	glEnable(GL_TEXTURE_2D);				  // Enable texture mapping
	glBindTexture(GL_TEXTURE_2D, texture[0]); // Bind the texture as the current one
}

void GLTexture::LoadBMP(char *name)
{
	// Create a place to store the texture
	AUX_RGBImageRec *TextureImage[1];

	// Set the pointer to NULL
	memset(TextureImage, 0, sizeof(void *) * 1);

	// Load the bitmap and assign our pointer to it
	TextureImage[0] = auxDIBImageLoad(name);

	// If the texture file was not found, return from the function
	if (!TextureImage[0])
	{
		return;
	}

	// Skip textures that are too large - they cause OpenGL issues
	if (TextureImage[0]->sizeX > 1024 || TextureImage[0]->sizeY > 1024)
	{
		// Free the large image
		if (TextureImage[0]->data)
			free(TextureImage[0]->data);
		free(TextureImage[0]);

		// Build a simple colored texture as placeholder
		BuildColorTexture(128, 128, 128);
		return;
	}

	// Just in case we want to use the width and height later
	width = TextureImage[0]->sizeX;
	height = TextureImage[0]->sizeY;

	// Generate the OpenGL texture id
	glGenTextures(1, &texture[0]);

	// Bind this texture to its id
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	// Use mipmapping filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Generate the mipmaps
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);

	// Cleanup
	if (TextureImage[0])
	{
		if (TextureImage[0]->data)
			free(TextureImage[0]->data);

		free(TextureImage[0]);
	}
}

void GLTexture::LoadTGA(char *name)
{
	GLubyte TGAheader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // Uncompressed TGA header
	GLubyte TGAcompare[12];										  // Used to compare TGA header
	GLubyte header[6];											  // First 6 useful bytes of the header
	GLuint bytesPerPixel;										  // Holds the number of bytes per pixel used
	GLuint imageSize;											  // Used to store the image size
	GLuint temp;												  // Temporary variable
	GLuint type = GL_RGBA;										  // Set the default type to RBGA (32 BPP)
	GLubyte *imageData;											  // Image data (up to 32 Bits)
	GLuint bpp;													  // Image color depth in bits per pixel.

	FILE *file = fopen(name, "rb"); // Open the TGA file

	// Load the file and perform checks
	if (file == NULL ||															// Does file exist?
		fread(TGAcompare, 1, sizeof(TGAcompare), file) != sizeof(TGAcompare) || // Are there 12 bytes to read?
		memcmp(TGAheader, TGAcompare, sizeof(TGAheader)) != 0 ||				// Is it the right format?
		fread(header, 1, sizeof(header), file) != sizeof(header))				// If so then read the next 6 header bytes
	{
		if (file == NULL) // If the file didn't exist then return
			return;
		else
		{
			fclose(file); // If something broke then close the file and return
			return;
		}
	}

	// Determine the TGA width and height (highbyte*256+lowbyte)
	width = header[1] * 256 + header[0];
	height = header[3] * 256 + header[2];

	// Check to make sure the targa is valid and is 24 bit or 32 bit
	if (width <= 0 ||						  // Is the width less than or equal to zero
		height <= 0 ||						  // Is the height less than or equal to zero
		(header[4] != 24 && header[4] != 32)) // Is it 24 or 32 bit?
	{
		fclose(file); // If anything didn't check out then close the file and return
		return;
	}

	bpp = header[4];							// Grab the bits per pixel
	bytesPerPixel = bpp / 8;					// Divide by 8 to get the bytes per pixel
	imageSize = width * height * bytesPerPixel; // Calculate the memory required for the data

	// Allocate the memory for the image data
	imageData = new GLubyte[imageSize];

	// Make sure the data is allocated write and load it
	if (imageData == NULL ||							   // Does the memory storage exist?
		fread(imageData, 1, imageSize, file) != imageSize) // Does the image size match the memory reserved?
	{
		if (imageData != NULL) // Was the image data loaded
			free(imageData);   // If so, then release the image data

		fclose(file); // Close the file
		return;
	}

	// Loop through the image data and swap the 1st and 3rd bytes (red and blue)
	for (GLuint i = 0; i < int(imageSize); i += bytesPerPixel)
	{
		temp = imageData[i];
		imageData[i] = imageData[i + 2];
		imageData[i + 2] = temp;
	}

	// We are done with the file so close it
	fclose(file);

	// Set the type
	if (bpp == 24)
		type = GL_RGB;

	// Generate the OpenGL texture id
	glGenTextures(1, &texture[0]);

	// Bind this texture to its id
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	// Use mipmapping filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Generate the mipmaps
	gluBuild2DMipmaps(GL_TEXTURE_2D, type, width, height, type, GL_UNSIGNED_BYTE, imageData);

	// Cleanup
	free(imageData);
}

void GLTexture::LoadBMPResource(char *name)
{
	// Find the bitmap in the bitmap resources
	HRSRC hrsrc = FindResource(0, name, RT_BITMAP);

	// If you can't find it then return
	if (hrsrc == 0)
		return;

	// Load the bitmap
	HGLOBAL resource = LoadResource(0, hrsrc);

	// If you can't load it then return
	if (resource == 0)
		return;

	// Load it into the buffer
	void *buffer = LockResource(resource);

	// Cast it into a bitmap
	BITMAP *bmp = (BITMAP *)buffer;

	// Get the height and width for future use
	width = bmp->bmWidth;
	height = bmp->bmHeight;

	// Reverse the blue colot bit and the red color bit
	unsigned char *ptr = (unsigned char *)buffer + sizeof(BITMAPINFO) + 2;
	unsigned char temp;

	for (int i = 0; i < width * height; i++)
	{
		temp = ptr[i * 3];
		ptr[i * 3] = ptr[i * 3 + 2];
		ptr[i * 3 + 2] = temp;
	}

	// Generate the OpenGL texture id
	glGenTextures(1, &texture[0]);

	// Bind this texture to its id
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	// Use mipmapping filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Generate the mipmaps
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, (unsigned char *)buffer + sizeof(BITMAPINFO) + 2);
	// gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, bmp->bmBits);

	// Cleanup
	free(buffer);
	free(bmp);
}

void GLTexture::LoadTGAResource(char *name)
{
	// struct to cast the resource into
	struct TGAstruct
	{
		GLubyte TGAcompare[12]; // Used to compare TGA header
		GLubyte header[6];		// First 6 useful bytes of the header
	};

	GLubyte TGAheader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // Uncompressed TGA header
	GLuint bytesPerPixel;										  // Holds the number of bytes per pixel used
	GLuint imageSize;											  // Used to store the image size
	GLuint temp;												  // Temporary variable
	GLuint type = GL_RGBA;										  // Set the default type to RBGA (32 BPP)
	GLubyte *imageData;											  // Image data (up to 32 Bits)
	GLuint bpp;													  // Image color depth in bits per pixel.

	// Find the targa in the "TGA" resources
	HRSRC hrsrc = FindResource(0, name, "TGA");

	// If you can't find it then return
	if (hrsrc == 0)
		return;

	// Load the targa
	HGLOBAL resource = LoadResource(0, hrsrc);

	// If you can't load it then return
	if (resource == 0)
		return;

	// Load it into the buffer
	void *buffer = LockResource(resource);

	// Cast it into the targa struct
	TGAstruct *top = (TGAstruct *)buffer;

	// Make sure it checks out against our comparison header
	if (memcmp(TGAheader, top, sizeof(TGAheader)) != 0)
		return;

	// Determine the TGA width and height (highbyte*256+lowbyte)
	width = top->header[1] * 256 + top->header[0];
	height = top->header[3] * 256 + top->header[2];

	// Check to make sure the targa is valid and is 24 bit or 32 bit
	if (width <= 0 ||									// Is the width less than or equal to zero
		height <= 0 ||									// Is the height less than or equal to zero
		(top->header[4] != 24 && top->header[4] != 32)) // Is it 24 or 32 bit?
	{
		// If anything didn't check out then close the file and return
		return;
	}

	bpp = top->header[4];						// Grab the bits per pixel
	bytesPerPixel = bpp / 8;					// Divide by 8 to get the bytes per pixel
	imageSize = width * height * bytesPerPixel; // Calculate the memory required for the data

	// Allocate the memory for the image data
	imageData = new GLubyte[imageSize];

	// Load the data in
	memcpy(imageData, (GLubyte *)buffer + 18, imageSize);

	// Loop through the image data and swap the 1st and 3rd bytes (red and blue)
	for (GLuint i = 0; i < int(imageSize); i += bytesPerPixel)
	{
		temp = imageData[i];
		imageData[i] = imageData[i + 2];
		imageData[i + 2] = temp;
	}

	// Set the type
	if (bpp == 24)
		type = GL_RGB;

	// Generate the OpenGL texture id
	glGenTextures(1, &texture[0]);

	// Bind this texture to its id
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	// Use mipmapping filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Generate the mipmaps
	gluBuild2DMipmaps(GL_TEXTURE_2D, type, width, height, type, GL_UNSIGNED_BYTE, imageData);

	// Cleanup
	free(imageData);
	free(buffer);
	free(top);
}

void GLTexture::BuildColorTexture(unsigned char r, unsigned char g, unsigned char b)
{
	unsigned char data[12]; // a 2x2 texture at 24 bits

	// Store the data
	for (int i = 0; i < 12; i += 3)
	{
		data[i] = r;
		data[i + 1] = g;
		data[i + 2] = b;
	}

	// Generate the OpenGL texture id
	glGenTextures(1, &texture[0]);

	// Bind this texture to its id
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Use mipmapping filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Generate the texture
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, 2, 2, GL_RGB, GL_UNSIGNED_BYTE, data);
}
