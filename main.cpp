#include <iostream>
#include <fstream>
#include <unistd.h>
#include "vector"
#include "Monah.cpp"

// Подключение пространства имён std.
using namespace std;

pthread_mutex_t roundMutex, getWinner, makePairs, getRandomNum;
std::vector<Monah> monahs;

// Поле для вывода данных в файл.
ofstream output;

int size_;

// Ввод/вывод с файла.
bool from_file = false;

void AddMonah(int ci) {
    monahs.emplace_back(Monah(ci));
}

int GetRandomNum(int min, int max) {
    // Мьют функции для выполнения её только текущим потоком.
    pthread_mutex_lock(&getRandomNum);
        // Генерация случайного числа из диапозона.
        srand(time(nullptr));
        int eps = min + rand() % (max - min + 1);
    // Разблокировка функции для других потоков.
    pthread_mutex_unlock(&getRandomNum);
    return eps;
}

// Получение начального числа монахов при режиме "r".
int GetRandomSize() {
    srand(time(nullptr));
    int eps = 1 + (int) rand() % static_cast<int>(10 - 1 + 1);
    return eps;
}

// Получение случайного ци в описанных пределах.
int GetRandomCi() {
    srand(time(nullptr));
    int eps = 2 + (int) rand() % static_cast<int>(INT_MAX - 2 + 1);
    return eps;
}

std::vector<std::pair<int, int>> MakePairs() {
    pthread_mutex_lock(&makePairs);
        std::vector<std::pair<int, int>> pairs; // Список итоговых пар битв.
        std::vector<int> peeks; // Список всех вершин.

        for (int i = 0; i < size_; ++i) {
            peeks.emplace_back(i);
        }

        std::vector<int> peeks_which_were (peeks.size()); // Список вершин, которые уже были, чтобы исключить двойное
        // или более участие одного монаха в раунде

        auto countPairs = size_ / 2;
        for (int k = 0; k < countPairs; ++k) {
            usleep(4000); // для исключения повторов генерации.
            int first = GetRandomNum(countPairs, size_ - 1);
            peeks_which_were[k] = 1;

            while (peeks_which_were[first] == 1) {
                usleep(3000);
                first = GetRandomNum(countPairs, size_ - 1);
            }
            peeks_which_were[first] = 1;
            pairs.emplace_back(std::make_pair(k, first));
        }

        // Случай, если число монахов нечётно.
        if (size_ % 2 == 1) {
            for (int i = 0; i < peeks_which_were.size(); ++i) {
                if (peeks_which_were[i] == 0) {
                    pairs.emplace_back(std::make_pair(i, -1));
                }
            }
        }
    pthread_mutex_unlock(&makePairs);
    return pairs;
}

Monah GetWinner(Monah first, Monah second) {
    pthread_mutex_lock(&getWinner);
        if (first.GetCi() > second.GetCi()) {
            pthread_mutex_unlock(&getWinner);
            return first;
        } else {
            pthread_mutex_unlock(&getWinner);
            return second;
        }
}

void* Round(void* round_number) {
    pthread_mutex_lock(&roundMutex);
        // Обработка, если надо в файл информацию выводить
        if (!from_file) {
            std::cout << "#######################Round#######################\n";
            std::cout << "\tList of monahs before fight:\n";
        } else {
            output << "#######################Round#######################\n";
            output << "\tList of monahs before fight:\n";
        }

        for (size_t i = 0; i < size_; ++i) {
            if (!from_file) {
                std::cout << "\t\t\t\t\t" << monahs[i] << '\n';
            } else {
                output << "\t\t\t\t\t" << monahs[i] << '\n';
            }
        }

        std::vector<std::pair<int, int>> pairs = MakePairs();
        if (!from_file) {
            std::cout << "\n\tPairs are generated and:\n";
        } else {
            output << "\n\tPairs are generated and:\n";
        }

        for (auto pair : pairs) {
            if (pair.second == -1) {
                if (!from_file) {
                    std::cout << "\t\t\t" << monahs[pair.first]
                              << " wasn't fight that's why his power will increase in 2 times\n";
                } else {
                    output << "\t\t\t" << monahs[pair.first]
                           << " wasn't fight that's why his power will increase in 2 times\n";
                }
                monahs[pair.first].InvreaseCi2Times(); // увеличение ци в 2 раза у монаха, если он не сражался в раунде.
            } else {
                if (!from_file) {
                    std::cout << "\t\t\t" << monahs[pair.first] << " beat " << monahs[pair.second] << '\n';
                } else {
                    output << "\t\t\t" << monahs[pair.first] << " beat " << monahs[pair.second] << '\n';
                }

                Monah winner = GetWinner(monahs[pair.first], monahs[pair.second]);

                if (!from_file) {
                    std::cout << "\t\t\t" << winner << " was won\n";
                } else {
                    output << "\t\t\t" << winner << " was won\n";
                }

                if (winner == monahs[pair.first]) {
                    monahs.erase(monahs.begin() + pair.second);
                } else {
                    monahs.erase(monahs.begin() + pair.first);
                }
            }
        }

        size_ = monahs.size();
        if (!from_file) {
            std::cout << "\n\tList of monahs after fight:\n";
        } else {
            output << "\n\tList of monahs after fight:\n";
        }

        for (size_t i = 0; i < size_; ++i) {
            if (!from_file) {
                std::cout << "\t\t\t\t\t" << monahs[i] << '\n';
            } else {
                output << "\t\t\t\t\t" << monahs[i] << '\n';
            }
        }

        if (!from_file) {
            std::cout << "\n\n\n";
        } else {
            output << "\n\n\n";
        }
    pthread_mutex_unlock(&roundMutex);
    return nullptr;
}

int main(int len, char** argv) {
    vector<pthread_t> threads;
    bool generate = false;
    ifstream input;

    if (len == 3) {
        // Инициализация файлов.
        input.open(argv[1]);
        output.open(argv[2]);
        from_file = true;
    } else if (len == 2) {
        // Число параметров в командной строке = len - 1.
        if (std::string(argv[1]) == "r") {
            generate = true;
        }
    } else if (len != 1) {
        cout << "There is unsuported count of args. It should be from 1 to 3\n";
        return 0;
    }

    if (len == 3) {
        int num;
        input >> num;
        size_ = num;
        int count = 0;
        if (size_ < 1) {
            output << "There is unsuported count of monahs.\n";
            output.close();
            input.close();
            return 0;
        }
        while ((input >> num)) {
            if (count < size_) {
                AddMonah(num);
                ++count;
            }
        }
        input.close();
    } else if (generate) {
        size_ = GetRandomSize();
        for (int i = 0; i < size_; ++i) {
            usleep(1000000);
            AddMonah(GetRandomCi());
        }
    } else {
        int count_monahs;
        cin >> count_monahs;
        if (count_monahs <= 1) {
            cout << "There is unsuported count of monahs.\n";
            return 0;
        }
        size_ = count_monahs;

        int ci;
        for (int i = 0; i < count_monahs; ++i) {
            cin >> ci;
            AddMonah(ci);
        }
    }
    auto size = size_ + size_ % 2;

    while (size / 2 != 1) {
        pthread_t th;
        pthread_create(&th, nullptr, Round, nullptr);
        threads.emplace_back(th);
        pthread_join(th, nullptr);
        size /= 2;
        size += size % 2;
    }

    if (size_ == 2) {
        Round(nullptr);
    }

    pthread_mutex_destroy(&roundMutex);
    pthread_mutex_destroy(&getWinner);
    pthread_mutex_destroy(&makePairs);
    pthread_mutex_destroy(&getRandomNum);
    if (len == 3) {
        output << monahs[0] << " won!\n";
    } else {
        std::cout << monahs[0] << " won!\n";
    }
    output.close();
}