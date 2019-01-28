#include <algorithm>
#include <array>
#include <cstdio>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <vector>
#include <string>
#include <streambuf>
#include <unordered_map>

#include "common.h"

using namespace std;

const int LETTERS_COUNT = 256;

void Encode(string input_filename, string output_filename)
{
    try
    {
        cout << "read data and calc symbols statistics" << endl;
        ifstream in(input_filename, ios::binary);
        array<CharFreqency, LETTERS_COUNT> letters;
        for_each(istreambuf_iterator<char>(in),
                 istreambuf_iterator<char>(),
                 [&](unsigned char c) { letters[c].freq++; });
        in.close();

        cout << "build tree nodes based on statistics" << endl;
        list<shared_ptr<CharFreqency>> nodes;
        for (unsigned char i = 0; ; i++)
        {
            if (letters[i].freq > 0)
            {
                letters[i].letter = i;
                nodes.emplace_back(make_shared<CharFreqency>(letters[i]));
            }

            if (i == 255)
                break;
        }

        cout << "build huffman tree" << endl;
        list<shared_ptr<CharFreqency>> leafs;
        shared_ptr<CharFreqency> root = nullptr;
        BuildHaffmanTree(nodes, leafs, root);

        cout << "fill code table" << endl;
        unordered_map<unsigned char, CodeInfo> code_table;
        for (auto e : leafs)
        {
            unsigned char key = 0;
            char offset = 0;
            for (auto cur_node = e;
                 cur_node->up.lock() != nullptr;
                 cur_node = cur_node->up.lock(), offset++)
                if (cur_node->up.lock()->left == cur_node)
                    key |= (1 << offset);

            offset = (offset == 0) ? offset : offset - 1;
            CodeInfo code = { key, offset };
            code_table.emplace(e->letter, code);
        }

        cout << "coding..." << endl;
        ifstream input(input_filename, ios::binary);
        input.seekg(0);
        string tmp_filename = output_filename + ".tmp";
        ofstream tmp_output(tmp_filename, ios::binary);

        const unsigned long encode_buf_size = 5'210'000;
        vector<unsigned char> encode_buffer(encode_buf_size);
        unsigned long bit_counter = 0;
        unsigned long byte_offset = 0;
        char bit_offset = 7;

        bool is_encoded = false;

        istreambuf_iterator<char> bos(input);
        istreambuf_iterator<char> eos;
        for (; bos != eos; bos++)
        {
            unsigned char c = *bos;
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
                        is_encoded = true;
                        bit_offset = 7;
                        byte_offset++;
                        if (byte_offset == encode_buf_size)
                        {
                            tmp_output.write(reinterpret_cast<char*>(&encode_buffer[0]),
                                             encode_buf_size);
                            byte_offset = 0;
                            is_encoded = false;
                        }
                    }

                    ci.offset--;
                }
            }
            else
            {
                cerr << "error: char \'" << c << "\' (" << static_cast<int>(c) << ") not in map" << endl;
                throw new exception();
            }
        }

        if (is_encoded)
            tmp_output.write(reinterpret_cast<char*>(&encode_buffer[0]), byte_offset);

        tmp_output.close();
        input.close();

        cout << "write header and encoded data" << endl;
        ofstream output(output_filename, ios::binary);
        output.write(reinterpret_cast<char*>(&bit_counter), sizeof(bit_counter));

        auto leafs_size = leafs.size();
        output.write(reinterpret_cast<char*>(&leafs_size), sizeof(leafs_size));
        for (auto leaf : leafs)
        {
            output.write(reinterpret_cast<char*>(&(leaf->letter)), sizeof(leaf->letter));
            output.write(reinterpret_cast<char*>(&(leaf->freq)), sizeof(leaf->freq));
        }

        ifstream tmp_input(tmp_filename, ios::binary);
        copy(istreambuf_iterator<char>(tmp_input),
             istreambuf_iterator<char>(),
             ostreambuf_iterator<char>(output));
        tmp_input.close();
        output.close();

        remove(tmp_filename.c_str());
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


