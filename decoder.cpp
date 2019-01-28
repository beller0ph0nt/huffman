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
        cout << "read data" << endl;
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

        cout << "build huffman tree" << endl;
        list<shared_ptr<CharFreqency>> leafs;
        shared_ptr<CharFreqency> root = nullptr;
        BuildHaffmanTree(nodes, leafs, root);

        cout << "decoding..." << endl;
        shared_ptr<CharFreqency> cur_node = root;

        const size_t encode_buf_size = 5'120'000;
        vector<char> encode_buffer(encode_buf_size);
        in.read(&encode_buffer[0], encode_buf_size);
        char bit_offset = 7;
        unsigned long byte_offset = 0;

        const size_t decode_buf_size = 5'120'000;
        vector<char> decode_buffer(decode_buf_size);
        unsigned long i = 0;

        ofstream output(output_filename, ios::binary);

        bool is_decoded = false;

        while (bit_counter > 0)
        {
            if ((encode_buffer[byte_offset] & (1 << bit_offset)) != 0)
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
                decode_buffer[i] = cur_node->letter;
                i++;
                is_decoded = true;
                if (i == decode_buf_size)
                {
                    output.write(&decode_buffer[0], decode_buf_size);
                    i = 0;
                    is_decoded = false;
                }
                cur_node = root;
            }

            bit_offset--;
            if (bit_offset < 0)
            {
                bit_offset = 7;
                byte_offset++;
                if (byte_offset == encode_buf_size)
                {
                    in.read(&encode_buffer[0], encode_buf_size);
                    byte_offset = 0;
                }
            }

            bit_counter--;
        }

        if (is_decoded)
            output.write(&decode_buffer[0], i);

        output.close();
        in.close();
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


