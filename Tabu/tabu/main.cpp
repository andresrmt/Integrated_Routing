#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <numeric>

#include <unistd.h>
#include <dirent.h>
//#include <limits.h>

using namespace std;

/* CONSTANTS */

#define EPSILON           0.00001
#define VIOLATION_TRIINEQ 0.1
#define VIOLATION_PART2   0.1
#define MAXTIME           3600.0
#define MINTENURE         5  //
#define MAXINFEASTENURE   40 // 40
#define MAXFEASTENURE     20 // 20

/*
 10 : MAXINFEASTENURE = 1, MAXFEASTENURE = 1
 12 : MAXINFEASTENURE = 1, MAXFEASTENURE = 10
 14 : MAXINFEASTENURE = 10, MAXFEASTENURE = 10
 16 : MAXINFEASTENURE = 20, MAXFEASTENURE = 10
 18 : MAXINFEASTENURE = 20, MAXFEASTENURE = 10
 20 : MAXINFEASTENURE = 10
 25 : OK
*/

const double BIGVALUE = 1e+10;

//COLORES
#define RESET       "\033[0m"
#define ROJO        "\033[31m"             /* Red */
#define VERDE       "\033[32m"             /* Green */
#define AMARILLO    "\033[93m"             /* Yellow */
#define AZUL        "\033[94m"             /* Blue */
#define MAGENTA     "\033[35m"             /* Magenta */
#define CYAN        "\033[36m"             /* Cyan */
#define BLANCO      "\033[37m"             /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

/* GENERAL FEATURES (COMMENT IT FOR TURNING OFF) */

#define USE_TABU_SEARCH
#define BETA_EQUALITY
//#define PUREBYB
#define ENABLE_CUTS
#define PART2_CUTS
#define PERCENTAGE_TRIINEQ_IN_FORM 20
#define PERCENTAGE_TRIINEQ_IN_CUTS 70
//#define HISTOGRAM
//#define ADD_ARTIFICIAL_NODES

#if PERCENTAGE_TRIINEQ_IN_FORM < 100
#define TRIINEQ_LAZYS
#endif

#ifdef HISTOGRAM
int hist[10];
#endif



struct triangular_ineq {
    int i, j, r;
    int sign_ij, sign_ir, sign_jr;
    float dist;
} *t_ineq;
int t_ineq_count, t_ineq_count_in_formulation, t_ineq_count_during_cuts; /* triangular combinations: "all", "inside formulation", "for cuts" */

/*
  **************************  voids  **************************
*/

// set_color - change font color
void set_color(string c)
{
    cout << c;
}

// bye - finalize execution and write a string on screen
void reportar(string s)
{
    set_color(ROJO);
    cout << s << endl;
    set_color(BLANCO);
    exit(1);
}

// auxiliary function to sort triangular inequalities
int cmpfunc_t_ineq(const void *a, const void *b)
{
    struct triangular_ineq *ta = (struct triangular_ineq*)a;
    struct triangular_ineq *tb = (struct triangular_ineq*)b;
    if (ta->dist < tb->dist) return -1;
    return 1;
}

// generates numbers between 0 and N-1 with uniform distribution
int Random(int N)
{
    return rand() % N;
}

// Read service times
vector<double> Lectura_Servicios(int n, string back)
{
    vector<double> T_servicio(n);
    ostringstream ARCHIVO;
    ARCHIVO << back + "data/Service_" << n << ".txt";
    ifstream f(ARCHIVO.str().c_str());
    for(unsigned int i=0; i < T_servicio.size(); i++)
        f >> T_servicio[i];
    f.close();
    return T_servicio;
}

// Read pollster walking times
vector<vector<double> > Lectura_Tiempos(int n, string back)
{
    vector< vector<double> > D_encuestador(n);
    for(unsigned int i=0; i < D_encuestador.size(); i++)
        D_encuestador[i].resize(n);

    ostringstream ARCHIVO;
    ARCHIVO << back + "data/Pollster_" << n << ".txt";

    ifstream f(ARCHIVO.str().c_str());
    for(unsigned int i=0; i < D_encuestador.size(); i++)
        for(unsigned int j=0; j < D_encuestador[i].size(); j++)
            f >> D_encuestador[i][j];

    f.close();
    return D_encuestador;
}



/*
  *******************************  Tabu algorithm  *******************************

* tabusearch - use a tabu search heuristic
* note: "best" must have an initial (not necessarily feasible) solution!
*/

void tabusearch(bool file, int n, int k,  vector<double> w,  vector< vector<double> > d, double WL, double WU, vector<int> best, double& best_cost, const string& filePath = "")
{
    double start_t; /* initial timestamp */

    int ndivk = n / k;

    /* ask for memory */
    vector<int> current (n);
    vector<int> sizecluster (k);            /* elements of each cluster */
    vector<long double> wcluster (k,0);     /* weight of each cluster */
    vector< vector<int> > currentv;         /* list of lists with vertices of each cluster */

    currentv.resize(k);
    for (int j = 0; j < k; j++)
        currentv[j].resize(ndivk + 1);

    vector< vector<int> > tenure;           /* life of a (vertex,cluster) in the tabu list */
    tenure.resize(n);
    for (int i = 0; i < n; i++)
    {
        tenure[i].resize(k,0);
    }

    /* set current solution */
    for (int i = 0; i < n; i++)
    {
        current[i] = best[i];
    }


    //COMPUTE THE WEIGHT OF EACH CLIQUE
    for (int i = 0; i < n; i++)
    {
        int j = current[i];
        wcluster[j] += w[i];
    }

    /* compute initial cost  ("long double" to diminish numerical errors) */
    double current_cost = 0.0;
    for (int i = 0; i < n - 1; i++)
        for (int j = i + 1; j < n; j++)
            if (current[i] == current[j])
                current_cost += d[i][j];

    for (int j = 0; j < k; j++)
    {
        long double wj = wcluster[j];
        if (wj < WL) current_cost += BIGVALUE;
        if (wj > WU) current_cost += BIGVALUE;
    }

    set_color(AZUL);
    cout << "Initial solution = " << current_cost << endl;
    best_cost = current_cost;

    long int maxiter = max((int)exp(0.06571*(double)n - 0.052978),2000);
    cout << "Number of iterations estimated = " << maxiter << endl;

    set_color(ROJO);
    /* make iterations until a limit is reached */
    int iter = 0;
    start_t = clock();
    while (iter < maxiter)
    {

        /* create solution from current one */
        for (int j = 0; j < k; j++)
        {
            sizecluster[j] = 0;
            currentv[j].clear();
        }

        for (int i = 0; i < n; i++)
        {
            int j = current[i];
            sizecluster[j]++;
            currentv[j].push_back(i);
        }

        /* explore solutions where a vertex of cluster j1 is sent to a cluster j2 satisfying the weight */
        long double min_cost = BIGVALUE*BIGVALUE;
        int min_v1 = -1;
        int min_v2 = -1;
        int min_j1 = -1;
        int min_j2 = -1;
        long double min_wj1 = -1.0;
        long double min_wj2 = -1.0;
        int min_movement = -1;

        for (int j1 = 0; j1 < k; j1++)
        {
            for (int p1 = 0; p1 < sizecluster[j1]; p1++)
            {
                    int v1 = currentv[j1][p1];
                    for (int j2 = 0; j2 < k; j2++)
                    {
                      if (j1!=j2)
                        {
                            if (tenure[v1][j2] == 0)
                            { /* is (v1,j2) tabu ? */
                                /* compute new cost */
                                long double new_cost = current_cost;
                                for (int pp1 = 0; pp1 < sizecluster[j1]; pp1++)
                                {
                                    if (pp1 != p1)
                                        new_cost -= d[v1][currentv[j1][pp1]];
                                }
                                for (int pp2 = 0; pp2 < sizecluster[j2]; pp2++)
                                {
                                    new_cost += d[v1][currentv[j2][pp2]];
                                }
                                /* compute new weights */
                                long double wj1 = wcluster[j1];
                                long double wj2 = wcluster[j2];
                                long double new_wj1 = wj1 - w[v1];
                                long double new_wj2 = wj2 + w[v1];
                                /* we also penalize solutions where weights do not hold */
                                if (wj1 < WL) new_cost -= BIGVALUE;
                                if (wj1 > WU) new_cost -= BIGVALUE;
                                if (wj2 < WL) new_cost -= BIGVALUE;
                                if (wj2 > WU) new_cost -= BIGVALUE;
                                if (new_wj1 < WL) new_cost += BIGVALUE;
                                if (new_wj1 > WU) new_cost += BIGVALUE;
                                if (new_wj2 < WL) new_cost += BIGVALUE;
                                if (new_wj2 > WU) new_cost += BIGVALUE;
                                if (new_cost < min_cost)
                                {
                                    /* if the cost is better, update! */
                                    min_cost = new_cost;
                                    min_v1 = v1;
                                    min_j1 = j1;
                                    min_j2 = j2;
                                    min_wj1 = new_wj1;
                                    min_wj2 = new_wj2;
                                    min_movement = 1;
                                }
                            }
                        }
                    }

            }
        }

        /* explore solutions where two vertices of clusters of the same size are interchanged */
        for (int j1 = 0; j1 < k - 1; j1++)
        {
            for (int j2 = j1 + 1; j2 < k; j2++)
            {
                    for (int p1 = 0; p1 < sizecluster[j1]; p1++)
                    {
                        int v1 = currentv[j1][p1];
                        if (tenure[v1][j2] == 0)
                        { /* is (v1,j2) tabu ? */
                            for (int p2 = 0; p2 < sizecluster[j2]; p2++)
                            {
                                int v2 = currentv[j2][p2];
                                if (tenure[v2][j1] == 0)
                                { /* is (v2,j1) tabu ? */
                                    /* compute new cost */
                                    long double new_cost = current_cost;
                                    for (int pp1 = 0; pp1 < sizecluster[j1]; pp1++)
                                    {
                                        if (pp1 != p1)
                                            new_cost += d[v2][currentv[j1][pp1]] - d[v1][currentv[j1][pp1]];
                                    }
                                    for (int pp2 = 0; pp2 < sizecluster[j2]; pp2++)
                                    {
                                        if (pp2 != p2)
                                            new_cost += d[v1][currentv[j2][pp2]] - d[v2][currentv[j2][pp2]];
                                    }
                                    /* compute new weights */
                                    long double wj1 = wcluster[j1];
                                    long double wj2 = wcluster[j2];
                                    long double new_wj1 = wj1 - w[v1] + w[v2];
                                    long double new_wj2 = wj2 + w[v1] - w[v2];
                                    /* we also penalize solutions where weights do not hold */
                                    if (wj1 < WL) new_cost -= BIGVALUE;
                                    if (wj1 > WU) new_cost -= BIGVALUE;
                                    if (wj2 < WL) new_cost -= BIGVALUE;
                                    if (wj2 > WU) new_cost -= BIGVALUE;
                                    if (new_wj1 < WL) new_cost += BIGVALUE;
                                    if (new_wj1 > WU) new_cost += BIGVALUE;
                                    if (new_wj2 < WL) new_cost += BIGVALUE;
                                    if (new_wj2 > WU) new_cost += BIGVALUE;
                                    if (new_cost < min_cost)
                                    {
                                        /* if the cost is better, update! */
                                        min_cost = new_cost;
                                        min_v1 = v1;
                                        min_v2 = v2;
                                        min_j1 = j1;
                                        min_j2 = j2;
                                        min_wj1 = new_wj1;
                                        min_wj2 = new_wj2;
                                        min_movement = 2;
                                    }
                                }
                            }
                        }
                    }
            }
        }

        /* update current solution */
        if (min_movement == -1)
            reportar("Tenure too high!");

        switch (min_movement)
        {
            case 1:
            {
                //cout << "Send vertex " << min_v1 << " to cluster " << min_j2 + 1 << ", new cost = " << min_cost << endl;
                current[min_v1] = min_j2;
                current_cost = min_cost;
                wcluster[min_j1] = min_wj1;
                wcluster[min_j2] = min_wj2;
                break;
            }
            case 2:
            {
                //cout << "Exchange vertices " << min_v1 << " and " << min_v2 << " from clusters " << min_j1 + 1 << " and " << min_j2 + 1 << ", new cost = " << min_cost << endl;
                current[min_v1] = min_j2;
                current[min_v2] = min_j1;
                current_cost = min_cost;
                wcluster[min_j1] = min_wj1;
                wcluster[min_j2] = min_wj2;
                break;
            }
            default:
            {
                reportar("Tenure too high!");
            }
        }

        /* update best solution if necessary (and if it is better in at least 0.001%) */
        if (min_cost < best_cost)
        {
            if ( (best_cost >= BIGVALUE) and (min_cost < BIGVALUE))
                cout << AMARILLO<<"Feasibility reached :)" <<VERDE<< endl;

            cout << "New best solution with cost = " << min_cost << " at iteration " << iter << "." << endl;
            best_cost = min_cost;
            for (int i = 0; i < n; i++)
                best[i] = current[i];
        }

        /* next iteration, mark (v1,j1) as tabu and decrease life of elements in tabu list */
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < k; j++)
            {
                if (tenure[i][j] > 0)
                    tenure[i][j]--;
            }
        }
        if (min_cost < BIGVALUE)
        {
            /* when solutions are feasible --> use a low tenure */
            tenure[min_v1][min_j1] = Random(MAXFEASTENURE - MINTENURE + 1) + MINTENURE;
        }
        else
        {
            /* when solutions are infeasible --> use a high tenure */
            tenure[min_v1][min_j1] = Random(MAXINFEASTENURE - MINTENURE + 1) + MINTENURE;
        }
        iter++;

        /* show time elapsed */
        if (iter % 1000 == 0)
        {
            cout << " REPORT: in iteration " << iter << ":  best = " << best_cost << ",  time = " << (clock()- start_t)/CLOCKS_PER_SEC << endl;
        }
    }
    cout << "Time elapsed in optimization: " << (clock()- start_t)/CLOCKS_PER_SEC << " sec." << endl;

    double w_cluster;
    int nc;

    if(file==false)
    {
        cout << endl << endl << ROJO << "Best solution with cost " << best_cost << ":" << VERDE << endl;
        for (int c = 0; c < k; c++)
        {
            w_cluster = 0;
            nc = 0;
            cout << "  Cluster " << c+1 << ":" << endl;;
            for (int p = 0; p < n; p++)
            {
                if(best[p] == c)
                {
                    cout << p+1 << "  ";
                    w_cluster += w[p];
                    nc++;
                }

            }
            cout << endl << "  nodes in clique= " << nc << " with weight = " << w_cluster << endl;
        }
    }
    else
    {
        string out_name = filePath + "Partition_" + to_string(n) + "_" + to_string(k) + ".txt";
        ofstream f(out_name);
        f << endl << endl << "Best solution with cost " << best_cost << ":" << endl;
        for (int c = 0; c < k; c++)
        {
            w_cluster = 0;
            nc = 0;
            f << "  Cluster " << c+1 << ":" << endl;;
            for (int p = 0; p < n; p++)
            {
                if(best[p]==c)
                {
                    f << p+1 << "  ";
                    w_cluster += w[p];
                    nc++;
                }

            }
            f << endl << "  nodes in clique= " << nc << " with weight = " << w_cluster << endl;
        }
        f.close();
    }
    for (int i = 0; i < n; i++)
        tenure[i].clear();
    tenure.clear();

    for (int j = 0; j < k; j++)
     currentv[j].clear();

    currentv.clear();
    sizecluster.clear();
    current.clear();
}






/*
  **************************  main - Funcion principal  **************************
*/

//int main(int argc, char **argv)
int main()
{
    /* GLOBAL VARIABLES */

    int n=100, k=6;              /* number of vertices and clusters */
    vector< vector<double> > d; /* distance matrix */
    vector<double> ws;          /* weights of vertices */
    double WL, WU;              /* weight bounds */
    vector<int> best;           /* best assignment so far */
    double best_cost;           /* cost of the best assignment */
    set_color(ROJO);
    cout << "KPART - Heuristic of weighted balanced k-clique partitioning problem." << endl << endl;
    
    // Show current directory
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    cout << "Current directory: " << cwd << endl;
    
    // We might have to move back some directories just to read the data
    string back = "/../../../../..";
    string parentDir = string(cwd) + back;
    cout << "Looking at directory: " << parentDir << endl;
    // Open the parent directory
    DIR *dir = opendir(parentDir.c_str());
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (string(entry->d_name) == "data") {
            cout << entry->d_name << " is here." << endl << endl;
            break;
        }
    }
    closedir(dir);
    

    /* read weights and distance matrix */
    ws = Lectura_Servicios(n, parentDir + "/");
    d  = Lectura_Tiempos(n, parentDir + "/");

    /* Bounds over points and partitions */
    if (n <= 3 || n >= 10000 || k <= 1 || k >= 5000){
        reportar("Out of bounds!");
    }

    /* Compute maximum */
    float max_t    = *max_element(ws.begin(), ws.end());
    float max_dist = accumulate(d.begin(), d.end(), d[0][0],
                          [](double max, const vector<double> &v)
                             {
                                 return std::max(max, *max_element(v.begin(), v.end()));
                             } );


    set_color(AZUL);
    cout << "We have n = " << n << " and k = " << k << "." << endl
         << "Maximum distance found = " << max_dist << endl
         << "Maximum time service = " << max_t << endl;


    /* compute WL and WU */
    float mu = accumulate(ws.begin(), ws.end(), 0)/(float)n;
    
    float sigma = inner_product(ws.begin(), ws.end(), ws.begin(), 0.0)/(float)n - mu * mu;
    sigma *= (float)n/(float)(n-1);
    sigma  = sqrt(sigma);
    
    WL = ((float)n / (float)k) * mu - sigma;
    WU = ((float)n / (float)k) * mu + sigma;


    cout << "Weights: mu = " << mu << ", sigma = " << sigma << ", WL = " << WL << ", WU = " << WU << "." << endl;

    // solucion inicial trivial
    best.resize(n);
    int cc = 0;
    for (int i = 0; i < n; i++){
        best[i] = cc++;
        if (cc == k) cc = 0;
    }

    
    /* Tabu algorithm */
    tabusearch(true, n, k, ws, d, WL, WU, best, best_cost,   parentDir + "/");
    
    
    
    
    
    
    
    
    
    
    
    /* // insert code here...
    for(unsigned int i=0; i < n; i++)
        cout << ws[i] << " ";
    cout << endl;
    
    // *max_element(d.begin(), d.end());
    cout << mu << " " << sigma;
    cout << endl << endl; */
    
    
    

    /* free memory */
    for (int i = 0; i < n; i++)
        d[i].clear();
    d.clear();
    ws.clear();
    
    return 0;
}


/*
namespace fs = std::__fs::filesystem;

string path = "/";
for (const auto & entry : fs::directory_iterator(path))
 cout << entry.path() << std::endl;
*/
