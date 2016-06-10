#include <map>
#include <iostream>

// Numeric must support +, <, and << operators.

template <typename Numeric>
class Histogram;

template <typename Numeric>
std::ostream& operator<<(std::ostream & stream, Histogram<Numeric> & hist);

template <typename Numeric>
class Histogram
{
	private:
	std::map<Numeric, unsigned> m_histogram;
	const Numeric m_start_from;
	const Numeric m_bucket_width;
	Numeric m_max_observation;
	
	public:
	Histogram(const Numeric & start_from, const Numeric & bucket_width) : 
	m_start_from(start_from), m_bucket_width(bucket_width), m_max_observation(start_from) {}
	
	void add_observation(const Numeric & observation)
	{
		// Find the appropriate bucket to insert the observation
		Numeric bucket_lower_bound = m_start_from;
		Numeric bucket_upper_bound = m_start_from + m_bucket_width;
		while (!(observation < bucket_upper_bound))
		{
			bucket_lower_bound = bucket_upper_bound;
			bucket_upper_bound = bucket_lower_bound + m_bucket_width;
		}
		
		// Add the observation to the bucket
		typename std::map<Numeric, unsigned>::iterator bucket = m_histogram.find(bucket_lower_bound);
		if (bucket == m_histogram.end())
		{
			m_histogram.insert(std::make_pair(bucket_lower_bound, 1));
		}
		else
		{
			bucket->second += 1;
		}
		
		// Update max observation
		if (m_max_observation < observation)
		{
			m_max_observation = observation;
		}
	}
	
	friend std::ostream& operator<< <>(std::ostream & stream, Histogram<Numeric> & hist);
};

template <typename Numeric>
std::ostream& operator<<(std::ostream & stream, Histogram<Numeric> & hist)
{
	typename std::map<Numeric, unsigned>::iterator i;
	for (i = hist.m_histogram.begin(); i != hist.m_histogram.end(); ++i)
	{
		stream << i->first << " secs <= obs < " << (i->first + hist.m_bucket_width) << " secs";
		stream << ", count: " << i->second << std::endl;
	}
	
	stream << "Max Observation: " << hist.m_max_observation << " secs" << std::endl;
	return stream;
}

