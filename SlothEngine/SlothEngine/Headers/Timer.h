//Edited 2015-01-20  23:00

#pragma once
#ifndef TIMER_H
#define TIMER_H
#endif


#include <windows.h>
#include <string.h>
#include <d3d11.h>
#include <d3dcompiler.h>

class Timer
{
public:
	Timer();

	float GameTime()const; // in seconds
	float TotalTime()const;
	void Reset(); // Resets GameTimer
	void Tick(); // Counts time
private:
	double mSecondsPerCount;
	//Time Variables must be int64 because its the format they are aquiered from
	__int64 mBaseTime;
	__int64 mPausedTime;
	__int64 mStopTime;
	__int64 mPrevTime;
	__int64 mCurrTime;

};
