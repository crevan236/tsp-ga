#include <iostream>
#include <math.h>
#include <time.h>
#include <cstdlib>
#include <limits.h>

using namespace std;

enum coordinates{x, y};

//////////////////////////////////////
namespace arrays_functions {

    void fillTableRandomly(double **tab, const int size) {
        for(int j = 0; j < size; j++) {
            for(int i = 0; i < 2; i++) {
                tab[j][i] = -150 + rand() % 251;
                tab[j][i] += (double)(rand()%10)/(double)INT_MAX;
            }
        }
    }
    void printTable(int *tab, const int size) {
        for(int i = 0; i < size; i++) {
            cout << tab[i] << "  ";
        }
        cout << endl;
    }
    void printPoints(double **tab, const int size) {
        cout << "Punkty w przestrzeni: \n" << "x\ty" << endl;
        for(int i = 0; i < size; i++) {
            for(int j = 0; j < 2; j++) {
                cout << tab[i][j] << "\t";
            }
            cout << endl;
        }
        cout << endl;
    }

}

namespace tsp_functions {
    double dist(const double xi, const double yi, const double xj, const double yj) {
        return sqrt(pow((xi - xj),2) + pow((yi - yj),2));
    }
    int findNextPoint(int N, const double **T, int *R, int start) {
        int min = start + 1;
        for(int i = start + 2; i < N; ++i) {
            if(dist(T[R[start]][x], T[R[start]][y], T[R[i]][x], T[R[i]][y]) < dist(T[R[start]][x], T[R[start]][y], T[R[min]][x], T[R[min]][y]))
                min = i;
        }
        return min;
    }
    void TSPGreedy(int N, const double **T, int *R) {
        for(int i = 0; i < N-2; ++i) {
            int min = findNextPoint(N, T, R, i);
            int temp = R[i+1];
            R[i+1] = R[min];
            R[min] = temp;
        }
    }
    double measureTheRoad(int *person, const double **Points, const int N) {
        double road = 0;
        for(int i = 0; i < N-1; ++i) {
            road += dist(Points[person[i]][x], Points[person[i]][y], Points[person[i+1]][x], Points[person[i+1]][y]);
        }
        road += dist(Points[person[N-1]][x], Points[person[N-1]][y], Points[person[0]][x], Points[person[0]][y]);
        return road;
    }
    void killPerson(int *person, const int N) {
        for(int i = 0; i < N; ++i) {
            person[i] = -1;
        }
    }
    void eviromentFunction(const int N, const int poplSize, const double **Points, int **population, double *roads) {
        // znajdujemy najdłuższe drogi i je oznaczamy (alg wyszukiwania maksimum)
        for(int i = 0; i < (poplSize - poplSize/10); ++i) {
        //średnio 90% znajdujemy najgorszego osobnika
        //ale nie zabijamy 90%
            int max = 0;
            for(int j = 0; j < poplSize; ++j) {
                if(roads[j] > roads[max])
                    max = j;
            }
            //oznaczamy do zabicia te które są najgorsze oraz nie miały szczęścia przeżyć
            //troche jak w naturze (słabe osobniki mają szansę przetrwać)
            if(rand() % 100 < 49) {
                roads[max] = -1;
            }
        }
        // zabijamy najsłabsze osobniki
        for(int i = 0; i < poplSize; ++i) {
            if(-1 == roads[i]) {
                killPerson(population[i], N);
            }
        }
    }
    void mutatePerson(int *person, const int N) {
        for(int i = 0; i < (1 + rand() % 5); ++i) {
            int first = 1 + rand() % (N - 1);
            int second = 1 + rand() % (N - 1);
            int temp = person[first];
            person[first] = person[second];
            person[second] = temp;
        }
    }
    void ModifiedOXCrossover(int *fparent, int *sparent, int *fchild, int *schild, const int N) {
        int mother[N], father[N];
        for(int i = 0; i < N; ++i) {
            mother[i] = fparent[i];
            father[i] = sparent[i];
        }
        ///////
        int p = 2 + rand() % (N/2 - 2);
        int q;
        do {
            q = (N/2) + rand() % (N/2 - 2);
        } while(p >= (q-2));
        ///////
        for(int i = p; i <= q; ++i) {
            fchild[i] = mother[i];
            schild[i] = father[i];
        }
        ///////
        for(int i = p; i <= q; ++i) {
            for(int j = 0; j < N; ++j) {
                if(father[j] == fchild[i])
                    father[j] = -1;
                if(mother[j] == schild[i])
                    mother[j] = -1;
            }
        }
        ///////
        int m = 0, n = 0;
        for(int i = 0; i < p; ++i, ++m, ++n) {
            while(-1 == father[m]) {
                ++m;
            }
            fchild[i] = father[m];
            while(-1 == mother[n]) {
                ++n;
            }
            schild[i] = mother[n];
        }
        for(int i = q+1; i < N; ++i, ++m, ++n) {
            while(-1 == father[m]) {
                ++m;
            }
            fchild[i] = father[m];
            while(-1 == mother[n]) {
                ++n;
            }
            schild[i] = mother[n];
        }
    }
    void TSPGenetic(int N, const double **Points, int *Route) {
        // Wyznaczamy ilość osobników w populacji
        int poplSize = N;
        if(N > 100) {
            poplSize = 100;
        }
        // Tworzymy populacje początkową
        int **population = new int *[poplSize];
        for(int i = 0; i < poplSize; ++i) {
            population[i] = new int [N]; //każdy osobnik ma N genów
        }
        // wypełniamy populację tak, aby potem odpalić algorytm zachłanny od różnych punktów
        for(int i = 0; i < poplSize; ++i) {
            int c = i;
            for(int j = 0; j < N; ++j) {
                if(c == N)
                    c = 0;
                population[i][j] = c;
                c++;
            }
        }
        // Algorytm zachłanny stworzy osobniki różniące się od siebie
        for(int i = 0; i < poplSize; ++i) {
            TSPGreedy(N, Points, population[i]);
        }
        // Szukamy najlepszego osobnika
        double roads[poplSize];
        for(int i = 0; i < poplSize; ++i) {
            roads[i] = measureTheRoad(population[i], Points, N);
        }
        int winner = 0;
        for(int i = 0; i < poplSize; ++i) {
            if(roads[i] < roads[winner]) //Algorytm wyszukiwania minimum (najkrótszej z dróg)
                winner = i;
        }
        for(int i = 0; i < N; ++i) { //Zapisujemy najkrótszą drogę do Route, żeby potem jej nie stracić
            Route[i] = population[winner][i];
        }
        double bestSolution = roads[winner];
        //////////
        // Główna pętla iterująca
        for(int t = 0; t < 1000; ++t) {
            //--------
            // Funkcja odpowiadająca środowisku, które wyeliminuje osobniki nie przystosowane
            eviromentFunction(N, poplSize, Points, population, roads);
            //--------
            // Zliczamy ilość wyeliminowanych osobników
            int deadPer = 0;
            for(int i = 0; i < poplSize; ++i) {
                if(-1 == population[i][1])
                    deadPer++;
            }
            int livingPer = poplSize - deadPer;
            // Grupujemy rodziców i miejsca na powstanie dzieci do Przeprowadzenia krzyżowania
            int children[deadPer], parents[livingPer];
            int c = 0, p = 0;
            for(int i = 0; i < poplSize; ++i) {
                if(population[i][1] == -1) {
                    children[c] = i;
                    c++;
                }
                else {
                    parents[p] = i;
                    p++;
                }
            }
            //--------
            // Jeżeli liczba osobników martwych jest nieparzysta, zwiekszamy ją o 1
            // dlatego, że rozmnażając dwa osobniki tworzymy dwójke potomstwa
            // zatem ilość dzieci musi być parzysta
            if(0 != (deadPer & 1)) {
                ++deadPer;
            }
            // Losujemy dwoje różnych rodziców
            for(int i = 0; i < deadPer; i += 2) {
                int mother, father, j = i;
                mother = parents[rand()%livingPer];
                do {
                    father = parents[rand()%livingPer];
                } while(mother == father);
                // Wybieramy dwa miejsca na potomstwo
                int fchild = children[j];
                // Jeżeli musieliśmy zinkrementować ilość dzieci
                // to musimy "nadpisać" piersze dziecko ostatnim stworzonym
                // Bo inaczej przekroczyli byśmy indeks tablicy (j+1)
                if((j+1 == deadPer-1) && (0 != ((poplSize-livingPer) & 1)))
                    j = 0;
                int schild = children[j+1];
                // Krzyżujemy
                ModifiedOXCrossover(population[mother], population[father], population[fchild], population[schild], N);
            }
            //--------
            // Mutujemy małą część społeczeństwa
            for(int i = 0; i < poplSize/20; ++i) {
                mutatePerson(population[rand()%poplSize], N);
            }
            //--------
            // Mierzymy długości dróg (znowu) żeby sprawdzić czy polepszyliśmy rozwiązania
            for(int i = 0; i < poplSize; ++i) {
                roads[i] = measureTheRoad(population[i], Points, N);
            }
            int winner = 0;
            for(int i = 0; i < poplSize; ++i) {
                if(roads[i] < roads[winner])
                    winner = i;
            }
            //--------
            // Jeżeli polepszyliśmy nasze rozwiązanie (bliskie optimum)
            // to zapisujemy je w miejsce poprzedniego
            if(roads[winner] < bestSolution) {
                // Zapisujemy winner do Route
                for(int i = 0; i < N; ++i) {
                    Route[i] = population[winner][i];
                }
                double bestSolution = roads[winner];
                //cout << "The best solution is: " << bestSolution << endl;
            }
        }
        //////////
        for(int i = 0; i < poplSize; ++i) {
            delete [] population[i];
        }
        delete [] population;
        population = NULL;
    }
}


int main()
{
    using namespace arrays_functions;
    using namespace tsp_functions;

    srand(time(0));
    int size;
    cout << "Podaj ilość losowych punktów w przestrzeni" <<endl;
    cin >> size;

    int genetic[size];

    double **Points = new double *[size];
    for(int i = 0; i < size; i++) {
        Points[i] = new double [2];
    }

    fillTableRandomly(Points, size);
    //punkty wypełnione
    cout << "wyniki działania genetic: " << endl;
    TSPGenetic(size, (const double **)Points, genetic);
    cout << "długość drogi: " << measureTheRoad(genetic, (const double **)Points, size) << endl;


    // *********** czyszczenie pamięci ********
    for(int i = 0; i < size; i++) {
        delete [] Points[i];
    }
    delete [] Points;
    Points = NULL;

    /*int mother[] = {2, 3, 4, 1, 5, 8, 9, 7, 0, 6};
    int father[] = {3, 6, 1, 0, 8, 9, 4, 7, 5, 2};
    int fchild[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    int schild[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    int m = 0;
    OXCrossover(mother, father, fchild, schild, 10);
    for(int i = 0; i < 10; ++i) {
            cout<<fchild[i]<<">>";
        }
    cout<<endl;
    for(int i = 0; i < 10; ++i) {
            cout<<schild[i]<<">>";
        }
    cout<<endl;*/

    return 0;
}
