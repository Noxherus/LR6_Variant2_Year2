#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

using U64 = std::uint64_t;
using Index = std::ptrdiff_t;
using Clock = std::chrono::steady_clock;

// Часть 1. Рекурсивные и итеративные алгоритмы.
void checkRange(int n, int maximum) {
    if (n < 0 || n > maximum)
        throw std::out_of_range("Argument is outside the supported range");
}

U64 factorialRecImpl(int n) {
    if (n <= 1) return 1;
    return static_cast<U64>(n) * factorialRecImpl(n - 1);
}
U64 factorialRecursive(int n) {
    checkRange(n, 20); // 21! не помещается в uint64_t.
    return factorialRecImpl(n);
}
U64 factorialIterative(int n) {
    checkRange(n, 20);
    U64 result = 1;
    for (int i = 2; i <= n; ++i) result *= i;
    return result;
}

U64 fibRecImpl(int n) {
    if (n <= 1) return static_cast<U64>(n);
    return fibRecImpl(n - 1) + fibRecImpl(n - 2);
}
U64 fibRecursive(int n) {
    // Ограничение защищает от слишком долгого вычисления.
    checkRange(n, 40);
    return fibRecImpl(n);
}
U64 fibIterative(int n) {
    checkRange(n, 93); // F(94) превышает диапазон uint64_t.
    if (n <= 1) return static_cast<U64>(n);
    U64 a = 0, b = 1;
    for (int i = 2; i <= n; ++i) {
        U64 c = a + b;
        a = b;
        b = c;
    }
    return b;
}
U64 fibMemoImpl(int n, std::array<U64, 94>& cache) {
    if (n <= 1) return static_cast<U64>(n);
    if (cache[n] == 0)
        cache[n] = fibMemoImpl(n - 1, cache) + fibMemoImpl(n - 2, cache);
    return cache[n];
}
U64 fibMemoized(int n) {
    checkRange(n, 93);
    std::array<U64, 94> cache{};
    return fibMemoImpl(n, cache);
}

// Деление на 10 работает и для отрицательных чисел без abs().
int countDigitsRecursive(long long n) {
    if (n > -10 && n < 10) return 1;
    return 1 + countDigitsRecursive(n / 10);
}
int countDigitsIterative(long long n) {
    int count = 1;
    while (n <= -10 || n >= 10) {
        n /= 10;
        ++count;
    }
    return count;
}

// Бинарный поиск требует предварительно отсортированный массив.
// Используется полуоткрытый диапазон [left, right).
Index binaryRecImpl(const std::vector<int>& a, int key,
                    Index left, Index right) {
    if (left >= right) return -1;
    Index mid = left + (right - left) / 2;
    if (a[mid] == key) return mid;
    if (a[mid] < key) return binaryRecImpl(a, key, mid + 1, right);
    return binaryRecImpl(a, key, left, mid);
}
Index binaryRecursive(const std::vector<int>& a, int key) {
    return binaryRecImpl(a, key, 0, static_cast<Index>(a.size()));
}
Index binaryIterative(const std::vector<int>& a, int key) {
    Index left = 0, right = static_cast<Index>(a.size());
    while (left < right) {
        Index mid = left + (right - left) / 2;
        if (a[mid] == key) return mid;
        if (a[mid] < key) left = mid + 1;
        else right = mid;
    }
    return -1;
}

// Часть 2. Простые сортировки.
struct Stats {
    U64 comparisons = 0;
    U64 moves = 0;
};

// Считаем сравнения ключей и перемещения значений.
// Один обмен соответствует трём перемещениям.
template <bool Count>
bool greater(int a, int b, Stats& s) {
    if constexpr (Count) ++s.comparisons;
    return a > b;
}
template <bool Count>
void exchange(int& a, int& b, Stats& s) {
    std::swap(a, b);
    if constexpr (Count) s.moves += 3;
}

template <bool Count>
Stats bubbleSort(std::vector<int>& a) {
    Stats s;
    for (std::size_t end = a.size(); end > 1; --end) {
        bool swapped = false;
        for (std::size_t j = 1; j < end; ++j) {
            if (greater<Count>(a[j - 1], a[j], s)) {
                exchange<Count>(a[j - 1], a[j], s);
                swapped = true;
            }
        }
        if (!swapped) break; // Досрочный выход.
    }
    return s;
}

template <bool Count>
Stats insertionSort(std::vector<int>& a) {
    Stats s;
    for (std::size_t i = 1; i < a.size(); ++i) {
        int key = a[i];
        if constexpr (Count) ++s.moves;
        std::size_t j = i;
        while (j > 0 && greater<Count>(a[j - 1], key, s)) {
            a[j] = a[j - 1];
            if constexpr (Count) ++s.moves;
            --j;
        }
        a[j] = key;
        if constexpr (Count) ++s.moves;
    }
    return s;
}

template <bool Count>
Stats selectionSort(std::vector<int>& a) {
    Stats s;
    for (std::size_t i = 0; i + 1 < a.size(); ++i) {
        std::size_t minIndex = i;
        for (std::size_t j = i + 1; j < a.size(); ++j) {
            if (greater<Count>(a[minIndex], a[j], s)) minIndex = j;
        }
        if (minIndex != i) exchange<Count>(a[i], a[minIndex], s);
    }
    return s;
}

int testsPassed = 0;
void test(const char* name, bool ok) {
    std::cout << name << ": " << (ok ? "PASS" : "FAIL") << '\n';
    if (!ok) throw std::runtime_error("Test failed");
    ++testsPassed;
}
template <class F>
bool rejects(F action) {
    try { action(); }
    catch (const std::out_of_range&) { return true; }
    return false;
}

void testRecursion() {
    std::cout << "=== PART 1: RECURSIVE ALGORITHMS ===\n";
    std::cout << "5! = " << factorialRecursive(5) << '\n';
    test("Factorial example", factorialRecursive(5) == 120);
    test("Factorial base cases", factorialRecursive(0) == 1 && factorialRecursive(1) == 1);
    bool ok = true;
    for (int i = 0; i <= 20; ++i)
        ok = ok && factorialRecursive(i) == factorialIterative(i);
    test("Factorial recursive vs iterative", ok);
    test("Factorial maximum", factorialRecursive(20) == 2432902008176640000ULL);
    test("Factorial invalid input", rejects([] { factorialRecursive(-1); }) &&
                                    rejects([] { factorialIterative(21); }));

    std::cout << "F(10) = " << fibRecursive(10) << '\n';
    test("Fibonacci example", fibRecursive(10) == 55);
    test("Fibonacci base cases", fibRecursive(0) == 0 && fibRecursive(1) == 1);
    ok = true;
    for (int i = 0; i <= 25; ++i)
        ok = ok && fibRecursive(i) == fibIterative(i) && fibMemoized(i) == fibIterative(i);
    test("Fibonacci three versions", ok);
    test("Fibonacci maximum", fibMemoized(93) == 12200160415121876738ULL &&
                              fibIterative(93) == fibMemoized(93));
    test("Fibonacci invalid input", rejects([] { fibRecursive(-1); }) &&
         rejects([] { fibRecursive(41); }) && rejects([] { fibMemoized(94); }));

    std::cout << "Digits in -123456 = " << countDigitsRecursive(-123456) << '\n';
    test("Digit count example", countDigitsRecursive(123456) == 6);
    test("Digit count zero", countDigitsRecursive(0) == 1 && countDigitsIterative(0) == 1);
    test("Negative digit count", countDigitsRecursive(-123456) == 6);
    auto min = (std::numeric_limits<long long>::min)();
    test("Minimum signed integer", countDigitsRecursive(min) == 19 && countDigitsIterative(min) == 19);
    ok = true;
    for (long long n : {0LL, 9LL, -9LL, 10LL, -100LL, 123456789LL, min})
        ok = ok && countDigitsRecursive(n) == countDigitsIterative(n);
    test("Digit count recursive vs iterative", ok);

    const std::vector<int> a{10, 20, 30, 40, 50};
    std::cout << "Search for 40: index " << binaryRecursive(a, 40) << '\n';
    test("Binary search example", binaryRecursive(a, 40) == 3);
    test("Binary search boundaries", binaryRecursive(a, 10) == 0 && binaryRecursive(a, 50) == 4);
    test("Binary search missing", binaryRecursive(a, 35) == -1 && binaryIterative(a, 60) == -1);
    test("Binary search empty", binaryRecursive({}, 1) == -1 && binaryIterative({}, 1) == -1);
    test("Binary search singleton", binaryRecursive({7}, 7) == 0 && binaryIterative({7}, 8) == -1);
    const std::vector<int> duplicates{1, 2, 2, 2, 3};
    Index pos = binaryRecursive(duplicates, 2);
    test("Binary search duplicates", pos >= 0 && duplicates[pos] == 2);
    ok = true;
    for (int key = 0; key <= 60; ++key)
        ok = ok && binaryRecursive(a, key) == binaryIterative(a, key);
    test("Binary search recursive vs iterative", ok);
}

// Сохраняем результаты, чтобы компилятор не удалил вычисления.
volatile U64 benchmarkSink = 0;
volatile int benchmarkInput = 0;

template <class F>
double measureCalls(int repetitions, F f) {
    std::array<double, 3> times{};
    for (double& t : times) {
        U64 sum = 0;
        auto start = Clock::now();
        for (int i = 0; i < repetitions; ++i) {
            benchmarkInput = i;
            sum += static_cast<U64>(f(benchmarkInput));
        }
        auto end = Clock::now();
        benchmarkSink = sum;
        t = std::chrono::duration<double, std::micro>(end - start).count();
    }
    std::sort(times.begin(), times.end());
    return times[1];
}

void printRecRow(const char* label, int reps, double rec, double iter) {
    std::cout << std::left << std::setw(20) << label << std::right
              << std::setw(9) << reps << std::setw(13) << rec
              << std::setw(13) << iter << '\n';
}

void recursionBenchmark() {
    std::cout << "\n=== RECURSION VS ITERATION ===\n";
    std::cout << "Median of 3 trials, total time (us)\n";
    std::cout << "Same inputs for both implementations in each row\n";
    std::cout << std::left << std::setw(20) << "Algorithm" << std::right
              << std::setw(9) << "Calls" << std::setw(13) << "Recursive"
              << std::setw(13) << "Iterative" << '\n';
    constexpr int reps = 100000;
    printRecRow("Factorial", reps,
        measureCalls(reps, [](int i) { return factorialRecursive(10 + i % 11); }),
        measureCalls(reps, [](int i) { return factorialIterative(10 + i % 11); }));
    printRecRow("Fibonacci", 20,
        measureCalls(20, [](int i) { return fibRecursive(28 + i % 3); }),
        measureCalls(20, [](int i) { return fibIterative(28 + i % 3); }));
    printRecRow("Digit count", reps,
        measureCalls(reps, [](int i) { return countDigitsRecursive(123456789LL + i); }),
        measureCalls(reps, [](int i) { return countDigitsIterative(123456789LL + i); }));
    std::vector<int> a(100000);
    std::iota(a.begin(), a.end(), 0);
    printRecRow("Binary search", reps,
        measureCalls(reps, [&](int i) { return binaryRecursive(a, i); }),
        measureCalls(reps, [&](int i) { return binaryIterative(a, i); }));
    std::cout << "Fibonacci memoized (20 calls, n=28..30): "
              << measureCalls(20, [](int i) { return fibMemoized(28 + i % 3); })
              << " us\n";
}

using SortFunction = Stats (*)(std::vector<int>&);

void testSort(const char* name, SortFunction f) {
    std::cout << name << '\n';
    const std::vector<std::vector<int>> samples{
        {}, {42}, {1, 2, 3, 4}, {4, 3, 2, 1}, {3, -1, 3, 0, -1}, {1, 3, 2, 4, 6, 5}
    };
    const char* labels[]{"Empty array", "Single element", "Sorted array",
                         "Reverse order", "Duplicates and negatives", "Nearly sorted"};
    for (std::size_t i = 0; i < samples.size(); ++i) {
        auto a = samples[i];
        auto expected = a;
        std::sort(expected.begin(), expected.end()); // Только эталон для теста.
        f(a);
        test(labels[i], a == expected);
    }
}

// Меняем местами 1% непересекающихся пар соседних элементов.
std::vector<int> nearlySorted(int n) {
    std::vector<int> a(n), pairs(n / 2);
    std::iota(a.begin(), a.end(), 0);
    std::iota(pairs.begin(), pairs.end(), 0);
    std::mt19937 rng(2026);
    std::shuffle(pairs.begin(), pairs.end(), rng);
    for (int i = 0; i < n / 100; ++i) {
        int p = 2 * pairs[i];
        std::swap(a[p], a[p + 1]);
    }
    return a;
}

double measureSort(const std::vector<int>& source, SortFunction f) {
    std::array<double, 3> times{};
    for (double& t : times) {
        auto a = source; // Копирование не входит в измерение.
        auto start = Clock::now();
        f(a);
        auto end = Clock::now();
        t = std::chrono::duration<double, std::micro>(end - start).count();
        if (!std::is_sorted(a.begin(), a.end())) throw std::runtime_error("Sort failed");
    }
    std::sort(times.begin(), times.end());
    return times[1];
}

void sortingBenchmark() {
    std::cout << "\n=== PART 2: SIMPLE SORTING TESTS ===\n";
    testSort("Bubble sort:", bubbleSort<true>);
    testSort("Insertion sort:", insertionSort<true>);
    testSort("Selection sort:", selectionSort<true>);
    const std::array<int, 4> sizes{1000, 5000, 10000, 20000};
    const std::array<SortFunction, 3> fast{bubbleSort<false>, insertionSort<false>, selectionSort<false>};
    const std::array<SortFunction, 3> counted{bubbleSort<true>, insertionSort<true>, selectionSort<true>};
    std::cout << "\n=== NEARLY SORTED ARRAYS: TIME ===\n";
    std::cout << "N/100 disjoint adjacent swaps; median of 3 (us)\n";
    std::cout << "Copying and operation counting excluded from timing\n";
    std::cout << std::setw(9) << "N" << std::setw(14) << "Bubble"
              << std::setw(14) << "Insertion" << std::setw(14) << "Selection" << '\n';
    for (int n : sizes) {
        auto a = nearlySorted(n);
        std::cout << std::setw(9) << n;
        for (auto f : fast) std::cout << std::setw(14) << measureSort(a, f);
        std::cout << '\n';
    }
    std::cout << "\n=== SORTING OPERATION COUNTS ===\n";
    std::cout << "Key comparisons; moves include temp (swap = 3)\n";
    std::cout << std::setw(8) << "N" << std::setw(12) << "Algorithm"
              << std::setw(15) << "Comparisons" << std::setw(13) << "Moves" << '\n';
    const char* names[]{"Bubble", "Insertion", "Selection"};
    for (int n : sizes) {
        auto source = nearlySorted(n);
        for (std::size_t i = 0; i < counted.size(); ++i) {
            auto a = source;
            Stats s = counted[i](a);
            for (int j = 0; j < n; ++j)
                if (a[j] != j) throw std::runtime_error("Incorrect sorting result");
            std::cout << std::setw(8) << n << std::setw(12) << names[i]
                      << std::setw(15) << s.comparisons << std::setw(13) << s.moves << '\n';
        }
    }
    test("Benchmark results verified", true);
}

int main() {
    try {
        std::cout << "LAB 6 - YEAR 2 - VARIANT 2\n\n";
        std::cout << std::fixed << std::setprecision(2);
        testRecursion();
        recursionBenchmark();
        sortingBenchmark();
        std::cout << "\n=== COMPLEXITY SUMMARY ===\n";
        std::cout << "Factorial: O(n) time; recursive O(n) stack, iterative O(1)\n"
                  << "Naive Fibonacci: O(2^n) time, O(n) stack\n"
                  << "Memoized Fibonacci: O(n) time and space (general model)\n"
                  << "Iterative Fibonacci: O(n) time, O(1) space\n"
                  << "Digits: O(d) time; recursive O(d) stack, iterative O(1)\n"
                  << "Binary search: O(log n) time; recursive O(log n) stack\n"
                  << "Iterative binary search: O(1) extra space\n"
                  << "Bubble/Insertion: best O(n), worst O(n^2), stable\n"
                  << "Selection: O(n^2) in all cases, not stable\n"
                  << "All three sorts: O(1) extra space\n";
        std::cout << "\nTests passed: " << testsPassed << '\n';
        std::cout << "Program completed successfully.\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
