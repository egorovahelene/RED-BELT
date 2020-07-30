#include "test_runner.h"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <set>
#include <utility>
#include <vector>

using namespace std;

template <typename T>
class PriorityCollection {
public:
    using Id = size_t;      /* тип, используемый для идентификаторов *///;
    using Priority = int;

  // Добавить объект с нулевым приоритетом
  // с помощью перемещения и вернуть его идентификатор
    Id Add(T object) {
        objects[counter] = move(object);
        id_to_pr[counter] = 0;
        pr_to_id[0].insert(counter);
        counter++;
        return counter - 1;
  }

  // Добавить все элементы диапазона [range_begin, range_end)
  // с помощью перемещения, записав выданные им идентификаторы
  // в диапазон [ids_begin, ...)
  template <typename ObjInputIt, typename IdOutputIt>
  void Add(ObjInputIt range_begin, ObjInputIt range_end,
      IdOutputIt ids_begin) {
      vector<Id> v;
      
      for (auto it = make_move_iterator(range_begin); it != make_move_iterator(range_end); ++it) {
         
          *(ids_begin++) = Add(*it);
          
      }


  }

  // Определить, принадлежит ли идентификатор какому-либо
  // хранящемуся в контейнере объекту
  bool IsValid(Id id) const {
      return id_to_pr.count(id);
  }

  // Получить объект по идентификатору
  const T& Get(Id id) const {
      return objects.at(id);
  }

  // Увеличить приоритет объекта на 1
  void Promote(Id id) {
      auto pr_of_id = id_to_pr[id];
      pr_to_id[pr_of_id].erase(id);

      if (pr_to_id[pr_of_id].size() == 0) {
          pr_to_id.erase(pr_of_id);
      }

      id_to_pr[id] ++;
      pr_to_id[id_to_pr[id]].insert(id);
      

  }

  // Получить объект с максимальным приоритетом и его приоритет
  pair<const T&, int> GetMax() const {
      
      /*pair<Id, Priority> pr = *std::max_element
      (
          std::begin(id_to_pr), std::end(id_to_pr),
          [](auto m1, auto m2) {
              return m1.second < m2.second;
          }
      );*/

      int max_pr = pr_to_id.rbegin()->first;
      
      auto max_id = *(pr_to_id.at(max_pr).rbegin());
    
    
     // pair<const T&, int> answ;
      //answ.first = objects.at(max_id);
      //answ.second = max_pr;

      return { objects.at(max_id), max_pr };
  }

  // Аналогично GetMax, но удаляет элемент из контейнера
  pair<T, int> PopMax() {
      
      auto max_pr = pr_to_id.rbegin()->first;
      auto max_id = *(pr_to_id[max_pr].rbegin());


      pair<T, int> answ = { move(objects.at(max_id)), max_pr };
      

      id_to_pr.erase(max_id);
      objects.erase(max_id);
      pr_to_id[max_pr].erase(max_id);

      if (pr_to_id[max_pr].size() == 0) {
          pr_to_id.erase(max_pr);
      }
      
      return answ;

  }

private:
    size_t counter = 0;
    
    std::map<Id, Priority> id_to_pr;
    std::map<Priority, std::set<Id>> pr_to_id;
    std::map<Id, T> objects;
};


class StringNonCopyable : public string {
public:
  using string::string;  // Позволяет использовать конструкторы строки
  StringNonCopyable(const StringNonCopyable&) = delete;
  StringNonCopyable(StringNonCopyable&&) = default;
  StringNonCopyable& operator=(const StringNonCopyable&) = delete;
  StringNonCopyable& operator=(StringNonCopyable&&) = default;
};

void TestNoCopy() {
  PriorityCollection<StringNonCopyable> strings;
  const auto white_id = strings.Add("white");
  const auto yellow_id = strings.Add("yellow");
  const auto red_id = strings.Add("red");

  strings.Promote(yellow_id);
  for (int i = 0; i < 2; ++i) {
    strings.Promote(red_id);
  }
  strings.Promote(yellow_id);
  {
    const auto item = strings.PopMax();
    ASSERT_EQUAL(item.first, "red");
    ASSERT_EQUAL(item.second, 2);
    const auto item1 = strings.GetMax();

    //vector<StringNonCopyable> v = { "a", "b", "c" };
    //vector<int> ids;
    //strings.Add(v.begin(), v.end(), ids.begin());

  }
  {
    const auto item = strings.PopMax();
    ASSERT_EQUAL(item.first, "yellow");
    ASSERT_EQUAL(item.second, 2);
  }
  {
    const auto item = strings.PopMax();
    ASSERT_EQUAL(item.first, "white");
    ASSERT_EQUAL(item.second, 0);
  }
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestNoCopy);
  return 0;
}
