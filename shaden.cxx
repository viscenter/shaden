/*
	SHADEN - The Sophisticated Handling of Artifacts Digitally Enhanced by Nrrd's
	Shaden Smith - Summer 2010

	What follows is the start of a port of Ryan Baumann's Seg3D plugin for 
	unwrapping scrolls. The port is incomplete and not guaranteed to work in any 
	way. This is just a start. 

	If anything is left undocumented you can probably ask Ryan Baumann. 
	
	See README file for more information.
*/

// System includes
#include <string>
#include <iostream>

// ITK includes
#include <itkImage.h>
#include <itkCommand.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

#include <itkImageLinearIteratorWithIndex.h>
#include <itkOtsuThresholdImageFilter.h>
#include <itkImageToImageFilter.h>

// OpenCV includes
#include <cv.h>

// Defines, taken exactly from Ryan's code
#define PI 3.14159
#define TORAD 1.0/PI
#define RADIAL_SAMPLES (720*2)
#define SOBEL_THRESH 100
#define MAX_LAYERS 1
#define THICKNESS 9
#define NUMUNWRAPS THICKNESS


// Typedefs for image types
typedef unsigned short PixelType;
const unsigned int Dimension = 2;
typedef itk::Image<PixelType, Dimension> ImageType;
typedef itk::ImageFileReader<ImageType> ReaderType;
typedef itk::ImageFileWriter<ImageType> WriterType;

// Functions
CvPoint CalcRayOuter(float theta, CvPoint center);
CvPoint GetCoordinateFromPosition(IplImage *image, CvLineIterator li, int p);
void radialSample(int width, int height, int slices, int data, float* ddata,
					CvMat *plookup, int slice);

int main(int argc, char** argv)
{
	if( argc < 3 )
	{
		std::cout << "Usage: " << argv[0] << "inputFile outputFile"
					<< std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// A direct port of Ryan's function
CvPoint CalcRayOuter(float theta, CvPoint center)
{
	CvPoint result;
	float hyp = (float) center.x;
	float dx = hyp * cos(theta);
	float dy = hyp * sin(theta);

	result.x = center.x - (int)dx;
	result.y = center.y - (int)dy;

	return result;
}

// A direct port of Ryan's function
CvPoint GetCoordinateFromPosition(IplImage *image, CvLineIterator li, int p)
{
	CvPoint result;
	int offset;

	for(int i = 0; i < p; i++)
	{
		CV_NEXT_LINE_POINT(li);
	}

	offset = li.ptr - (uchar*)(image->imageData);
	result.y = offset/image->widthStep;
	result.x = (offset - result.y * image->widthStep)/(sizeof(uchar));
	
	return result;
}

void RadialSample(int width, int height, int slices, int data, float* ddata,
					CvMat *plookup, int slice)
{
	IplImage *cvcast = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);

	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			//cvSet2D(cvcast, y, x, cvScalarAll(data[y*width+x]));
		}
	}

	CvMat *cvcastd = cvCreateMat(height, width, CV_32FC1);
	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			cvSet2D(cvcastd,y,x,cvScalarAll(ddata[y*width+x]));
		}
	}

	if(slice == 0)
	{
		//cvSaveImage("slicem.png", cvcast);
	}
	int cx = width/2;
	int cy = height/2;

	CvPoint center = cvPoint(cx,cy);
	unsigned char* linebuf;
	float* dlinebuf;
	for(int sample = 0; sample < RADIAL_SAMPLES; sample++) 
	{
		float theta = 0.785398163+((float)sample)*((2.0*PI)/(float)RADIAL_SAMPLES);
		CvPoint outer = CalcRayOuter(theta, center);

		// printf("%g:\t%d,%d\n", theta*(180.0/PI), outer.x, outer.y);
		cvClipLine(cvSize(width, height), &outer, &center);
		int linesize = abs(center.x-outer.x)+abs(center.y-outer.y)+1;
		
		linebuf = (unsigned char*)malloc(linesize);
		dlinebuf = (float*)malloc(sizeof(float)*linesize);
		
		cvSampleLine(cvcast,outer,center,linebuf,4);
		
		CvLineIterator curline;
		
		cvSampleLine(cvcastd,outer,center,dlinebuf,4);
		
		IplImage *castline = cvCreateImageHeader(cvSize(linesize,1), 
												IPL_DEPTH_8U, 1);
		castline->imageData = (char*)linebuf;

		int layer = 0;
		for(int i = 0; (i < linesize) && (layer < MAX_LAYERS); i++)
		{
			if((int)cvGetReal1D(castline,i) > 0)
			{
				float max = 0;
				int	max_i = 0;
				int min = 255, min_i = 0;
				int j = i;
				for(; (i < linesize) && ((i-j) < THICKNESS); i++)
				{
					int maskval = (int)cvGetReal1D(castline,i);
					float curval = dlinebuf[i];
					if(maskval == 0) break;
					if(curval > max) 
					{
						max = curval;
						max_i = i;
					}
					if(curval < min)
					{
						min = (int)curval;
						min_i = i;
					}
				}
				int sampledpos = j; // j+((i-j)/2);
				for(int unwrapPos = 0; unwrapPos < NUMUNWRAPS; unwrapPos++, sampledpos++) 
				{
					cvInitLineIterator(cvcast,outer,center,&curline,4);
					CvPoint sampledpoint = GetCoordinateFromPosition(cvcast,curline,sampledpos);
					
					//printf("T: %g, d: %d - %d,%d\n",theta,max_i,sampledpoint.x,sampledpoint.y);
					
					plookup->data.i[(unwrapPos*3*plookup->width*slices)+slice*3*plookup->width+((layer*RADIAL_SAMPLES)+sample)*3+0] = sampledpoint.x;
					plookup->data.i[(unwrapPos*3*plookup->width*slices)+slice*3*plookup->width+((layer*RADIAL_SAMPLES)+sample)*3+1] = sampledpoint.y;
					plookup->data.i[(unwrapPos*3*plookup->width*slices)+slice*3*plookup->width+((layer*RADIAL_SAMPLES)+sample)*3+2] = slice;
				}
				
				//cvSet2D(plookup,slice,(layer*RADIAL_SAMPLES)+sample,cvScalar(0,0,slice));
				
				layer++;
				// i = j + THICKNESS;
				
				while((i < linesize) && ((int)cvGetReal1D(castline,i)>0))
					i++;
			}
		}
		cvReleaseImageHeader(&castline);

		free(linebuf);
		free(dlinebuf);
	} // end for( int sample = 0; ..)
	cvReleaseImage(&cvcast);
	cvReleaseMat(&cvcastd);
} // end radialSample
