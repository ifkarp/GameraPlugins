
  //=================== STABLE PATH FUNCTIONS ===================
  //=============================================================

  /*
	Preprocessing:
		1. Compute staffspaceheight and stafflineheight
		*2. Compute weights of the graph

	Main Cycle:
		*1. Compute stable paths
		2. Validate paths with blackness and shape
		3. Erase valid paths from image
		4. Add valid paths to list of stafflines
		5. End of cycle if no valid path was found

	Postprocessing
		1. Uncross stafflines
		2. Organize stafflines in staves
		3. Smooth and trim stafflines

		Notes:
			- Big difference is that in original code the black values are assumed to have a value of 0, whites have any other value
			- Currently being implemented only for one bit images
	*/

#include <vector>

using namespace std;
using namespace Gamera;

//Copied from stableStaffLineFinder.h
class stableStaffLineFinder {
public:
	typedef int weight_t;
	enum e_NEIGHBOUR {NEIGHBOUR4 = 0, NEIGHBOUR8};
	typedef enum e_NEIGHBOUR NEIGHBOUR;

	struct NODE 
	{
		Point previous;
		weight_t weight;
		Point start;
	};
	struct NODEGRAPH 
	{
		weight_t weight_up;
		weight_t weight_hor;
		weight_t weight_down;
	};

	//Values taken from stableStaffLineFinder.cpp lines 106-107
	static const double MIN_BLACK_PER = 0.25;
	static const weight_t TOP_VALUE = (INT_MAX/2);

	int* verRun; //length of vertical run of same color pixels. 
	int* verDistance; 
	NODE* graphPath;
	NODEGRAPH* graphWeight;

	string img_path;
	int staffLineHeight;
	int staffSpaceDistance;
	time_t globalStart;

	template<class T>
	void constructGraphWeights(T &image) 
	{
		unsigned char WHITE = 0;

		//Find vertical run values
		// ***USE VECTOR ITERATORS WITH ROW ON THE OUTSIDE TO INCREASE PERFORMANCE IF THERE'S TIME***
		for (int c = 0; c < image.ncols(); c++) 
		{
			int run = 0;
			unsigned char val = WHITE;
			for (int r = 0; r < image.nrows(); r++) 
			{
				unsigned char pel = image.get(Point(r,c));
				if (pel == val) 
				{
					run++;
				}
				else {
					int len = run;
					for (int row = r-1; len > 0; len--, row--) 
					{
						verRun[row*image.ncols() + c] = run;
					}
					val = !val; //Changes value from 0 to 1 or from 1 to 0
				}
				if (run > 0) 
				{
					//Last run on the column
					int len = run;
					for (int row = image.nrows()-1; len > 0; len--, row--) 
					{
						verRun[row*image.ncols() + c] = run;
					}
				}
			}
		}

		//Find Vertical Distance
		for (int c = 0; c < image.ncols(); c++) 
		{
			for (int r = 0; r < image.nrows(); r++) 
			{
				unsigned char pel = image.get(Point(r,c));
				int row = r;
				unsigned char curr_pel = pel;
				while (row > 0 && curr_pel == pel) 
				{
					row--;
					curr_pel = image.get(Point(r,c));
				}

				int run1 = 1;
				while (row > 0 && curr_pel != pel) 
				{
					row--;
					curr_pel = image.get(Point(r,c));
					run1++;
				}

				row = r;
				curr_pel = pel;
				while (row < image.nrows()-1 && curr_pel == pel) 
				{
					row++;
					curr_pel = image.get(Point(r,c));
				}

				int run2 = 1;
				while (row < image.nrows()-1 && curr_pel != pel) 
				{
					row++;
					curr_pel = image.get(Point(r,c));
					run2++;
				}

				verDistance [r*image.ncols() + c] = min(run1, run2);
			}
		}

		//Find Graph Weights
		for (int r = 0; r < image.nrows(); r++) 
		{
			for (int c = 0; c < image.ncols()-1; c++) 
			{
				graphWeight[r*image.ncols() + c].weight_hor = weightFunction(image, Point(r,c), Point(r, c+1), NEIGHBOUR4);
				if (r > 0)
					graphWeight[r*image.ncols() + c].weight_up = weightFunction(image, Point(r,c), Point(r-1, c+1), NEIGHBOUR8);
				else
					graphWeight[r*image.ncols() + c].weight_up = TOP_VALUE;
				if (r < image.nrows()-1)
					graphWeight[r*image.ncols() + c].weight_down = weightFunction(image, Point(r,c), Point(r+1, c+1), NEIGHBOUR8);
				else
					graphWeight[r*image.ncols() + c].weight_down = TOP_VALUE;
			}
		}		
	}

	template<class T>
	weight_t weightFunction(T &image, Point pixelVal1, Point pixelVal2, stableStaffLineFinder::NEIGHBOUR neigh) 
	{
		unsigned int pel1 = image.get(pixelVal1); //Gets the pixel value of Point 1
		unsigned int pel2 = image.get(pixelVal2); //Gets pixel value of Point 2

		int dist1 = verDistance[pixelVal1.y()*image.nrows() + pixelVal1.x()]; //Vertical Distance taken from array of values created in constructGraphWeights
		int dist2 = verDistance[pixelVal2.y()*image.nrows() + pixelVal2.x()];
		int vRun1 = verRun[pixelVal1.y()*image.nrows() + pixelVal1.x()]; //Vertical Runs taken from array of values created in constructGraphWeights
		int vRun2 = verRun[pixelVal2.y()*image.nrows() + pixelVal2.x()]; 

		int pel = min(pel1, pel2); //take the minimum pixel value
		
		//Weights for a 4-Neighborhood
		int y1 = 4; //Black pixels
		int y0 = 8; //White pixels
		if (neigh == NEIGHBOUR8) //Weights for an 8-Neighborhood
		{
			y1 = 6; //Black
			y0 = 12; //White
		}
		int y = (pel == 0 ? y0:y1);
		if ( (pel) && (min(vRun1, vRun2) <= staffLineHeight))
			--y;
		if (max(dist1, dist2) > 2*staffLineHeight+staffSpaceDistance)
			y++;
		return y;
	}

	template<class T>
	stableStaffLineFinder(T &image) 
	{
		graphPath = new NODE[image.nrows()*image.ncols()];
		graphWeight = new NODEGRAPH[image.nrows()*image.ncols()];
		verRun = new int[image.nrows()*image.ncols()];
		verDistance = new int[image.nrows()*image.ncols()];
		memset (verDistance, 0, sizeof(int)*image.nrows()*image.ncols());
		staffLineHeight = image.nrows()*image.ncols();
	}

	template<class T>
	int findAllStablePaths(T &image, int startCol, int endCol, std::vector <vector<Point> > &stablePaths) 
	{
		stablePaths.clear();
		int width = image.ncols();
		int height = image.nrows();
		vector<int> startRow_i;

		int endCol_i = width - 1 - startCol;
		int startCol_i = width - 1 - endCol;

		for (int row = 0; row < height; row++) 
		{
			graphPath[row*width + startCol_i].weight = static_cast<weight_t>(0);
			graphPath[row*width + startCol_i].start = Point(startCol_i, row);
		}

		for (int col = startCol_i + 1; col <= endCol_i; col++) 
		{
			for (int row = 0; row < height; row++) 
			{
				weight_t weight10, weight20, weight30;
				weight_t value1, value2, value3;
				weight10 = weight20 = weight30 = TOP_VALUE;
				value1 = value2 = value3 = TOP_VALUE;

				if (row > 0) 
				{
					weight10 = graphWeight[row*width + width-1-col].weight_up;
					value1 = weight10 + graphPath[(row-1)*width + (col-1)].weight;
				}

				weight20 = graphWeight[row*width + width-1-col].weight_hor;
				value2 = weight20 + graphPath[(row+0)*width + (col-1)].weight;
			
				if (row < height-1)
				{
					weight30 = graphWeight[row*width + width-1-col].weight_down; 
					value3 = weight30 + graphPath[(row+1)*width + (col-1)].weight;
				}	

				if ((value3)<= (value2) && (value3)<= (value1))
				{				
					graphPath[(row)*width + (col)].previous = Point(col-1, row+1);
					graphPath[(row)*width + (col)].weight = value3;
					graphPath[(row)*width + (col)].start = graphPath[(row+1)*width + (col-1)].start;
				}

				else if ((value2)<= (value1) && (value2)<= (value3))
				{
					graphPath[(row)*width + (col)].previous = Point(col-1, row);
					graphPath[(row)*width + (col)].weight = value2;
					graphPath[(row)*width + (col)].start = graphPath[(row+0)*width + (col-1)].start;
				}

				else
				{
					graphPath[(row)*width + (col)].previous = Point(col-1, row-1);
					graphPath[(row)*width + (col)].weight = value1;
					graphPath[(row)*width + (col)].start = graphPath[(row-1)*width + (col-1)].start;
				}	
			}
		}

		for (int i = 0; i < height; i++)
		{
			startRow_i.push_back(graphPath[i*width + endCol_i].start.y());
		}

		for (int row = 0; row < height; row++)
		{
			graphPath[row*width + startCol].weight = static_cast<weight_t>(0.0);
			graphPath[row*width + startCol].start = Point(startCol, row);
		}

		for (int c = startCol+1; c <= endCol; c++)
		{
			int col = c;

			for (int row = 0; row < height; row++)
			{
				weight_t weight10, weight20, weight30;
				weight_t value1, value2, value3;
				weight10 = weight20 = weight30 = TOP_VALUE;
				value1 = value2 = value3  = TOP_VALUE; 

				if (row > 0)
				{
					weight10 = graphWeight[(row-1)*width + col-1].weight_down;
					value1 = weight10 + graphPath[(row-1)*width + (col-1)].weight;
				}

				weight20 = graphWeight[row*width + col-1].weight_hor; 
				value2 = weight20 + graphPath[(row+0)*width + (col-1)].weight;

				if (row < height-1)
				{
					weight30 = graphWeight[(row+1)*width + col-1].weight_up; 
					value3 = weight30 + graphPath[(row+1)*width + (col-1)].weight;
				}

				if ((value3)<= (value2) && (value3)<= (value1))
				{
					graphPath[(row)*width + (col)].previous = Point(col-1, row+1);
					graphPath[(row)*width + (col)].weight = value3;
					graphPath[(row)*width + (col)].start = graphPath[(row+1)*width + (col-1)].start;
				}

				else if ((value2)<= (value1) && (value2)<= (value3))
				{
					graphPath[(row)*width + (col)].previous = Point(col-1, row);
					graphPath[(row)*width + (col)].weight = value2;
					graphPath[(row)*width + (col)].start = graphPath[(row+0)*width + (col-1)].start;
				}

				else 
				{
					graphPath[(row)*width + (col)].previous = Point(col-1, row-1);
					graphPath[(row)*width + (col)].weight = value1;
					graphPath[(row)*width + (col)].start = graphPath[(row-1)*width + (col-1)].start;
				}
			}
		}

		for (int i = 0; i < height; i++)
		{
			int startR = graphPath[i*width + endCol].start.y();

			if (startRow_i[startR] == i)
			{
				Point p = Point(endCol, i);
				vector<Point> contour;
				contour.resize(endCol - startCol + 1);
				int pos = endCol - startCol;
				contour[pos] = p;
				pos--;
				while (p.x() != startCol)
				{
					p = graphPath[p.y()*width + p.x()].previous;
					contour[pos] = p;
					pos--;
				}
				stablePaths.push_back(contour);
			}
		}
		return 0;
	}

	//Used for testing
	int fillValues() 
	{
		int x;
		for (x = 0; x < staffLineHeight; x++) 
		{
			verRun[x] = 1;
		}
		return x;
	}
};


template<class T>
float returnGraphWeights(T &image) 
{
	vector <vector<Point> > validStaves;
	stableStaffLineFinder slf1 (image);
	slf1.constructGraphWeights(image);
	return slf1.verRun[500];
}