#pragma once

namespace GameCore::Optimizer {

struct MemoryResult {
    double freedMb;
    double beforeMb;
    double afterMb;
};

class MemoryOptimizer {
public:
    MemoryResult FreeMemory();

private:
    static void EmptyWorkingSets();
    static void FlushFileSystemCache();
    static void CompactHeap();
};

} // namespace GameCore::Optimizer