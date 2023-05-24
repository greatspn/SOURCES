

	inline double uniform(double min, double max, mt19937_64& generator){

		if(min<0 || max<min){
			throw Exception("The parameters of the Uniform distribution are wrong. The second must be greater than the first and they must be greater than 0.\n\n");
		}
		uniform_real_distribution<double> unf_dis(min, max);
		double value = unf_dis(generator);
		return value;
	}

	inline double dirac_delta(double value){

		if(value<0){
			throw Exception("The value must be greater than 0.\n\n");
		}

		return value;

	}

	inline double binomial(double max, double min, mt19937_64& generator){

		if(min<0 || max<=min){
			throw Exception("The parameters of the Binomial distribution are wrong. They must be different and greater than 0.\n\n");
		}

		binomial_distribution<> bin_dis(min, max);
		double value = bin_dis(generator);
		return value;
	}

	inline double gamma(double alpha, double beta, mt19937_64& generator){
		std::gamma_distribution<double> gamma_dis(alpha, beta);
		double value = gamma_dis(generator);
		return value;
	}

	//aspetta ma questa è una unary function!
	inline double exponential(double lambda, double nan, mt19937_64& generator){

		exponential_distribution<double> exp_dis(lambda);
		double value = exp_dis(generator);
		return value;
	}

	//anche questa in realtà
	inline double poisson(double mean, mt19937_64& generator){

		if(mean<0){
			throw Exception("The parameter of the Poisson distribution is wrong. It must be greater than 0.\n\n");
		}

		poisson_distribution<> pois_dis(mean);
		int value = pois_dis(generator);
		return value;
	}

	inline double normal(double min, double max, mt19937_64& generator){
		normal_distribution<double> norm_dis(min, max);
		double value = norm_dis(generator);
		return value;

	}

