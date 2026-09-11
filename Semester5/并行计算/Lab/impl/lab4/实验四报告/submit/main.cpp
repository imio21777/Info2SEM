/* 
 * logDataVSPrior is a function to calculate 
 * the accumulation from ABS of two groups of complex data
 * *************************************************************************/

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <complex>
#include <chrono>

using namespace std;

typedef complex<double> Complex;
typedef chrono::high_resolution_clock Clock;

const int m=1638400;	// DO NOT CHANGE!!
const int K=100000;	// DO NOT CHANGE!!

double logDataVSPrior(const Complex* dat, const Complex* pri, const double* ctf, const double* sigRcp, const int num, const double disturb0);

int main ( int argc, char *argv[] )
{ 
    Complex *dat = new Complex[m];
    Complex *pri = new Complex[m];
    double *ctf = new double[m];
    double *sigRcp = new double[m];
    double *disturb = new double[K];
    double dat0, dat1, pri0, pri1, ctf0, sigRcp0;

    /***************************
     * Read data from input.dat
     * *************************/
    ifstream fin;

    fin.open("input.dat");
    if(!fin.is_open())
    {
        cout << "Error opening file input.dat" << endl;
        exit(1);
    }
    int i=0;
    while( !fin.eof() ) 
    {
        fin >> dat0 >> dat1 >> pri0 >> pri1 >> ctf0 >> sigRcp0;
        dat[i] = Complex (dat0, dat1);
        pri[i] = Complex (pri0, pri1);
        ctf[i] = ctf0;
        sigRcp[i] = sigRcp0;
        i++;
        if(i == m) break;
    }
    fin.close();

    fin.open("K.dat");
    if(!fin.is_open())
    {
	cout << "Error opening file K.dat" << endl;
	exit(1);
    }
    i=0;
    while( !fin.eof() )
    {
	fin >> disturb[i];
	i++;
	if(i == K) break;
    }
    fin.close();

    /***************************
     * main computation is here
     * ************************/
    auto startTime = Clock::now(); 

    ofstream fout;
    fout.open("result.dat");
    if(!fout.is_open())
    {
         cout << "Error opening file for result" << endl;
         exit(1);
    }

    for(unsigned int t = 0; t < K; t++)
    {
        double result = logDataVSPrior(dat, pri, ctf, sigRcp, m, disturb[t]);
        fout << t+1 << ": " << result << endl;
    }
    fout.close();

    auto endTime = Clock::now(); 

    auto compTime = chrono::duration_cast<chrono::microseconds>(endTime - startTime);
    cout << "Computing time=" << compTime.count() << " microseconds" << endl;

    delete[] dat;
    delete[] pri;

    delete[] ctf;
    delete[] sigRcp;
    delete[] disturb;
    return EXIT_SUCCESS;
}

double logDataVSPrior(const Complex* dat, const Complex* pri, const double* ctf, const double* sigRcp, const int num, const double disturb0)
{
    double result = 0.0;

    for (int i = 0; i < num; i++)
    {

          result += ( norm( dat[i] - ctf[i] * pri[i] ) * sigRcp[i] );

    }
    return result*disturb0;
}
