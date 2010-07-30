// This is a sample Confidence Connected filter. Most of this code is also 
// found in the Examples directory that ITK provides, however I modified
// some of it and posted it here. More documentation never hurts.

// Several parts of this are left uncommented because of the exact code found
// in my directAccess.cxx example file. I would recommend reading through that
// first. This file should be a tutorial on how to use filters in ITK.
#include <iostream>

#include "itkImage.h"
#include "itkCastImageFilter.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkConfidenceConnectedImageFilter.h"

int main( int argc, char** argv )
{
	if( argc < 5 )
	{
		std::cerr << "Usage: " << argv[0] << " inputFile outputFile seedX seedY" 
			<< std::endl;
		return EXIT_FAILURE;
	}

	typedef unsigned short PixelType;
	const unsigned int Dimension = 2;
	typedef itk::Image<PixelType, Dimension> ImageType;

	typedef itk::ImageFileReader<ImageType> ReaderType;
	typedef itk::ImageFileWriter<ImageType> WriterType;
	ReaderType::Pointer reader = ReaderType::New();
	WriterType::Pointer writer = WriterType::New();

	reader->SetFileName( argv[1] );
	writer->SetFileName( argv[2] );

	// Setting up filters are pretty similar to any other class in ITK.
	typedef itk::ConfidenceConnectedImageFilter<ImageType, ImageType> ConnectedFilterType;
	ConnectedFilterType::Pointer confConnected = ConnectedFilterType::New();

	// Just set the filter to receive its input from the reader, then pipe that to the writer.
	confConnected->SetInput( reader->GetOutput() );
	writer->SetInput( confConnected->GetOutput() );
	
	// Some confConnected-specific parameters. Feel free to noodle with these
	// values to get a different result.
	confConnected->SetMultiplier( 2.5 );
	confConnected->SetNumberOfIterations( 2 );
	confConnected->SetReplaceValue( 255 );

	// Pass x,y for the seed via command line
	ImageType::IndexType index;
	index[0] = atoi( argv[3] );
	index[1] = atoi( argv[4] );

	confConnected->SetSeed( index );
	confConnected->SetInitialNeighborhoodRadius( 2 );

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
