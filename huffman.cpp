#include "huffman.hpp"

void huffman ::createVec()
{
    for (int i = 0; i < 128; i++)
    {
        vec.push_back(new Node());
        vec[i]->data = i;
        vec[i]->freq = 0;
    }
}

void huffman::traverse(Node *r, string s)
{
    if (r->left == NULL && r->right == NULL)
    {
        r->code = s;
        return;
    }

    traverse(r->left, s + '0');
    traverse(r->right, s + '1');
}

int huffman::binToDec(string s)
{
    int ans = 0;
    int n = s.size();

    for (int i = 0; i < n; i++)
    {
        ans = ans * 2 + s[i] - '0';
    }

    return ans;
}

string huffman::decToBin(int n)
{
    string temp = "";
    string ans = "";

    while (n > 0)
    {
        temp += (n % 2 + '0');
        n = n / 2;
    }

    ans.append(8 - temp.length(), '0');

    for (int i = temp.length() - 1; i >= 0; i--)
    {
        ans += temp[i];
    }
    return ans;
}

void huffman::buildTree(char char_code, string &path)
{
    Node *curr = root;
    for (int i = 0; i < path.length(); i++)
    {
        if (path[i] == '0')
        {
            if (curr->left == NULL)
            {
                curr->left = new Node();
            }
            curr = curr->left;
        }
        else if (path[i] == '1')
        {
            if (curr->right == NULL)
            {
                curr->right = new Node();
            }
            curr = curr->right;
        }
    }
    curr->data = char_code;
}

void huffman::createMinHeap()
{
    char id;
    inFile.open(inFileName, ios::in);
    inFile.get(id);

    /*  Incrementing frequency of characters that appear in the input file  */
    while (!inFile.eof())
    {
        vec[id]->freq++;
        inFile.get(id);
    }
    inFile.close();

    /*  Pushing the Nodes which appear in the file into the priority queue (Min Heap)  */
    for (int i = 0; i < 128; i++)
    {
        if (vec[i]->freq > 0)
        {
            minHeap.push(vec[i]);
        }
    }
}

void huffman::createTree()
{
    /*  Creating Huffman Tree with the Min Heap created earlier  */

    Node *left, *right;
    priority_queue<Node *, vector<Node *>, compare> tPQ(minHeap);
    while (tPQ.size() != 1)
    {
        left = tPQ.top();
        tPQ.pop();

        right = tPQ.top();
        tPQ.pop();

        root = new Node();
        root->freq = left->freq + right->freq;

        root->left = left;
        root->right = right;
        tPQ.push(root);
    }
}

void huffman::createCodes()
{
    /*  Traversing the Huffman Tree and assigning specific codes to each character  */
    traverse(root, "");
}

void huffman::saveEncodedFile()
{
    /*  Saving encoded (.huf) file  */
    inFile.open(inFileName, ios::in);
    outFile.open(outFileName, ios::out | ios::binary);
    string in = "";
    string s = "";
    char id;

    /*  Saving the meta data (huffman tree)  */
    in += (char)minHeap.size();
    priority_queue<Node *, vector<Node *>, compare> tempPQ(minHeap);
    while (!tempPQ.empty())
    {
        Node *curr = tempPQ.top();
        in += curr->data;
        /*  Saving 16 decimal values representing code of curr->data  */
        s.assign(127 - curr->code.length(), '0');
        s += '1';
        s += curr->code;
        /*  Saving decimal values of every 8-bit binary code  */
        in += (char)binToDec(s.substr(0, 8));
        for (int i = 0; i < 15; i++)
        {
            s = s.substr(8);
            in += (char)binToDec(s.substr(0, 8));
        }
        tempPQ.pop();
    }
    s.clear();

    /*  Saving codes of every charachter appearing in the input file  */
    inFile.get(id);
    while (!inFile.eof())
    {
        s += vec[id]->code;
        /*  Saving decimal values of every 8-bit binary code  */
        while (s.length() > 8)
        {
            in += (char)binToDec(s.substr(0, 8));
            s = s.substr(8);
        }
        inFile.get(id);
    }

    /*  Finally if bits remaining are less than 8, append 0's  */
    int count = 8 - s.length();
    if (s.length() < 8)
    {
        s.append(count, '0');
    }
    in += (char)binToDec(s);
    /*  append count of appended 0's  */
    in += (char)count;

    /*  write the in string to the output file  */
    outFile.write(in.c_str(), in.size());
    inFile.close();
    outFile.close();
}

void huffman::saveDecodedFile()
{
    inFile.open(inFileName, ios::in | ios::binary);
    outFile.open(outFileName, ios::out);
    unsigned char size;
    inFile.read(reinterpret_cast<char *>(&size), 1);
    /*  Reading count at the end of the file which is number of bits appended to make final value 8-bit  */
    inFile.seekg(-1, ios::end);
    char count0;
    inFile.read(&count0, 1);
    /*  Ignoring the meta data (huffman tree) (1 + 17 * size) and reading remaining file  */
    inFile.seekg(1 + 17 * size, ios::beg);

    vector<unsigned char> text;
    unsigned char textseg;
    inFile.read(reinterpret_cast<char *>(&textseg), 1);
    while (!inFile.eof())
    {
        text.push_back(textseg);
        inFile.read(reinterpret_cast<char *>(&textseg), 1);
    }

    Node *curr = root;
    string path;
    for (int i = 0; i < text.size() - 1; i++)
    {
        /*  Converting decimal number to its equivalent 8-bit binary code  */
        path = decToBin(text[i]);
        if (i == text.size() - 2)
        {
            path = path.substr(0, 8 - count0);
        }
        /*  Traversing huffman tree and appending resultant data to the file  */
        for (int j = 0; j < path.size(); j++)
        {
            if (path[j] == '0')
            {
                curr = curr->left;
            }
            else
            {
                curr = curr->right;
            }

            if (curr->left == NULL && curr->right == NULL)
            {
                outFile.put(curr->data);
                curr = root;
            }
        }
    }
    inFile.close();
    outFile.close();
}

void huffman::getTree()
{
    inFile.open(inFileName, ios::in | ios::binary);

    /*  Reading size of MinHeap  */
    unsigned char size;
    inFile.read(reinterpret_cast<char *>(&size), 1);
    root = new Node();

    /*  next size * (1 + 16) characters contain (char)data and (string)code[in decimal]  */
    for (int i = 0; i < size; i++)
    {
        char aCode;
        unsigned char hCodeC[16];
        inFile.read(&aCode, 1);
        inFile.read(reinterpret_cast<char *>(hCodeC), 16);

        /*  converting decimal characters into their binary equivalent to obtain code  */
        string hCodeStr = "";
        for (int i = 0; i < 16; i++)
        {
            hCodeStr += decToBin(hCodeC[i]);
        }

        /*  Removing padding by ignoring first (127 - curr->code.length()) '0's and next '1' character  */
        int j = 0;
        while (hCodeStr[j] == '0')
        {
            j++;
        }
        hCodeStr = hCodeStr.substr(j + 1);

        /*  Adding node with aCode data and hCodeStr string to the huffman tree  */
        buildTree(aCode, hCodeStr);
    }
    inFile.close();
}

void huffman::compress()
{
    createMinHeap();
    createTree();
    createCodes();
    saveEncodedFile();
}

void huffman::decompress()
{
    getTree();
    saveDecodedFile();
}