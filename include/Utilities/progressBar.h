#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <string>
#include <sstream>

class ProgressBar
{
public:
    ProgressBar( const unsigned int& times = 0, const std::string& message =""):m_curIteration(0),m_numIterations(times),m_message(message),m_curPercentage(0.01)
    {   
    }
    
    void operator++()
    {
		if( !m_curIteration) std::cout << "\r" << m_message << "..." << "0%" << std::flush;
        ++m_curIteration;
        if ( double(m_curIteration)/m_numIterations > m_curPercentage)
        {
            std::cout << "\r" << m_message << "..." << (unsigned int)(m_curPercentage*100) << "%" << std::flush;
            m_curPercentage += 0.01;
        } 
        
        if ( m_curIteration == m_numIterations)
        {
           std::cout << "\r" << m_message << "...done!\n" << std::flush;
        }
    }
    
    void reset(const unsigned int& times, const std::string& message ="")
    {
        m_curIteration = 0;
        m_numIterations = times;
        m_message = message;
        m_curPercentage = 0.01;
    }
    
private:
    unsigned int m_curIteration;
    unsigned int m_numIterations;
    std::string m_message;
    double m_curPercentage;
};


class ProgressStream
{
public:
    ProgressStream( const unsigned int& times):m_curIteration(0),m_numIterations(times),m_curPercentage(0.01)
    {
    }
    
    void operator++()
    {
        if( m_curIteration)
        {
            ++m_curIteration;
            if ( double(m_curIteration)/m_numIterations > m_curPercentage)
            {
                std::cout << "\r" << m_label.str() << "..." << (unsigned int)(m_curPercentage*100) << "%" << std::flush;
                m_curPercentage += 0.01;
            } 
            
            if ( m_curIteration == m_numIterations)
            {
               std::cout << "\r" << m_label.str() << "...done!\n" << std::flush;
            }
        }
        else
        {
            std::cout << "\r" << m_label.str() << "..." << "0%" << std::flush;
            ++m_curIteration;
        }
    }

    std::stringstream& label()
    {
        return m_label;
    }

    void reset(const unsigned int& times)
    {
        m_curIteration = 0;
        m_numIterations = times;
        m_curPercentage = 0.01;
    }
    
private:
    unsigned int m_curIteration;
    unsigned int m_numIterations;
    double m_curPercentage;
    std::stringstream m_label;
};

#endif //PROGRESSBAR_H
