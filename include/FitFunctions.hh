// Adapted from https://root-forum.cern.ch/t/crystalball-fitting-function/26973
// and https://en.wikipedia.org/wiki/Crystal_Ball_function

#ifndef _FitFunctions_hh
#define _FitFunctions_hh

// Alpha source particulars
const int FF_num_alpha_peaks = 4;
// Nuclides used here are 148Gd, 239Pu, 241Am, 244Cm. Energies listed in order of energy (keV).
// Energies taken from NNDC in 2022 --> centroids using weighted average with intensities
const float FF_alpha_peak_energy[FF_num_alpha_peaks] = {
	3182.69,	// % 148Gd
	5134.70,	// % 239Pu
	5478.57,	// % 241Am
	5795.00 	// % 244Cm
};
const float FF_alpha_peak_energy_error[FF_num_alpha_peaks] = {
	0.024,	// 100    % 148Gd
	0.5,	//  70.77 % 239Pu
	0.14,	//  84.8  % 241Am
	1 	//  76.90 % 244Cm
};


//const int gAlphaPeakWidthEstimate = 30; 	// Rough estimate of peak width in channels (so you don't select local maxima) - best if even!
//const int gAlphaPeakChannelCheckLimit = 4;	// How far to look either side of a given peak to check it's legit (in channels)

// FITTING PARAMETERS
//const double gPeakHeightThresholdFraction = 0.08;	// The smallest fraction that is allowed as a peak (between 0 and 1)
//const double gPeakHeightDipFraction = 0.6;			// Ensure that the peak has dipped to this fraction of its height before being ready to record another peak --> this might be awkward if you have multiplets...
//const int gPeakChannelLowerThreshold = 360;			// Set to 0 if you want to sample peaks from the whole range of the histograms
//const int gPeakChannelUpperThreshold = 700;			// Set to 4096 if you want to sample peaks from the whole range of the histograms

// CALIBRATION


// Enum for different fit shapes
enum fit_shape{
	gaussian,
	crystalball
};

///////////////////////////////////////////////////////////////////////////////
//                         FIT SHAPE 0: GAUSSIAN FIT                         //
///////////////////////////////////////////////////////////////////////////////
inline double AlphaGaussian( double *x, double *p ){
	
	// p[0]   sigma
	// p[1]   amp
	// p[2]   mean

	//static float sqrt2pi = TMath::Sqrt( 2.0 * TMath::Pi() );
	static float sqrt2 = TMath::Sqrt( 2.0 );

	double arg = ( x[0] - p[2] ) / ( sqrt2 * p[0] );
	double gaus = p[1] * TMath::Exp( -1.0*TMath::Power( arg, 2.0 ) );

	return gaus;
	
}
///////////////////////////////////////////////////////////////////////////////
inline double AlphaGaussianBG( double *x, double *p ){

	// p[0]   gauss constant
	// p[1]   gauss mean
	// p[2]   gauss width
	// p[3]   gauss bg
	
	return AlphaGaussian(x,&p[1]) + p[0];
}

///////////////////////////////////////////////////////////////////////////////
inline double MultiAlphaGaussianBG( double *x, double *p ) {

	// par[0]   background constant
	// par[1]   gauss width
	// par[2,4,6,...] gauss constant
	// par[3,5,7,...] gauss mean

	double bg = p[0];
	double gaus_par[3];

	double sum = 0;
	for( int i = 0; i < FF_num_alpha_peaks; i++ ){
		
		gaus_par[0] = p[1];
		gaus_par[1] = p[2*i+2];
		gaus_par[2] = p[2*i+3];
		sum += AlphaGaussian( x, gaus_par );
		
	}

	return sum + bg;

}

///////////////////////////////////////////////////////////////////////////////
//                         FIT SHAPE 1: CRYSTAL BALL                         //
///////////////////////////////////////////////////////////////////////////////
inline double CrystalBallFunction( double x, double sigma, double alpha, double n, double amp, double mean ) {

	// Check sigma, n, alpha, and amp have good values
	if ( sigma <= 0.0 || alpha <= 0.0 || n <= 0.0 || amp <= 0.0){
		return 0.0;
	}
	
	// Define z and absalpha
	double z = (x - mean)/sigma;
	double abs_alpha = TMath::Abs( alpha );
	
	// Define A,B,C,D,N
	double A = TMath::Power( n/abs_alpha, n)*TMath::Exp( -0.5*TMath::Power( abs_alpha, 2 ) );
	double B = ( n/abs_alpha ) - abs_alpha;
	
	// Wikipedia defines C & D, but they're not needed here!
	//double C = ( n/( abs_alpha*( n - 1 ) ) )*TMath::Exp( -0.5*TMath::Power( abs_alpha, 2 ) );
	//double D = TMath::Sqrt( TMath::Pi()/2 )*( 1 + TMath::Erf( abs_alpha/TMath::Sqrt(2) ) );
	
	// Define function
	if ( z > -abs_alpha ){
		return amp*TMath::Exp( -0.5*z*z );
	}
	else {
		return amp*A*TMath::Power( B - z, -n );
	}
	
}

///////////////////////////////////////////////////////////////////////////////
inline double CrystalBallFunction( double *x, double *p) {

	// p[0] sigma
	// p[1] alpha
	// p[2] n
	// p[3] amp
	// p[4] mean
	
	return CrystalBallFunction( x[0], p[0], p[1], p[2], p[3], p[4] );
}

///////////////////////////////////////////////////////////////////////////////
inline double CrystalBallFunctionBG( double *x, double *p) {
	return CrystalBallFunction( x[0], p[1], p[2], p[3], p[4], p[5] ) + p[0];
}

///////////////////////////////////////////////////////////////////////////////
inline double MultiCrystalBallFunctionBG( double *x, double *p ){
	
	double bg = p[0];
	double cb_par[5];
	
	double sum = 0;
	for ( int i = 0; i < FF_num_alpha_peaks; ++i ){
	
		cb_par[0] = p[1];
		cb_par[1] = p[2];
		cb_par[2] = p[3];
		cb_par[3] = p[2*i+4];
		cb_par[4] = p[2*i+5];		
		sum += CrystalBallFunction( x, cb_par);
		
	}
	return sum + bg;
	
}



#endif
