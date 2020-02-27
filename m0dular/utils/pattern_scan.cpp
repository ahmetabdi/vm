#include "pattern_scan.h"
#include "memutils.h"
#include "string.h"
#include <vector>
#include "assert.h"
#include "stdlib.h"

struct pOperation
{
	short op;
	intptr_t offset, v1;

	pOperation(short o = 0, intptr_t off = 0, intptr_t v = 0)
	{
		op = o;
		offset = off;
		v1 = v;
	}

	uintptr_t RunOp(uintptr_t addr)
	{
		switch(op) {
		  case 0:
			  return Read<uintptr_t>(addr + offset);
		  case 1:
			  return addr + offset;
		  case 2:
			  return GetAbsoluteAddress(addr, offset, v1);
		  case 3:
			  return Read<unsigned int>(addr + offset);
		}
		return addr;
	}

};

uintptr_t ScanPattern(uintptr_t start, uintptr_t end, uintptr_t length, uintptr_t* data, uintptr_t* mask);

static void ParsePattern(const char* pattern, short*& patternBytes, size_t& length, std::vector<pOperation>& operations)
{
	char* p = (char*)(uintptr_t)pattern-1;
	bool inRelDeref = false;
	[[maybe_unused]] bool derefDone = false;
	int relIdx = 0;
	int relStartIdx = 0;
	int idx = 0;
	int initDerefIdx = 0;

	length = strlen(pattern);
	patternBytes = new short[length];

	while((++p) - pattern <= (long)length && *p) {

		while (*p == ' ') p++;

		if (*p == '?') {
			if (*(p+1) == '?')
				p++;
			patternBytes[idx++] = -1;
		} else if (*p == '@') {
			assert(!inRelDeref && !derefDone && operations.size() == 0);
			if (idx)
				operations.emplace_back(pOperation(1, idx));
			derefDone = true;
		} else if (*p == '[') {
			assert(!inRelDeref && !derefDone);
			inRelDeref = true;
			relStartIdx = idx;
			if (idx) {
				relIdx++;
				operations.emplace_back(pOperation(1, idx));
			}
			operations.emplace_back(pOperation());
		} else if (*p == ']') {
			assert(inRelDeref);
			inRelDeref = false;
			derefDone = true;

			pOperation& op = operations.at(relIdx);

			op.op = 2;
			op.offset = initDerefIdx - relStartIdx;
			op.v1 = idx - relStartIdx;
		} else if (*p == '*' || *p == '^') {
			assert(!derefDone);
			derefDone = true;

			initDerefIdx = idx;

			if (!inRelDeref)
				operations.emplace_back(pOperation((*p == '*') ? 0 : 3, idx));
			p++;

			while (*p == '+' || *p == '-' || *p == '*' || *p == '^' || *p == ':') {
				if (*p == '*')
					operations.emplace_back(pOperation(*p++ == '*' ? 0 : 1));
				else if (*p == '^')
					operations.emplace_back(pOperation(*p++ == '^' ? 3 : 1));
				else if (*p == ':') {
					pOperation op = pOperation();
					p++;
					op.offset = strtol(p, &p, 10);
					p++;
					op.v1 = strtol(p, &p, 10);
					op.op = 2;
					p--;
					operations.emplace_back(op);
				} else {
					pOperation op = pOperation();
					if (*p == '+' || *p == '-')
						op.offset = strtol(p, &p, 10);
					//Compress the offset operation into a dereference
					op.op = (*p == '*') ? 0 : ((*p == '^') ? 3 : 1);
					if (*p == '*' || *p == '^')
						p++;
					operations.emplace_back(op);
				}
			}

			if (*p == '?')
				p--;

		} else {
			patternBytes[idx++] = (uint8_t)strtoul(p, &p, 16);
			if (*p != ' ')
				p--;
		}
	}

	length = idx;
}

//Optimize the parsed pattern into larger long sized values to be compared. This way we will utilize the full potential of the CPUs native register size when reading the memory. Going wider (into SIMD) is not worth it, because if the pattern does not match, it will usually be within the first 4-8 instructions.
static void ProduceScanData(short* parsedData, uintptr_t*& data, uintptr_t*& mask, size_t& size)
{
	constexpr size_t iSize = sizeof(long);
	size_t size2 = (size - 1) / iSize + 1;

	data = new uintptr_t[size2];
	mask = new uintptr_t[size2];

	for (size_t i = 0; i < size2; i++) {
		data[i] = 0;
		mask[i] = 0;

		for (size_t o = 0; o < iSize; o++) {
			if (i * iSize + o >= size || parsedData[i * iSize + o] < 0)
				mask[i] |= (0xffll << (8ll * o));
			if (i * iSize + o < size)
				data[i] |= (((uintptr_t)((parsedData[i * iSize + o]) & 0xffll)) << (8ll * o));

		}
		data[i] |= mask[i];
	}

	size = size2;
}

uintptr_t PatternScan::FindPattern(const char* pattern, uintptr_t start, uintptr_t end)
{
	short* patternBytes = nullptr;
	size_t length = 0;
	std::vector<pOperation> operations;

	uintptr_t addr = 0;

	ParsePattern(pattern, patternBytes, length, operations);

	uintptr_t* data;
	uintptr_t* mask;
	ProduceScanData(patternBytes, data, mask, length);
	delete[] patternBytes;

	addr = ScanPattern(start, end, length, data, mask);

	if (addr)
		for (auto& i : operations)
			addr = i.RunOp(addr);

	delete[] data;
	delete[] mask;
	return addr;
}

uintptr_t PatternScan::FindPattern(const char* __restrict pattern, const char* __restrict module)
{
	ModuleInfo info = Handles::GetModuleInfo(module);
	return FindPattern(pattern, info.address, info.address + info.size);
}

#ifndef PATTERN_SCAN_CUSTOM_SCAN
#ifdef PATTERN_SCAN_PAGE_SCAN
//Page scanning is very useful in instances where memory reads have high latency. It can be hundreds of times faster than reading long-by-long
uintptr_t ScanPattern(uintptr_t start, uintptr_t end, uintptr_t length, uintptr_t* data, uintptr_t* mask)
{
	uintptr_t llength = sizeof(long) * length;
	char* buf = (char*)alloca(0x1000 + llength);
	char* page = buf + llength;
	//On the first round, we do not want to scan the part where a part of the previous page buffer would be copied on - since we have no "previous page"
	uintptr_t sOffset = llength;
	for (uintptr_t i = start; i < end - llength; i += 0x1000) {
		memcpy(buf, buf + 0x1000, llength);
		ReadArr(i & ~0xfff, page, 0x1000);
		for (uintptr_t u = (start & 0xfff) + sOffset; u < 0x1000; u++) {
			bool miss = false;
			for (uintptr_t o = 0; o < length && !miss; o++)
				miss = data[o] ^ (*(uintptr_t*)(buf + u + o * sizeof(uintptr_t)) | mask[o]);

			if (!miss)
				return u + (i & ~0xfff) - llength;
		}
		sOffset = 0;
	}

	return 0;
}
#else
uintptr_t ScanPattern(uintptr_t start, uintptr_t end, uintptr_t length, uintptr_t* data, uintptr_t* mask)
{
	uintptr_t llength = sizeof(long) * length;
	for (uintptr_t i = start; i < end - llength; i++) {
		bool miss = false;
		for (uintptr_t o = 0; o < length && !miss; o++)
			miss = data[o] ^ (Read<uintptr_t>(i + o * sizeof(uintptr_t)) | mask[o]);

		if (!miss)
			return i;
	}

	return 0;
}
#endif
#endif
