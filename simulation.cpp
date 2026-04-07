// Dynamic Memory Allocation Simulator -- First-Fit, Next-Fit, Best-Fit, Worst-Fit

#include <iostream>
#include <vector>
#include <iomanip>
using namespace std;

const string RED = "\033[31m", GREEN = "\033[32m", CYAN = "\033[36m",
             YELLOW = "\033[33m", BOLD = "\033[1m", RESET = "\033[0m";

struct MemoryBlock {
    int id;
    int startAddress;
    int size;
    bool isFree;
    int processID;
};

int nextFitIndex = 0;

void renumberBlocks(vector<MemoryBlock>& pool) {
    for (int i = 0; i < (int)pool.size(); i++)
        pool[i].id = i;
}

vector<MemoryBlock> initPool() {
    return {
        {0,0,100,true,-1},  {1,100,60,true,-1},  {2,160,200,true,-1},
        {3,360,45,true,-1}, {4,405,80,true,-1},  {5,485,150,true,-1},
        {6,635,30,true,-1}, {7,665,120,true,-1}, {8,785,75,true,-1},
        {9,860,90,true,-1}, {10,950,184,true,-1}
    };
}

void displayMemory(const vector<MemoryBlock>& pool, const string& label) {
    int totalFree = 0, totalUsed = 0;
    cout << "\n  [" << label << "]\n";
    cout << "  " << string(56, '-') << "\n";
    cout << "  " << setw(5) << "BLK" << setw(9) << "Start"
         << setw(9) << "Size" << setw(10) << "Status" << setw(10) << "PID" << "\n";
    cout << "  " << string(56, '-') << "\n";
    for (auto& b : pool) {
        string col = b.isFree ? GREEN : RED;
        cout << "  " << col
             << setw(5) << b.id
             << setw(9) << b.startAddress
             << setw(7) << b.size << "KB"
             << setw(10) << (b.isFree ? "FREE" : "ALLOC")
             << setw(10) << (b.isFree ? "---" : "P" + to_string(b.processID))
             << RESET << "\n";
        b.isFree ? totalFree += b.size : totalUsed += b.size;
    }
    cout << "  " << string(56, '-') << "\n";
    cout << "  Used:" << RED << totalUsed << "KB" << RESET
         << "  Free:" << GREEN << totalFree << "KB" << RESET << "\n";
}

void splitBlock(vector<MemoryBlock>& pool, int idx, int reqSize) {
    int leftover = pool[idx].size - reqSize;
    if (leftover > 20) {
        MemoryBlock nb = {0, pool[idx].startAddress + reqSize, leftover, true, -1};
        pool[idx].size = reqSize;
        pool.insert(pool.begin() + idx + 1, nb);
        if (idx < nextFitIndex) nextFitIndex++;
        renumberBlocks(pool);
        cout << YELLOW << "    [SPLIT] BLK" << idx << ": " << reqSize
             << "KB alloc, " << leftover << "KB remainder -> BLK" << idx+1 << RESET << "\n";
    }
}

void deallocate(vector<MemoryBlock>& pool, int pid) {
    int fi = -1;
    for (int i = 0; i < (int)pool.size(); i++)
        if (!pool[i].isFree && pool[i].processID == pid) { fi = i; break; }
    if (fi == -1) { cout << RED << "  P" << pid << " not found!\n" << RESET; return; }

    cout << GREEN << "  [DEALLOC] P" << pid << " <- BLK" << fi
         << " (" << pool[fi].size << "KB)" << RESET << "\n";
    pool[fi].isFree = true;
    pool[fi].processID = -1;

    if (fi+1 < (int)pool.size() && pool[fi+1].isFree) {
        cout << YELLOW << "    [MERGE] BLK" << fi << " + BLK" << fi+1 << RESET << "\n";
        pool[fi].size += pool[fi+1].size;
        pool.erase(pool.begin() + fi + 1);
        if (nextFitIndex > fi+1) nextFitIndex--;
        if (nextFitIndex >= (int)pool.size()) nextFitIndex = 0;
        renumberBlocks(pool);
    }
    if (fi-1 >= 0 && pool[fi-1].isFree) {
        cout << YELLOW << "    [MERGE] BLK" << fi-1 << " + BLK" << fi << RESET << "\n";
        pool[fi-1].size += pool[fi].size;
        pool.erase(pool.begin() + fi);
        if (nextFitIndex >= fi) nextFitIndex--;
        if (nextFitIndex < 0) nextFitIndex = 0;
        renumberBlocks(pool);
    }
}

bool firstFit(vector<MemoryBlock>& pool, int size, int pid) {
    for (int i = 0; i < (int)pool.size(); i++) {
        if (pool[i].isFree && pool[i].size >= size) {
            cout << GREEN << "  [ALLOC] First-Fit: P" << pid << " (" << size
                 << "KB) -> BLK" << i << " (" << pool[i].size << "KB)" << RESET << "\n";
            pool[i].isFree = false; pool[i].processID = pid;
            splitBlock(pool, i, size);
            return true;
        }
    }
    cout << RED << "  [FAIL] P" << pid << " (" << size << "KB) -- no fit!\n" << RESET;
    return false;
}

bool nextFit(vector<MemoryBlock>& pool, int size, int pid) {
    int n = pool.size();
    if (nextFitIndex >= n) nextFitIndex = 0;
    for (int c = 0; c < n; c++) {
        int i = (nextFitIndex + c) % n;
        if (pool[i].isFree && pool[i].size >= size) {
            cout << GREEN << "  [ALLOC] Next-Fit: P" << pid << " (" << size
                 << "KB) -> BLK" << i << " (" << pool[i].size << "KB)" << RESET << "\n";
            pool[i].isFree = false; pool[i].processID = pid;
            splitBlock(pool, i, size);
            nextFitIndex = (i + 1) % (int)pool.size();
            return true;
        }
    }
    cout << RED << "  [FAIL] P" << pid << " (" << size << "KB) -- no fit!\n" << RESET;
    return false;
}

bool bestFit(vector<MemoryBlock>& pool, int size, int pid) {
    int bi = -1, bs = 999999;
    for (int i = 0; i < (int)pool.size(); i++)
        if (pool[i].isFree && pool[i].size >= size && pool[i].size < bs)
            { bs = pool[i].size; bi = i; }
    if (bi != -1) {
        cout << GREEN << "  [ALLOC] Best-Fit:  P" << pid << " (" << size
             << "KB) -> BLK" << bi << " (" << pool[bi].size << "KB)" << RESET << "\n";
        pool[bi].isFree = false; pool[bi].processID = pid;
        splitBlock(pool, bi, size);
        return true;
    }
    cout << RED << "  [FAIL] P" << pid << " (" << size << "KB) -- no fit!\n" << RESET;
    return false;
}

bool worstFit(vector<MemoryBlock>& pool, int size, int pid) {
    int wi = -1, ws = -1;
    for (int i = 0; i < (int)pool.size(); i++)
        if (pool[i].isFree && pool[i].size >= size && pool[i].size > ws)
            { ws = pool[i].size; wi = i; }
    if (wi != -1) {
        cout << GREEN << "  [ALLOC] Worst-Fit: P" << pid << " (" << size
             << "KB) -> BLK" << wi << " (" << pool[wi].size << "KB)" << RESET << "\n";
        pool[wi].isFree = false; pool[wi].processID = pid;
        splitBlock(pool, wi, size);
        return true;
    }
    cout << RED << "  [FAIL] P" << pid << " (" << size << "KB) -- no fit!\n" << RESET;
    return false;
}

void fragReport(const vector<MemoryBlock>& pool, const string& name) {
    int totalFree = 0, count = 0, largest = 0;
    for (auto& b : pool)
        if (b.isFree) { totalFree += b.size; count++; if (b.size > largest) largest = b.size; }
    bool extFrag = (count > 1) && (largest < totalFree);
    cout << CYAN << "  [FRAG] " << name << ": Free=" << totalFree
         << "KB | Blocks=" << count << " | Largest=" << largest
         << "KB | ExtFrag=" << (extFrag ? "YES" : "NO") << RESET << "\n";
}

typedef bool (*AllocFunc)(vector<MemoryBlock>&, int, int);

void runSimulation(vector<MemoryBlock>& pool, AllocFunc alloc, const string& name) {
    cout << "\n" << CYAN << BOLD << "=== " << name << " ===" << RESET << "\n";
    displayMemory(pool, "Initial State");

    cout << "\n  -- Operations --\n";
    alloc(pool, 95, 1);
    alloc(pool, 170, 2);
    alloc(pool, 40, 3);
    alloc(pool, 120, 4);
    alloc(pool, 500, 5);   // intentional fail
    deallocate(pool, 2);
    deallocate(pool, 4);
    alloc(pool, 80, 6);

    displayMemory(pool, "Final State");
    fragReport(pool, name);
}

void stressTest(AllocFunc alloc, const string& name, const vector<MemoryBlock>& base) {
    cout << "\n" << CYAN << BOLD << "  [STRESS] " << name << RESET << "\n";
    vector<MemoryBlock> pool = base;
    srand(42);
    int ok = 0, fail = 0;
    for (int i = 0; i < 10; i++) {
        int sz = (rand() % 141) + 10;
        int pid = 10 + i;
        bool res = alloc(pool, sz, pid);
        res ? ok++ : fail++;
        cout << (res ? GREEN : RED) << "    #" << i+1 << ": P" << pid
             << " (" << sz << "KB) -- " << (res ? "OK" : "FAIL") << RESET << "\n";
        if (i == 3) deallocate(pool, 10);
        if (i == 6) deallocate(pool, 13);
    }
    cout << "  Results: " << GREEN << ok << " OK" << RESET
         << " | " << RED << fail << " FAIL" << RESET << "\n";
    fragReport(pool, name + " Stress");
}

int main() {
    cout << CYAN << BOLD << "\nMemory Allocation Simulator\n"
         << "First-Fit | Next-Fit | Best-Fit | Worst-Fit\n" << RESET << "\n";

    vector<MemoryBlock> base = initPool();
    vector<MemoryBlock> ff = base, nf = base, bf = base, wf = base;

    runSimulation(ff, firstFit,  "First-Fit");
    nextFitIndex = 0;
    runSimulation(nf, nextFit,   "Next-Fit");
    runSimulation(bf, bestFit,   "Best-Fit");
    runSimulation(wf, worstFit,  "Worst-Fit");

    cout << "\n" << CYAN << BOLD << "=== STRESS TEST (seed=42) ===" << RESET << "\n";
    stressTest(firstFit, "First-Fit", base);
    nextFitIndex = 0;
    stressTest(nextFit,  "Next-Fit",  base);
    stressTest(bestFit,  "Best-Fit",  base);
    stressTest(worstFit, "Worst-Fit", base);

    cout << CYAN << BOLD << "\nSimulation complete.\n" << RESET;
    return 0;
}