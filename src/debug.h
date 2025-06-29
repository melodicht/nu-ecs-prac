#pragma once

#if SKL_INTERNAL

#define NAMED_TIMED_BLOCK_(name, number, ...) TimedBlock timedBlock_##number = TimedBlock(__COUNTER__, __FILE__, __LINE__, #name, ## __VA_ARGS__)
#define NAMED_TIMED_BLOCK(name, ...) NAMED_TIMED_BLOCK_(name, __LINE__, ## __VA_ARGS__)

#define TIMED_BLOCK_(number, ...) NAMED_TIMED_BLOCK_(__FUNCTION__, number, ## __VA_ARGS__)
#define TIMED_BLOCK(...) TIMED_BLOCK_(__LINE__, ## __VA_ARGS__)

#else

#define NAMED_TIMED_BLOCK_(name, number, ...) 
#define NAMED_TIMED_BLOCK(name, ...) 

#define TIMED_BLOCK_(number, ...) 
#define TIMED_BLOCK(...) 

#endif

struct DebugRecord
{
    const char *fileName;
    u32 lineNumber;
    const char *blockName;
    u64 cycleCount;
    u32 hitCount;
};

extern DebugRecord debugRecordArray[];

struct TimedBlock
{
    DebugRecord *debugRecord;

    #if SKL_INTERNAL
    TimedBlock(u32 index, const char *fileName, u32 lineNumber,
               const char *blockName, u32 hitCount = 1)
    {
        debugRecord = debugRecordArray + index;
        debugRecord->fileName = fileName;
        debugRecord->lineNumber = lineNumber;
        debugRecord->blockName = blockName;
        debugRecord->cycleCount -= ReadCPUTimer();
        debugRecord->hitCount += hitCount;
    }

    ~TimedBlock()
    {
        debugRecord->cycleCount += ReadCPUTimer();
    }
    #endif
};
