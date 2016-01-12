#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string> 
#include <sstream>
using namespace std;

#define MAXVAL 100

template <typename T>
string NumberToString ( T Number )
{
	ostringstream ss;
    ss << Number;
    return ss.str();
}

typedef struct {
	int x, y, z;
} pixel;

void generatePPM(pixel** matrix, int width, int height) {
	
	srand(time(NULL));
	string filename;
	filename = "image" + NumberToString(rand()) + ".ppm";
	ofstream in(filename.c_str());
	in << "P6\n" << width << " " << height << "\n" << MAXVAL;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			in << matrix[i][j].x << " " << matrix[i][j].y << " " << matrix[i][j].z << "\t";
		}
	}
	in.close();
	return;	
}

int main()
{
	pixel** m;
	generatePPM(m, 100, 100);
	return 0;
}

