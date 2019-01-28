#include <array>
#include <algorithm>
#include <iostream>
#include <list>
#include <memory>

#include "common.h"

using namespace std;

void help(char* app)
{
    cerr << "using:" << endl << endl;
    cerr << "   " << app << " input_filename output_filename" << endl;
}

void BuildHaffmanTree(list<shared_ptr<CharFreqency>>& nodes,
                      list<shared_ptr<CharFreqency>>& leafs,
                      shared_ptr<CharFreqency>& root)
{
    // sort by frequency
    nodes.sort([](auto a, auto b) { return a->freq < b->freq; });

    // build binary code tree
    while (nodes.size() > 1)
    {
        // search min pair
        auto min_a = nodes.begin();
        auto min_b = ++(nodes.begin());
        auto min_sum = (*min_a)->freq + (*min_b)->freq;
        for (auto x = min_a, y = min_b; y != nodes.end(); x++, y++)
        {
            auto cur_sum = (*x)->freq + (*y)->freq;
            if (cur_sum < min_sum)
            {
                min_sum = cur_sum;
                min_a = x;
                min_b = y;
            }
        }

        auto a = *min_a;
        auto b = *min_b;

        if (a->left == nullptr && a->right == nullptr)
            leafs.push_back(a);

        if (b->left == nullptr && b->right == nullptr)
            leafs.push_back(b);

        auto new_node = make_shared<CharFreqency>();
        new_node->freq = a->freq + b->freq;
        new_node->left = a;
        new_node->right = b;
        a->up = new_node;
        b->up = new_node;

        nodes.insert(min_b, new_node);
        nodes.erase(min_a);
        nodes.erase(min_b);
    }

    if (nodes.size() == 1)
    {
        root = nodes.front();

        if (leafs.empty())
            leafs.emplace_back(nodes.front());

        nodes.pop_front();
    }
    else
        root = nullptr;
}
