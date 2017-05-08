#include <iostream>
#include <limits>
#include <memory>
#include <unordered_map>
#include <cstdio>
#include <cstdint>
#include <string>
#include <sstream>

void printBinaryRepresentation(int z)
{
  //Чепятает в обратном порядке :)
  size_t nBytes = sizeof(z);
  char* rawPtr((char*)(&z));
  for (size_t byte = 0; byte < nBytes; byte++) {
    for (size_t bit = 0; bit < CHAR_BIT; bit++) {
      std::cout << (((rawPtr[byte]) >> bit) & 1);
    }
  }
  std::cout << std::endl;
}

void RemoveDups(char *pStr)
{
  char* p1 = pStr;
  char* pActSymb = pStr;
  while (*p1 != 0)
  {
    char currentSymb = *p1;
    do
    {
      ++p1;
    }
    while (currentSymb == *p1 && p1 != 0);
    *pActSymb = currentSymb;
    ++pActSymb;
  }
  *pActSymb = 0;
}

struct ListNode {
  ListNode * prev;
  ListNode * next;
  ListNode * rand; // указатель на произвольный элемент данного списка либо NULL
  std::string data;
};

void handleCRetCode(int code)
{
  if (code != 0)
    throw std::exception("Some problem while working with files");
}

class List {
private:
  List(const List &) = delete;
  List(const List &&) = delete;
  List& operator=(const List &) = delete;

  void serialize(FILE * file, const ListNode& node)
  {
    int64_t size = node.data.size();
    fwrite(&size, sizeof(size), 1, file);
    fwrite(node.data.data(), 1, node.data.size(), file);
  }

  void deserialize(FILE* file, ListNode& node)
  {
    int64_t size;
    fread(&size, sizeof(size), 1, file);
    node.data.resize((size_t) size);
    fread((void*) node.data.data(), 1, (size_t)size, file);
  }


  void writeCache(FILE * file, const std::unordered_map<ListNode*, int64_t>& serializationCache, const std::vector<ListNode*> & rands)
  {
    for each (ListNode * p in rands)
    {
      const auto it = serializationCache.find(p);
      const int64_t index = it == serializationCache.end() ? rands.size() : it->second;
      fwrite(&index, sizeof(index), 1, file);
    }
  }

  std::vector<int64_t> readRandIndexes(FILE * file, size_t count)
  {
    std::vector<int64_t> randIndexes(count);
    fread(randIndexes.data(), sizeof(int64_t), randIndexes.size(), file);
    return randIndexes;
  }

  std::vector<ListNode*> initSkeleton()
  {
    std::vector<ListNode*> nodes(count + 2, nullptr);
    for (int i = 1; i < count + 1; ++i)
    {
      nodes[i] = new ListNode();
    }

    for (int i = 1; i < count + 1; ++i)
    {
      nodes[i]->prev = nodes[i - 1];
      nodes[i]->next = nodes[i + 1];
      nodes[i]->rand = nullptr;
    }
    head = nodes[1];
    tail = nodes[count + 1];
    return nodes;
  }

  void destroy()
  {
    ListNode* p = head;
    for (int i = 0; i < count; ++i)
    {
      ListNode* next = p->next;
      delete p;
      p = next;
    }
    head = nullptr;
    tail = nullptr;
    count = 0;
  }

public:
  List(const std::vector<std::string> & inputData)
  {
    count = inputData.size();
    initSkeleton();
    ListNode* p = head;
    for (int i = 0; i < count; ++i)
    {
      p->data = inputData[i];
      p = p->next;
    }
  }

  ~List()
  {
    destroy();
  }

  std::string ToString()
  {
    std::stringstream ss;
    ListNode* p = head;
    for (int i = 0; i < count; ++i)
    {
      ss << p->data << " ,";
      p = p->next;
    }
    ss << std::endl;

    p = head;
    ss << "Rands: ";
    for (int i = 0; i < count; ++i)
    {
      if (p->rand == nullptr)
        ss << "()";
      else
        ss << p->rand->data;

      p = p->next;
      ss<< " ,";
    }
    ss << std::endl;
    return ss.str();
  }

  void Serialize(FILE * file) // сохранение в файл (файл открыт с помощью fopen(path, &quot; wb&quot;))
  {
    int64_t tmpCount = count;
    fwrite(&tmpCount, sizeof(tmpCount), 1, file);
    std::vector<ListNode*> rands;
    std::unordered_map<ListNode*, int64_t> serializationCache;
    ListNode* p = head;
    int id = 0;

    //Оставляем место для random номеров
    fpos_t rndCachePos;
    handleCRetCode(fgetpos(file, &rndCachePos));
    handleCRetCode(fseek(file, sizeof(int64_t)*count, SEEK_CUR));

    while (id <count)
    {
      if (p == nullptr)
        throw std::runtime_error("Invalid struct");
      serialize(file, *p);
      rands.push_back(p->rand);
      serializationCache[p] = id;
      p = p->next;
      ++id;
    }

    handleCRetCode(fsetpos(file, &rndCachePos));
    writeCache(file, serializationCache, rands);
  }



  void Deserialize(FILE * file) // загрузка из файла (файл открыт с помощью fopen(path, &quot; rb&quot;))
  {
    destroy();
    int64_t tmpCount = count;
    fread(&tmpCount, sizeof(tmpCount), 1, file);
    count = (int) tmpCount;
    if (count == 0)
      return;

    std::vector<ListNode*> nodes = initSkeleton();
    const std::vector<int64_t> randIndexes = readRandIndexes(file, count);
    for (int i = 1; i < count + 1; ++i)
    {
      const int randInd = (size_t) randIndexes[i - 1];
      nodes[i]->rand = randInd < count ? nodes[randInd] : nullptr;
    }

    for (int i = 1; i < count +1; ++i)
    {
      deserialize(file, *nodes[i]);
    }
  }
private:
  ListNode * head;
  ListNode * tail;
  int count;
};

typedef std::unique_ptr<List> ListPtr;

class Test
{
  void checkList(const std::vector<std::string> & sample)
  {
    auto pList = std::make_unique<List>(sample);
    std::string before = pList->ToString();
    std::cout << before;
    {
      FILE* file;
      handleCRetCode(fopen_s(&file, "tmp.dat", "wb"));
      pList->Serialize(file);
      fclose(file);
    }
    {
      FILE* file;
      handleCRetCode(fopen_s(&file, "tmp.dat", "rb"));
      pList->Deserialize(file);
      fclose(file);
    }
    std::string after = pList->ToString();
    std::cout << after;
    if (before != after)
      throw std::exception("Assertion failed");
  }
  
  void printHelperForTest1(int num)
  {
    std::cout << num << " in binary: ";
    printBinaryRepresentation(num);
  }

  void test1()
  {
    std::cout << "=======Task 1" << std::endl;
    printHelperForTest1(1);
    printHelperForTest1(-1);
    printHelperForTest1(0);
    printHelperForTest1(std::numeric_limits<int>::min());
    printHelperForTest1(std::numeric_limits<int>::max());
  }

  void test2()
  {
    std::cout << "=======Task 2" << std::endl;
    char z[] = "AAA BBB AAA";
    RemoveDups(z);
    if (strcmp(z, "A B A") != 0)
      throw std::exception("assertion failed");
  }

  void test3()
  {
    std::cout << "=======Task 3" << std::endl;
    checkList(std::vector<std::string>{ "_1_", "__2__", "3", "4" });
    checkList(std::vector<std::string>{});
    checkList(std::vector<std::string>{ "_1_"});
  }

public:
  void TestAll()
  {
    test1();
    test2();
    test3();
  }
};  

int main()
{
  Test().TestAll();
  return 0;
}