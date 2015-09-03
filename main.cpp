#include <cstdlib>
#include "image.h"
#include <cmath>
#include <map>
#include <vector>
#include <algorithm>

// ===================================================================================================
// ===================================================================================================


//this funtion is used to sort the 'tran' vector, so that we can map from offset
//to hash by size
bool compare_size(std::pair<std::pair<int,int>, std::vector<std::pair<int,int> > >a,
	std::pair<std::pair<int,int>, std::vector<std::pair<int,int> > > b){
		return a.second.size() > b.second.size();
}

//call this funtion to increase the (dx, dy)
bool plus_plus(Offset& off, int s){
	if (s >= 16)
		s = 15;

	if (off.dx < s){
		off.dx++;
		return true;
	}
	if (off.dx == s && off.dy < s){
		off.dx = 0; off.dy++;
		return true;
	}
	if (off.dx == s && off.dy == s)
		return false;
}

//check if a pair (dx, dy) work for all the pixels in that unit of offset
bool IsValid(int s_hash, std::pair<std::pair<int,int>, std::vector<std::pair<int,int> > > a, Offset off, Image<Color> hash_data){
	
	std::vector<std::pair<int, int> > track;

	int dx = off.dx;
	int dy = off.dy;

	for (int i = 0; i < a.second.size(); i++){
		//check if one position has been used by some pixel in the some position of offset
		if (track.size() != 0){
			for (int j = 0; j < track.size(); j++){
				if ((a.second[i].first+dx)%s_hash == track[j].first && (a.second[i].second+dy)%s_hash == track[j].second)
					return false;
			}
		}
		//check if certain position in hash is white
		if (!hash_data.GetPixel((a.second[i].first+dx)%s_hash, (a.second[i].second+dy)%s_hash).isWhite())
			return false;
		track.push_back(std::make_pair((a.second[i].first+dx)%s_hash, (a.second[i].second+dy)%s_hash));
	}
	return true;
}

void Compress(const Image<Color> &input, 
              Image<bool> &occupancy, Image<Color> &hash_data, Image<Offset> &offset) {

  // you must implement this function
int width = input.Width();
int height = input.Height();
//initilization
occupancy.Allocate(width,height);
occupancy.SetAllPixels(true);

int count = 0;
//count the occupied pixels
for (int i = 0; i < width; i++){
	for (int j = 0; j< height; j++){
		if (input.GetPixel(i,j).isWhite() == true){
			count++;
			occupancy.SetPixel(i,j,false);
		}
	}	
}
//calculate the least offset and hash dimention
int p = width*height - count;
int s_hash = ceil(sqrt(1.01*p));
int s_offset = ceil(sqrt((double)(p/4)));
//if not find a solution dimention + 1 and go back to next, run again
next:

//std::cout<<p<<std::endl;
//std::cout<<s_hash<<std::endl;
//std::cout<<s_offset<<std::endl;

//some initializations
offset.Allocate(s_offset,s_offset);
hash_data.Allocate(s_hash,s_hash);

Offset off; off.dx = 0; off.dy = 0;
Color c;

offset.SetAllPixels(off);
hash_data.SetAllPixels(c);
std::map<std::pair<int,int>, std::vector<std::pair<int,int> > > tmp;

//grasp the ocupied pixels into a map structure
//pair.first is the offset coordinates
//pair.second is all the coordinates of related to that offset coordinates
for (int i = 0; i < width; i++){
	for (int j = 0; j< height; j++){
		if (input.GetPixel(i,j).isWhite() == false) 
		tmp[std::make_pair(i%s_offset,j%s_offset)].push_back(std::make_pair(i, j));
	}
}
//convert that map into a vector, so we can re-sort
std::vector<std::pair<std::pair<int,int>, std::vector<std::pair<int,int> > > > tran;

std::map<std::pair<int,int>, std::vector<std::pair<int,int> > >::iterator ite;
for (ite = tmp.begin(); ite != tmp.end(); ite++)
	tran.push_back(std::make_pair(ite->first, ite->second));
//resort by size of the offset
	std::sort(tran.begin(),tran.end(),compare_size);

for (int i = 0; i < tran.size(); i++){

	Offset off(offset.GetPixel(tran[i].first.first, tran[i].first.second).dx, offset.GetPixel(tran[i].first.first, tran[i].first.second).dy);
//if we find a (dx, dy) is not valid for all in that offset, call plus_plus	
	while (IsValid(s_hash, tran[i], off, hash_data) == false){
		bool change = plus_plus(off, s_hash);
//if reach the end of increasing...increase the dimension of offset and hash
		if (change == false){
			s_offset++;
			s_hash++;
			goto next;
		}
		}
//every thinf good, assign the value to offset
	offset.SetPixel(tran[i].first.first, tran[i].first.second, off);

	int dx = offset.GetPixel(tran[i].first.first, tran[i].first.second).dx;
	int dy = offset.GetPixel(tran[i].first.first, tran[i].first.second).dy;
//every thinggood, assign the values to hash
	for (int j = 0; j < tran[i].second.size(); j++){

		Color c;
		c.b = input.GetPixel(tran[i].second[j].first, tran[i].second[j].second).b;
		c.g = input.GetPixel(tran[i].second[j].first, tran[i].second[j].second).g;
		c.r = input.GetPixel(tran[i].second[j].first, tran[i].second[j].second).r;

		hash_data.SetPixel((tran[i].second[j].first+dx)%s_hash, (tran[i].second[j].second+dy)%s_hash, c);
	}
			
}
}






void UnCompress(const Image<bool> &occupancy, const Image<Color> &hash_data, const Image<Offset> &offset, 
                Image<Color> &output) {

  // you must implement this function
int width = occupancy.Width();
int height = occupancy.Height();

int s_offset = offset.Height();
int s_hash = hash_data.Height();

output.Allocate(width,height);

for (int i = 0; i < width; i++){
	for (int j = 0; j < height; j++){
		if (occupancy.GetPixel(i,j) == false){
			Color c;
			output.SetPixel(i,j,c);
		}
		else{
			Color c;
			unsigned char dx = offset.GetPixel(i%s_offset, j%s_offset).dx;
			unsigned char dy = offset.GetPixel(i%s_offset, j%s_offset).dy;
			c.b = hash_data.GetPixel((i+dx)%s_hash, (j+dy)%s_hash).b;
			c.g = hash_data.GetPixel((i+dx)%s_hash, (j+dy)%s_hash).g;
			c.r = hash_data.GetPixel((i+dx)%s_hash, (j+dy)%s_hash).r;

			output.SetPixel(i,j,c);
		}
	}
}









}


// ===================================================================================================
// ===================================================================================================

// Takes in two 24-bit color images as input and creates a b&w output
// image (black where images are the same, white where different)
void Compare(const Image<Color> &input1, const Image<Color> &input2, Image<bool> &output) {

  // confirm that the files are the same size
  if (input1.Width() != input2.Width() ||
      input1.Height() != input2.Height()) {
    std::cerr << "Error: can't compare images of different dimensions: " 
         << input1.Width() << "x" << input1.Height() << " vs " 
         << input2.Width() << "x" << input2.Height() << std::endl;
  } else {
    // make sure that the output images is the right size to store the
    // pixel by pixel differences
    output.Allocate(input1.Width(),input1.Height());
    int count = 0;
    for (int i = 0; i < input1.Width(); i++) {
      for (int j = 0; j < input1.Height(); j++) {
        Color c1 = input1.GetPixel(i,j);
        Color c2 = input2.GetPixel(i,j);
        if (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b)
          output.SetPixel(i,j,true);
        else {
          count++;
          output.SetPixel(i,j,false);
        }
      }      
    }     

    // confirm that the files are the same size
    if (count == 0) {
      std::cout << "The images are identical." << std::endl;
    } else {
      std::cout << "The images differ at " << count << " pixel(s)." << std::endl;
    }
  }
}

// ===================================================================================================
// ===================================================================================================

// to allow visualization of the custom offset image format
void ConvertOffsetToColor(const Image<Offset> &input, Image<Color> &output) {
  // prepare the output image to be the same size as the input image
  output.Allocate(input.Width(),input.Height());
  for (int i = 0; i < output.Width(); i++) {
    for (int j = 0; j < output.Height(); j++) {
      // grab the offset value for this pixel in the image
      Offset off = input.GetPixel(i,j);
      // set the pixel in the output image
      int dx = off.dx;
      int dy = off.dy;
      assert (dx >= 0 && dx <= 15);
      assert (dy >= 0 && dy <= 15);
      // to make a pretty image with purple, cyan, blue, & white pixels:
      int red = dx * 16;
      int green = dy *= 16;
      int blue = 255;
      assert (red >= 0 && red <= 255);
      assert (green >= 0 && green <= 255);
      output.SetPixel(i,j,Color(red,green,blue));
    }
  }
}

// ===================================================================================================
// ===================================================================================================

void usage(char* executable) {
  std::cerr << "Usage:  4 options" << std::endl;
  std::cerr << "  1)  " << executable << " compress input.ppm occupancy.pbm data.ppm offset.offset" << std::endl;
  std::cerr << "  2)  " << executable << " uncompress occupancy.pbm data.ppm offset.offset output.ppm" << std::endl;
  std::cerr << "  3)  " << executable << " compare input1.ppm input2.ppm output.pbm" << std::endl;
  std::cerr << "  4)  " << executable << " visualize_offset input.offset output.ppm" << std::endl;
}

// ===================================================================================================
// ===================================================================================================

int main(int argc, char* argv[]) {
  if (argc < 2) { usage(argv[0]); exit(1); }

  if (argv[1] == std::string("compress")) {
    if (argc != 6) { usage(argv[0]); exit(1); }
    // the original image:
    Image<Color> input;
    // 3 files form the compressed representation:
    Image<bool> occupancy;
    Image<Color> hash_data;
    Image<Offset> offset;
    input.Load(argv[2]);
    Compress(input,occupancy,hash_data,offset);
    // save the compressed representation
    occupancy.Save(argv[3]);
    hash_data.Save(argv[4]);
    offset.Save(argv[5]);

  } else if (argv[1] == std::string("uncompress")) {
    if (argc != 6) { usage(argv[0]); exit(1); }
    // the compressed representation:
    Image<bool> occupancy;
    Image<Color> hash_data;
    Image<Offset> offset;
    occupancy.Load(argv[2]);
    hash_data.Load(argv[3]);
    offset.Load(argv[4]);
    // the reconstructed image
    Image<Color> output;
    UnCompress(occupancy,hash_data,offset,output);
    // save the reconstruction
    output.Save(argv[5]);
  
  } else if (argv[1] == std::string("compare")) {
    if (argc != 5) { usage(argv[0]); exit(1); }
    // the original images
    Image<Color> input1;
    Image<Color> input2;    
    input1.Load(argv[2]);
    input2.Load(argv[3]);
    // the difference image
    Image<bool> output;
    Compare(input1,input2,output);
    // save the difference
    output.Save(argv[4]);

  } else if (argv[1] == std::string("visualize_offset")) {
    if (argc != 4) { usage(argv[0]); exit(1); }
    // the 8-bit offset image (custom format)
    Image<Offset> input;
    input.Load(argv[2]);
    // a 24-bit color version of the image
    Image<Color> output;
    ConvertOffsetToColor(input,output);
    output.Save(argv[3]);

  } else {
    usage(argv[0]);
    exit(1);
  }
}

// ===================================================================================================
// ===================================================================================================
