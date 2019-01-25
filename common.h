#pragma once

#include <memory>

using namespace std;

struct CharFreqency
{
    unsigned char letter = '\0';
    unsigned long freq = 0;
    weak_ptr<CharFreqency> up;
    shared_ptr<CharFreqency> left = nullptr;
    shared_ptr<CharFreqency> right = nullptr;
};

struct CodeInfo
{
    unsigned char code = 0;
    char offset = 0;
};

void help(char* app);
void BuildHaffmanTree(list<shared_ptr<CharFreqency>>& nodes,
    list<shared_ptr<CharFreqency>>& leafs,
    shared_ptr<CharFreqency>& root);
