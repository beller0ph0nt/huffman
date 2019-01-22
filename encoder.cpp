#include <iostream>
#include <fstream>
#include <iterator>
#include <array>
#include <vector>
#include <algorithm>
#include <list>
#include <unordered_map>
#include <string>
#include <exception>

#include "common.h"

using namespace std;

const int LETTERS_COUNT = 255;

void Encode(string input_filename, string output_filename)
{
    try
    {
        cout << "encoding..." << endl;
        // reading data
        ifstream input(input_filename, ios::binary);
        vector<unsigned char> buffer(istreambuf_iterator<char>(input), {});
        input.close();

        if (buffer.size() == 0)
        {
            cerr << "error: input file is empty" << endl;
            throw new exception();
        }

        // calc symbols statistics
        array<CharFreqency, LETTERS_COUNT> letters;
        for_each(begin(buffer), end(buffer), [&](unsigned char c) { letters[c].freq++; });

        // build tree nodes based on statistics
        list<shared_ptr<CharFreqency>> nodes;
        for (unsigned char i = 0; i < letters.size(); i++)
        {
            if (letters[i].freq > 0)
            {
                letters[i].letter = i;
                nodes.emplace_back(make_shared<CharFreqency>(letters[i]));
            }
        }

        list<shared_ptr<CharFreqency>> leafs;
        shared_ptr<CharFreqency> root = nullptr;
        BuildHaffmanTree(nodes, leafs, root);

        // fill code table
        unordered_map<char, CodeInfo> code_table;
        for (auto e : leafs)
        {
            unsigned char key = 0;
            char offset = 0;
            for (auto cur_node = e; cur_node->up.lock() != nullptr; cur_node = cur_node->up.lock(), offset++)
                if (cur_node->up.lock()->left == cur_node)
                    key |= (1 << offset);

            offset = (offset == 0) ? offset : offset - 1;
            CodeInfo code = { key, offset };
            code_table.emplace(e->letter, code);
        }

        // coding
        vector<unsigned char> encode_buffer(buffer.size(), 0);
        unsigned long bit_counter = 0;
        unsigned long byte_offset = 0;
        char bit_offset = 7;
        for (auto c : buffer)
        {
            auto it = code_table.find(c);
            if (it != code_table.end())
            {
                CodeInfo ci = it->second;
                while (ci.offset >= 0)
                {
                    if ((ci.code & (1 << ci.offset)) != 0)
                        encode_buffer[byte_offset] |= (1 << bit_offset);

                    bit_counter++;

                    bit_offset--;
                    if (bit_offset < 0)
                    {
                        bit_offset = 7;
                        byte_offset++;
                    }

                    ci.offset--;
                }
            }
            else
            {
                cerr << "error: char not in map" << endl;
                throw new exception();
            }
        }

        // write header and encoded data
        ofstream output(output_filename, ios::binary);
        output.write(reinterpret_cast<char*>(&bit_counter), sizeof(bit_counter));

        auto leafs_size = leafs.size();
        output.write(reinterpret_cast<char*>(&leafs_size), sizeof(leafs_size));
        for (auto leaf : leafs)
        {
            output.write(reinterpret_cast<char*>(&(leaf->letter)), sizeof(leaf->letter));
            output.write(reinterpret_cast<char*>(&(leaf->freq)), sizeof(leaf->freq));
        }

        for (unsigned long i = 0; i <= byte_offset; i++)
            output.write(reinterpret_cast<char*>(&encode_buffer[i]), sizeof(encode_buffer[i]));
        output.close();
    }
    catch (exception& ex)
    {
        cerr << ex.what();
    }
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        help(argv[0]);
        return -1;
    }

    string input_filename = argv[1];
    string output_filename = argv[2];

    Encode(input_filename, output_filename);

    return 0;
}


