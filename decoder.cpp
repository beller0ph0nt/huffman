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

void Decode(string input_filename, string output_filename)
{
    try
    {
        cout << "decoding..." << endl;
        ifstream file(input_filename, ios::ate | ios::binary);
        streampos fsize = file.tellg();
        file.close();

        if (fsize == 0)
        {
            cerr << "error: input file is empty" << endl;
            throw new exception();
        }

        ifstream in(input_filename, ios::binary);
        unsigned long bit_counter = 0;
        in.read(reinterpret_cast<char*>(&bit_counter), sizeof(bit_counter));

        size_t nodes_size = 0;
        in.read(reinterpret_cast<char*>(&nodes_size), sizeof(nodes_size));

        list<shared_ptr<CharFreqency>> nodes;
        for (size_t i = 0; i < nodes_size; i++)
        {
            auto cf = make_shared<CharFreqency>();
            in.read(reinterpret_cast<char*>(&(cf->letter)), sizeof(cf->letter));
            in.read(reinterpret_cast<char*>(&(cf->freq)), sizeof(cf->freq));
            nodes.emplace_back(cf);
        }

        vector<unsigned char> ecode_buffer(istreambuf_iterator<char>(in), {});
        in.close();

        if (ecode_buffer.size() * 8 < bit_counter)
        {
            cerr << "error: incorrect input file header structure" << endl;
            throw new exception();
        }

        list<shared_ptr<CharFreqency>> leafs;
        shared_ptr<CharFreqency> root = nullptr;
        BuildHaffmanTree(nodes, leafs, root);

        shared_ptr<CharFreqency> cur_node = root;
        unsigned long byte_offset = 0;
        char bit_offset = 7;
        list<char> decode_buffer;
        while (bit_counter > 0)
        {
            if ((ecode_buffer[byte_offset] & (1 << bit_offset)) != 0)
            {
                if (cur_node->left != nullptr)
                    cur_node = cur_node->left;
            }
            else
            {
                if (cur_node->right != nullptr)
                    cur_node = cur_node->right;
            }

            if (cur_node->left == nullptr && cur_node->right == nullptr)
            {
                decode_buffer.emplace_back(cur_node->letter);
                cur_node = root;
            }

            bit_offset--;
            if (bit_offset < 0)
            {
                bit_offset = 7;
                byte_offset++;
            }

            bit_counter--;
        }

        ofstream output(output_filename, ios::binary);
        for (auto c : decode_buffer)
            output << c;
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

    Decode(input_filename, output_filename);

    return 0;
}


