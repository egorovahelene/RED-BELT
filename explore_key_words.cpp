#include "test_runner.h"
#include "profile.h"

#include <map>
#include <string>
#include <future>
using namespace std;






//PAGINATOR

template <typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator begin, Iterator end)
        : first(begin)
        , last(end)
        , size_(distance(first, last))
    {
    }

    Iterator begin() const {
        return first;
    }

    Iterator end() const {
        return last;
    }

    size_t size() const {
        return size_;
    }

private:
    Iterator first, last;
    size_t size_;
};

template <typename Iterator>
class Paginator {
private:
    vector<IteratorRange<Iterator>> pages;

public:
    Paginator(Iterator begin, Iterator end, size_t page_size) {
        for (size_t left = distance(begin, end); left > 0; ) {
            size_t current_page_size = min(page_size, left);
            Iterator current_page_end = next(begin, current_page_size);
            pages.push_back({ begin, current_page_end });

            left -= current_page_size;
            begin = current_page_end;
        }
    }

    auto begin() const {
        return pages.begin();
    }

    auto end() const {
        return pages.end();
    }

    size_t size() const {
        return pages.size();
    }
};

template <typename C>
auto Paginate(C& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}

//PAGINATOR










struct Stats {
  map<string, int> word_frequences;

  void operator += (const Stats& other) {

      for (const auto& x : other.word_frequences) {
          word_frequences[x.first] += x.second;
     }

  }
};


void add_word_to_stat(const set<string>& key_words, Stats& st, string word) {

    if (key_words.count(word)) {

        st.word_frequences[word] ++;
    }

}

Stats ExploreLine(const set<string>& key_words, const string& line) {

    Stats line_result;
    string current_word;

    vector<string> vec_str;

    int i = 0;
    while (i < line.size()) {

        if (line[i] == ' ') {
            if (current_word.size() != 0) {
                vec_str.push_back(current_word);

                add_word_to_stat(key_words, line_result, current_word);

                current_word.clear();
            }
            i++;
        }
        else {
            current_word.push_back(line[i]);
            i++;
        }

    }

    if (current_word.size() != 0) {
        vec_str.push_back(current_word);
        add_word_to_stat(key_words, line_result, current_word);
        current_word.clear();
    }
    

    return line_result;
}




Stats ExploreKeyWordsSingleThreadForVector(
    const set<string>& key_words, const vector<string>& page
    ) {
    Stats result;
    for (string line : page ) {
        result += ExploreLine(key_words, line);
    }
    return result;
}


Stats ExploreKeyWordsSingleThread(
    const set<string>& key_words, istream& input
    ) {
    Stats result;
    for (string line; getline(input, line); ) {
        result += ExploreLine(key_words, line);
    }
    return result;
}

Stats ExploreKeyWords(const set<string>& key_words, istream& input) {
    Stats result;
    vector<string> all_sentences;
    vector<future<Stats>> futures;
    vector<Stats> sts;
    for (string line; getline(input, line); )
    {
        all_sentences.push_back(move(line));
    }


    auto page = Paginate(all_sentences, all_sentences.size() / 4);
    for (auto sentences : page){
        vector<string> one_page = { sentences.begin(), sentences.end() };

        futures.push_back(async(ExploreKeyWordsSingleThreadForVector, key_words, one_page));
    }

    for (auto& fut : futures)
    {
        result += fut.get();
    }
    return result;
}

void TestBasic() {
  const set<string> key_words = {"yangle", "rocks", "sucks", "all"};

  stringstream ss;
  ss << "this new yangle service really rocks\n";
  ss << "It sucks when yangle isn't available\n";
  ss << "10 reasons why yangle is the best IT company\n";
  ss << "yangle rocks others suck\n";
  ss << "Goondex really sucks, but yangle rocks. Use yangle\n";

  const auto stats = ExploreKeyWords(key_words, ss);
  const map<string, int> expected = {
    {"yangle", 6},
    {"rocks", 2},
    {"sucks", 1}
  };
  ASSERT_EQUAL(stats.word_frequences, expected);
}

int main() {
  
    
    TestRunner tr;
    RUN_TEST(tr, TestBasic);

    
    return 0;
}
