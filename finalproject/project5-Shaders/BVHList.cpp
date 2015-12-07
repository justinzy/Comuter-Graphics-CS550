#include "BVHList.h"


BVHList::BVHList()
{
}


BVHList::~BVHList()
{
	size_t sz = bvhList.size();
	for (size_t i = 0; i < sz; ++i)
		delete bvhList[i];
}

void BVHList::AddBVH(BVH* bvh)
{
	bvhList.push_back(bvh);
	frame_no.push_back(0);
	animation_time.push_back(0);
}

int BVHList::GetSize()
{
	return bvhList.size();
}

BVH* BVHList::GetBVHIndex(int i)
{
	return bvhList[i];
}

int BVHList::GetFrameNo(int i)
{
	return frame_no[i];
}

void BVHList::SetFrameNo(int i, int frameNumber)
{
	frame_no[i] = frameNumber;
}

void BVHList::IncreFrameNo(int i)
{
	frame_no[i]++;
}


void BVHList::DecreFrameNo(int i)
{
	frame_no[i]--;
}


float BVHList::GetAnimationTime(int i)
{
	return animation_time[i];
}

void BVHList::SetAnimationTime(int i, float time)
{
	animation_time[i] = time;
}


void BVHList::IncreAnimationTime(int i)
{
	animation_time[i] += bvhList[i]->GetInterval();
}

void BVHList::DecreAnimationTime(int i)
{
	animation_time[i] -= bvhList[i]->GetInterval();
}

void BVHList::RemoveBVHIndex(int i)
{
	delete bvhList[i];
	//bvhList.pop_back();
	bvhList.erase(bvhList.begin() + i);
	animation_time.erase(animation_time.begin() + i);
	frame_no.erase(frame_no.begin() + i);

	//animation_time.pop_back();
	//frame_no.pop_back();
}