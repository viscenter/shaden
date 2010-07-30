

// An example on how to directly access a pixel in a 2d image and then write to file.


#include <string>
#include <iostream>

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

// Typedefs for image types. Get used to using typedefs all the time in ITK.
typedef unsigned short PixelType;
const unsigned int Dimension = 2;
typedef itk::Image<PixelType, Dimension> ImageType;
typedef itk::ImageFileReader<ImageType> ReaderType;
typedef itk::ImageFileWriter<ImageType> WriterType;
typedef ImageType::IndexType Point;

int main(int argc, char** argv)
{
	if(argc < 3)
	{
		std::cout << "Usage: " << argv[0] << " inputFile outputFile" << std::endl;
		return EXIT_FAILURE;
	}
	
	// These lines should explain quite clearly why we use so many typedefs.
	ReaderType::Pointer reader = ReaderType::New();
	WriterType::Pointer writer = WriterType::New();

	// Just set the IO filenames
	reader->SetFileName(argv[1]);
	writer->SetFileName(argv[2]);

	// Read the input into memory
	reader->Update();
	
	// ITK uses reference counting and smart pointers so garbage collection is handled
	// for you.
	ImageType::Pointer image = reader->GetOutput();

	// Most objects in ITK communicate this way, by directing IO to each other.
	// I like to think of this as piping.
	writer->SetInput(image);

	// Coordinate 0,0. pixelIndex[0] is the x-axis, [1] is the y-axis.
	// 0,0 is the bottom-left corner of the screen.
	Point pixelIndex;
	pixelIndex[0] = 0;
	pixelIndex[1] = 0;
	
	// Set pixel to white
	image->SetPixel(pixelIndex, 255);

	// Always wrap your update in exception handling.
	try
	{
		writer->Update();
	}
	catch( itk::ExceptionObject &ex )
	{
		std::cerr << "Exception caught!" << std::endl;
		std::cerr << ex << std::endl;
	}

	return EXIT_SUCCESS;
}
