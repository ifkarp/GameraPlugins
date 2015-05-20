#ifndef stable_paths_toolkit_fill_white
#define stable_paths_toolkit_fill_white

#include "gamera.hpp"


using namespace std;
using namespace Gamera;
  /* This is a very simple plugin that simply fills the image with white

     See the Gamera plugin documentation for more information on how to
     write Gamera plugins.
   */
  template<class T>
  void clear(T& image) {
    std::fill(image.vec_begin(), image.vec_end(), white(image));
  }

  //=================== STABLE PATH FUNCTIONS ===================
  //=============================================================

  /*
	Preprocessing:
		1. Compute staffspaceheight and stafflineheight
		2. Compute weights of the graph

	Main Cycle:
		1. Compute stable paths
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

	//Copied from stableStaffLineFinder.h
	class stableStaffLineFinder {
	public:
		typedef int weight_t;
		enum e_NEIGHBOUR {NEIGHBOUR4 = 0, NEIGHBOUR8};
		typedef enum e_NEIGHBOUR NEIGHBOUR;

		struct NODE {
			Point previous;
			weight_t weight;
			Point start;
		};
		struct NODEGRAPH {
			weight_t weight_up;
			weight_t weight_hor;
			weight_t weight_down;
		};

		static const double MIN_BLACK_PER;
		static const weight_t TOP_VALUE;

		int* verRun; //length of vertical run of same color pixels. 
		int* verDistance; 
		NODE* graphPath;
		NODEGRAPH* graphWeight;

		string img_path;
		int staffLineHeight;
		int staffSpaceDistance;
		time_t globalStart;

		template<class T>
		void constructGraphWeights(T &image) {
			unsigned char WHITE = 0;

			//Find vertical run values
			// ***USE VECTOR ITERATORS WITH ROW ON THE OUTSIDE TO INCREASE PERFORMANCE IF THERE'S TIME***
			for (int c = 0; c < image.ncols(); c++) {
				int run = 0;
				unsigned char val = WHITE;
				for (int r = 0; r < image.nrows(); r++) {
					unsigned char pel = image.get(Point(r,c));
					if (pel == val) {
						run++;
					}
					else {
						int len = run;
						for (int row = r-1; len > 0; len--, row--) {
							verRun[row*image.ncols() + c] = run;
						}
						val = !val; //Changes value from 0 to 1 or from 1 to 0
					}
					if (run > 0) {
						//Last run on the column
						int len = run;
						for (int row = image.nrows()-1; len > 0; len--, row--) {
							verRun[row*image.ncols() + c] = run;
						}
					}
				}
			}

			//Find Vertical Distance
			for (int c = 0; c < image.ncols(); c++) {
				for (int r = 0; r < image.nrows(); r++) {
					unsigned char pel = image.get(Point(r,c));
					int row = r;
					unsigned char curr_pel = pel;
					while (row > 0 && curr_pel == pel) {
						row--;
						curr_pel = image.get(Point(r,c));
					}

					int run1 = 1;
					while (row > 0 && curr_pel != pel) {
						row--;
						curr_pel = image.get(Point(r,c));
						run1++;
					}

					row = r;
					curr_pel = pel;
					while (row < image.nrows()-1 && curr_pel == pel) {
						row++;
						curr_pel = image.get(Point(r,c));
					}

					int run2 = 1;
					while (row < image.nrows()-1 && curr_pel != pel) {
						row++;
						curr_pel = image.get(Point(r,c));
						run2++;
					}

					verDistance [r*image.ncols() + c] = min(run1, run2);
				}
			}

			//Find Graph Weights
			for (int r = 0; r < image.nrows(); r++) {
				for (int c = 0; c < image.ncols()-1; c++) {
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
		inline weight_t weightFunction(T &image, Point pixelVal1, Point pixelVal2, stableStaffLineFinder::NEIGHBOUR neigh) {
			unsigned int pel1 = image.get(pixelVal1); //Gets the pixel value of Point 1
			unsigned int pel2 = image.get(pixelVal2); //Gets pixel value of Point 2

			int dist1 = verDistance[pixelVal1.x()*image.nrows() + pixelVal1.y()]; //Vertical Distance taken from array of values created in constructGraphWeights
			int dist2 = verDistance[pixelVal2.x()*image.nrows() + pixelVal2.y()];
			int vRun1 = verRun[pixelVal1.x()*image.nrows() + pixelVal1.y()]; //Vertical Runs taken from array of values created in constructGraphWeights
			int vRun2 = verRun[pixelVal2.x()*image.nrows() + pixelVal2.y()]; 

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

	};

	template<class T>
  	float returnGraphWeights(T &image) {
	  	stableStaffLineFinder *tester = new stableStaffLineFinder();
	  	tester->constructGraphWeights(image);
	  	return tester->graphWeight[0].weight_down;
	}

#endif
