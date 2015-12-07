#include "BVH.h"
using std::vector;
#pragma once

class BVHList
{
public:

	BVHList();
	~BVHList();

	void AddBVH(BVH* bvh);  //add both a bvh and its corresponding frame and animation time
	int GetSize();
	BVH* GetBVHIndex(int i);
	int GetFrameNo(int i);
	void SetFrameNo(int i, int frameNumber);
	void IncreFrameNo(int i);
	void DecreFrameNo(int i);
	float GetAnimationTime(int i);
	void SetAnimationTime(int i, float time);
	void IncreAnimationTime(int i);
	void DecreAnimationTime(int i);
	void RemoveBVHIndex(int i);

private:

	vector<BVH*> bvhList;
	vector<int> frame_no;
	vector<float> animation_time;

};