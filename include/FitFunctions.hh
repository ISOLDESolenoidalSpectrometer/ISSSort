// Adapted from https://root-forum.cern.ch/t/crystalball-fitting-function/26973
// and https://en.wikipedia.org/wiki/Crystal_Ball_function

#ifndef _FitFunctions_hh
#define _FitFunctions_hh

// Alpha source particulars
const int gNumberOfAlphaPeaks = 4;
const int gAlphaPeakWidthEstimate = 30; 	// Rough estimate of peak width in channels (so you don't select local maxima) - best if even!
const int gAlphaPeakChannelCheckLimit = 4;	// How far to look either side of a given peak to check it's legit (in channels)

// FITTING PARAMETERS
const double gPeakHeightThresholdFraction = 0.15;	// The smallest fraction that is allowed as a peak (between 0 and 1)
const double gPeakHeightDipFraction = 0.6;			// Ensure that the peak has dipped to this fraction of its height before being ready to record another peak --> this might be awkward if you have multiplets...
const int gPeakChannelLowerThreshold = 320;			// Set to 0 if you want to sample peaks from the whole range of the histograms
const int gPeakChannelUpperThreshold = 700;			// Set to 4096 if you want to sample peaks from the whole range of the histograms

// CALIBRATION
// Nuclides used here are 148Gd, 239Pu, 241Am, 244Cm. Energies listed in order of energy (keV).
// Energies taken from NNDC in 2022 --> centroids using weighted average with intensities
const float gAlphaParticleEnergy[gNumberOfAlphaPeaks] = {
	3182.69,	// % 148Gd
	5134.70,	// % 239Pu
	5478.57,	// % 241Am
	5795.00 	// % 244Cm
};
const float gAlphaParticleErrors[gNumberOfAlphaPeaks] = {
	0.024,	// 100    % 148Gd
	0.5,	//  70.77 % 239Pu
	0.14,	//  84.8  % 241Am
	1 	//  76.90 % 244Cm
};

// --------------------------------------------------------------------------------------------- //
inline double CrystalBallFunction( double x, double alpha, double n, double sigma, double mean, double amp ) {

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

// --------------------------------------------------------------------------------------------- //
inline double CrystalBallFunction( double *x, double *p) {
	return CrystalBallFunction( x[0], p[0], p[1], p[2], p[3], p[4] );
}

// --------------------------------------------------------------------------------------------- //
inline double CrystalBallFunctionBG( double *x, double *p) {
	return CrystalBallFunction( x[0], p[0], p[1], p[2], p[3], p[4] ) + p[5];
}

// --------------------------------------------------------------------------------------------- //
inline double MultiCrystalBallFunction( double *x, double *p ){
	
	double sum = 0;
	for ( int i = 0; i < gNumberOfAlphaPeaks; i++ ){
		sum += CrystalBallFunction( x, &p[5*i] );
	}
	return sum;
	
}

// --------------------------------------------------------------------------------------------- //
inline double MultiCrystalBallFunctionBG( double *x, double *p ){
	return MultiCrystalBallFunction( x, p ) + p[gNumberOfAlphaPeaks*5];
}

// --------------------------------------------------------------------------------------------- //
inline double AlphaGaussian( double *x, double *p ){
	
	// p[0]   gauss constant
	// p[1]   gauss mean
	// p[2]   gauss width

	static float sqrt2pi = TMath::Sqrt( 2.0 * TMath::Pi() );
	static float sqrt2 = TMath::Sqrt( 2.0 );

	double arg = ( x[0] - p[1] ) / ( sqrt2 * p[2] );
	double gaus = p[0] * TMath::Exp( -1.0*TMath::Power( arg, 2.0 ) );
	gaus /=  sqrt2pi * p[2];

	return gaus;
	
}

// --------------------------------------------------------------------------------------------- //
inline double MultiAlphaGaussianBG( double *x, double *p ) {

	// par[0]   background constant
	// par[1]   gauss width

	double bg = p[0];
	double gaus_par[3];

	double sum = 0;
	for( int i = 0; i < gNumberOfAlphaPeaks; i++ ){
		
		gaus_par[0] = p[i*2+2];
		gaus_par[1] = p[i*2+3];
		gaus_par[2] = p[1];
		sum += AlphaGaussian( x, gaus_par );
		
	}

	return sum + bg;

}

#endif
