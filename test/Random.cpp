#include "TestAPI.h"
#include <mgpcl/Random.h>
#include <fstream>
#include <ctime>
#include <cstdlib>

Declare Test("random"), Priority(4.0);

TEST
{
    volatile StackIntegrityChecker sic;

    m::Random<> r;
    int counts[10] = { 0 };

    for(int i = 0; i < 100000; i++) {
        int result = r.nextInt(10);
        testAssert(result >= 0 && result < 10, "nextInt() did bs!");
        counts[result]++;
    }

    for(int i = 0; i < 10; i++) {
        double prob = static_cast<double>(counts[i]) / 100000.0;
        std::cout << "[i]\t>> p(" << i << ") = " << std::dec << prob << std::endl;
        testAssert(prob >= 0.095 && prob <= 0.105, "probability is weird!");
    }

    return true;
}

TEST
{
    volatile StackIntegrityChecker sic;
    m::Random<> r;

    for(int i = 0; i < 100; i++) {
        //Simply check bounds here...
        double d = r.nextDouble(16.0, 32.0);
        testAssert(d >= 16.0 && d < 32.0, "nextDouble() did bs");
    }

    return true;
}

TEST
{
    volatile StackIntegrityChecker sic;
    m::Random<> r;

    uint8_t buf[68];
    r.nextBytes(buf, 64);
    r.nextBytes(buf, 68);

    std::cout << "[i]\t(" << std::hex << static_cast<int>(buf[16]) << ')' << std::endl; //Print a random byte so that the compiler doesn't optimize the test out...
    return true;
}

static void g_histogram(const double *samples, int nSamples, int *counts, int nCounts)
{
    const double step = 1.0 / static_cast<double>(nCounts);
    double pos = 0.0;
    m::mem::zero(counts, nCounts * sizeof(int));

    for(int i = 0; i < nCounts; i++) {
        double pos2 = pos + step;

        for(int j = 0; j < nSamples; j++) {
            if(samples[j] >= pos && samples[j] < pos2)
                counts[i]++;
        }

        pos = pos2;
    }
}

#define N_SAMPLES 1000000
#define HISTO_SIZE 20

DISABLED_TEST
{
    volatile StackIntegrityChecker sic;
    double *samples = new double[N_SAMPLES];
    int *histo = new int[HISTO_SIZE];

    std::ofstream out("random.csv");

    //Test with Xoroshiro
    m::Random<> random;
    for(int i = 0; i < N_SAMPLES; i++)
        samples[i] = random.nextDouble(1.0);

    g_histogram(samples, N_SAMPLES, histo, HISTO_SIZE);
    out << "PRNG:,Xoroshiro" << std::endl;

    for(int i = 0; i < HISTO_SIZE; i++)
        out << i << ',' << histo[i] << std::endl;

    //Test with stdlib's rand()
    srand(static_cast<unsigned>(time(nullptr)));
    for(int i = 0; i < N_SAMPLES; i++)
        samples[i] = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);

    g_histogram(samples, N_SAMPLES, histo, HISTO_SIZE);
    out << "PRNG:,rand()" << std::endl;

    for(int i = 0; i < HISTO_SIZE; i++)
        out << i << ',' << histo[i] << std::endl;

    out.close();
    delete[] samples;
    delete[] histo;
    return true;
}
