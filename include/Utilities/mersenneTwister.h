#ifndef MERSENNETWISTER_H
#define MERSENNETWISTER_H

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include <time.h>

class MersenneTwister
{
public:
    //MersenneTwister():gen(time (0)),dist(0,1),die(gen,dist)
    MersenneTwister():gen(),dist(0,1),die(gen,dist)
    {
    }

    double getRandomNormalizedDouble()
    {
        return die();
    }

private:
    boost::mt19937 gen;
    boost::uniform_real<> dist;
    boost::variate_generator<boost::mt19937&, boost::uniform_real<> > die;    
};

#endif //MERSENNETWISTER_H
